
/**	\file	device_projectTango_device.cxx

Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

//--- Class declaration
#include "device_projectTango_device.h"
#include "NetworkTango.h"

#include "GL\glew.h"

#include "DataExchange.h"

//--- Registration defines
#define ORDEVICETEMPLATE__CLASS		ORDEVICETEMPLATE__CLASSNAME
#define ORDEVICETEMPLATE__NAME		ORDEVICETEMPLATE__CLASSSTR
#define ORDEVICETEMPLATE__LABEL		"Device Project Tango"
#define ORDEVICETEMPLATE__DESC		"Device Project Tango"
#define ORDEVICETEMPLATE__PREFIX	"DeviceTango"

//--- FiLMBOX implementation and registration
FBDeviceImplementation	(	ORDEVICETEMPLATE__CLASS	);
FBRegisterDevice		(	ORDEVICETEMPLATE__NAME,
							ORDEVICETEMPLATE__CLASS,
							ORDEVICETEMPLATE__LABEL,
							ORDEVICETEMPLATE__DESC,
							"ic_launcher.png");	// Icon filename (default=Open Reality icon)


/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool Device_ProjectTango::FBCreate()
{
	mHardware.SetParent( this );

	// props
	FBPropertyPublish(this, SocketPort, "Socket Port", nullptr, nullptr);
	FBPropertyPublish(this, DeviceAddress, "Device Address", nullptr, nullptr);
	FBPropertyPublish(this, DevicePort, "Device Port", nullptr, nullptr);
	FBPropertyPublish(this, SpaceScale, "Space Scale", nullptr, nullptr);
	FBPropertyPublish(this, RelativeTimestamp, "Relative Timestamp", nullptr, nullptr);

	FBPropertyPublish(this, SendImages, "Send Images", nullptr, nullptr);

	FBPropertyPublish(this, SyncStateFrameRate, "Sync State FrameRate", nullptr, nullptr);
	FBPropertyPublish(this, ImageFrameRate, "Image FrameRate", nullptr, nullptr);

	FBPropertyPublish(this, ScreenPoint, "Screen Point", nullptr, nullptr);
	FBPropertyPublish(this, ScreenSize, "Screen Size", nullptr, nullptr);

	FBPropertyPublish(this, SendPacketsRate, "Send Packets Rate", GetAvgSendRate, nullptr);
	FBPropertyPublish(this, SendPacketsSize, "Send Packets Size", GetAvgSendSize, nullptr);
	FBPropertyPublish(this, RecvPacketsRate, "Recv Packets Rate", GetAvgRecvRate, nullptr);
	FBPropertyPublish(this, RecvPacketsSize, "Recv Packets Size", GetAvgRecvSize, nullptr);

	FBPropertyPublish(this, SendTimestamp, "Send Timestamp", nullptr, nullptr);
	FBPropertyPublish(this, RecvTimestamp, "Recv Timestamp", nullptr, nullptr);

	SocketPort = 8887;
	DeviceAddress = "192.168.1.76";
	DevicePort = 8889;

	SpaceScale = 100.0;
	RelativeTimestamp = true;

	SendImages = true;

	// how many states per second
	SyncStateFrameRate = 30;
	ImageFrameRate = 10;

	ScreenSize = FBVector2d(1920.0, 1080.0);

	ScreenPoint.SetAnimated(true);
	ScreenSize.SetAnimated(true);

	SendPacketsRate.ModifyPropertyFlag(kFBPropertyFlagReadOnly, true);
	SendPacketsSize.ModifyPropertyFlag(kFBPropertyFlagReadOnly, true);
	RecvPacketsRate.ModifyPropertyFlag(kFBPropertyFlagReadOnly, true);
	RecvPacketsSize.ModifyPropertyFlag(kFBPropertyFlagReadOnly, true);

	SendTimestamp.ModifyPropertyFlag(kFBPropertyFlagReadOnly, true);
	RecvTimestamp.ModifyPropertyFlag(kFBPropertyFlagReadOnly, true);

	// Create animation nodes
	mNodeCamera_InT	= AnimationNodeOutCreate( 0, "Translation",	ANIMATIONNODE_TYPE_LOCAL_TRANSLATION	);
	mNodeCamera_InR	= AnimationNodeOutCreate( 1, "Rotation",	ANIMATIONNODE_TYPE_LOCAL_ROTATION		);
	mNodeCamera_InFOVX = AnimationNodeOutCreate(2, "FieldOfView", "FieldOfView");
	mNodeCamera_InFOVY = AnimationNodeOutCreate(3, "FOV Vertical", ANIMATIONNODE_TYPE_FIELDOFVIEWY);

	const char *triggerNames[6] = { "Trigger1", "Trigger2", "Trigger3", "Trigger4", "Trigger5", "Trigger6" };

	for (int i = 0; i < 6; ++i)
	{
		mNodeDevice_Trigger[i] = AnimationNodeOutCreate(4 + i, triggerNames[i], ANIMATIONNODE_TYPE_NUMBER);
	}
	

	// Create model templates
	mTemplateRoot	= new FBModelTemplate( ORDEVICETEMPLATE__PREFIX, "Reference",	kFBModelTemplateRoot	);
	mTemplateCamera	= new FBModelTemplate( ORDEVICETEMPLATE__PREFIX, "Camera",		kFBModelTemplateCamera	);

	// Build model template hierarchy
	ModelTemplate.Children.Add(mTemplateRoot);
	mTemplateRoot->Children.Add(mTemplateCamera);

	// Bind the model templates (if applicable) to device's animation nodes
	mTemplateCamera->Bindings.Add(mNodeCamera_InR);
	mTemplateCamera->Bindings.Add(mNodeCamera_InT);
	mTemplateCamera->Bindings.Add(mNodeCamera_InFOVX);
	mTemplateCamera->Bindings.Add(mNodeCamera_InFOVY);

	for (int i = 0; i < 6; ++i)
		mTemplateCamera->Bindings.Add(mNodeDevice_Trigger[i]);

	// Set sampling rate to 60 Hz
	FBTime	lPeriod;
	lPeriod.SetSecondDouble(1.0/60.0);
	SamplingPeriod	= lPeriod;

	CommType = kFBCommTypeNetworkUDP;	// TCP or UDP ?!
	mSetCandidate = false;

	// by default ready to record
	RecordMode = true;

	mNeedPlayPrep = false;
	mLastTransportMode = kStop;
	mPacketStartTime = FBTime::Zero;
	mLocalStartTime = FBTime::Zero;

	mLastSync.frameRate = 1.0 / 30.0;
	mLastSync.lastSendTime = 0.0;
	mLastSync.lastSyncTime = 0.0;

	mWaitToCameraExchange = false;

	mLastPBOSize = 0;
	mCurPBO = 0;
	mPBOs[0] = mPBOs[1] = 0;

	mSystem.OnUIIdle.Add(this, (FBCallback)&Device_ProjectTango::OnUIIdleEvent);
	FBEvaluateManager::TheOne().OnRenderingPipelineEvent.Add(this, (FBCallback)&Device_ProjectTango::OnRenderEvent);

	return true;
}

int Device_ProjectTango::GetAvgSendRate(HIObject pObject)
{
	Device_ProjectTango *pBase = FBCast<Device_ProjectTango>(pObject);
	if (pBase)
	{
		return pBase->mSendRateStat.GetAvgValue();
	}
	return 0;
}

int Device_ProjectTango::GetAvgSendSize(HIObject pObject)
{
	Device_ProjectTango *pBase = FBCast<Device_ProjectTango>(pObject);
	if (pBase)
	{
		return pBase->mSendSizeStat.GetAvgValue();
	}
	return 0;
}

int Device_ProjectTango::GetAvgRecvRate(HIObject pObject)
{
	Device_ProjectTango *pBase = FBCast<Device_ProjectTango>(pObject);
	if (pBase)
	{
		return pBase->mRecvRateStat.GetAvgValue();
	}
	return 0;
}

int Device_ProjectTango::GetAvgRecvSize(HIObject pObject)
{
	Device_ProjectTango *pBase = FBCast<Device_ProjectTango>(pObject);
	if (pBase)
	{
		return pBase->mRecvSizeStat.GetAvgValue();
	}
	return 0;
}

/************************************************
 *	FiLMBOX Destructor.
 ************************************************/
