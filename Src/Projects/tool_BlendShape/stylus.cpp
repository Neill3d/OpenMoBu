
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: stylus.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include <Windows.h>

#include "msgpack.h"
#include "wintab.h"
#define PACKETDATA	(PK_BUTTONS | PK_NORMAL_PRESSURE | PK_ORIENTATION | PK_CURSOR)
#define PACKETMODE	PK_BUTTONS
#include "pktdef.h"
#include "Utils.h"

#include "stylus.h"


const char* gpszProgramName = "PressureTest";

bool m_wintab = false;
/** Tablet data for GHOST */
//GHOST_TabletData* m_tabletData;

/** Stores the Tablet context if detected Tablet features using WinTab.dll */
HCTX m_tablet = nullptr;
LONG m_maxPressure = 0;
LONG m_maxAzimuth, m_maxAltitude;

StylusData		gStylusData;


StylusData::StylusData()
	: wintab(false)
	, type(kStylusType_None)
	, pressure(0.0f)
{}

void GetStylusData( StylusData &data )
{
	data.pressure = gStylusData.pressure;
	data.type = gStylusData.type;
	data.wintab = gStylusData.wintab;
}

bool TabletPumpMessages()
{
	if (m_tablet)
	{
		StylusData::StylusType	type = StylusData::kStylusType_None;
		float pressure = 0.0f;

		PACKET pkt_buffer[128];
		int n = gpWTPacketsGet( m_tablet, 128, pkt_buffer);

		if (n == 0)
		{
			gpWTEnable(m_tablet, TRUE);
			gpWTOverlap(m_tablet, TRUE);
		}

		for(int i=0; i<n; ++i)
		{
			PACKET &pkt = pkt_buffer[i];

			switch(pkt.pkCursor) {
			case 0: // first device
			case 3: // second device
				type = StylusData::kStylusType_None;
				break;
			case 1:
			case 4:
				// stylus
				type = StylusData::kStylusType_Pen;
				break;
			case 2:
			case 5:
				// eraser
				type = StylusData::kStylusType_Eraser;
				break;
			}

			if (m_maxPressure > 0) {
				pressure = (float) pkt.pkNormalPressure / (float) m_maxPressure;
			}
			else
			{
				pressure = 1.0f;
			}
		}

		gStylusData.pressure = pressure;
		gStylusData.type = type;
		gStylusData.wintab = true;

		if (n > 0)
			return true;
	}

	gStylusData.wintab = false;

	return false;
}



char windowTitle[128];

HWND FindInParent(HWND wnd)
{
	if (wnd == nullptr)
		return nullptr;

	GetWindowText( wnd, windowTitle, 128 );

	if (strstr( windowTitle, "MotionBuilder" ) != nullptr )
	{
		return wnd;
	}

	return GetParent(wnd);
}

HWND GetMoBuHWND()
{
	HWND wnd = GetActiveWindow();
	return FindInParent(wnd);
}

bool TabletInit()
{

	gStylusData.wintab = false;

	m_wintab = (TRUE == LoadWintab());
	if (m_wintab) {
		// let's see if we can initialize tablet here
		/* check if WinTab available. */
		if (gpWTInfoA(0, 0, NULL)) {
			// Now init the tablet
			LOGCONTEXT lc;
			AXIS TabletX, TabletY, Pressure, Orientation[3]; /* The maximum tablet size, pressure and orientation (tilt) */

			// Open a Wintab context

			// Get default context information
			gpWTInfoA( WTI_DEFCONTEXT, 0, &lc );

			// Open the context
			lc.lcPktData = PACKETDATA;
			lc.lcPktMode = PACKETMODE;
			lc.lcOptions |= /* CXO_MESSAGES | */ CXO_SYSTEM;
			lc.lcOptions &= ~CXO_MESSAGES;

			/* Set the entire tablet as active */
			gpWTInfoA(WTI_DEVICES,DVC_X,&TabletX);
			gpWTInfoA(WTI_DEVICES,DVC_Y,&TabletY);

			/* get the max pressure, to divide into a float */
			BOOL pressureSupport = gpWTInfoA(WTI_DEVICES, DVC_NPRESSURE, &Pressure);
			if (pressureSupport)
				m_maxPressure = Pressure.axMax;
			else
				m_maxPressure = 0;

			/* get the max tilt axes, to divide into floats */
			BOOL tiltSupport = gpWTInfoA(WTI_DEVICES, DVC_ORIENTATION, &Orientation);
			if (tiltSupport) {
				/* does the tablet support azimuth ([0]) and altitude ([1]) */
				if (Orientation[0].axResolution && Orientation[1].axResolution) {
					/* all this assumes the minimum is 0 */
					m_maxAzimuth = Orientation[0].axMax;
					m_maxAltitude = Orientation[1].axMax;
				}
				else {  /* no so dont do tilt stuff */
					m_maxAzimuth = m_maxAltitude = 0;
				}
			}

			HWND wnd = GetMoBuHWND();
			m_tablet = gpWTOpenA( wnd, &lc, TRUE );
			if (m_tablet) {
				//m_tabletData = new GHOST_TabletData();
				//m_tabletData->Active = GHOST_kTabletModeNone;

			   // request a deep queue, to capture every pen point
			   int tabletQueueSize = 128;
			   while (!gpWTQueueSizeSet(m_tablet, tabletQueueSize))
					 --tabletQueueSize;
			   printf("tablet queue size: %d\n", tabletQueueSize);

			   gStylusData.wintab = true;
			   return true;
			}
		}
	}

	return false;
}

void TabletCleanup()
{
	gStylusData.wintab = false;
	UnloadWintab();
}