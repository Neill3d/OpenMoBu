

/**	\file	device_projectTango_hardware.cxx

Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include <winsock2.h>
#include "Ws2tcpip.h"

//--- Class declaration
#include "device_projectTango_hardware.h"
#include "GL\glew.h"

#include <math.h>

//#include "MemPool.h"



#pragma warning(push)
#pragma warning(disable:4265)
#include <mutex>
#include <thread>
#include <atomic>
#pragma warning(pop)

#include "DataExchange.h"

//#include "miniz.h"
#include "zlib.h"

#define NETWORK_MOBU_TRACKER	9001
#define NETWORK_IMAGE_PORT		8886

///////////////////////////////////////////////////////////////////////////////////
//

std::atomic <uint32_t> g_status;
std::atomic <uint64_t> g_index;

int NewTCPSocket()
{
	int tcpSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (-1 == tcpSocket)
		printf("[CStreamThread] Failed to open a stream socket!\n");

	return tcpSocket;
}

void CloseTCPSocket(int socketHandle)
{
#ifdef WIN32
	if (socketHandle >= 0)
		closesocket(socketHandle);
#else
	if (socketHandle >= 0)
		close(socketHandle);
#endif
}

bool BindSocketToPort(int sd, unsigned short port)
{
	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = PF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(port);
	printf("[CStreamThread] Binding to %d\n", port);

	if (-1 == ::bind(sd, (struct sockaddr*) &address, sizeof(address))) {
		printf("[CStreamThread] failed to bind to port %d\n", errno);
		return false;
	}

	return true;
}

bool Connect(int sd, const char *address, unsigned int port)
{
	struct sockaddr_in server;

	unsigned long sAddr = 0;
	InetPton(AF_INET, address, &sAddr);

	if (sAddr > 0)
	{
		struct addrinfo hints, *res;

		memset(&hints, 0, sizeof(hints));
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_family = AF_INET;

		int err;
		if ((err = getaddrinfo(address, nullptr, &hints, &res)) != 0) {
			printf("error %d\n", err);
			return 1;
		}

		struct in_addr addr;
		addr.S_un = ((struct sockaddr_in*)(res->ai_addr))->sin_addr.S_un;

		server.sin_addr = addr;

		freeaddrinfo(res);
	}
	else
	{
		unsigned char a = 192;
		unsigned char b = 168;
		unsigned char c = 0;
		unsigned char d = 26;

		unsigned int _address = (a << 24) | (b << 16) | (c << 8) | d;
		server.sin_addr.s_addr = htonl(_address);
		//server.sin_addr.s_addr = inet_addr(address);
	}

	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	
	if (connect(sd, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		printf("connect failed. Error\n");
		return false;
	}
	return true;
}

int StartClient(const char *address, unsigned short port)
{
	int lsocket = 0;

	lsocket = NewTCPSocket();

	if (lsocket > 0)
	{
		if (false == Connect(lsocket, address, port))
		{
			CloseTCPSocket(lsocket);
			return 0;
		}
	}
	/*
	// disable Nagle algorithm
	int value = 1;
	setsockopt(lsocket, IPPROTO_TCP, TCP_NODELAY, (char*) &value, sizeof(int));
	*/
	return lsocket;
}


enum
{
	PROCESS_STREAM_WAITING,
	PROCESS_STREAM_HEADER,
	PROCESS_STREAM_POSE,
	
};

void MoveLastBytes(unsigned char *ptr, int curpos, int bytesToCopy)
{
	if (bytesToCopy > 0)
	{
		for (int j = 0; j<bytesToCopy; ++j)
			ptr[j] = ptr[j + curpos];
	}
}

