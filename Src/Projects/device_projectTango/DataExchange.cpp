
// DataExchange.cpp
/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "DataExchange.h"

#include <GL/glew.h>

#include <vector>
#include <atomic>

//
#define IMAGE_HOLDER_BUFFER_CAPACITY		65536

struct ImageHolder
{
	double timestamp;
	Network::CImageHeader header;

	std::vector<unsigned char>	buffer;

	ImageHolder()
	{
		timestamp = 0.0;
		buffer.resize(IMAGE_HOLDER_BUFFER_CAPACITY);
	}
};

struct SyncHolder
{
	double timestamp;
	Network::CSyncControl	state;

	SyncHolder()
	{
		timestamp = 0.0;
	}
};
/*
struct PoseHolder
{
	double timestamp;
	Network::CDeviceData    pose;

	PoseHolder()
	{
		timestamp = 0.0;
	}
};
*/
/////////////////////////////////////////

ImageHolder		g_imageHolders[3];
SyncHolder		g_syncHolders[3];
//PoseHolder		g_poseHolders[3];

std::atomic<uint32_t>	g_imageIndex(0);
std::atomic<uint32_t>	g_syncIndex(0);
//std::atomic<uint32_t>   g_poseIndex(0);

std::vector<Network::CCameraInfo> g_cameraVector;
std::atomic<uint32_t>	g_cameraIndex(0);	// state to send information about cameras

///////////////////////////////////////////////////////

void ResetExchange()
{
	g_cameraIndex = 0;
}

void ExchangeWriteCamerasFinish()
{
	g_cameraIndex = 2;
}

bool ExchangeWriteCameras(double timestamp)
{
	uint32_t current = g_cameraIndex;

	if (current > 0)
		return false;

	// lock a state
	g_cameraIndex = 1;

	return true;
}

bool ExchangeReadCameras(double timestamp)
{
	uint32_t current = g_cameraIndex;

	return (current > 1);
}

std::vector<Network::CCameraInfo>	&ExchangeGetCameraInfo()
{
	return g_cameraVector;
}

void ExchangeReadCamerasFinish()
{
	g_cameraIndex = 0;
}

bool ExchangeWriteImage(double timestamp, int w, int h, int internalFormat, double aspect, unsigned char *data, unsigned int size)
{
	uint32_t current = g_imageIndex;
	
	// triple buffer
	current = current + 1;
	if (current > 2)
		current = 0;

	g_imageHolders[current].timestamp = timestamp;

	Network::CImageHeader head;

	head.aspect = aspect;
//	head.dataFormat = GL_UNSIGNED_BYTE;
	head.dataSize = size;
	head.height = h;
	head.internalFormat = internalFormat;
//	head.numTiles = 0;
//	head.tileSize = 0;
	head.width = w;

	g_imageHolders[current].header = head;
	
	memcpy(g_imageHolders[current].buffer.data(), data, size);

	g_imageIndex = current;

	return true;
}

// cut a read operation by comparing last read timestamp
bool ExchangeReadImage(double &lastreadStamp, Network::CImageHeader &header, unsigned char *buffer, unsigned int buffer_size)
{
	bool lSuccess = false;
	uint32_t current = g_imageIndex;

	if (g_imageHolders[current].timestamp > 0.0 && g_imageHolders[current].timestamp > lastreadStamp
		&& buffer_size >= g_imageHolders[current].header.dataSize)
	{
		header = g_imageHolders[current].header;
		memcpy(buffer, g_imageHolders[current].buffer.data(), header.dataSize);
	
		lastreadStamp = g_imageHolders[current].timestamp;
		lSuccess = true;
	}

	return lSuccess;
}



bool ExchangeWriteSyncState(double timestamp, Network::CSyncControl &state)
{
	uint32_t current = g_syncIndex;

	// triple buffer
	current = current + 1;
	if (current > 2)
		current = 0;

	g_syncHolders[current].timestamp = timestamp;
	g_syncHolders[current].state = state;

	g_syncIndex = current;

	return true;
}

// cut a read operation by comparing last read timestamp
bool ExchangeReadSyncState(double &lastreadStamp, Network::CSyncControl &state)
{
	bool lSuccess = false;
	uint32_t current = g_syncIndex;

	if (g_syncHolders[current].timestamp > 0.0 && g_syncHolders[current].timestamp > lastreadStamp)
	{
		state = g_syncHolders[current].state;
		lastreadStamp = g_syncHolders[current].timestamp;
		lSuccess = true;
	}

	return lSuccess;
}



/*
bool ExchangeWriteDeviceData(double timestamp, Network::CDeviceData &data)
{
	uint32_t current = g_poseIndex;

	// triple buffer
	current = current + 1;
	if (current > 2)
		current = 0;

	g_poseHolders[current].timestamp = timestamp;
	g_poseHolders[current].pose = data;

	g_poseIndex = current;

	return true;
}

bool ExchangeReadDeviceData(double &lastreadStamp, Network::CDeviceData &data)
{
	bool lSuccess = false;
	uint32_t current = g_poseIndex;

	if (g_poseHolders[current].timestamp > 0.0 && g_poseHolders[current].timestamp > lastreadStamp)
	{
		data = g_poseHolders[current].pose;
		lastreadStamp = g_poseHolders[current].timestamp;
		lSuccess = true;
	}

	return lSuccess;
}
*/