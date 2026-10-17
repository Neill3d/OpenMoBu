
#pragma once

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

#include <cstdint>
#include <string>
#include <string_view>

// NOTE: every hashed string is registered in a debug registry, so a hash can be resolved back into a string.
//  A hash collision (two different strings with the same hash) is reported in the log, the first string is kept.

uint32_t xxhash32(const char* str, size_t len, uint32_t seed = 0);

// NOTE: string_view takes const char* and std::string without making a temporary std::string
inline uint32_t xxhash32(std::string_view str, uint32_t seed = 0)
{
	return xxhash32(str.data(), str.size(), seed);
}

// a registered string for a given hash, "<unknown>" if the hash has not been registered
const char* ResolveHash32(uint32_t hash);