
// DataExchange.h
// thread safe read/write access to a data
//

/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#pragma once

#include "NetworkTango.h"
#include <vector>

//////////////

bool ExchangeWriteImage(double timestamp, int w, int h, int internalFormat, double aspect, unsigned char *data, unsigned int size);

// cut a read operation by comparing last read timestamp
bool ExchangeReadImage(double &lastreadStamp, Network::CImageHeader &header, unsigned char *buffer, unsigned int buffer_size);

/////////////

bool ExchangeWriteSyncState(double timestamp, Network::CSyncControl &state);
bool ExchangeReadSyncState(double &lastreadStamp, Network::CSyncControl &state);

/////////////

bool ExchangeWriteCameras(double timestamp);
void ExchangeWriteCamerasFinish();

std::vector<Network::CCameraInfo>	&ExchangeGetCameraInfo();

bool ExchangeReadCameras(double timestamp);
void ExchangeReadCamerasFinish();

// TODO: exchange camera data
/*
bool ExchangeWriteDeviceData(double timestamp, Network::CDeviceData &data);
bool ExchangeReadDeviceData(double &lastreadStamp, Network::CDeviceData &data);
*/