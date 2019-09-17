#pragma once

/**	\file	device_facecap_layout.h
*	Developed by Sergei <Neill3d> Solokhin 2019
*	e-mail to: s@neill3d.com
*	twitter: @Neill3d
*
* OpenMoBu github - https://github.com/Neill3d/OpenMoBu
*/


//--- Class declaration
#include "device_facecap_device.h"

//! Simple device layout.
class CDevice_FaceCap_Layout : public FBDeviceLayout
{
	//--- FiLMBOX declaration.
	FBDeviceLayoutDeclare(CDevice_FaceCap_Layout, FBDeviceLayout );
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
	void	EventButtonAboutClick(HISender pSender, HKEvent pEvent);

	// Layout 1: Events
	
	void	EventEditNetworkPortChange				( HISender pSender, HKEvent pEvent );
	
	// Layout configuration functions
	void	SelectCommunicationType( int pType );

private:
	FBTabPanel			mTabPanel;

	FBLayout			mLayoutGeneral;
		FBLabel				mLabelSamplingRate;
		FBEditNumber		mEditNumberSamplingRate;
		FBLabel				mLabelSamplingType;
		FBList				mListSamplingType;
		FBButton			mButtonSetCandidate;
		FBButton			mButtonAbout;

	FBLayout			mLayoutCommunication;
		
			FBLabel				mLabelNetworkPort;
			FBEdit				mEditNetworkPort;
			
private:
	FBSystem			mSystem;		//!< System interface.
	CDevice_FaceCap*	mDevice;		//!< Handle onto device.
};