void Device_ProjectTango::FBDestroy()
{
	FBEvaluateManager::TheOne().OnRenderingPipelineEvent.Remove(this, (FBCallback)&Device_ProjectTango::OnRenderEvent);
	mSystem.OnUIIdle.Remove(this, (FBCallback)&Device_ProjectTango::OnUIIdleEvent);
}


/************************************************
 *	Device operation.
 ************************************************/
bool Device_ProjectTango::DeviceOperation(kDeviceOperations pOperation)
{
	switch (pOperation)
	{
		case kOpInit:	return Init();
		case kOpStart:	return Start();
		case kOpStop:	return Stop();
		case kOpReset:	return Reset();
		case kOpDone:	return Done();
	}
	return FBDevice::DeviceOperation( pOperation );
}


/************************************************
 *	Initialization of device.
 ************************************************/
bool Device_ProjectTango::Init()
{
    FBProgress	lProgress;

    lProgress.Caption	= "Device Template";
	lProgress.Text		= "Initializing device...";

	mHardware.SetDeviceAddress(DeviceAddress);
	mHardware.SetDevicePort(DevicePort);

	return true;
}


/************************************************
 *	Device is put online.
 ************************************************/
bool Device_ProjectTango::Start()
{
	FBProgress	lProgress;
	lProgress.Caption	= "Starting up device";

	// Step 1: Open device communications
	lProgress.Text	= "Opening device communications";
	Status			= "Opening device communications";

	mHardware.SetDeviceAddress(DeviceAddress);
	mHardware.SetDevicePort(DevicePort);

	if(!mHardware.Open())
	{
		Status = "Could not open device";
		return false;
	}

	// Step 2: Ask hardware to get channel information
	lProgress.Text	= "Device found, getting setup information";
	Status			= "Getting setup information";

	if(!mHardware.GetSetupInfo())
	{
		Status = "Could not get setup information from device.";
		return false;
	}
	else
	{
		HardwareVersionInfo = "Device Project Tango, v1.0";
		Information			= "";
	}

	if( mHardware.GetStreaming() )
	{
		// Step 3: Start streaming data from device
		lProgress.Text	= "Sending START STREAM command";
		Status			= "Starting device streaming";

		if(!mHardware.StartStream())
		{
			Status	= "Could not start stream mode";
			return false;
		}
	}
	Status = "Ok";
	return true;
}


