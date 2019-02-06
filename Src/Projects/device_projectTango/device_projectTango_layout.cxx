
/**	\file	device_projectTango_layout.cxx

Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

//--- Class declarations
#include "device_projectTango_device.h"
#include "device_projectTango_layout.h"

//--- Registration define
#define ORDEVICETEMPLATE__LAYOUT	Device_ProjectTango_Layout

//--- FiLMBOX implementation and registration
FBDeviceLayoutImplementation(	ORDEVICETEMPLATE__LAYOUT	);
FBRegisterDeviceLayout		(	ORDEVICETEMPLATE__LAYOUT,
								ORDEVICETEMPLATE__CLASSSTR,
								FB_DEFAULT_SDK_ICON			);	// Icon filename (default=Open Reality icon)

#define DATA_CLASSNAME			"PostPersistentData"
#define DATA_DEFAULT_NAME		"Post Processing"

/************************************************
 *	FiLMBOX constructor.
 ************************************************/
bool Device_ProjectTango_Layout::FBCreate()
{
	// Get a handle on the device.
	mDevice = ((Device_ProjectTango *)(FBDevice *)Device);

	// Create/configure UI
	UICreate	();
	UIConfigure	();
	UIReset		();

	// Add device & system callbacks
	mDevice->OnStatusChange.Add(this, (FBCallback)&Device_ProjectTango_Layout::EventDeviceStatusChange);
	OnIdle.Add(this, (FBCallback)&Device_ProjectTango_Layout::EventUIIdle);

	return true;
}

/************************************************
 *	FiLMBOX destructor.
 ************************************************/
void Device_ProjectTango_Layout::FBDestroy()
{
	// Remove device & system callbacks
	OnIdle.Remove(this, (FBCallback)&Device_ProjectTango_Layout::EventUIIdle);
	mDevice->OnStatusChange.Remove(this, (FBCallback)&Device_ProjectTango_Layout::EventDeviceStatusChange);
}


/************************************************
 *	Create the UI.
 ************************************************/
void Device_ProjectTango_Layout::UICreate()
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
void Device_ProjectTango_Layout::UICreateLayout0()
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
	mLayoutGeneral.AddRegion ( "ButtonTest",	"ButtonTest",
													0,		kFBAttachLeft,		"ButtonSetCandidate",	1.0,
													lS,		kFBAttachBottom,	"ButtonSetCandidate",	1.0,
													0,		kFBAttachWidth,		"ButtonSetCandidate",	1.0,
													0,		kFBAttachHeight,	"ButtonSetCandidate",	1.0 );

	

	// Assign regions
	mLayoutGeneral.SetControl("LabelSamplingRate",		mLabelSamplingRate		);
	mLayoutGeneral.SetControl("EditNumberSamplingRate", mEditNumberSamplingRate	);
	mLayoutGeneral.SetControl("LabelSamplingType",		mLabelSamplingType		);
	mLayoutGeneral.SetControl("ListSamplingType",		mListSamplingType		);
	mLayoutGeneral.SetControl("ButtonSetCandidate",		mButtonSetCandidate		);
	mLayoutGeneral.SetControl("ButtonTest",				mButtonSetup				);

}


/************************************************
 *	Create the communications layout.
 ************************************************/
