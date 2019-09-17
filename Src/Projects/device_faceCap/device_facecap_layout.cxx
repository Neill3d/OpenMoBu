
/**	\file	device_facecap_layout.cxx
*	Developed by Sergei <Neill3d> Solokhin 2019
*	e-mail to: s@neill3d.com
*	twitter: @Neill3d
*
* OpenMoBu github - https://github.com/Neill3d/OpenMoBu
*/


//--- Class declarations
#include "device_facecap_device.h"
#include "device_facecap_layout.h"

//--- Registration define
#define CDEVICEFACECAP__LAYOUT	CDevice_FaceCap_Layout

//--- FiLMBOX implementation and registration
FBDeviceLayoutImplementation(	CDEVICEFACECAP__LAYOUT	);
FBRegisterDeviceLayout		(	CDEVICEFACECAP__LAYOUT,
								CDEVICEFACECAP__CLASSSTR,
								"character_actor.png"			);	// Icon filename (default=Open Reality icon)

/************************************************
 *	FiLMBOX constructor.
 ************************************************/
bool CDevice_FaceCap_Layout::FBCreate()
{
	// Get a handle on the device.
	mDevice = ((CDevice_FaceCap *)(FBDevice *)Device);

	// Create/configure UI
	UICreate	();
	UIConfigure	();
	UIReset		();

	// Add device & system callbacks
	mDevice->OnStatusChange.Add	( this,(FBCallback)&CDevice_FaceCap_Layout::EventDeviceStatusChange		);
	OnIdle.Add					( this,(FBCallback)&CDevice_FaceCap_Layout::EventUIIdle					);

	return true;
}

/************************************************
 *	FiLMBOX destructor.
 ************************************************/
void CDevice_FaceCap_Layout::FBDestroy()
{
	// Remove device & system callbacks
	OnIdle.Remove					( this,(FBCallback)&CDevice_FaceCap_Layout::EventUIIdle				);
	mDevice->OnStatusChange.Remove	( this,(FBCallback)&CDevice_FaceCap_Layout::EventDeviceStatusChange	);
}


/************************************************
 *	Create the UI.
 ************************************************/
void CDevice_FaceCap_Layout::UICreate()
{
	int lS, lH;		// space, height
	lS = 4;
	lH = 25;

	// Create regions
	AddRegion	( "TabPanel",	"TabPanel",		0,		kFBAttachLeft,		"",			1.00,
												0,		kFBAttachTop,		"",			1.00,
												0,		kFBAttachRight,		"",			1.00,
												lH,		kFBAttachNone,		NULL,		1.00 );
	AddRegion	( "MainLayout",	"MainLayout",	lS,		kFBAttachLeft,		"TabPanel",	1.00,
												lS,		kFBAttachBottom,	"TabPanel",	1.00,
												-lS,	kFBAttachRight,		"TabPanel",	1.00,
												-lS,	kFBAttachBottom,	"",			1.00 );

	// Assign regions
	SetControl	( "TabPanel",	mTabPanel		);
	SetControl	( "MainLayout",	mLayoutGeneral	);

	// Create sub layouts
	UICreateLayout0();
	UICreateLayout1();
}


/************************************************
 *	Create User layout.
 ************************************************/
void CDevice_FaceCap_Layout::UICreateLayout0()
{
	int lS, lW, lH;		// space, width, height.
	lS = 4;
	lW = 150;
	lH = 18;

	// Add regions
	mLayoutGeneral.AddRegion ( "LabelSamplingRate",	"LabelSamplingRate",
													lS,		kFBAttachLeft,		"",		1.00,
													lS,		kFBAttachTop,		"",		1.00,
													100,	kFBAttachNone,		NULL,	1.00,
													lH,		kFBAttachNone,		NULL,	1.00 );
	mLayoutGeneral.AddRegion ( "EditNumberSamplingRate",	"EditNumberSamplingRate",
													lS,		kFBAttachRight,		"LabelSamplingRate",	1.0,
													0,		kFBAttachTop,		"LabelSamplingRate",	1.0,
													100,	kFBAttachNone,		NULL,					1.0,
													0,		kFBAttachHeight,	"LabelSamplingRate",	1.0 );
	mLayoutGeneral.AddRegion ( "LabelSamplingType",	"LabelSamplingType",
													0,		kFBAttachLeft,		"LabelSamplingRate",	1.0,
													lS,		kFBAttachBottom,	"LabelSamplingRate",	1.0,
													0,		kFBAttachWidth,		"LabelSamplingRate",	1.0,
													0,		kFBAttachHeight,	"LabelSamplingRate",	1.0 );
	mLayoutGeneral.AddRegion ( "ListSamplingType",	"ListSamplingType",
													lS,		kFBAttachRight,		"LabelSamplingType",	1.0,
													0,		kFBAttachTop,		"LabelSamplingType",	1.0,
													150,	kFBAttachNone,		NULL,					1.0,
													0,		kFBAttachHeight,	"LabelSamplingType",	1.0 );
	mLayoutGeneral.AddRegion ( "ButtonSetCandidate",	"ButtonSetCandidate",
													0,		kFBAttachLeft,		"ListSamplingType",	1.0,
													lS,		kFBAttachBottom,	"ListSamplingType",	1.0,
													100,	kFBAttachNone,		"ListSamplingType",	1.0,
													0,		kFBAttachHeight,	"ListSamplingType",	1.0 );
	mLayoutGeneral.AddRegion("ButtonAbout", "ButtonAbout",
		0, kFBAttachLeft, "ListSamplingType", 1.0,
		lS, kFBAttachBottom, "ButtonSetCandidate", 1.0,
		100, kFBAttachNone, "", 1.0,
		0, kFBAttachHeight, "ListSamplingType", 1.0);

	// Assign regions
	mLayoutGeneral.SetControl("LabelSamplingRate",		mLabelSamplingRate		);
	mLayoutGeneral.SetControl("EditNumberSamplingRate", mEditNumberSamplingRate	);
	mLayoutGeneral.SetControl("LabelSamplingType",		mLabelSamplingType		);
	mLayoutGeneral.SetControl("ListSamplingType",		mListSamplingType		);
	mLayoutGeneral.SetControl("ButtonSetCandidate",		mButtonSetCandidate		);
	mLayoutGeneral.SetControl("ButtonAbout", mButtonAbout);
}