/************************************************
 *	Device is stopped (offline).
 ************************************************/
bool Device_ProjectTango::Stop()
{
	FBProgress	lProgress;
	lProgress.Caption	= "Shutting down device";

	if( mHardware.GetStreaming() )
	{
		// Step 1: Stop streaming data
		lProgress.Text	= "Sending STOP STREAM command";
		Status			= "Stopping device streaming";

		if(!mHardware.StopStream())
		{
			Status = "Could not stop streaming";
			return false;
		}
	}

	// Step 1: Stop streaming data
	lProgress.Text	= "Stopping device communications";
	Status			= "Stopping device communications";

	if(!mHardware.Close())
	{
		Status = "Could not close device";
		return false;
	}

	Status			= "?";

    return false;
}


/************************************************
 *	Removal of device.
 ************************************************/
bool Device_ProjectTango::Done()
{
	return false;
}


/************************************************
 *	Reset of device.
 ************************************************/
bool Device_ProjectTango::Reset()
{
    Stop();
    return Start();
}


/************************************************
 *	Real-Time Engine Evaluation.
 ************************************************/
bool Device_ProjectTango::AnimationNodeNotify(FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo)
{
	double	lPos[3];
	double	lRot[3];
	double lFovX, lFovY;
	double lTrigger;
	double	lCameraMoveMult;
	double	lCameraFly;

	mHardware.GetPosition( lPos );
	mHardware.GetRotation( lRot );
	mHardware.GetFieldOfView(&lFovX, &lFovY);
	mHardware.GetCameraMoveScale(&lCameraMoveMult);
	mHardware.GetCameraFly(&lCameraFly);

	if (0.0 != lCameraFly)
	{
		// TODO: move ref point in camera forward / backword direction
		FBVector4d v(0.01 * lCameraFly * lCameraMoveMult, 0.0, 0.0, 1.0);
		FBVector3d pos;

		FBMatrix m;
		FBRotationToMatrix(m, FBRVector(lRot));
		FBVectorMatrixMult(v, m, v);

		if (mTemplateRoot->Model)
		{
			mTemplateRoot->Model->GetVector(pos);

			for (int i = 0; i < 3; ++i)
				pos[i] += v[i];

			mTemplateRoot->Model->SetVector(pos);
		}
	}

	mNodeCamera_InT->WriteData( lPos, pEvaluateInfo );
	mNodeCamera_InR->WriteData( lRot, pEvaluateInfo );
	mNodeCamera_InFOVX->WriteData(&lFovX, pEvaluateInfo);
	mNodeCamera_InFOVY->WriteData(&lFovY, pEvaluateInfo);

	for (int i = 0; i < 6; ++i)
	{
		mHardware.GetDeviceTrigger(i, &lTrigger);
		mNodeDevice_Trigger[i]->WriteData(&lTrigger, pEvaluateInfo);
	}
	

    return true;
}


/************************************************
 *	Device Evaluation Notify.
 ************************************************/
bool Device_ProjectTango::DeviceEvaluationNotify(kTransportMode pMode, FBEvaluateInfo* pEvaluateInfo)
{
	if (mLastTransportMode != pMode)
	{
		mNeedPlayPrep = true;
		mLocalStartTime = pEvaluateInfo->GetLocalTime();
		mLastTransportMode = pMode;
	}
		

	return true;
}


/************************************************
 *	Real-Time Synchronous Device IO.
 ************************************************/
void Device_ProjectTango::DeviceIONotify(kDeviceIOs  pAction, FBDeviceNotifyInfo &pDeviceNotifyInfo)
{
//	int		lNumberOfPackets = 0;
//	FBTime	lPacketTimeCode;
	FBTime sysTime = pDeviceNotifyInfo.GetSystemTime();
	const double sysTimeSecs = sysTime.GetSecondDouble();

    switch (pAction)
	{
		// Output devices
		case kIOPlayModeWrite:
		case kIOStopModeWrite:
		{
			//
			size_t sendBytes=0;
			int sendPackets= mHardware.PollData(sendBytes, sysTimeSecs);

			if (sendPackets > 0)
			{
				mSendRateStat.PushValue(sysTimeSecs, sendPackets);
				mSendSizeStat.PushValue(sysTimeSecs, sendBytes);

				SendTimestamp = sysTimeSecs;
			}

			SendTimestamp = sysTimeSecs;

		}
		break;

		// Input devices
		case kIOStopModeRead:
		case kIOPlayModeRead:
		{
				// check sync state timestamp to compute a difference

				size_t recvBytes = 0;
				double recvTimestamp = 0.0;

				int recvPackets = mHardware.FetchData(recvBytes, recvTimestamp);
				
				for (int i = 0; i < recvPackets; ++i)
				{
					DeviceRecordFrame(pDeviceNotifyInfo);

					

					//for (int i = 0; i < recvPackets; ++i)
						AckOneSampleReceived();
				
					
				}

				if (recvPackets > 0)
				{
					mRecvRateStat.PushValue(sysTimeSecs, recvPackets);
					mRecvSizeStat.PushValue(sysTimeSecs, recvBytes);
					RecvTimestamp = recvTimestamp;
				}
		break;
		}
	}
}


