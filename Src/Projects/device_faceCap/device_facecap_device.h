
#pragma once

/**	\file	device_facecap_device.h
*	Developed by Sergei <Neill3d> Solokhin 2019
*	e-mail to: neill3d@gmail.com
*	twitter: @Neill3d
*
* OpenMoBu github - https://github.com/Neill3d/OpenMoBu
*/

//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Class declaration
#include "device_facecap_hardware.h"

//--- Registration defines
#define CDEVICEFACECAP__CLASSNAME		CDevice_FaceCap
#define CDEVICEFACECAP__CLASSSTR		"CDevice_FaceCap"

//! Device Template.
class CDevice_FaceCap : public FBDevice
{
	//--- FiLMBOX declaration
	FBDeviceDeclare(CDevice_FaceCap, FBDevice );
public:
	//--- FiLMBOX Construction/Destruction
	virtual bool FBCreate() override;		//!< FiLMBOX constructor.
	virtual void FBDestroy() override;		//!< FiLMBOX destructor.

	//--- The following will be called by the real-time engine.
	virtual bool AnimationNodeNotify	(	FBAnimationNode* pAnimationNode,	FBEvaluateInfo* pEvaluateInfo			);	//!< Real-time evaluation for node.
	virtual void DeviceIONotify			(	kDeviceIOs  pAction,				FBDeviceNotifyInfo &pDeviceNotifyInfo	);	//!< Notification of/for Device IO.
    virtual bool DeviceEvaluationNotify	(	kTransportMode pMode,				FBEvaluateInfo* pEvaluateInfo			);	//!< Evaluation the device (write to hardware).
	virtual bool DeviceOperation		(	kDeviceOperations pOperation												);	//!< Operate device.

	//--- Initialisation/Shutdown
	bool		Init();			//!< Initialize/create device.
	bool		Start();		//!< Start device (online).
	bool		Reset();		//!< Reset device.
	bool		Stop();			//!< Stop device (offline).
	bool		Done();			//!< Remove device.

	//--- Recording
	void		DeviceRecordFrame( FBDeviceNotifyInfo &pDeviceNotifyInfo );

	//--- Aggregation of hardware parameters
	
	void		SetNetworkPort		(int pPort)				{ mHardware.SetNetworkPort(pPort);				}
	int			GetNetworkPort		()						{ return mHardware.GetNetworkPort();			}
	void		SetStreaming		(bool pStreaming)		{ mHardware.SetStreaming( pStreaming );			}
	bool		GetStreaming		()						{ return mHardware.GetStreaming();				}

	//--- Aggregation of simulator parameters
	
	bool		GetSetCandidate		()						{ return mSetCandidate;							}
	void		SetSetCandidate		( bool pState )			{ mSetCandidate = pState;						}
	double		GetSamplingRate		()						{ return mSamplingRate;							}
	void		SetSamplingRate		( double pRate )		{ mSamplingRate = pRate;						}
	FBDeviceSamplingMode	GetSamplingType()								{ return mSamplingType;			}
	void					SetSamplingType( FBDeviceSamplingMode pType )	{ mSamplingType = pType;		}

	void		SetCandidates		();

public:

	FBPropertyDouble			SpaceScale;
	FBPropertyDouble			ShapeValueMult;

public:
	FBModelTemplate*				mTemplateRoot;			//!< Root model template.
	FBModelTemplate*				mTemplateHead;			//!< Head model template.
	FBModelTemplate*				mTemplateLeftEye;			//!< Left Eye model template.
	FBModelTemplate*				mTemplateRightEye;			//!< Right Eye model template.
	
	FBAnimationNode*				mNodeHead_InT;		//!< Head input animation node (translation).
	FBAnimationNode*				mNodeHead_InR;		//!< Head input animation node (rotation).

	FBAnimationNode*				mNodeHead_Blendshapes[static_cast<uint32_t>(EHardwareBlendshapes::count)];

	FBAnimationNode*				mNodeLeftEye_InR;		//!< Left Eye input animation node (rotation).
	FBAnimationNode*				mNodeRightEye_InR;		//!< Right Eye input animation node (rotation).

private:
	bool							mSetCandidate;			//!< Are we setting the candidate or writing to the connector?
	double							mSamplingRate;			//!< Device sampling rate.
	FBDeviceSamplingMode			mSamplingType;			//!< Device sampling type.
	CDevice_FaceCap_Hardware		mHardware;				//!< Hardware member.
	FBPlayerControl					mPlayerControl;			//!< In order to query the play state for recording.
};

