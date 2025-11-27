

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

////////////////////////////////////////////////////////////////

uint32_t xxhash32(const char* str, size_t len, uint32_t seed)
{
	constexpr uint32_t PRIME1 = 0x9E3779B1U;
	constexpr uint32_t PRIME2 = 0x85EBCA77U;
	constexpr uint32_t PRIME3 = 0xC2B2AE3DU;
	constexpr uint32_t PRIME4 = 0x27D4EB2FU;
	constexpr uint32_t PRIME5 = 0x165667B1U;

	const uint8_t* data = reinterpret_cast<const uint8_t*>(str); // str.data());
	//size_t len = str.size();
	uint32_t h32;

	if (len >= 16)
	{
		const uint8_t* const end = data + len;
		const uint8_t* const limit = end - 16;

		uint32_t v1 = seed + PRIME1 + PRIME2;
		uint32_t v2 = seed + PRIME2;
		uint32_t v3 = seed + 0;
		uint32_t v4 = seed - PRIME1;

		do
		{
			auto read32 = [](const uint8_t* ptr) {
				uint32_t val;
				memcpy(&val, ptr, sizeof(val));
				return val;
				};

			v1 += read32(data) * PRIME2; v1 = ((v1 << 13) | (v1 >> 19)) * PRIME1; data += 4;
			v2 += read32(data) * PRIME2; v2 = ((v2 << 13) | (v2 >> 19)) * PRIME1; data += 4;
			v3 += read32(data) * PRIME2; v3 = ((v3 << 13) | (v3 >> 19)) * PRIME1; data += 4;
			v4 += read32(data) * PRIME2; v4 = ((v4 << 13) | (v4 >> 19)) * PRIME1; data += 4;
		} while (data <= limit);

		h32 = ((v1 << 1) | (v1 >> 31)) + ((v2 << 7) | (v2 >> 25)) +
			((v3 << 12) | (v3 >> 20)) + ((v4 << 18) | (v4 >> 14));
	}
	else
	{
		h32 = seed + PRIME5;
	}

	h32 += static_cast<uint32_t>(len);

	while (len >= 4)
	{
		uint32_t k1;
		memcpy(&k1, data, sizeof(k1));
		h32 += k1 * PRIME3;
		h32 = ((h32 << 17) | (h32 >> 15)) * PRIME4;
		data += 4;
		len -= 4;
	}

	while (len > 0)
	{
		h32 += (*data) * PRIME5;
		h32 = ((h32 << 11) | (h32 >> 21)) * PRIME1;
		data++;
		len--;
	}

	h32 ^= h32 >> 15;
	h32 *= PRIME2;
	h32 ^= h32 >> 13;
	h32 *= PRIME3;
	h32 ^= h32 >> 16;

	return h32;
}