/************************************************
 *	Record a frame of the device (recording).
 ************************************************/
void Device_ProjectTango::DeviceRecordFrame(FBDeviceNotifyInfo &pDeviceNotifyInfo)
{
	FBAnimationNode*	lDataT	= nullptr;
	FBAnimationNode*	lDataR = nullptr;
	FBAnimationNode		*lDataFovX = nullptr;
	FBAnimationNode		*lDataFovY = nullptr;
	FBAnimationNode		*lDataTrigger1 = nullptr;

	double	lPos[3];
	double	lRot[3];
	double	lFovX = 40.0;
	double lFovY = 40.0;
	double lFly = 0.0;
	double	lTriggers[6];

	FBTime	lTime, lPacketTime;

	Network::CPacketDevice camdata;
	
	const double playSpeed = mPlayerControl.GetPlaySpeed();

	if (kFBTransportPlay != mPlayerControl.GetTransportMode())
		return;

	if (false == mHardware.PopDeviceData(camdata))
		return;

	mHardware.ExtractTR(camdata, SpaceScale, lPos, lRot, &lFovX, &lFly, lTriggers, &lPacketTime);

	if (true == mNeedPlayPrep)
	{
		mPacketStartTime = lPacketTime;
		mNeedPlayPrep = false;
	}

	lTime = pDeviceNotifyInfo.GetLocalTime();

	if (RelativeTimestamp)
	{
		double secs = mLocalStartTime.GetSecondDouble() + playSpeed * (lPacketTime.GetSecondDouble() - mPacketStartTime.GetSecondDouble());
		lTime.SetSecondDouble(secs);
		//lTime = mLocalStartTime + lPacketTime - mPacketStartTime;
	}
	else
	{
		lTime = lPacketTime;
	}

	//mHardware.GetTime(&lTime);

	// Translation information.
	lDataT = mNodeCamera_InT->GetAnimationToRecord();
	if (lDataT)
	{
		//mHardware.GetPosition( lPos );

		switch( SamplingMode.AsInt() )
		{
			case kFBHardwareTimestamp:
			case kFBSoftwareTimestamp:
			{
				lDataT->KeyAdd(lTime, lPos);
			}
			break;

			case kFBHardwareFrequency:
			case kFBAutoFrequency:
			{
				lDataT->KeyAdd(lPos);
			}
			break;
		}
	}

	// Rotation information.
	lDataR = mNodeCamera_InR->GetAnimationToRecord();
	if (lDataR)
	{
		//mHardware.GetRotation( lRot );
		switch( SamplingMode.AsInt() )
		{
			case kFBHardwareTimestamp:
			case kFBSoftwareTimestamp:
			{
				lDataR->KeyAdd(lTime, lRot);
			}
			break;

			case kFBHardwareFrequency:
			case kFBAutoFrequency:
			{
				lDataR->KeyAdd(lRot);
			}
			break;
		}
	}

	// Field Of View information.
	lDataFovX = mNodeCamera_InFOVX->GetAnimationToRecord();
	lDataFovY = mNodeCamera_InFOVY->GetAnimationToRecord();
	if (lDataFovX && lDataFovY)
	{
		//mHardware.GetFieldOfView(&lFovX, &lFovY);
		switch (SamplingMode.AsInt())
		{
		case kFBHardwareTimestamp:
		case kFBSoftwareTimestamp:
		{
			lDataFovX->KeyAdd(lTime, &lFovX);
			lDataFovY->KeyAdd(lTime, &lFovY);
		}
		break;

		case kFBHardwareFrequency:
		case kFBAutoFrequency:
		{
			lDataFovX->KeyAdd(&lFovX);
			lDataFovY->KeyAdd(&lFovY);
		}
		break;
		}
	}

	// Trigger information.
	for (int i = 0; i < 6; ++i)
	{
		lDataTrigger1 = mNodeDevice_Trigger[i]->GetAnimationToRecord();
		if (lDataTrigger1)
		{
			//mHardware.GetDeviceTrigger(&lTrigger1);
			switch (SamplingMode.AsInt())
			{
			case kFBHardwareTimestamp:
			case kFBSoftwareTimestamp:
			{
				lDataTrigger1->KeyAdd(lTime, &lTriggers[i]);
			}
				break;

			case kFBHardwareFrequency:
			case kFBAutoFrequency:
			{
				lDataTrigger1->KeyAdd(&lTriggers[i]);
			}
				break;
			}
		}
	}
}

