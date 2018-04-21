#ifndef __ORDEVICE_TEMPLATE_DEVICE_H__
#define __ORDEVICE_TEMPLATE_DEVICE_H__


/**	\file	ordevice_template_device.h

Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Class declaration
#include "device_projectTango_hardware.h"

//--- Registration defines
#define ORDEVICETEMPLATE__CLASSNAME		Device_ProjectTango
#define ORDEVICETEMPLATE__CLASSSTR		"Device_ProjectTango"

//! Device Template.
class Device_ProjectTango : public FBDevice
{
	//--- FiLMBOX declaration
	FBDeviceDeclare(Device_ProjectTango, FBDevice);
public:
	//--- FiLMBOX Construction/Destruction
	virtual bool FBCreate();		//!< FiLMBOX constructor.
	virtual void FBDestroy();		//!< FiLMBOX destructor.

	//--- The following will be called by the real-time engine.
	virtual bool AnimationNodeNotify(FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo) override;	//!< Real-time evaluation for node.
	virtual void DeviceIONotify(kDeviceIOs  pAction, FBDeviceNotifyInfo &pDeviceNotifyInfo) override;	//!< Notification of/for Device IO.
	virtual bool DeviceEvaluationNotify(kTransportMode pMode, FBEvaluateInfo* pEvaluateInfo) override;	//!< Evaluation the device (write to hardware).
	virtual bool DeviceOperation(kDeviceOperations pOperation) override;	//!< Operate device.

	virtual bool ModelTemplateBindNotify(FBModel* pModel, int pIndex, FBModelTemplate* pModelTemplate) override;

	void OnUIIdleEvent(HIRegister pSender, HKEvent pEvent);
	void OnRenderEvent(HIRegister pSender, HKEvent pEvent);

	//--- FBX Load/Save.
	virtual bool FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat) override;	//!< Store in FBX file.
	virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat) override;	//!< Retrieve from FBX file.

	//--- Initialisation/Shutdown
	bool		Init();			//!< Initialize/create device.
	bool		Start();		//!< Start device (online).
	bool		Reset();		//!< Reset device.
	bool		Stop();			//!< Stop device (offline).
	bool		Done();			//!< Remove device.

	//--- Recording
	void		DeviceRecordFrame( FBDeviceNotifyInfo &pDeviceNotifyInfo );

	//--- Aggregation of hardware parameters
	void		SetCommunicationType( FBCommType pType)		{ mHardware.SetCommunicationType( pType );		}
	int			GetCommunicationType()						{ return mHardware.GetCommunicationType();		}

	void		SetNetworkPort(int pPort)				{ mHardware.SetNetworkPort(pPort); }
	int			GetNetworkPort()						{ return mHardware.GetNetworkPort(); }
	void		SetDeviceAddress	(const char* pIPAddress){ mHardware.SetDeviceAddress(pIPAddress);		}
	const char*	GetDeviceAddress	()						{ return mHardware.GetDeviceAddress();			}
	void		SetDevicePort		(int pPort)				{ mHardware.SetDevicePort(pPort);				}
	int			GetDevicePort		()						{ return mHardware.GetDevicePort();			}
	
	void		SetStreaming		(bool pStreaming)		{ mHardware.SetStreaming( pStreaming );			}
	bool		GetStreaming		()						{ return mHardware.GetStreaming();				}

	bool		GetSetCandidate		()						{ return mSetCandidate;							}
	void		SetSetCandidate		( bool pState )			{ mSetCandidate = pState;						}
	double		GetSamplingRate		()						{ return mSamplingRate;							}
	void		SetSamplingRate		( double pRate )		{ mSamplingRate = pRate;						}
	FBDeviceSamplingMode	GetSamplingType()								{ return mSamplingType;			}
	void					SetSamplingType( FBDeviceSamplingMode pType )	{ mSamplingType = pType;		}

	void		SetCandidates		();

public:

	FBPropertyInt					SocketPort;
	FBPropertyString				DeviceAddress;
	FBPropertyInt					DevicePort;

	FBPropertyDouble				SpaceScale;

	FBPropertyBool					RelativeTimestamp;

	FBPropertyInt					SyncStateFrameRate;		// how many times per second we are sending sync state // 30
	FBPropertyInt					ImageFrameRate;			// how many times per second we are sending images // 10

	FBPropertyBool					SendImages;

	// stats (average for last 5 seconds)
	FBPropertyInt					SendPacketsRate;
	FBPropertyInt					RecvPacketsRate;

	FBPropertyInt					SendPacketsSize;
	FBPropertyInt					RecvPacketsSize;

	FBPropertyDouble				SendTimestamp;
	FBPropertyDouble				RecvTimestamp;

	FBPropertyAnimatableVector2d				ScreenPoint;
	FBPropertyAnimatableVector2d				ScreenSize;

	static int GetAvgSendRate(HIObject pObject);
	static int GetAvgSendSize(HIObject pObject);
	static int GetAvgRecvRate(HIObject pObject);
	static int GetAvgRecvSize(HIObject pObject);

public:
	FBModelTemplate*				mTemplateRoot;			//!< Root model template.
	FBModelTemplate*				mTemplateCamera;		//!< Marker model template.
	FBAnimationNode*				mNodeCamera_InT;		//!< Camera input animation node (translation).
	FBAnimationNode*				mNodeCamera_InR;		//!< Camera input animation node (rotation).
	FBAnimationNode*				mNodeCamera_InFOVX;
	FBAnimationNode*				mNodeCamera_InFOVY;
	FBAnimationNode*				mNodeDevice_Trigger[6];

private:
	FBSystem						mSystem;
	FBPlayerControl					mPlayerControl;			//!< In order to query the play state for recording.

	bool						mNeedPlayPrep;
	FBTime						mPacketStartTime;
	FBTime						mLocalStartTime;

	kTransportMode				mLastTransportMode;

	bool							mSetCandidate;			//!< Are we setting the candidate or writing to the connector?
	double							mSamplingRate;			//!< Device sampling rate.
	FBDeviceSamplingMode			mSamplingType;			//!< Device sampling type.
	Device_ProjectTango_Hardware		mHardware;				//!< Hardware member.
	
	struct
	{
		double							frameRate;
		double							lastSendTime;
		double							lastSyncTime;

		Network::CSyncControl			state;

	} mLastSync;
	
	struct
	{

		double				sendFrameRate;
		double				lastSendTime;

		double				syncFrameRate;
		double				lastSyncTime;

		// TODO: image data

	} mLastImage;
	
	//
	/*
	struct
	{
		double				lastSendTime;
		double				lastSyncTime;

		std::vector<Network::CCameraInfo>	cameraInfo;
	} mLastCameras;
	*/
	bool			mWaitToCameraExchange;

	// async read of a compressed texture
	size_t			mLastPBOSize;
	int				mCurPBO;
	unsigned int	mPBOs[2];

	std::vector<unsigned char>		mCompressedData;

	// compute stats
	struct CStatistics
	{
		double	lasttime;	// used to compute per second value

		int		counter;
		
		int		values[5];	// computed countr for last 5 seconds

		CStatistics()
		{
			lasttime = 0.0;
			counter = 0;
			values[0] = values[1] = values[2] = values[3] = values[4] = 0;
		}

		void PushValue(const double timestamp, const int val)
		{
			if (0.0 == lasttime)
				lasttime = timestamp;

			if (timestamp - lasttime > 1.0)
			{
				counter += 1;
				if (counter >= 5)
					counter = 0;

				values[counter] = val;
				lasttime = timestamp;
			}
			else
			{
				values[counter] += val;
			}
		}

		const int GetAvgValue()
		{
			int sum = 0;
			for (int i = 0; i < 5; ++i)
			{
				if (i != counter)
					sum += values[i];
			}
			
			sum = sum / 4;
			return sum;
		}
	};

	CStatistics		mSendRateStat;
	CStatistics		mSendSizeStat;
	CStatistics		mRecvRateStat;
	CStatistics		mRecvSizeStat;

	void SwitchToCamera(const int cameraId);
	void AlignToCamera(const int cameraId);
	void ParentToCamera(const int cameraId);
	void CreateANewCamera();

	bool MapCompressedData(FBVideoMemory *pVideoMemory, 
		const int origWidth, const int origHeight,
		const int previewWidth, const int previewHeight,
		const double timestamp, 
		unsigned char *buffer,
		const size_t buffer_size,
		Network::CImageHeader &header);

	void AllocPBOs(const size_t size);
	void FreePBOs();

	bool VideoMemoryToPackets(double timestamp, FBVideoMemory *pVideoMemory, const int w, const int h, const double aspect);

};

#endif /* __ORDEVICE_TEMPLATE_DEVICE_H__ */