/************************************************
 *	Create the communications layout.
 ************************************************/
void CDevice_FaceCap_Layout::UICreateLayout1()
{
	int lS		= 4;
	int lSx		= 10;
	int lSy		= 15;

	int lW		= 90;
	int lH		= 18;
	int lHlr	= 55;
	int lWlr	= 200;

	int lSlbx	= 15;
	int lSlby	= 10;
	int lWlb	= 80;


	// Add regions (network)
	mLayoutCommunication.AddRegion( "LayoutRegionNetwork",	"LayoutRegionNetwork",
													lSx,	kFBAttachLeft,		"",						1.00,
													lSy,	kFBAttachTop,		"",						1.00,
													lWlr,	kFBAttachNone,		NULL,					1.00,
													lHlr,	kFBAttachNone,		NULL,					1.00 );
	mLayoutCommunication.AddRegion( "LabelNetworkPort",		"LabelNetworkPort",
													lSlbx,	kFBAttachLeft,		"LayoutRegionNetwork",	1.00,
													lSlby,	kFBAttachTop,		"LayoutRegionNetwork",	1.00,
													lWlb,	kFBAttachNone,		NULL,					1.00,
													lH,		kFBAttachNone,		NULL,					1.00);
	mLayoutCommunication.AddRegion( "EditNetworkPort",		"EditNetworkPort",
													lS,		kFBAttachRight,		"LabelNetworkPort",	1.00,
													0,		kFBAttachTop,		"LabelNetworkPort",	1.00,
													lW,		kFBAttachNone,		NULL,					1.00,
													lH,		kFBAttachNone,		NULL,					1.00 );
	

	// Assign regions (network)
	mLayoutCommunication.SetControl( "LabelNetworkPort",		mLabelNetworkPort		);
	mLayoutCommunication.SetControl( "EditNetworkPort",			mEditNetworkPort		);

}


/************************************************
 *	Configure the UI.
 ************************************************/
void CDevice_FaceCap_Layout::UIConfigure()
{
	SetBorder ("MainLayout", kFBStandardBorder, false,true, 1, 0,90,0);

	mTabPanel.Items.SetString("General~Communication");
	mTabPanel.OnChange.Add( this, (FBCallback) &CDevice_FaceCap_Layout::EventTabPanelChange );

	UIConfigureLayout0();
	UIConfigureLayout1();
}


/************************************************
 *	Configure the user layout.
 ************************************************/
void CDevice_FaceCap_Layout::UIConfigureLayout0()
{
	mLabelSamplingRate.Caption = "Sampling Rate :";
	mLabelSamplingType.Caption = "Sampling Type :";

	mListSamplingType.Items.Add( "kFBHardwareTimestamp",	kFBHardwareTimestamp	);
	mListSamplingType.Items.Add( "kFBHardwareFrequency",	kFBHardwareFrequency	);
	mListSamplingType.Items.Add( "kFBAutoFrequency",		kFBAutoFrequency		);
	mListSamplingType.Items.Add( "kFBSoftwareTimestamp",	kFBSoftwareTimestamp	);

	mEditNumberSamplingRate.LargeStep = 0.0;
	mEditNumberSamplingRate.SmallStep = 0.0;
	mEditNumberSamplingRate.OnChange.Add( this, (FBCallback)&CDevice_FaceCap_Layout::EventEditNumberSamplingRateChange );
	mListSamplingType.OnChange.Add( this, (FBCallback)&CDevice_FaceCap_Layout::EventListSamplingTypeChange );

	mButtonSetCandidate.Style	= kFB2States;
	mButtonSetCandidate.OnClick.Add(this, (FBCallback)&CDevice_FaceCap_Layout::EventButtonSetCandidateClick );
	mButtonSetCandidate.Caption = "Set Candidate";

	mButtonAbout.OnClick.Add(this, (FBCallback)&CDevice_FaceCap_Layout::EventButtonAboutClick);
	mButtonAbout.Caption = "About";
}