void Device_ProjectTango::SetCandidates()
{
	double	lPos[3];
	double	lRot[3];
	double lFovH, lFovV;
	double lTrigger;

	mHardware.GetPosition( lPos );
	mHardware.GetRotation( lRot );
	mHardware.GetFieldOfView(&lFovH, &lFovV);
	
	mNodeCamera_InT->SetCandidate( lPos );
	mNodeCamera_InR->SetCandidate( lRot );
	mNodeCamera_InFOVX->SetCandidate(&lFovH);
	mNodeCamera_InFOVY->SetCandidate(&lFovV);
	
	for (int i = 0; i < 6; ++i)
	{
		mHardware.GetDeviceTrigger(i, &lTrigger);
		mNodeDevice_Trigger[i]->SetCandidate(&lTrigger);
	}
	
}

//--- FBX load/save tags
#define FBX_VERSION_TAG		"Version"
#define FBX_VERSION_VAL		100
#define FBX_COMMPARAM_TAG	"Communication"

/************************************************
 *	Store data in FBX.
 ************************************************/
bool Device_ProjectTango::FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	if (pStoreWhat & kAttributes)
	{
		// Write version
		pFbxObject->FieldWriteI		(FBX_VERSION_TAG,FBX_VERSION_VAL);

		// Write communications settings
		pFbxObject->FieldWriteBegin	(FBX_COMMPARAM_TAG);
		{
		}
	    pFbxObject->FieldWriteEnd();
	}
	return true;
}


/************************************************
 *	Retrieve data from FBX.
 ************************************************/
bool Device_ProjectTango::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	int			Version;
	FBString	name;

	if (pStoreWhat & kAttributes)
	{
		// Get version
		Version	= pFbxObject->FieldReadI(FBX_VERSION_TAG);

		// Get communications settings
		if (pFbxObject->FieldReadBegin(FBX_COMMPARAM_TAG))
		{

			pFbxObject->FieldReadEnd();
		}
	}

	return true;
}

bool Device_ProjectTango::MapCompressedData(	FBVideoMemory *pVideoMemory,
												const int origWidth, const int origHeight,
												const int previewWidth, const int previewHeight,
												const double timestamp,
												unsigned char *buffer,
												const size_t buffer_size,
												Network::CImageHeader &header)
{

	bool lSuccess = false;
	const GLuint id = pVideoMemory->TextureOGLId;
	
	if (0 == id)
		return false;

	glBindTexture(GL_TEXTURE_2D, id);

	GLint compressed = 0;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED, &compressed);

	if (compressed == GL_TRUE)
	{
		GLint compressed_size;
		GLint internalFormat;

		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &compressed_size);

		if (compressed_size + 1 < buffer_size)
		{
			//mCompressedData.resize(compressed_size + 1); // sizeof(CompressImageHeader));

			//CompressImageHeader *pHeader = (CompressImageHeader*)mCompressedData.data();
			//pHeader->timestamp = timestamp;
			header.aspect = 1.0f * (float)origWidth / (float)origHeight;
			header.width = (unsigned int)previewWidth;
			header.height = (unsigned int)previewHeight;
			header.internalFormat = internalFormat;
			header.dataSize = compressed_size;
//			header.dataFormat = GL_UNSIGNED_BYTE;

			unsigned char *ptr = buffer;
			glGetCompressedTexImage(GL_TEXTURE_2D, 0, ptr);

			//printf("compressed size - %d\n", compressed_size);
			lSuccess = true;
		}
		
	}
	else
	{
		GLint internalFormat;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);

		GLint size = origWidth * origHeight * 3;
		//mCompressedData.resize(size + 1); // sizeof(CompressImageHeader));

		if (size + 1 < buffer_size)
		{
			header.aspect = 1.0f * (float)origWidth / (float)origHeight;
			header.width = (unsigned int)previewWidth;
			header.height = (unsigned int)previewHeight;
			header.internalFormat = GL_RGB8;
			header.dataSize = size;
//			header.dataFormat = GL_UNSIGNED_BYTE;

			unsigned char *ptr = buffer;
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, ptr);

			lSuccess = true;
		}
		
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	return lSuccess;
}