bool ProcessClientStream(int sd, int startlength, int &bufferOffset, Network::CHeader &lHeader, std::vector<unsigned char> &buffer)
{
	int lStatus = PROCESS_STREAM_WAITING;

	int curpos = 0;
	unsigned char *readPtr = buffer.data();

	int len = startlength;
	int readingMore = 0;
	bool readingUsed = false;

	while (curpos < len)
	{
		unsigned char *curptr = readPtr + curpos;

		if (readingMore > 0)
		{
			int res = recv(sd, (char*) (readPtr + len), readingMore, MSG_WAITALL);

			if (res < 0)
			{
				return false;
			}
			len += res; // we have readed more bytes
			readingMore = 0;
			readingUsed = true;
		}

		int bytesToCopy = len - curpos;

		switch (lStatus)
		{
		case PROCESS_STREAM_WAITING:

			if (bytesToCopy >= 3)
			{
				if (Network::CheckMagicNumber(curptr))
				{
					if (bytesToCopy >= sizeof(Network::CHeader))
					{
						memcpy(&lHeader, curptr, sizeof(Network::CHeader));

						double dval = (double)lHeader.timestamp;
						uint64_t *val = (uint64_t*)&dval;
						g_index = *val;

						curpos += sizeof(Network::CHeader);
						lStatus = PROCESS_STREAM_HEADER;

					}
					else
					{
						MoveLastBytes(buffer.data(), curpos, bytesToCopy);
						bufferOffset = bytesToCopy;
						return false;
					}
				}
				else
					curpos += 1;
			}
			else curpos += 1;

			break;

		case PROCESS_STREAM_HEADER:
			if (PACKET_REASON_REGISTER == lHeader.reason || PACKET_REASON_FEEDBACK == lHeader.reason)
			{
				printf("received an invitation %.2f\n", lHeader.timestamp);
				curpos += 1;
				lStatus = PROCESS_STREAM_WAITING;
			}
			else if (PACKET_REASON_CAMERA == lHeader.reason)
			{
				if (bytesToCopy >= sizeof(Network::CDeviceData))
				{
					//LOGI("received a control %.2f\n", lHeader.timestamp);

//					Network::CDeviceData *pData = (Network::CDeviceData*) curptr;
					//ExchangeWriteDeviceData((float)lHeader.timestamp, *pData);
					
					curpos += sizeof(Network::CDeviceData);
					lStatus = PROCESS_STREAM_WAITING;

				}
				else if (bytesToCopy > 0)
				{
					if (false == readingUsed)
					{
						readingMore = sizeof(Network::CDeviceData) - bytesToCopy;
						continue;
					}
					else
					{
						MoveLastBytes(buffer.data(), curpos, bytesToCopy);
						bufferOffset = bytesToCopy;
						return false;
					}
				}
			}
			break;
		
		default:
			curpos += 1;
		}
	}

	bufferOffset = 0;
	return true;
}

