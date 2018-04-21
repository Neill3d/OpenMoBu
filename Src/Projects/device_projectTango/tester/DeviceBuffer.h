
#include "tango_client_api.h"
#include <math.h>

// Port number to use for server
const int  PORTNUMBER	= 8889;		// Port number for communication
const int  SIM_FPS		= 60;		// Tested for 30,60,120

#if defined( WIN32 )
typedef unsigned __int64 nsTime;
#else
typedef unsigned long long nsTime;
#endif

struct sDataBuffer
{
	TangoPoseData		mData;
	

	sDataBuffer()
	{
		mData.timestamp = 0.0;
	}

	void Simulate(double timestamp)
	{
		mData.timestamp = timestamp;

		double	a = sin(timestamp);

		mData.translation[0] = a - 150.0;
		mData.translation[1] = 0.0;
		mData.translation[2] = 0.0;
	}
};