void Device_ProjectTango::OnRenderEvent(HIRegister pSender, HKEvent pEvent)
{
	//if (false == mHardware.IsSynced())
	//	return;

	FBEventEvalGlobalCallback	lEvent(pEvent);

	if (kFBGlobalEvalCallbackAfterRender == lEvent.GetTiming())
	{

		// DONE: use sync rate to grab image with a specified period

		FBScene *pScene = mSystem.Scene;
#if (K_KERNEL_VERSION > 16000)
		FBTime sysTime(lEvent.GetEvaluateInfo()->GetSystemTime());
#else
		FBTime sysTime(mSystem.SystemTime);
#endif
		const double sysTimeSecs = sysTime.GetSecondDouble();
		const double syncStep = (1.0 / ImageFrameRate);

		bool timeForImage = (0.0 == mLastImage.lastSyncTime || (sysTimeSecs - mLastImage.lastSyncTime) > syncStep);

		if (Online && SendImages && timeForImage)
		{
			// find current post processing object
			FBVideo *pVideo = nullptr;

			int w (1);
			int h (1);
			double aspect(1.0);

			for (int i = 0, count = pScene->UserObjects.GetCount(); i < count; ++i)
			{
				FBUserObject *pObject = pScene->UserObjects[i];

				FBProperty *pProp = pObject->PropertyList.Find("UniqueClassId");
				if (nullptr != pProp && 55 == pProp->AsInt())
				{
					pProp = pObject->PropertyList.Find("Output Preview");
					if (nullptr != pProp && pProp->AsInt() > 0)
					{
						pProp = pObject->PropertyList.Find("Output Video");
						if (nullptr != pProp)
						{
							FBPropertyListObject *pList = (FBPropertyListObject*)pProp;
							if (pList->GetCount() > 0)
							{
								pVideo = (FBVideo*)pList->GetAt(0);
							}
						}

						if (nullptr != pVideo)
						{
							pProp = pObject->PropertyList.Find("Output Width");
							if (nullptr != pProp)
								w = pProp->AsInt();
							pProp = pObject->PropertyList.Find("Output Height");
							if (nullptr != pProp)
								h = pProp->AsInt();
							pProp = pObject->PropertyList.Find("Output Aspect");
							if (nullptr != pProp)
								pProp->GetData(&aspect, sizeof(double));
						}
					}
				}
			}

			//
			if (FBIS(pVideo, FBVideoMemory))
			{
				FBVideoMemory *pmem = (FBVideoMemory*)pVideo;

				if (true == VideoMemoryToPackets(sysTimeSecs, pmem, w, h, aspect))
				{
					mLastImage.lastSyncTime = sysTimeSecs;
				}
			}

		}
	}
}

void Device_ProjectTango::FreePBOs()
{
	if (mPBOs[0] > 0)
	{
		glDeleteBuffers(2, mPBOs);
		mPBOs[0] = mPBOs[1] = 0;
		mCurPBO = 0;
	}
}

void Device_ProjectTango::AllocPBOs(const size_t size)
{
	if (size != mLastPBOSize)
	{
		FreePBOs();

		glGenBuffers(2, mPBOs);

		// create 2 pixel buffer objects, you need to delete them when program exits.
		// glBufferDataARB with NULL pointer reserves only memory space.
		for (int i = 0; i<2; ++i)
		{
			glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBOs[i]);
			glBufferData(GL_PIXEL_PACK_BUFFER, size, 0, GL_STREAM_DRAW);
		}
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

		mLastPBOSize = size;
	}
}

bool Device_ProjectTango::VideoMemoryToPackets(double timestamp, FBVideoMemory *pVideoMemory, const int w, const int h, const double aspect)
{
	const unsigned int texId = pVideoMemory->TextureOGLId;

	if (0 == texId)
		return false;

	glBindTexture(GL_TEXTURE_2D, texId);

	GLint compressed = 0;
	GLint compressed_size = 0;
	GLint internalFormat = 0;

	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED, &compressed);

	if (compressed == GL_TRUE)
	{
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &compressed_size);
	}
	else
	{
		compressed_size = w * h * 3;
		internalFormat = GL_RGB8;
	}

	// DONE: use PBO to read compressed texture async

	size_t imageSize = compressed_size;

#ifndef GRAB_COMPRESSED_WITH_PBO

	AllocPBOs(imageSize);
	mCurPBO = 1 - mCurPBO;

	// read pixel into pbo

	glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBOs[mCurPBO]);

	if (GL_TRUE == compressed)
		glGetCompressedTexImage(GL_TEXTURE_2D, 0, 0);	// ptr 0 - we are reading into PBO
	else
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB8, GL_UNSIGNED_BYTE, 0); // into PBO

	glBindTexture(GL_TEXTURE_2D, 0);
	

	// map another PBO for reading data from it
	bool lSuccess = false;

	glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBOs[1 - mCurPBO]);
	GLubyte *ptr = (GLubyte*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);

	if (nullptr != ptr)
	{
		// DONE: convert data into packets !
		//int numberOfPackets = 0;
		//numberOfPackets = mHardware.SendImage(timestamp, w, h, internalFormat, aspect, ptr, (unsigned)imageSize);
		


		// compress data and send packet
		int lResult = mHardware.SendImage(timestamp, w, h, internalFormat, aspect, ptr, (unsigned)imageSize);
		lSuccess = (lResult > 0);
		//lSuccess = ExchangeWriteImage(timestamp, w, h, internalFormat, aspect, ptr, (unsigned)imageSize);

		glUnmapBuffer(GL_PIXEL_PACK_BUFFER);

		//lSuccess = (numberOfPackets > 0);
	}

	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

#else

	mCompressedData.resize(imageSize);

	if (GL_TRUE == compressed)
		glGetCompressedTexImage(GL_TEXTURE_2D, 0, mCompressedData.data());	// ptr 0 - we are reading into PBO
	else
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB8, GL_UNSIGNED_BYTE, mCompressedData.data()); // into PBO
	glBindTexture(GL_TEXTURE_2D, 0);

	bool lSuccess = ExchangeWriteImage(timestamp, w, h, internalFormat, aspect, mCompressedData.data(), (unsigned)imageSize);

#endif

	return lSuccess;
}

