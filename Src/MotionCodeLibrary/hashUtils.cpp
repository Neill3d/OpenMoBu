

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
//

#include <unordered_map>
#include <string>
#include <mutex>
#include <shared_mutex>
#include <cassert>

class HashDebugRegistry
{
public:
	static void Register(uint32_t hash, std::string_view view)
	{
		std::unique_lock lock(GetMutex());
		
		auto [it, inserted] = GetMap().emplace(hash, std::string(view));
		if (!inserted && it->second != view)
		{
			assert(false && "Hash collision detected!");
			abort();
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
	static std::unordered_map<uint32_t, std::string>& GetMap()
	{
		static std::unordered_map<uint32_t, std::string> map;
		return map;
	}

	static std::shared_mutex& GetMutex()
	{
		static std::shared_mutex m;
		return m;
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