void MainClientLoop()
{

	ExchangeReadCamerasFinish();

	int handle = StartClient("192.128.0.26", 8889);

	if (handle <= 0)
		return;

	// send invitation
	FBSystem	lSystem;
	FBTime sysTime(lSystem.SystemTime);

	std::vector<unsigned char>		recvbuf(8192);


	Network::CHeader      recvHeader;
	recvHeader.reason = 0;
	recvHeader.timestamp = 0.0f;

	double lastImageStamp = 0.0;
	Network::CImageHeader imageHeader;

//	double lastCameraInfoStamp = 0.0;
	//Network::CCameraInfo cameraInfoHeader;

	double lastSyncStamp = 0.0;
	Network::CSyncControl syncControl;

	std::vector<char>			sendbuf(65536 * 2);
	std::vector<unsigned char>	compressbuf(65536);


	//char	buffer[8] = { PACKET_MAGIC_1, PACKET_MAGIC_2, PACKET_MAGIC_3 };
	//float *pval = (float*) &buffer[4];
	//*pval = (float)sysTime.GetSecondDouble();

	Network::CHeader lHeader;
	Network::FillBufferWithHeader((unsigned char*)&lHeader, PACKET_REASON_REGISTER, (float)sysTime.GetSecondDouble());

	int res = send(handle, sendbuf.data(), sizeof(Network::CHeader), 0);
	if (res < 0)
	{
		printf("Send failed - %d\n", WSAGetLastError());
		CloseTCPSocket(handle);
		return;
	}

	g_status = 1;
	int bufferOffset = 0;
	const int readCapacity = (int)recvbuf.size() / 2;

	while (1)
	{

		uint32_t current = g_status;
		if (0 == current)
			break;

		// read
		//char recvbuf[8];
	
		res = recv(handle, (char*)recvbuf.data(), readCapacity, 0);
		if (res < 0)
		{
			printf("Recv failed - %d\n", WSAGetLastError());
			break;
		}

		ProcessClientStream(handle, res, bufferOffset, recvHeader, recvbuf);

		// read a timestamp
		//uint64_t *ptime = (uint64_t*)recvbuf.data();
		//g_index = *ptime;

		//
		// write
		sysTime = lSystem.SystemTime;

		int sendFlag = MSG_DONTROUTE;

		bool hasCameras = ExchangeReadCameras(0.0);

		if (hasCameras)
		{
			// send cameras stream
			std::vector<Network::CCameraInfo>	&cameraInfo = ExchangeGetCameraInfo();

			Network::FillBufferWithHeader((unsigned char*)&lHeader, PACKET_REASON_CAMERA_INFO, (float)sysTime.GetSecondDouble());

			for (auto iter = begin(cameraInfo); iter != end(cameraInfo); ++iter)
			{
				res = send(handle, (char*)&lHeader, sizeof(Network::CHeader), sendFlag);
				if (res < 0)
				{
					printf("Send failed - %d\n", WSAGetLastError());
					break;
				}

				// camera info data
				char *ptr = (char*)&(*iter);
				int numberOfBlocks = sizeof(Network::CCameraInfo) / 8;
				
				for (int i = 0; i < numberOfBlocks; ++i, ptr += 8)
				{
					res = send(handle, ptr, 8, 0);
					if (res < 0)
					{
						printf("Send failed - %d\n", WSAGetLastError());
						break;
					}
				}
			}

			//
			//
			ExchangeReadCamerasFinish();

		}
		else
		{
			bool hasImage = ExchangeReadImage(lastImageStamp, imageHeader, (unsigned char*)sendbuf.data(), sendbuf.size());
			hasImage = false;
			if (hasImage)
			{
				// compress image

				uLongf destLen = compressbuf.size();

				int err = compress(compressbuf.data(), &destLen, (const unsigned char*)sendbuf.data(), imageHeader.dataSize); // MZ_BEST_COMPRESSION

				if (Z_OK == err)
					imageHeader.compressed = destLen;
				else
					imageHeader.compressed = 0;

				// send image stream

				Network::FillBufferWithHeader((unsigned char*)&lHeader, PACKET_REASON_IMAGE, (float)sysTime.GetSecondDouble());

				res = send(handle, (char*)&lHeader, sizeof(Network::CHeader), sendFlag);
				if (res < 0)
				{
					printf("Send failed - %d\n", WSAGetLastError());
					break;
				}

				// image header
				char *ptr = (char*)&imageHeader;
				int numberOfBlocks = sizeof(Network::CImageHeader) / 8;
				/*
				res = send(handle, ptr, sizeof(Network::CImageHeader), sendFlag);
				if (res < 0)
				{
				printf("Send failed - %d\n", WSAGetLastError());
				break;
				}
				*/

				for (int i = 0; i < numberOfBlocks; ++i, ptr += 8)
				{
					res = send(handle, ptr, 8, 0);
					if (res < 0)
					{
						printf("Send failed - %d\n", WSAGetLastError());
						break;
					}
				}


				// image data

				if (Z_OK == err)
				{
					ptr = (char*) compressbuf.data();

					res = send(handle, ptr, imageHeader.compressed, sendFlag);
					if (res < 0)
					{
						printf("Send failed - %d\n", WSAGetLastError());
						break;
					}
				}
				else
				{
					ptr = sendbuf.data();
					numberOfBlocks = imageHeader.dataSize / 8;

					for (int i = 0; i < numberOfBlocks; ++i, ptr += 8)
					{
						res = send(handle, ptr, 8, sendFlag);
						if (res < 0)
						{
							printf("Send failed - %d\n", WSAGetLastError());
							break;
						}
					}
				}
			}
			else
			{
				// send a feedback stream (or sync state)

				bool hasSync = ExchangeReadSyncState(lastSyncStamp, syncControl);

				if (hasSync)
				{
					//printf("sending sync state %2.lf\n", sysTime.GetSecondDouble());
					Network::FillBufferWithHeader((unsigned char*)&lHeader, PACKET_REASON_CONTROL, (float)sysTime.GetSecondDouble());

					res = send(handle, (char*)&lHeader, sizeof(Network::CHeader), sendFlag);
					if (res < 0)
					{
						printf("Send failed - %d\n", WSAGetLastError());
						break;
					}

					// sync data
					char *ptr = (char*)&syncControl;
					int numberOfBlocks = sizeof(Network::CSyncControl) / 8;

					for (int i = 0; i < numberOfBlocks; ++i, ptr += 8)
					{
						res = send(handle, ptr, 8, sendFlag);
						if (res < 0)
						{
							printf("Send failed - %d\n", WSAGetLastError());
							break;
						}
					}

				}
				else
				{
					Network::FillBufferWithHeader((unsigned char*)&lHeader, PACKET_REASON_FEEDBACK, (float)sysTime.GetSecondDouble());

					res = send(handle, (char*)&lHeader, sizeof(Network::CHeader), sendFlag);
					if (res < 0)
					{
						printf("Send failed - %d\n", WSAGetLastError());
						break;
					}
				}
			}
		}
		
		
		std::this_thread::sleep_for(std::chrono::milliseconds(33));
		//std::this_thread::yield();
	}

	CloseTCPSocket(handle);

}