void Device_ProjectTango::OnUIIdleEvent(HIRegister pSender, HKEvent pEvent)
{
	FBTime systemTime(mSystem.SystemTime);
	const double timestamp = systemTime.GetSecondDouble();

	if (false == mHardware.IsSynced())
	{
		if (true == mHardware.IsTimeForInvitation())
		{
			mHardware.SendInvitation(timestamp);
		}

		if (false == mHardware.IsOpen())
		{
			return;
		}
	}
	
	bool prepCameraInfo = false;

	Network::CCommand	cmd;

	while (true == mHardware.PopCommand(cmd))
	{
		if (PACKET_COMMAND_PLAY == cmd.command)
		{
			if (mPlayerControl.IsPlaying)
				mPlayerControl.Stop();
			else
				mPlayerControl.Play();
		}
		else if (PACKET_COMMAND_STOP == cmd.command)
		{
			mPlayerControl.Stop();
		}
		else if (PACKET_COMMAND_RECORD == cmd.command)
		{
			if (false == mPlayerControl.IsRecording)
				mPlayerControl.Record();
		}
		else if (PACKET_COMMAND_LIVE == cmd.command)
		{
			Live.SetInt(1 - Live.AsInt());
		}
		else if (PACKET_COMMAND_REWIND == cmd.command)
		{
			mPlayerControl.GotoStart();
		}
		else if (PACKET_COMMAND_LOOP == cmd.command)
		{
#if PRODUCT_VERSION >= 2024
			mPlayerControl.LoopMode = (mPlayerControl.LoopMode == FBTransportLoopMode::kFBTransportNoLoop) ? FBTransportLoopMode::kFBTransportLoopCurrentTake : FBTransportLoopMode::kFBTransportNoLoop;
#else
			mPlayerControl.LoopActive = !mPlayerControl.LoopActive;
#endif
		}
		else if (PACKET_COMMAND_FRAME_CURR == cmd.command)
		{
			mPlayerControl.Goto(FBTime(0, 0, 0, cmd.value));
		}
		else if (PACKET_COMMAND_FRAME_START == cmd.command)
		{
			mPlayerControl.LoopStart = FBTime(0, 0, 0, cmd.value);
		}
		else if (PACKET_COMMAND_FRAME_STOP == cmd.command)
		{
			mPlayerControl.LoopStop = FBTime(0, 0, 0, cmd.value);
		}
		else if (PACKET_COMMAND_SPEED_NDX == cmd.command)
		{
#if (K_KERNEL_VERSION >= 16000)
			mPlayerControl.SetPlaySpeedMode(static_cast<FBTransportPlaySpeed>(cmd.value));
#else
			mPlayerControl.SetPlaySpeed((FBTransportPlaySpeed)cmd.value);
#endif
		}
		else if (PACKET_COMMAND_TAKE_NDX == cmd.command)
		{
			int takeIndex = cmd.value;
			if (takeIndex >= 0 && takeIndex < mSystem.Scene->Takes.GetCount())
				mSystem.CurrentTake = mSystem.Scene->Takes[takeIndex];
		}
		else if (PACKET_COMMAND_SCREEN_POS == cmd.command)
		{
			int x = cmd.value >> 16;
			int y = (short)(cmd.value & 0xFFFF);

			ScreenPoint = FBVector2d((double)x, (double)y);
		}
		else if (PACKET_COMMAND_SCREEN_SIZE == cmd.command)
		{
			int x = cmd.value >> 16;
			int y = (short)(cmd.value & 0xFFFF);

			ScreenSize = FBVector2d((double)x, (double)y);
		}
		else if (PACKET_COMMAND_CAMERALIST_REQ == cmd.command)
		{
			// DONE: send packets with camera list
			prepCameraInfo = true;
		}
		else if (PACKET_COMMAND_CAMERA_SWITCH == cmd.command)
		{
			SwitchToCamera(cmd.value);
		}
		else if (PACKET_COMMAND_CAMERA_ALIGN == cmd.command)
		{
			AlignToCamera(cmd.value);
		}
		else if (PACKET_COMMAND_CAMERA_PARENT == cmd.command)
		{
			ParentToCamera(cmd.value);
		}
		else if (PACKET_COMMAND_CAMERA_NEW == cmd.command)
		{
			CreateANewCamera();
			//
			prepCameraInfo = true;
		}
		else if (PACKET_COMMAND_TAKE_REM == cmd.command)
		{
			// TODO: remove current take
			FBTake *pTake = mSystem.CurrentTake;
			if (nullptr != pTake)
				pTake->FBDelete();
		}
	}

	//
	if (true == prepCameraInfo)
	{
		mWaitToCameraExchange = true;
	}

	if (true == mWaitToCameraExchange)
	{
		if (true == ExchangeWriteCameras(timestamp))
		{
			std::vector<Network::CCameraInfo> &info = ExchangeGetCameraInfo();
			mHardware.PrepSceneCamerasInfo(info);

			ExchangeWriteCamerasFinish();

			mWaitToCameraExchange = false;
		}
	}

	// DONE: send a sync state

	if (0.0 == mLastSync.lastSendTime || (timestamp - mLastSync.lastSendTime) > mLastSync.frameRate)
	{
		mLastSync.lastSendTime = timestamp;

		Network::CSyncControl		syncState;

		FBTime loopStart(mPlayerControl.LoopStart);
		FBTime loopStop(mPlayerControl.LoopStop);
		FBTime localTime(mSystem.LocalTime);

		syncState.currFrame = localTime.GetFrame();
		syncState.startFrame = loopStart.GetFrame();
		syncState.stopFrame = loopStop.GetFrame();

		syncState.playMode = (mPlayerControl.IsPlaying) ? 1 : 0;
		syncState.recordMode = (mPlayerControl.IsRecording) ? 1 : 0;
#if PRODUCT_VERSION >= 2024
		syncState.loopMode = (mPlayerControl.LoopMode != FBTransportLoopMode::kFBTransportNoLoop) ? 1 : 0;
#else
		syncState.loopMode = (mPlayerControl.LoopActive) ? 1 : 0;
#endif		
		syncState.liveMode = (Live) ? 1 : 0;

		syncState.takeCount = (unsigned char)mSystem.Scene->Takes.GetCount();
		syncState.take = (unsigned char)mSystem.Scene->Takes.Find(mSystem.CurrentTake);
#if (K_KERNEL_VERSION > 16000)
		syncState.speed = (unsigned char)(int)mPlayerControl.GetPlaySpeedMode();
#else
		syncState.speed = (unsigned char)(int)mPlayerControl.GetPlaySpeed();
#endif
		//mHardware.SendSyncState(timestamp, syncState);
		ExchangeWriteSyncState(timestamp, syncState);
	}

}

