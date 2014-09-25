
/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/MoBu/blob/master/LICENSE
//
// GitHub repository - https://github.com/Neill3d/MoBu
//
// Author Sergey Solohin (Neill3d) 2014
//  e-mail to: s@neill3d.com
//		www.neill3d.com
/////////////////////////////////////////////////////////////////////////////////////////


//--- Class declarations
#include "manip_skelet_manip.h"
#include <fbsdk\fbundomanager.h>
#include <fbsdk\fbsdk-opengl.h>

//--- Registration defines
#define MANIPSKELET__CLASS	MANIPSKELET__CLASSNAME
#define MANIPSKELET__LABEL	"Skeleton Manipulator (Neill3d)"
#define MANIPSKELET__DESC		"Skeleton Manipulator (Neill3d)"

//--- FiLMBOX implementation and registration
FBManipulatorImplementation	(	MANIPSKELET__CLASS		);
FBRegisterManipulator		(	MANIPSKELET__CLASS,
								MANIPSKELET__LABEL,
								MANIPSKELET__DESC,
								FB_DEFAULT_SDK_ICON			);	// Icon filename (default=Open Reality icon)
/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool Manip_Skelet::FBCreate()
{
	if( FBManipulator::FBCreate() )
	{
		// Properties
		AlwaysActive		= true;
		DefaultBehavior		= true;
		ViewerText			= "Skeleton Manipulator (Neill3d)";

		// Members
		pivotModel = NULL;
		pivotMode = false;

		return true;
	}
	return false;
}


/************************************************
 *	FiLMBOX Destructor.
 ************************************************/
void Manip_Skelet::FBDestroy()
{
	FBManipulator::FBDestroy();
}


/************************************************
 *	Draw function for manipulator
 ************************************************/
void Manip_Skelet::ViewExpose()
{
	if( pivotMode )
	{
		//--- Transformation for camera viewpoint
		glMatrixMode( GL_MODELVIEW );
		glPushMatrix();
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);

			glColor4f( 1.0, 1.0, 0.0, 0.5 );
			glBegin(GL_POLYGON);
				glVertex2f( 10.0, 10.0 );
				glVertex2f( 20.0, 10.0 );
				glVertex2f( 20.0, 20.0 );
				glVertex2f( 10.0, 20.0 );
			glEnd();

			glDisable(GL_BLEND);
		}
		glPopMatrix();
	}
}


/************************************************
 *	Deal with maniplator input.
 ************************************************/

bool Manip_Skelet::ViewInput(int pMouseX, int pMouseY, FBInputType pAction, int pButtonKey, int pModifier)
{
	bool manipTrans = false;
	FBString name;
	for (int i=0; i<mSystem.Manipulators.GetCount(); i++)
	{
		name = mSystem.Manipulators[i]->Name;
		if (name == "Transformation")
		{
			manipTrans = mSystem.Manipulators[i]->Active;
			break;
		}
	}

	switch( pAction )
	{
		case kFBKeyPress:
		{
			// When a keyboard key is pressed.
			pivotMode = false;
			if (manipTrans)
				if (pButtonKey == kFBKeyTab)
				{
					pivotMode = true;

					// hold child tms
					FBModelList		list;
					FBGetSelectedModels(list);
					if (list.GetCount() )
					{
						pivotModel = list[0];
						int count = pivotModel->Children.GetCount();

						objsTM.SetCount(count);
						for (int i=0; i<count; i++)
							pivotModel->Children[i]->GetMatrix( objsTM[i] );

						// add models to undo manager
						if (FBUndoManager().TransactionIsOpen() )
						{
							for (int i=0; i<count; i++)
								FBUndoManager().TransactionAddModelTRS(pivotModel->Children[i]);
						}
						else
						{
							FBUndoManager().TransactionBegin("Skeleton drag");

							for (int i=0; i<count; i++)
								FBUndoManager().TransactionAddModelTRS(pivotModel->Children[i]);

							FBUndoManager().TransactionEnd();
						}
					}
				}
		}
		break;
		case kFBKeyRelease:
		case kFBKeyReleaseRaw:
		{
			// When a keyboard key is released.
			if (pButtonKey == kFBKeyTab)
				pivotMode = false;
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
			// fetch object children TM
			if (manipTrans && pivotMode)
			{
				if (pivotModel)
				{
					int count = pivotModel->Children.GetCount();
					for (int i=0; i<count; i++)
							pivotModel->Children[i]->SetMatrix( objsTM[i] );
				}
			}
		}
		break;
		case kFBMotionNotify:
		{
			// When there is mouse movement in the viewer window
			if (manipTrans && pivotMode)
			{
				if (pivotModel)
				{
					int count = pivotModel->Children.GetCount();
					for (int i=0; i<count; i++)
							pivotModel->Children[i]->SetMatrix( objsTM[i] );
				}
			}
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
