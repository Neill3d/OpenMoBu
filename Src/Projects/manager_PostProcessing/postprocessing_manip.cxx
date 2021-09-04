
/**	\file	ormanip_template_manip.cxx

Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declarations
#include "postprocessing_manip.h"
#include <fbsdk/fbsdk-opengl.h>

//--- Registration defines
#define ORMANIPTEMPLATE__CLASS	ORMANIPTEMPLATE__CLASSNAME
#define ORMANIPTEMPLATE__LABEL	"OR - Manip Template"
#define ORMANIPTEMPLATE__DESC	"OR - Manipulator Template Description"

//--- FiLMBOX implementation and registration
FBManipulatorImplementation	(	ORMANIPTEMPLATE__CLASS		);
FBRegisterManipulator		(	ORMANIPTEMPLATE__CLASS,
								ORMANIPTEMPLATE__LABEL,
								ORMANIPTEMPLATE__DESC,
								FB_DEFAULT_SDK_ICON			);	// Icon filename (default=Open Reality icon)


extern bool GRenderAfterRender();

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool ORManip_Template::FBCreate()
{
	if( FBManipulator::FBCreate() )
	{
		// Properties
		Active				= true;
		AlwaysActive		= true;
		DefaultBehavior		= true;
		ViewerText			= "Post Processing Manipulator";

		return true;
	}
	return false;
}


/************************************************
 *	FiLMBOX Destructor.
 ************************************************/
void ORManip_Template::FBDestroy()
{
	FBManipulator::FBDestroy();
}


/************************************************
 *	Draw function for manipulator
 ************************************************/
void ORManip_Template::ViewExpose()
{
	glPushAttrib(GL_VIEWPORT_BIT | GL_TRANSFORM_BIT);

	GRenderAfterRender();

	glPopAttrib();
}


/************************************************
 *	Deal with maniplator input.
 ************************************************/
bool ORManip_Template::ViewInput(int pMouseX, int pMouseY, FBInputType pAction, int pButtonKey, int pModifier)
{
	switch( pAction )
	{
		case kFBKeyPress:
		{
			// When a keyboard key is pressed.
		}
		break;
		case kFBKeyRelease:
		{
			// When a keyboard key is released.
		}
		break;
		case kFBButtonDoubleClick:
		{
			// Mouse button double-clicked.
		}
		break;
		case kFBButtonPress:
		{
			// Mouse button clicked.
		}
		break;
		case kFBButtonRelease:
		{
			// Mouse button released.
		}
		break;
		case kFBMotionNotify:
		{
			// When there is mouse movement in the viewer window
		}
		break;
		case kFBDragging:
		{
			// Items are being dragged.
		}
		break;
		case kFBDropping:
		{
			// Items are being dropped.
		}
		break;
	}
	return true;
}