bool Device_ProjectTango::ModelTemplateBindNotify(FBModel* pModel, int pIndex, FBModelTemplate* pModelTemplate)
{
	
	FBModel *pModelTemplateModel = mTemplateCamera->Model;

	if (FBIS(pModelTemplateModel, FBCamera))
	{
		((FBCamera*)pModelTemplateModel)->FieldOfView.SetAnimated(true);
	}

	pModelTemplateModel = mTemplateRoot->Model;
	if (nullptr != pModelTemplateModel)
	{
		pModelTemplateModel->Translation.SetAnimated(true);
	}

	return true;
}

void Device_ProjectTango::SwitchToCamera(const int cameraId)
{
	if (cameraId >= 0)
	{
		FBRenderer *pRenderer = mSystem.Renderer;
		FBScene *pScene = mSystem.Scene;

		int totalCount = 0;

		for (int i = 0, count = pScene->Cameras.GetCount(); i < count; ++i)
		{
			FBCamera *pCamera = pScene->Cameras[i];

			if (true == pCamera->SystemCamera)
				continue;

			if (cameraId == totalCount)
			{
#if (PRODUCT_VERSION >= 2024)
				int paneId = pRenderer->GetSelectedPaneIndex();
				pRenderer->SetCameraInPane(pCamera, paneId);
#else
				pRenderer->CurrentCamera = pCamera;
#endif
				break;
			}

			totalCount += 1;
		}
	}
}

void Device_ProjectTango::AlignToCamera(const int cameraId)
{
	FBScene *pScene = mSystem.Scene;

	if (-1 == cameraId)
	{
		// reset position
		FBVector3d v(0.0, 0.0, 0.0);

		if (nullptr != mTemplateRoot->Model)
			mTemplateRoot->Model->SetVector(v);
	}
	else
	{
		int totalCount = 0;

		for (int i = 0, count = pScene->Cameras.GetCount(); i < count; ++i)
		{
			FBCamera *pCamera = pScene->Cameras[i];

			if (true == pCamera->SystemCamera)
				continue;

			if (cameraId == totalCount)
			{
				FBVector3d v;
				pCamera->GetVector(v);

				if (nullptr != mTemplateRoot->Model)
					mTemplateRoot->Model->SetVector(v);

				break;
			}

			totalCount += 1;
		}
	}
}

void Device_ProjectTango::ParentToCamera(const int cameraId)
{
	FBScene *pScene = mSystem.Scene;

	if (nullptr != mTemplateRoot->Model && nullptr != mTemplateCamera->Model)
	{
		if (-1 == cameraId)
		{
			// DONE: unparent ref
			mTemplateRoot->Model->Parent = nullptr;
		}
		else
		{
			// DONE: parent ref to a specified camera (CHECK if new camera is not our current template camera)

			int totalCount = 0;

			for (int i = 0, count = pScene->Cameras.GetCount(); i < count; ++i)
			{
				FBCamera *pCamera = pScene->Cameras[i];

				if (true == pCamera->SystemCamera)
					continue;

				totalCount += 1;

				if (mTemplateCamera->Model == pCamera)
					continue;

				if (cameraId == totalCount)
				{
					
					if (nullptr != mTemplateRoot->Model)
						mTemplateRoot->Model->Parent = pCamera;

					break;
				}
			}
		}
	}
	
}

void Device_ProjectTango::CreateANewCamera()
{
	// DONE: create a null and camera and assign it into template slots
	/*
	FBModelRoot		*pNewRoot = new FBModelRoot("DeviceTango:Reference");
	pNewRoot->Show = true;

	FBCamera *pNewCamera = new FBCamera("DeviceTango:Camera");
	pNewCamera->Show = true;
	pNewCamera->Parent = pNewRoot;
	*/
	ModelBindingCreate();
}