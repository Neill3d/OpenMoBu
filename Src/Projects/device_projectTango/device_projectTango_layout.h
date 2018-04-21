#ifndef __ORDEVICE_TEMPLATE_LAYOUT_H__
#define __ORDEVICE_TEMPLATE_LAYOUT_H__

/**	\file	fbsimpleoutput_layout.h

Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

//--- Class declaration
#include "device_projectTango_device.h"

//////////////////////////////////////////////////////////////////////////////////////////
//! Device_ProjectTango_Layout

class Device_ProjectTango_Layout : public FBDeviceLayout
{
	//--- FiLMBOX declaration.
	FBDeviceLayoutDeclare(Device_ProjectTango_Layout, FBDeviceLayout);
public:
	//--- FiLMBOX Creation/Destruction.
	virtual bool FBCreate();		//!< FiLMBOX Constructor.
	virtual void FBDestroy();		//!< FiLMBOX Destructor.

	// UI Management
	void	UICreate				();
	void		UICreateLayout0		();
	void		UICreateLayout1		();
	void	UIConfigure				();
	void		UIConfigureLayout0	();
	void		UIConfigureLayout1	();
	void	UIRefresh				();		// Idle refresh
	void	UIReset					();		// Reset from device values

	// Main Layout: Events
	void	EventDeviceStatusChange					( HISender pSender, HKEvent pEvent );
	void	EventUIIdle								( HISender pSender, HKEvent pEvent );
	void	EventTabPanelChange						( HISender pSender, HKEvent pEvent );

	// Layout 0: Events
	void	EventEditNumberSamplingRateChange		( HISender pSender, HKEvent pEvent );
	void	EventListSamplingTypeChange				( HISender pSender, HKEvent pEvent );
	void	EventButtonSetCandidateClick			( HISender pSender, HKEvent pEvent );
	void	EventButtonSetupClick					( HISender pSender, HKEvent pEvent );

private:
	FBTabPanel			mTabPanel;

	FBLayout			mLayoutGeneral;
		FBLabel				mLabelSamplingRate;
		FBEditNumber		mEditNumberSamplingRate;
		FBLabel				mLabelSamplingType;
		FBList				mListSamplingType;
		FBButton			mButtonSetCandidate;
		FBButton			mButtonSetup;

	FBLayout			mLayoutCommunication;
		
		
		FBEditProperty		mEditNetworkPort;

		FBEditProperty		mEditDeviceAddress;
		FBEditProperty		mEditDevicePort;
		
		FBMemo				mMemoLog;		// log with receiving events from a tango tablet
		FBLabel				mLabelMessage;
		FBEdit				mEditMessage;	// enter a message to be send to a tablet
		FBButton			mButtonSend;

private:
	FBSystem				mSystem;		//!< System interface.
	Device_ProjectTango*	mDevice;		//!< Handle onto device.
};

#endif /* __ORDEVICE_TEMPLATE_HARDWARE_H__ */
