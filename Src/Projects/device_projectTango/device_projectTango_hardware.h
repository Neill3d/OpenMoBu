#ifndef __ORDEVICE_TEMPLATE_HARDWARE_H__
#define __ORDEVICE_TEMPLATE_HARDWARE_H__


/**	\file	ordevice_template_hardware.h

Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

//--- SDK include
#include <fbsdk/fbsdk.h>

//#include "tango_client_api.h"
#include "NetworkTango.h"
#include "NetworkUtils.h"

#include <vector>

//
#define MAX_BUFFER_SIZE		1500

// forward
class Device_ProjectTango;


////////////////////////////////////////////////////////////////////////////////////
// CStaticQueue

template <class T>
class CStaticQueue
{
public:

	//! a constructor
	CStaticQueue()
	{
		mCount = 0;
		mQueue.resize(16);
	}

	bool IsEmpty()
	{
		return (0 == mCount);
	}

	void Push(T &data)
	{
		if (mCount >= mQueue.size() - 2)
		{
			mQueue.resize(mQueue.size() + 16);
		}

		mQueue[mCount] = data;
		mCount += 1;
	}

	bool Pop(T &data)
	{
		bool lSuccess = false;
		if (mCount > 0)
		{
			mCount -= 1;
			data = mQueue[mCount];
			lSuccess = true;
		}

		return lSuccess;
	}

	bool Last(T &data)
	{
		bool lSuccess = false;
		if (mCount > 0)
		{
			data = mQueue[mCount-1];
			lSuccess = true;
		}

		return lSuccess;
	}

protected:
	//
	size_t				mCount;
	std::vector<T>		mQueue;

};


/////////////////////////////////////////////////////////////////////////////////////
// CSendQeueu


class CSendQueue
{

};



////////////////////////////////////////////////////////////////////////////////////
//! Device ProjectTango Hardware

class Device_ProjectTango_Hardware
{
public:
	Device_ProjectTango_Hardware();							//!< Constructor.
	~Device_ProjectTango_Hardware();							//!< Destructor.

	void		SetParent(FBDevice* pParent);

	//--- Communications
	bool		Open();										//!< Open the device.
	bool		Close();									//!< Close the device.
	int			FetchData			(size_t &recvBytes, double &recvTimestamp);						//!< Fetch a data packet.
	//int			FetchCommands();
	int		PollData			(size_t &sendBytes, double sendTimestamp);						//!< Poll for new data.

	bool IsOpen();

	//--- Attribute management
	void		SetStreaming		(bool pStreaming)		{	mStreaming = pStreaming;		}
	bool		GetStreaming		()						{	return mStreaming;				}
	void		SetCommunicationType(FBCommType pType);
	int			GetCommunicationType();
	void		SetNetworkPort		(int pPort)				{ 				}
	int			GetNetworkPort		()						{ 	return 0;			}
	
	void		SetDeviceAddress(const char *address)				{ mDeviceAddress.SetAddress(address); }
	
	const char  *GetDeviceAddress()						{  }
	void		SetDevicePort(int pPort)				{ mDeviceAddress.SetPortOnly(pPort); }
	int			GetDevicePort()						{ return mDeviceAddress.GetPort(); }
	
	const double GetSpaceScale() { return mSpaceScale;  }
	void SetSpaceScale(double value) { mSpaceScale = value;  }

	//--- Device interaction
	bool		GetSetupInfo		();
	bool		StartStream			();
	bool		StopStream			();

	//--- Hardware abstraction of device positional information

	bool PopDeviceData(Network::CPacketDevice &camdata);
	static void ExtractTR(const Network::CPacketDevice &camdata, const double spaceScale, double *pPos, double *pRot, double *pFOV, double *pFly, double *pTriggers, FBTime *pTime);

	bool PopCommand(Network::CCommand &cmd);

	// last values for real-time preview
	void		GetPosition			( double* pPos );
	void		GetRotation			( double* pRot );
	void		GetFieldOfView(double *pH, double *pV);
	void		GetCameraMoveScale(double *value);
	void		GetCameraFly(double *value);
	void		GetDeviceTrigger(int ndx, double *pValue);
	void		GetTime(FBTime *pTime);
	
	//
	//

	int EmptyUDPIncomingQueue();

	bool IsTimeForInvitation();

	bool IsSynced() const {
		return mIsSynced;
	}

	bool PrepSceneCamerasInfo(std::vector<Network::CCameraInfo> &infoVector);

	//
	// add packets to a memory pool waiting for sending to a device

	bool SendInvitation(double timestamp);
	bool SendSyncState(double timestamp, const Network::CSyncControl &syncState);
	int SendImage(double timestamp, int w, int h, int internalFormat, double aspect, const unsigned char *ptr, const unsigned bufsize);
	int SendImageTiles(double timestamp, int w, int h, int internalFormat, double aspect, const unsigned char *ptr, const unsigned bufsize);
	bool SendCameras(double timestamp, const std::vector<Network::CCameraInfo> &infoVector);
	bool SendTakes(double timestamp, const std::vector<Network::CPacketTakeInfo> &takeVector);

	
private:
	//--- Utility members
	FBSystem		mSystem;								//!< System interface.
	FBDevice*		mParent;

	//--- Data extraction members
	//ORReadState		mReadState;								//!< Current read state.

	bool			mIsSynced;	// did we make a connection with some device and receive it's address
	FBTime			mLastInvitationTime;
	FBTime			mInvitationPeriod;

	unsigned char	mBuffer	[ MAX_BUFFER_SIZE+1 ];			//!< Read buffer.
	//TangoPoseData	mData;
	
	double			mSpaceScale;

	double					mLastCameraTimestamp;
	CStaticQueue<Network::CPacketDevice>	mDeviceData;

	std::vector<unsigned char>		mCompressedData;

	//Network::CCameraData	mCameraData;		// last received camera data // TODO: it should be buffer for all last received packets !!!

	int				mDataCount;								//!< Count for read into data packet buffer.

	//--- Communications members

	//
	Network::Socket			mSocketPoses;
	Network::Socket			mSocketImage;

	Network::Address		mDeviceAddress;
	//FBString			mDeviceAddress;
	//int					mDevicePort;

	//---- Exchange data members
	
	struct CTimeStamps
	{
		double		systemTimestamp;	// timestamp of a data grab (OnUIEvent, OnRenderEvent, etc.)
		double		sendTimestamp;		// timestamp of a poll data (tcp send)
		double		deviceTimestamp;	// timestamp of a last feedback or request command (for a sync state for example)

		int			currBuffer;

		int			sendCount;			// how many packets of a pending data has been send
		int			totalCount;			// pending count (of cameras, takes, tiles, etc.)
	};

	
	CTimeStamps							mSyncTimestamps;


	//----------

	bool			mStreaming;								//!< Is device in streaming mode?

	//---------- some stats

	int				mSendPacketsSec;		// how many packets we have send per second
	int				mSendBytesSec;			// how many bytes of data we have send per second

	int				mRecvPacketsSec;		// received Packets per second
	int				mRecvBytesSec;			// bytes per second received


	//--- Device channel status
	double			mPosition[3];							//!< Position of input from hardware.
	double			mRotation[3];							//!< Rotation of input from hardware.
	double			mFieldOfViewX;
	double			mFieldOfViewY;
	double			mCameraMoveScale;
	double			mCameraFly;
	double			mDeviceTriggers[6];
	FBTime			mLastTime;

	//
	CStaticQueue<Network::CCommand>		mCommands;

};

#endif /* __ORDEVICE_TEMPLATE_HARDWARE_H__ */
