
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: stylus.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


struct StylusData
{
	enum StylusType
	{
		kStylusType_None,
		kStylusType_Pen,
		kStylusType_Eraser
	};

	//
	bool			wintab;		// are we connected to any tablet

	StylusType		type;

	// final calculated value in a range[0; 1]
	float			pressure;

	//! a constructor
	StylusData();
};


void GetStylusData( StylusData &data );

//

bool TabletPumpMessages();
bool TabletInit();
void TabletCleanup();