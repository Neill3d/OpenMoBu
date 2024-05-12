
/**	\file	device_facecap_device.cxx
*	Developed by Sergei <Neill3d> Solokhin 2019
*	e-mail to: neill3d@gmail.com
*	twitter: @Neill3d
*
* OpenMoBu github - https://github.com/Neill3d/OpenMoBu
*/

//--- Class declaration
#include "device_facecap_device.h"

//--- Registration defines
#define CDEVICEFACECAP__CLASS		CDEVICEFACECAP__CLASSNAME
#define CDEVICEFACECAP__NAME		CDEVICEFACECAP__CLASSSTR
#define CDEVICEFACECAP__LABEL		"FaceCap OSC Device"
#define CDEVICEFACECAP__DESC		"FaceCap OSC Device"
#define CDEVICEFACECAP__PREFIX		"FaceCap"

//--- FiLMBOX implementation and registration
FBDeviceImplementation	(	CDEVICEFACECAP__CLASS	);
FBRegisterDevice		(	CDEVICEFACECAP__NAME,
							CDEVICEFACECAP__CLASS,
							CDEVICEFACECAP__LABEL,
							CDEVICEFACECAP__DESC,
							"character_actor.png");	// Icon filename (default=Open Reality icon)


/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool CDevice_FaceCap::FBCreate()
{
	mHardware.SetParent( this );

	FBPropertyPublish(this, SpaceScale, "Space Scale", nullptr, nullptr);
	FBPropertyPublish(this, ShapeValueMult, "Shape Value Mult", nullptr, nullptr);

	SpaceScale = 100.0;
	ShapeValueMult = 100.0;

	// Create animation nodes
	mNodeHead_InT	= AnimationNodeOutCreate( 0, "Translation",	ANIMATIONNODE_TYPE_LOCAL_TRANSLATION	);
	mNodeHead_InR	= AnimationNodeOutCreate( 1, "Rotation",	ANIMATIONNODE_TYPE_LOCAL_ROTATION		);

	mNodeLeftEye_InR = AnimationNodeOutCreate(2, "LeftEye Rotation", ANIMATIONNODE_TYPE_LOCAL_ROTATION);
	mNodeRightEye_InR = AnimationNodeOutCreate(3, "RightEye Rotation", ANIMATIONNODE_TYPE_LOCAL_ROTATION);

	for (uint32_t i = 0; i < static_cast<uint32_t>(EHardwareBlendshapes::count); ++i)
	{
		mNodeHead_Blendshapes[i] = AnimationNodeOutCreate(i+4, blendshape_names[i], ANIMATIONNODE_TYPE_NUMBER);
	}

	// default values
	mNodeHead_InT->SetCandidate(FBVector3d(0.0, 5.0, 0.0));
	
	
	// Create model templates
	mTemplateRoot	= new FBModelTemplate( CDEVICEFACECAP__PREFIX, "Reference",	kFBModelTemplateRoot	);
	mTemplateHead	= new FBModelTemplate( CDEVICEFACECAP__PREFIX, "Head",		kFBModelTemplateMarker	);

	mTemplateLeftEye = new FBModelTemplate(CDEVICEFACECAP__PREFIX, "LeftEye", kFBModelTemplateMarker);
	mTemplateRightEye = new FBModelTemplate(CDEVICEFACECAP__PREFIX, "RightEye", kFBModelTemplateMarker);

	// Build model template hierarchy
	ModelTemplate.Children.Add(mTemplateRoot);
	mTemplateRoot->Children.Add(mTemplateHead);

	mTemplateHead->Children.Add(mTemplateLeftEye);
	mTemplateHead->Children.Add(mTemplateRightEye);

	// Bind the model templates (if applicable) to device's animation nodes
	mTemplateHead->Bindings.Add( mNodeHead_InR );
	mTemplateHead->Bindings.Add( mNodeHead_InT );

	mTemplateLeftEye->Bindings.Add(mNodeLeftEye_InR);
	mTemplateRightEye->Bindings.Add(mNodeRightEye_InR);

	mTemplateHead->DefaultTranslation = FBVector3d(0.0, 5.0, 0.0);
	mTemplateLeftEye->DefaultTranslation = FBVector3d(-5.0, 5.0, 0.0);
	mTemplateRightEye->DefaultTranslation = FBVector3d(5.0, 5.0, 0.0);

	// Set sampling rate to 60 Hz
	FBTime	lPeriod;
	lPeriod.SetSecondDouble(1.0/60.0);
	SamplingPeriod	= lPeriod;

	CommType = kFBCommTypeNetworkUDP;
	mSetCandidate = false;

	return true;
}