/************************************************
 *	Constructor.
 ************************************************/
Device_ProjectTango_Hardware::Device_ProjectTango_Hardware()
{
	mParent				= NULL;

	
	//mDeviceAddress		= "192.168.0.131";
	//mDevicePort			= 8889;
	mDeviceAddress.Set(192, 168, 0, 131, 8889);

	mIsSynced = false;

	mStreaming			= true;
	//mReadState		= eORReadStateHeader;
	mDataCount			= 0;
	mLastCameraTimestamp		= 0.0;

	mSpaceScale = 100.0;

	mPosition[0]		= 0.0;
	mPosition[1]		= 0.0;
	mPosition[2]		= 0.0;

	mRotation[0]		= 0.0;
	mRotation[1]		= 0.0;
	mRotation[2]		= 0.0;

	mFieldOfViewX = 40.0;
	mFieldOfViewY = 40.0;

	mCameraMoveScale = 1.0;
	mCameraFly = 0.0;

	for (int i = 0; i < 6; ++i)
		mDeviceTriggers[i] = 0.0;

	//mSocket.Open( (unsigned short) mSocketPort);

	//
	mLastInvitationTime = FBTime::Zero;
	mInvitationPeriod = FBTime(0, 0, 3);

	
}

/************************************************
 *	Destructor.
 ************************************************/
Device_ProjectTango_Hardware::~Device_ProjectTango_Hardware()
{


}

/************************************************
 *	Set the parent.
 ************************************************/
void Device_ProjectTango_Hardware::SetParent(FBDevice* pParent)
{
	mParent = pParent;
}

/************************************************
 *	Open device communications.
 ************************************************/

int Device_ProjectTango_Hardware::EmptyUDPIncomingQueue()
{
	
	return 1;
}

bool Device_ProjectTango_Hardware::IsTimeForInvitation()
{
	FBTime sysTime = mSystem.SystemTime;
	FBTime diff = sysTime - mLastInvitationTime;

	return (diff > mInvitationPeriod);
}

bool Device_ProjectTango_Hardware::Open()
{
	bool lSuccess = true;
	//bool lSuccessTracker = false;
	//bool lSuccessCommands = false;

	mIsSynced = false;
	
	return lSuccess;
}


/************************************************
 *	Close device communications.
 ************************************************/