void Device_ProjectTango_Layout::UICreateLayout1()
{
	int lS		= 4;
	int lSx		= 10;
	int lSy		= 15;

//	int lW		= 90;
	int lH		= 18;
	int lHlr2	= 80;
	int lWlr	= 230;

	// Add regions (network)
	mLayoutCommunication.AddRegion( "LayoutRegionNetwork",	"LayoutRegionNetwork",
													lSx,	kFBAttachLeft,		"",		1.00,
													lSy,	kFBAttachTop,		"",		1.00,
													lWlr,	kFBAttachNone,		"",		1.00,
													lHlr2,	kFBAttachNone,		"",		1.00 );
	
	mLayoutCommunication.AddRegion("EditNetworkPort", "EditNetworkPort",
												lS, kFBAttachLeft, "LayoutRegionNetwork", 1.00,
												lS, kFBAttachTop, "LayoutRegionNetwork", 1.00,
												-lS, kFBAttachRight, "LayoutRegionNetwork", 1.00,
												lH, kFBAttachNone, "", 1.00);

	mLayoutCommunication.AddRegion( "EditDeviceAddress",		"EditDeviceAddress",
													lS,		kFBAttachLeft,		"LayoutRegionNetwork",	1.00,
													lS,		kFBAttachBottom,	"EditNetworkPort",		1.00,
													-lS, kFBAttachRight, "LayoutRegionNetwork", 1.00,
													lH,		kFBAttachNone,		NULL,					1.00 );
	
	mLayoutCommunication.AddRegion( "EditDevicePort",	"EditDevicePort",
													lS,		kFBAttachLeft,		"LayoutRegionNetwork",	1.00,
													lS,		kFBAttachBottom,	"EditDeviceAddress",	1.00,
													-lS, kFBAttachRight, "LayoutRegionNetwork", 1.00,
													lH,		kFBAttachNone,		"",						1.00 );
	
	mLayoutCommunication.AddRegion("Memo", "Memo",
		250 + lS, kFBAttachLeft, "", 1.00,
		lS, kFBAttachTop, "", 1.00,
		-5, kFBAttachRight, "", 1.00,
		-40, kFBAttachBottom, "", 1.00);

	mLayoutCommunication.AddRegion("Msg", "Msg",
		0, kFBAttachLeft, "Memo", 1.00,
		lS, kFBAttachBottom, "Memo", 1.00,
		-65, kFBAttachRight, "", 1.00,
		25, kFBAttachNone, "", 1.00);

	mLayoutCommunication.AddRegion("ButtonSend", "ButtonSend",
		lS, kFBAttachRight, "Msg", 1.00,
		lS, kFBAttachBottom, "Memo", 1.00,
		-lS, kFBAttachRight, "", 1.00,
		25, kFBAttachNone, "", 1.00);

	// Assign regions (network)
	
	mLayoutCommunication.SetControl( "EditNetworkPort",			mEditNetworkPort		);
	mLayoutCommunication.SetControl( "EditDeviceAddress",		mEditDeviceAddress		);
	mLayoutCommunication.SetControl( "EditDevicePort",			mEditDevicePort		);

	mLayoutCommunication.SetControl("Memo", mMemoLog);
	mLayoutCommunication.SetControl("Msg", mEditMessage);
	mLayoutCommunication.SetControl("ButtonSend", mButtonSend);
}


/************************************************
 *	Configure the UI.
 ************************************************/
void Device_ProjectTango_Layout::UIConfigure()
{
	SetBorder ("MainLayout", kFBStandardBorder, false,true, 1, 0,90,0);

	mTabPanel.Items.SetString("General~Communication");
	mTabPanel.OnChange.Add(this, (FBCallback)&Device_ProjectTango_Layout::EventTabPanelChange);

	UIConfigureLayout0();
	UIConfigureLayout1();
}


/************************************************
 *	Configure the user layout.
 ************************************************/
void Device_ProjectTango_Layout::UIConfigureLayout0()
{
	mLabelSamplingRate.Caption = "Sampling Rate :";
	mLabelSamplingType.Caption = "Sampling Type :";

	mListSamplingType.Items.Add( "kFBHardwareTimestamp",	kFBHardwareTimestamp	);
	mListSamplingType.Items.Add( "kFBHardwareFrequency",	kFBHardwareFrequency	);
	mListSamplingType.Items.Add( "kFBAutoFrequency",		kFBAutoFrequency		);
	mListSamplingType.Items.Add( "kFBSoftwareTimestamp",	kFBSoftwareTimestamp	);

	mEditNumberSamplingRate.LargeStep = 0.0;
	mEditNumberSamplingRate.SmallStep = 0.0;
	mEditNumberSamplingRate.OnChange.Add(this, (FBCallback)&Device_ProjectTango_Layout::EventEditNumberSamplingRateChange);
	mListSamplingType.OnChange.Add(this, (FBCallback)&Device_ProjectTango_Layout::EventListSamplingTypeChange);

	mButtonSetCandidate.Style	= kFB2States;
	mButtonSetCandidate.OnClick.Add(this, (FBCallback)&Device_ProjectTango_Layout::EventButtonSetCandidateClick);
	mButtonSetCandidate.Caption = "Set Candidate";

	mButtonSetup.OnClick.Add(this, (FBCallback)&Device_ProjectTango_Layout::EventButtonSetupClick);
	mButtonSetup.Caption = "Setup A Preview";

}