/************************************************
 *	FiLMBOX Destructor.
 ************************************************/
void CDevice_FaceCap::FBDestroy()
{
}


/************************************************
 *	Device operation.
 ************************************************/
bool CDevice_FaceCap::DeviceOperation( kDeviceOperations pOperation )
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
bool CDevice_FaceCap::Init()
{
    FBProgress	lProgress;

    lProgress.Caption	= "Device Template";
	lProgress.Text		= "Initializing device...";

	return true;
}


/************************************************
 *	Device is put online.
 ************************************************/
bool CDevice_FaceCap::Start()
{
	FBProgress	lProgress;
	lProgress.Caption	= "Starting up device";

	// Step 1: Open device communications
	lProgress.Text	= "Opening device communications";
	Status			= "Opening device communications";

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
		HardwareVersionInfo = "Device Template, v1.0";
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
bool CDevice_FaceCap::Stop()
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
bool CDevice_FaceCap::Done()
{
	return false;
}


/************************************************
 *	Reset of device.
 ************************************************/
bool CDevice_FaceCap::Reset()
{
    Stop();
    return Start();
}


/************************************************
 *	Real-Time Engine Evaluation.
 ************************************************/
bool CDevice_FaceCap::AnimationNodeNotify(FBAnimationNode* pAnimationNode ,FBEvaluateInfo* pEvaluateInfo)
{
	double	lPos[3];
	double	lRot[3];

	const double space_scale = 0.01 * SpaceScale;

	// head position and rotation

	mHardware.GetPosition( lPos );
	mHardware.GetRotation( lRot );

	for (int i = 0; i < 3; ++i)
	{
		lPos[i] *= space_scale;
	}

	mNodeHead_InT->WriteData( lPos, pEvaluateInfo );
	mNodeHead_InR->WriteData( lRot, pEvaluateInfo );

	// left eye
	mHardware.GetLeftEyeRotation(lRot);
	lRot[2] = 0.0;

	mNodeLeftEye_InR->WriteData(lRot, pEvaluateInfo);

	// right eye
	mHardware.GetRightEyeRotation(lRot);
	lRot[2] = 0.0;

	mNodeRightEye_InR->WriteData(lRot, pEvaluateInfo);

	// blendshapes

	for (int i = 0; i < mHardware.GetNumberOfBlendshapes(); ++i)
	{
		double value = ShapeValueMult * mHardware.GetBlendshapeValue(i);
		mNodeHead_Blendshapes[i]->WriteData(&value, pEvaluateInfo);
	}

    return true;
}


/************************************************
 *	Device Evaluation Notify.
 ************************************************/
bool CDevice_FaceCap::DeviceEvaluationNotify( kTransportMode pMode, FBEvaluateInfo* pEvaluateInfo )
{
	return true;
}


/************************************************
 *	Real-Time Synchronous Device IO.
 ************************************************/
void CDevice_FaceCap::DeviceIONotify( kDeviceIOs  pAction,FBDeviceNotifyInfo &pDeviceNotifyInfo)
{
	int		i;
	int		lNumberOfPackets;
	FBTime	lPacketTimeCode;

    switch (pAction)
	{
		// Output devices
		case kIOPlayModeWrite:
		case kIOStopModeWrite:
		{
		}
		break;

		// Input devices
		case kIOStopModeRead:
		case kIOPlayModeRead:
		{
			lNumberOfPackets = mHardware.FetchData();

			for( i=0; i<lNumberOfPackets; i++ )
			{
				DeviceRecordFrame	( pDeviceNotifyInfo );
				AckOneSampleReceived( );
			}

			if( !mHardware.GetStreaming() )
			{
				mHardware.PollData();
			}
		break;
		}
	}
}


/************************************************
 *	Record a frame of the device (recording).
 ************************************************/
void CDevice_FaceCap::DeviceRecordFrame( FBDeviceNotifyInfo &pDeviceNotifyInfo )
{
	double	lPos[3];
	double	lRot[3];

	FBTime	lTime;

	const double space_scale = 0.01 * SpaceScale;

	lTime = pDeviceNotifyInfo.GetLocalTime();

	//

	if( mPlayerControl.GetTransportMode() == kFBTransportPlay )
	{
		mHardware.GetPosition(lPos);
		mHardware.GetRotation(lRot);

		for (int i = 0; i < 3; ++i)
		{
			lPos[i] *= space_scale;
		}

		switch( SamplingMode.AsInt() )
		{
			case kFBHardwareTimestamp:
			case kFBSoftwareTimestamp:
			{
				if (FBAnimationNode* data = mNodeHead_InT->GetAnimationToRecord())
				{
					data->KeyAdd(lTime, lPos);
				}
				if (FBAnimationNode* data = mNodeHead_InR->GetAnimationToRecord())
				{
					data->KeyAdd(lTime, lRot);
				}
				if (FBAnimationNode* data = mNodeLeftEye_InR->GetAnimationToRecord())
				{
					mHardware.GetLeftEyeRotation(lRot);
					lRot[2] = 0.0;
					data->KeyAdd(lTime, lRot);
				}
				if (FBAnimationNode* data = mNodeRightEye_InR->GetAnimationToRecord())
				{
					mHardware.GetRightEyeRotation(lRot);
					lRot[2] = 0.0;
					data->KeyAdd(lTime, lRot);
				}
				for (int i = 0; i < mHardware.GetNumberOfBlendshapes(); ++i)
				{
					if (FBAnimationNode* data = mNodeHead_Blendshapes[i]->GetAnimationToRecord())
					{
						double value = ShapeValueMult * mHardware.GetBlendshapeValue(i);
						data->KeyAdd(lTime, &value);
					}
				}
			}
			break;

			case kFBHardwareFrequency:
			case kFBAutoFrequency:
			{
				if (FBAnimationNode* data = mNodeHead_InT->GetAnimationToRecord())
				{
					data->KeyAdd(lPos);
				}
				if (FBAnimationNode* data = mNodeHead_InR->GetAnimationToRecord())
				{
					data->KeyAdd(lRot);
				}
				if (FBAnimationNode* data = mNodeLeftEye_InR->GetAnimationToRecord())
				{
					mHardware.GetLeftEyeRotation(lRot);
					lRot[2] = 0.0;
					data->KeyAdd(lRot);
				}
				if (FBAnimationNode* data = mNodeRightEye_InR->GetAnimationToRecord())
				{
					mHardware.GetRightEyeRotation(lRot);
					lRot[2] = 0.0;
					data->KeyAdd(lRot);
				}
				for (int i = 0; i < mHardware.GetNumberOfBlendshapes(); ++i)
				{
					if (FBAnimationNode* data = mNodeHead_Blendshapes[i]->GetAnimationToRecord())
					{
						double value = ShapeValueMult * mHardware.GetBlendshapeValue(i);
						data->KeyAdd(&value);
					}
				}
			}
			break;
		}
	}
}

void CDevice_FaceCap::SetCandidates()
{
	double	lPos[3];
	double	lRot[3];

	mHardware.GetPosition( lPos );
	mHardware.GetRotation( lRot );

	const double space_scale = 0.01 * SpaceScale;

	for (int i = 0; i < 3; ++i)
	{
		lPos[i] *= space_scale;
	}

	mNodeHead_InT->SetCandidate( lPos );
	mNodeHead_InR->SetCandidate( lRot );

	// left / right eyes
	mHardware.GetLeftEyeRotation(lRot);
	lRot[2] = 0.0;
	mNodeLeftEye_InR->SetCandidate(lRot);

	mHardware.GetRightEyeRotation(lRot);
	lRot[2] = 0.0;
	mNodeRightEye_InR->SetCandidate(lRot);

	// blendshapes
	for (int i = 0; i < mHardware.GetNumberOfBlendshapes(); ++i)
	{
		double value = ShapeValueMult * mHardware.GetBlendshapeValue(i);
		mNodeHead_Blendshapes[i]->SetCandidate(&value);
	}
}