bool Device_ProjectTango_Hardware::Close()
{
	bool lSuccess = true;
	mIsSynced = false;
	
	g_status = 0;

	return lSuccess;
}


// TODO: should be thread safe
struct CLastCompressedImage
{

};

#define TEST_BUFFER_CAPACITY		65536
#define TEST_BUFFER_WRITE_SIZE		8
#define TEST_BUFFER_READ_SIZE		8
std::vector<unsigned char>		mTestBuffer;

/************************************************
 *	Poll device.
 ************************************************/
int Device_ProjectTango_Hardware::PollData(size_t &sendBytes, double sendTimestamp)
{
	size_t numberOfPackets = 0;
	sendBytes = 0;

	//lSuccess = WriteData( mRequest, MAX_REQUEST_SIZE, &lBytesWritten );

	return (int) numberOfPackets;
}


/************************************************
 *	Fetch a data packet from the device.
 ************************************************/
int Device_ProjectTango_Hardware::FetchData(size_t &recvBytes, double &recvTimestamp)
{
	int numberOfPackets = 0;

	Network::Address sender;
	Network::CPacketDevice packet;

	int recv = 1;

	while (recv > 0)
	{
		recv = mSocketPoses.Receive(sender, &packet, sizeof(Network::CPacketDevice));

		if (recv > 0)
		{
			if (recv == sizeof(Network::CPacketDevice))
			{
				mDeviceData.Push(packet);
				numberOfPackets += 1;
			}
			else if (recv == sizeof(Network::CPacketCommand))
			{
				Network::CPacketCommand *pPacket = (Network::CPacketCommand*) &packet;
				mCommands.Push(pPacket->body);
			}
		}
	}
	
	if (numberOfPackets > 0)
	{
		ExtractTR(packet, mSpaceScale, mPosition, mRotation, &mFieldOfViewX, &mCameraFly, mDeviceTriggers, nullptr);
	}

	return numberOfPackets;
}


bool Device_ProjectTango_Hardware::PopDeviceData(Network::CPacketDevice &camdata)
{
	return mDeviceData.Pop(camdata);
}

void Device_ProjectTango_Hardware::ExtractTR(const Network::CPacketDevice &camdata, const double spaceScale, double *pPos, double *pRot, double *pFOV, double *pFly,
	double *pTriggers, FBTime *pTime)
{
	FBMatrix pre, curr, res;
	FBMatrix mx, my, mz;

	FBRVector rot;

	FBRotationToMatrix(pre, FBRVector(0.0, 90, 0.0));

	FBQuaternionToRotation(rot, FBQuaternion((double)camdata.body.orientation[0], (double)camdata.body.orientation[1],
		(double)camdata.body.orientation[2], (double)camdata.body.orientation[3]));

	FBRotationToMatrix(mx, FBRVector(-rot[2], 0.0, 0.0));
	FBRotationToMatrix(my, FBRVector(0.0, rot[1], 0.0));
	FBRotationToMatrix(mz, FBRVector(0.0, 0.0, rot[0]));

	FBMatrixMult(res, pre, mx);
	FBMatrixMult(res, res, my);
	FBMatrixMult(res, res, mz);

	FBMatrixToRotation(rot, res);

	for (int i = 0; i < 3; ++i)
	{
		pPos[i] = spaceScale * (double)camdata.body.translation[i];
		pRot[i] = rot[i];
	}

	if (nullptr != pFOV)
	{
		pFOV[0] = (double)camdata.body.lens;
	}

	if (nullptr != pFly)
	{
		*pFly = (double)camdata.body.fly;
	}

	if (nullptr != pTriggers)
	{
		for (int i = 0; i<6; ++i)
			pTriggers[i] = (camdata.body.triggers[i] > 0) ? 1.0 : 0.0;
	}

	if (nullptr != pTime)
		pTime->SetSecondDouble( (double)camdata.header.timestamp);
}

/************************************************
 *	Get the device setup information.
 ************************************************/
bool Device_ProjectTango_Hardware::GetSetupInfo()
{
	bool			lSuccess = true;

	return lSuccess;
}


