
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

#include <string>


uint32_t xxhash32(const char* str, size_t len, uint32_t seed = 0);

inline uint32_t xxhash32(const std::string& str, uint32_t seed = 0)
{
	return xxhash32(str.c_str(), str.size(), seed);
}