/************************************************
 *	Configure the communications layout.
 ************************************************/
void Device_ProjectTango_Layout::UIConfigureLayout1()
{
    mLayoutCommunication.SetBorder( "LayoutRegionNetwork",	kFBEmbossBorder,false,true,2,1,90.0,0);
    
	mEditNetworkPort.Property = &mDevice->SocketPort;
	mEditNetworkPort.Caption = "Network Port:";

	mEditDeviceAddress.Property = &mDevice->DeviceAddress;
	mEditDeviceAddress.Caption = "Device Address:";

	mEditDevicePort.Property = &mDevice->DevicePort;
	mEditDevicePort.Caption = "Device Port:";

	mButtonSend.Caption = "Send";
}


/************************************************
 *	Refresh the UI.
 ************************************************/
void Device_ProjectTango_Layout::UIRefresh()
{
	// Update real-time values from device (i.e. spreadsheet, etc.)
}


/************************************************
 *	Reset the UI values from the device.
 ************************************************/
void Device_ProjectTango_Layout::UIReset()
{
	char	lBuffer[40];
	sprintf( lBuffer, "%d", mDevice->GetNetworkPort());

	mEditNumberSamplingRate.Value		= 1.0/((FBTime)mDevice->SamplingPeriod).GetSecondDouble();

	mListSamplingType.ItemIndex			= mListSamplingType.Items.Find( mDevice->SamplingMode );
	mButtonSetCandidate.State			= mDevice->GetSetCandidate();
}


/************************************************
 *	Tab panel change callback.
 ************************************************/
void Device_ProjectTango_Layout::EventTabPanelChange(HISender pSender, HKEvent pEvent)
{
	switch( mTabPanel.ItemIndex )
	{
		case 0:	SetControl("MainLayout", mLayoutGeneral			);	break;
		case 1:	SetControl("MainLayout", mLayoutCommunication	);	break;
	}
}

/************************************************
 *	Test button callback.
 ************************************************/
void Device_ProjectTango_Layout::EventButtonSetupClick(HISender pSender, HKEvent pEvent)
{

	FBObject *obj = nullptr;
	
	FBScene *pScene = FBSystem::TheOne().Scene;

	for (int i = 0, count = pScene->UserObjects.GetCount(); i < count; ++i)
	{
		if (0 == strcmp(DATA_CLASSNAME, pScene->UserObjects.GetAt(i)->ClassName()))
		{
			obj = pScene->UserObjects[i];
			break;
		}
	}

	if (nullptr == obj)
	{
		obj = FBCreateObject("FbxStorable/User", DATA_CLASSNAME, DATA_DEFAULT_NAME);
	}
	

	if (nullptr != obj)
	{
		// setup properties to send preview

		FBProperty *prop = nullptr;

		prop = obj->PropertyList.Find("Active");
		if (nullptr != prop)
			prop->SetInt(1);

		prop = obj->PropertyList.Find("Color Correction");
		if (nullptr != prop)
			prop->SetInt(1);

		prop = obj->PropertyList.Find("Output Preview");
		if (nullptr != prop)
			prop->SetInt(1);

		double defScale = 15.0;
		prop = obj->PropertyList.Find("Output Scale Factor");
		if (nullptr != prop)
			prop->SetData(&defScale);
	}

}

/************************************************
 *	Device status change callback.
 ************************************************/
void Device_ProjectTango_Layout::EventDeviceStatusChange(HISender pSender, HKEvent pEvent)
{
	UIReset();
}


/************************************************
 *	UI Idle callback.
 ************************************************/
void Device_ProjectTango_Layout::EventUIIdle(HISender pSender, HKEvent pEvent)
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


void Device_ProjectTango_Layout::EventEditNumberSamplingRateChange(HISender pSender, HKEvent pEvent)
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

void Device_ProjectTango_Layout::EventListSamplingTypeChange(HISender pSender, HKEvent pEvent)
{
	mDevice->SamplingMode = (FBDeviceSamplingMode)mListSamplingType.Items.GetReferenceAt( mListSamplingType.ItemIndex );
	UIReset();
}

void Device_ProjectTango_Layout::EventButtonSetCandidateClick(HISender pSender, HKEvent pEvent)
{
	mDevice->SetSetCandidate( mButtonSetCandidate.State != 0 );
	UIReset();
}