/************************************************
 *	Start the device streaming mode.
 ************************************************/
bool Device_ProjectTango_Hardware::StartStream()
{
	bool			lSuccess = true;
	
	std::thread  lthread(MainClientLoop);
	lthread.detach();

	// open UDP socket to receive devices poses
	mSocketPoses.Open(NETWORK_MOBU_TRACKER, false);
	mSocketImage.Open(NETWORK_IMAGE_PORT, false);

	if (false == mSocketPoses.IsOpen() || false == mSocketImage.IsOpen())
	{
		lSuccess = false;
		g_status = 0;
	}

	return lSuccess;
}


/************************************************
 *	Stop the device streaming mode.
 ************************************************/
bool Device_ProjectTango_Hardware::StopStream()
{
	bool			lSuccess = true;
	
	mSocketPoses.Close();
	mSocketImage.Close();

	g_status = 0;

	return lSuccess;
}


/************************************************
 *	Get the current position.
 ************************************************/
void Device_ProjectTango_Hardware::GetPosition(double* pPos)
{
	pPos[0] = mPosition[0];
	pPos[1] = mPosition[1];
	pPos[2] = mPosition[2];
}


/************************************************
 *	Get the current rotation.
 ************************************************/
void Device_ProjectTango_Hardware::GetRotation(double* pRot)
{
	pRot[0] = mRotation[0];
	pRot[1] = mRotation[1];
	pRot[2] = mRotation[2];
}

void Device_ProjectTango_Hardware::GetFieldOfView(double *pH, double *pV)
{
	*pH = mFieldOfViewX;
	*pV = mFieldOfViewY;
}

void Device_ProjectTango_Hardware::GetCameraMoveScale(double *scale)
{
	*scale = mCameraMoveScale;
}

void Device_ProjectTango_Hardware::GetCameraFly(double *fly)
{
	*fly = mCameraFly;
}

void Device_ProjectTango_Hardware::GetDeviceTrigger(int ndx, double *pValue)
{
	*pValue = mDeviceTriggers[ndx];
}

void Device_ProjectTango_Hardware::GetTime(FBTime *pTime)
{
	*pTime = mLastTime;
}

/************************************************
 *	Communications type.
 ************************************************/
void Device_ProjectTango_Hardware::SetCommunicationType(FBCommType pType)
{
	mParent->CommType = pType;
}
int Device_ProjectTango_Hardware::GetCommunicationType()
{
	return mParent->CommType;
}

bool Device_ProjectTango_Hardware::SendInvitation(double timestamp)
{
	bool lSuccess = false;

	unsigned char *ptr = nullptr; 

	if (nullptr != ptr)
	{
		uint32_t *packetSize = (uint32_t*)ptr;
		*packetSize = sizeof(Network::CHeader);

		unsigned char *curptr = ptr + sizeof(uint32_t);
		Network::FillBufferWithHeader(curptr, PACKET_REASON_REGISTER, (float)timestamp);

		lSuccess = true;
	}

	return lSuccess;
}

bool Device_ProjectTango_Hardware::SendSyncState(double timestamp, const Network::CSyncControl &syncState)
{
	bool lSuccess = false;

	unsigned char *ptr = nullptr; 

	if (nullptr != ptr)
	{
		uint32_t *packetSize = (uint32_t*)ptr;
		*packetSize = sizeof(Network::CPacketControl);

		unsigned char *curptr = ptr + sizeof(uint32_t);
		Network::FillBufferWithHeader(curptr, PACKET_REASON_CONTROL, (float)timestamp);

		curptr += sizeof(Network::CHeader);
		memcpy(curptr, &syncState, sizeof(Network::CSyncControl));

		lSuccess = true;
	
	}

	return lSuccess;
}

