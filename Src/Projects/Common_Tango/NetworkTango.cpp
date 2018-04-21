
// NetworkTango.cpp
/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "NetworkTango.h"
#include <stdio.h>

namespace Network
{

	void FillBufferWithHeader(unsigned char *mBuffer, unsigned char reason, const float timestamp)
	{
		CHeader *pHeader = (CHeader*) mBuffer;
		pHeader->magic[0] = PACKET_MAGIC_1;
		pHeader->magic[1] = PACKET_MAGIC_2;
		pHeader->magic[2] = PACKET_MAGIC_3;

		pHeader->reason = reason;
		pHeader->timestamp = timestamp;
	}

	bool CheckMagicNumber(unsigned char *mBuffer)
	{
		return (PACKET_MAGIC_1 == mBuffer[0]
			&& PACKET_MAGIC_2 == mBuffer[1]
			&& PACKET_MAGIC_3 == mBuffer[2]);
	}
}