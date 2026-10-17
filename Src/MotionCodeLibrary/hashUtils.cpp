

/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/MoBu/blob/master/LICENSE
//
// GitHub repository - https://github.com/Neill3d/MoBu
//
// Author Sergei Solokhin (Neill3d) 2025
//  e-mail to: neill3d@gmail.com
// 
/////////////////////////////////////////////////////////////////////////////////////////


#include "hashUtils.h"
#include "xxhash.h"
#include "mobu_logging.h"
//

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <mutex>
#include <shared_mutex>

class HashDebugRegistry
{
public:
	static void Register(uint32_t hash, std::string_view view)
	{
		// most of the hashes are registered already, check that without an exclusive lock and allocations
		{
			std::shared_lock lock(GetMutex());

			const auto& map = GetMap();
			const auto it = map.find(hash);
			if (it != map.end())
			{
				if (it->second == view)
					return;
			}
		}

		std::unique_lock lock(GetMutex());

		auto [it, inserted] = GetMap().try_emplace(hash, view);
		if (!inserted && it->second != view)
		{
			// NOTE: a collision must not stop the application, the first registered string is kept
			//  and the collision is reported once per hash
			if (GetReportedCollisions().insert(hash).second)
			{
				LOGE("[hashUtils] hash collision %u between \"%s\" and \"%.*s\"\n",
					hash, it->second.c_str(), static_cast<int>(view.size()), view.data());
			}
		}
	}

	static const char* Resolve(uint32_t hash)
	{
		std::shared_lock lock(GetMutex());

		auto& map = GetMap();
		auto it = map.find(hash);
		return (it != map.end()) ? it->second.c_str() : "<unknown>";
	}

	static std::string_view ResolveString(uint32_t hash)
	{
		std::shared_lock lock(GetMutex());

		static std::string unknown = "<unknown>";
		auto& map = GetMap();
		auto it = map.find(hash);
		if (it != map.end())
		{
			return it->second;
		}
		return unknown;
	}

private:
	// NOTE: the registry is never destroyed on purpose, strings could be hashed or resolved
	//  from destructors of static objects during the application exit

	static std::unordered_map<uint32_t, std::string>& GetMap()
	{
		static auto* map = new std::unordered_map<uint32_t, std::string>();
		return *map;
	}

	static std::unordered_set<uint32_t>& GetReportedCollisions()
	{
		static auto* hashes = new std::unordered_set<uint32_t>();
		return *hashes;
	}

	static std::shared_mutex& GetMutex()
	{
		static auto* m = new std::shared_mutex();
		return *m;
	}
};

const char* ResolveHash32(uint32_t hash)
{
	return HashDebugRegistry::Resolve(hash);
}

////////////////////////////////////////////////////////////////

uint32_t xxhash32(const char* str, size_t len, uint32_t seed)
{
	const uint32_t h32 = XXH32(str, len, seed);

	// lookup hash server registry
	HashDebugRegistry::Register(h32, std::string_view(str, len));

	return h32;
}