int Device_ProjectTango_Hardware::SendImage(double timestamp, int w, int h, int internalFormat, double aspect, const unsigned char *imageData, const unsigned imageSize)
{

	int lResult = 0;


	if (false == mSocketImage.IsOpen())
		return lResult;

	mCompressedData.resize(65536);

	Network::CHeader *pHeader = (Network::CHeader*) mCompressedData.data();
	Network::CImageHeader *pImageHeader = (Network::CImageHeader*) (mCompressedData.data() + sizeof(Network::CHeader));

	unsigned char *ptr = mCompressedData.data() + sizeof(Network::CHeader) + sizeof(Network::CImageHeader);
	uLongf dstLen = mCompressedData.size() - sizeof(Network::CHeader) - sizeof(Network::CImageHeader);

	int err = compress(ptr, &dstLen, imageData, imageSize);

	if (Z_OK == err)
	{
		Network::FillBufferWithHeader((unsigned char *)pHeader, PACKET_REASON_IMAGE, (float)timestamp);

		pImageHeader->internalFormat = internalFormat;
		pImageHeader->dataSize = imageSize;
		pImageHeader->aspect = aspect;
		pImageHeader->width = w;
		pImageHeader->height = h;
		pImageHeader->compressed = dstLen;

		Network::Address address(192, 168, 0, 26, NETWORK_IMAGE_PORT);

		mSocketImage.Send(address, mCompressedData.data(), dstLen + sizeof(Network::CHeader) + sizeof(Network::CImageHeader));

		lResult = 1;
	}

	return lResult;

}

int Device_ProjectTango_Hardware::SendImageTiles(double timestamp, int w, int h, int internalFormat, double aspect, const unsigned char *imageData, const unsigned imageSize)
{
	int lResult = 0;

	// memory pool, take place for a new packets

	const unsigned tileSize = DEFAULT_TILE_SIZE;
	const unsigned numberOfTiles = 1 + imageSize / tileSize;	// 1k for each tile

	const unsigned numberOfPackets = 1 + numberOfTiles;	// header (could contain a first tile) + tiles

	unsigned char *ptr = nullptr; 

	if (nullptr != ptr)
	{
		uint32_t *packetSize = (uint32_t*)ptr;
		*packetSize = sizeof(Network::CPacketImage);

		// copy data into packets
		unsigned char *curptr = ptr + sizeof(uint32_t);

		Network::CPacketImage	*imagePtr = (Network::CPacketImage*) curptr;
		Network::FillBufferWithHeader(curptr, PACKET_REASON_IMAGE, (float)timestamp);

		imagePtr->imageheader.internalFormat = internalFormat;
		imagePtr->imageheader.dataSize = imageSize;
//		imagePtr->imageheader.dataFormat = GL_UNSIGNED_BYTE;
		imagePtr->imageheader.aspect = aspect;
		imagePtr->imageheader.width = w;
		imagePtr->imageheader.height = h;
//		imagePtr->imageheader.numTiles = numberOfTiles;
//		imagePtr->imageheader.tileSize = tileSize;

		curptr = ptr + 256;
		for (unsigned i = 0; i < numberOfTiles; ++i, curptr += 256)
		{
			packetSize = (uint32_t*)curptr;

			Network::CPacketImageTile *pImageTile = (Network::CPacketImageTile*) (curptr + sizeof(uint32_t));
			Network::FillBufferWithHeader( (unsigned char*)pImageTile, PACKET_REASON_IMAGE_TILE, (float)timestamp);

			const unsigned char *tileData = imageData + tileSize * i;
			unsigned int iTileSize = imageSize - tileSize * i;
			if (iTileSize > tileSize)
				iTileSize = tileSize;

			pImageTile->tileheader.tileCount = numberOfTiles;
			pImageTile->tileheader.tileIndex = i;
			pImageTile->tileheader.tileSize = iTileSize;
			pImageTile->tileheader.tileOffset = tileSize * i;

			//
			unsigned char *dst = curptr + sizeof(uint32_t) + sizeof(Network::CPacketImageTile);
			memcpy(dst, tileData, iTileSize);

			*packetSize = sizeof(Network::CPacketImageTile) + iTileSize;
		}

		lResult = numberOfPackets;
		
	}

	return lResult;
}