/************************************************
 *	Configure the communications layout.
 ************************************************/
void CDevice_FaceCap_Layout::UIConfigureLayout1()
{
 
    mLayoutCommunication.SetBorder( "LayoutRegionNetwork",	kFBEmbossBorder,false,true,2,1,90.0,0);
  
	
	mLabelNetworkPort.Caption		= "Port :";
	mEditNetworkPort.Text			= "9000";
	mEditNetworkPort.OnChange.Add( this, (FBCallback) &CDevice_FaceCap_Layout::EventEditNetworkPortChange );

	SelectCommunicationType( kFBCommTypeNetworkUDP );
}


/************************************************
 *	Refresh the UI.
 ************************************************/
void CDevice_FaceCap_Layout::UIRefresh()
{
	// Update real-time values from device (i.e. spreadsheet, etc.)
}


/************************************************
 *	Reset the UI values from the device.
 ************************************************/
void CDevice_FaceCap_Layout::UIReset()
{
	char	lBuffer[40];
	sprintf( lBuffer, "%d", mDevice->GetNetworkPort());

	mEditNetworkPort.Text				= lBuffer;
	
	mEditNumberSamplingRate.Value		= 1.0/((FBTime)mDevice->SamplingPeriod).GetSecondDouble();

	mListSamplingType.ItemIndex			= mListSamplingType.Items.Find( mDevice->SamplingMode );
	mButtonSetCandidate.State			= mDevice->GetSetCandidate();
}


/************************************************
 *	Tab panel change callback.
 ************************************************/
void CDevice_FaceCap_Layout::EventTabPanelChange( HISender pSender, HKEvent pEvent )
{
	switch( mTabPanel.ItemIndex )
	{
		case 0:	SetControl("MainLayout", mLayoutGeneral			);	break;
		case 1:	SetControl("MainLayout", mLayoutCommunication	);	break;
	}
}

/************************************************
 *	Network port change callback.
 ************************************************/
void CDevice_FaceCap_Layout::EventEditNetworkPortChange( HISender pSender, HKEvent pEvent )
{
	int		lPort;
	char	lBuffer[40];

	sscanf(mEditNetworkPort.Text.AsString(), "%d", &lPort);
	mDevice->SetNetworkPort(lPort);
	lPort = mDevice->GetNetworkPort();
	sprintf(lBuffer, "%d", lPort );
	mEditNetworkPort.Text = lBuffer;
}


/************************************************
 *	Device status change callback.
 ************************************************/
void CDevice_FaceCap_Layout::EventDeviceStatusChange( HISender pSender, HKEvent pEvent )
{
	UIReset();
}


/************************************************
 *	UI Idle callback.
 ************************************************/
void CDevice_FaceCap_Layout::EventUIIdle( HISender pSender, HKEvent pEvent )
{
	if( mDevice->Online && mDevice->Live )
	{
		if( mDevice->GetSetCandidate() )
		{
			mDevice->SetCandidates();
		}
		UIRefresh();
	}
}


/************************************************
 *	Select the communication type.
 ************************************************/
void CDevice_FaceCap_Layout::SelectCommunicationType( int pType )
{
	bool	lNetworkTrue	= true;

	mLabelNetworkPort.Enabled				= lNetworkTrue;
	mEditNetworkPort.Enabled				= lNetworkTrue;
}

void CDevice_FaceCap_Layout::EventEditNumberSamplingRateChange( HISender pSender, HKEvent pEvent )
{
	bool lOnline = mDevice->Online;
	double lVal = mEditNumberSamplingRate.Value;

	if( lVal > 0.0 )
	{
		if( lOnline )
		{
			mDevice->DeviceSendCommand( FBDevice::kOpStop );
		}

		FBTime lTime;
		lTime.SetSecondDouble( 1.0 / lVal );
		mDevice->SamplingPeriod = lTime;

		if( lOnline )
		{
			mDevice->DeviceSendCommand( FBDevice::kOpStart );
		}

		UIReset();
	}
}

void CDevice_FaceCap_Layout::EventListSamplingTypeChange(  HISender pSender, HKEvent pEvent )
{
	mDevice->SamplingMode = (FBDeviceSamplingMode)mListSamplingType.Items.GetReferenceAt( mListSamplingType.ItemIndex );
	UIReset();
}

void CDevice_FaceCap_Layout::EventButtonSetCandidateClick( HISender pSender, HKEvent pEvent )
{
	mDevice->SetSetCandidate( mButtonSetCandidate.State != 0 );
	UIReset();
}

void CDevice_FaceCap_Layout::EventButtonAboutClick(HISender pSender, HKEvent pEvent)
{
	FBMessageBox("FaceCap OSC Device Plugin", "Developed by Sergei <Neill3d> Solokhin 2019\n E-mail to: s@neill3d.com\n twitter: @Neill3d", "Ok");
}