bool Device_ProjectTango_Hardware::SendCameras(double timestamp, const std::vector<Network::CCameraInfo> &infoVector)
{
	bool lSuccess = false;

	// memory pool, take place for a new packets

	const unsigned numberOfCameras = (unsigned) infoVector.size();	// 1k for each tile
	
	unsigned char *ptr = nullptr; 

	if (nullptr != ptr)
	{
		for (unsigned i = 0; i < numberOfCameras; ++i)
		{
			// copy data into packets
			unsigned char *curptr = ptr + i * 256;

			uint32_t *packetSize = (uint32_t*)curptr;
			*packetSize = sizeof(Network::CPacketCameraInfo);

			Network::CPacketCameraInfo	*packetPtr = (Network::CPacketCameraInfo*) (curptr + sizeof(uint32_t));
			Network::FillBufferWithHeader(curptr, PACKET_REASON_CAMERA_INFO, (float)timestamp);

			//
			memcpy(&packetPtr->body, &infoVector[i], sizeof(Network::CCameraInfo));
		}
		
		lSuccess = true;
	}

	return lSuccess;
}

bool Device_ProjectTango_Hardware::SendTakes(double timestamp, const std::vector<Network::CPacketTakeInfo> &takeVector)
{
	bool lSuccess = false;

	// memory pool, take place for a new packets

	const unsigned numberOfTakes = (unsigned) takeVector.size();	// 1k for each tile
	
	unsigned char *ptr = nullptr; 

	if (nullptr != ptr)
	{
		for (unsigned i = 0; i < numberOfTakes; ++i)
		{
			// copy data into packets
			unsigned char *curptr = ptr + i * 256;

			uint32_t *packetSize = (uint32_t*)curptr;
			*packetSize = sizeof(Network::CPacketTakeInfo);

			Network::CPacketTakeInfo	*packetPtr = (Network::CPacketTakeInfo*) (curptr + sizeof(uint32_t));
			Network::FillBufferWithHeader(curptr, PACKET_REASON_TAKE_INFO, (float)timestamp);

			//
			memcpy(&packetPtr->body, &takeVector[i], sizeof(Network::CPacketTakeInfo));
		}

		lSuccess = true;
	}

	return lSuccess;
}

bool Device_ProjectTango_Hardware::PopCommand(Network::CCommand &cmd)
{
	return mCommands.Pop(cmd);
}



bool Device_ProjectTango_Hardware::PrepSceneCamerasInfo(std::vector<Network::CCameraInfo> &infoVector)
{
	
	FBScene *pScene = mSystem.Scene;
	//FBTime sysTime = mSystem.SystemTime;
	//double sysTimeSecs = sysTime.GetSecondDouble();

	int totalCount = 0;

	for (int i = 0, count = pScene->Cameras.GetCount(); i < count; ++i)
	{
		FBCamera *pCamera = pScene->Cameras[i];

		if (false == pCamera->SystemCamera)
			totalCount += 1;
	}

	infoVector.resize(totalCount);

	//
	int curNdx = 0;

	for (int i = 0, count = pScene->Cameras.GetCount(); i < count; ++i)
	{
		FBCamera *pCamera = pScene->Cameras[i];

		if (true == pCamera->SystemCamera)
			continue;

		Network::CCameraInfo &body = infoVector[curNdx];
		
		body.totalCount = totalCount;
		body.id = (unsigned char)curNdx;
		
		curNdx += 1;

		body.farPlane = pCamera->FarPlaneDistance;
		body.nearPlane = pCamera->NearPlaneDistance;

		FBVector3d v;
		pCamera->GetVector(v);

		for (int i = 0; i < 3; ++i)
		{
			body.translation[i] = v[i];
			body.orientation[i] = 0.0;
		}

		body.lens = pCamera->FieldOfView;

		FBString name(pCamera->LongName);
		sprintf_s(body.name.raw, sizeof(char)* 32, "%s", (char*)name);
	}

	return (totalCount > 0);
}

bool Device_ProjectTango_Hardware::IsOpen()
{
	uint32_t lstatus = g_status;

	return (lstatus > 0);
}