
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
#include "manip_hudext_manip.h"
#include <fbsdk/fbsdk-opengl.h>
//#include <python-2.6.4/include/python.h>

//--- Registration defines
#define ORMANIPTEMPLATE__CLASS	ORMANIPTEMPLATE__CLASSNAME
#define ORMANIPTEMPLATE__LABEL	"HUD extension"
#define ORMANIPTEMPLATE__DESC	"Heads up display extension"

//--- FiLMBOX implementation and registration
FBManipulatorImplementation	(	ORMANIPTEMPLATE__CLASS		);
FBRegisterManipulator		(	ORMANIPTEMPLATE__CLASS,
								ORMANIPTEMPLATE__LABEL,
								ORMANIPTEMPLATE__DESC,
								FB_DEFAULT_SDK_ICON			);	// Icon filename (default=Open Reality icon)
/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool Manip_HUDext::FBCreate()
{
	if( FBManipulator::FBCreate() )
	{
		// Properties
		DefaultBehavior		= true;
		ViewerText			= "Heads Up Display Extension";

		// Members
		mTestState			= 0;
		mModel				= NULL;
		mRenderInModelSpace = true;
		mDrawSquare			= true;

		mDown				= false;
		mProperty			= nullptr;
		mLastX				= 0;
		mLastY				= 0;

		mScriptExecute		= false;
		mScriptPath			= "";
		FBSystem().OnUIIdle.Add( this, (FBCallback) &Manip_HUDext::EventIdle );

		mLastElement		= nullptr;

		return true;
	}
	return false;
}


/************************************************
 *	FiLMBOX Destructor.
 ************************************************/
void Manip_HUDext::FBDestroy()
{
	FBManipulator::FBDestroy();
}

/************************************************
 *	Event UI idle callback
 ************************************************/
void Manip_HUDext::EventIdle( HISender pSender, HKEvent pEvent )
{
	
	if (mScriptExecute) {
		mScriptExecute = false;
		FBApplication().ExecuteScript(mScriptPath);
		mScriptPath = "";
	}

	// for rect manipulator, lets update slider position
	if (!FBPlayerControl().IsPlaying && !FBPlayerControl().IsPlotting && !FBPlayerControl().IsRecording)
		UpdateRectElements();
}

/************************************************
 *	Draw function for manipulator
 ************************************************/
void Manip_HUDext::ViewExpose()
{
}


/************************************************
 *	Deal with maniplator input.
 ************************************************/

void Manip_HUDext::UpdateRectElements()
{
	FBScene *pScene = FBSystem().Scene;

	for (int i=0; i<pScene->HUDs.GetCount(); ++i)
	{
		FBHUD *pHUD = (FBHUD*) pScene->HUDs.GetAt(i);
				
		FBCamera *pCamera = nullptr;
		FBProperty *prop = pHUD->PropertyList.Find("CameraName");
		if (prop) {
			FBString name = prop->AsString();
			for (int k=0; k<pScene->Cameras.GetCount(); ++k)
				if (name == pScene->Cameras.GetAt(k)->Name)
				{
					pCamera = (FBCamera*) pScene->Cameras.GetAt(k);
					break;
				}
		}

		if (!pCamera) continue; // skip processing without assigned camera

		for (int j=0; j<pHUD->Elements.GetCount(); ++j)
		{
			FBHUDElement *elem = (FBHUDElement*) pHUD->Elements.GetAt(j);

			if (elem->Is( FBHUDRectElement::TypeInfo ) )
			{
				double minX, maxX, minY, maxY;
				// if we don't have offset values, then skip any interactivity
				if (!GetElementOffsetValues(elem, minX, maxX, minY, maxY) ) continue;

				FBProperty *pProp = FindProperty(elem, false, "Active");
				if (pProp && (pProp->AsInt() > 0) )
				{
					pProp = FindProperty(elem, true);

					// we have found a property which we can connect to rect position
					if (pProp && (pProp->GetPropertyType() == kFBPT_Vector3D) ) {
						FBVector3d values;
						pProp->GetData( values, sizeof(double) * 3);

						double camWidth = pCamera->CameraViewportWidth;
						double camHeight = pCamera->CameraViewportHeight;
						
						double w = elem->Width;
						double h = elem->Height;
						if (elem->ScaleByPercent)
						{
							w = 0.01 * w * pCamera->CameraViewportWidth;
							h = 0.01 * h * pCamera->CameraViewportHeight;
						}

						double x = 0.01 * values[0] * (maxX - minX - w) + minX;
						double y = 0.01 * values[1] * (maxY - minY - h) + minY;

						if (elem->PositionByPercent && camWidth > 0.0 && camHeight > 0.0)
						{
							x = 100.0 * x / camWidth;
							y = 100.0 * y / camHeight;
						}
						
						elem->X = x;
						elem->Y = y;
					}
				}
			}
		}
	}
}

void Manip_HUDext::CalculateOffset(FBHUDElement *pElem, int pMouseX, int pMouseY)
{
	FBScene *pScene = FBSystem().Scene;

	for (int i=0; i<pScene->HUDs.GetCount(); ++i)
	{
		FBHUD *pHUD = (FBHUD*) pScene->HUDs.GetAt(i);
				
		FBCamera *pCamera = nullptr;
		FBProperty *prop = pHUD->PropertyList.Find("CameraName");
		if (prop) {
			FBString name = prop->AsString();
			for (int k=0; k<pScene->Cameras.GetCount(); ++k)
				if (name == pScene->Cameras.GetAt(k)->Name)
				{
					pCamera = (FBCamera*) pScene->Cameras.GetAt(k);
					break;
				}
		}

		if (!pCamera) continue; // skip processing without assigned camera

		for (int j=0; j<pHUD->Elements.GetCount(); ++j)
		{
			FBHUDElement *elem = (FBHUDElement*) pHUD->Elements.GetAt(j);

			if (elem == pElem) {

				double x = pElem->X;
				double y = pElem->Y;
				double w = pElem->Width;
				double h = pElem->Height;

				if (pElem->PositionByPercent)
				{
					x = 0.01 * x * pCamera->CameraViewportWidth;
					y = 0.01 * y * pCamera->CameraViewportHeight;
				}
				if (pElem->ScaleByPercent)
				{
					w = 0.01 * w * pCamera->CameraViewportWidth;
					h = 0.01 * h * pCamera->CameraViewportHeight;
				}

				int invMouseY = pMouseY;
				if (pCamera) {
					invMouseY = pCamera->WindowHeight - invMouseY;
				}

				mOffsetX = x - pMouseX;
				mOffsetY = y - invMouseY;

				return;
			}
		}
	}
}

bool Manip_HUDext::GetElementOffsetValues(FBHUDElement *pElem, double &minx, double &maxx, double &miny, double &maxy)
{
	FBProperty *prop;
	bool res = true;
	
	prop = FindProperty(pElem, false, "minX");
	if (prop) prop->GetData( &minx, sizeof(double) );
	else res = false;
	
	prop = FindProperty(pElem, false, "maxX");
	if (prop) prop->GetData( &maxx, sizeof(double) );
	else res = false;

	prop = FindProperty(pElem, false, "minY");
	if (prop) prop->GetData( &miny, sizeof(double) );
	else res = false;

	prop = FindProperty(pElem, false, "maxY");
	if (prop) prop->GetData( &maxy, sizeof(double) );
	else res = false;

	return res;
}

void Manip_HUDext::MoveHUDElement(FBHUDElement *pElem, int pMouseX, int pMouseY, int minX, int minY, int maxX, int MaxY)
{
	FBScene *pScene = FBSystem().Scene;

	for (int i=0; i<pScene->HUDs.GetCount(); ++i)
	{
		FBHUD *pHUD = (FBHUD*) pScene->HUDs.GetAt(i);
				
		FBCamera *pCamera = nullptr;
		FBProperty *prop = pHUD->PropertyList.Find("CameraName");
		if (prop) {
			FBString name = prop->AsString();
			for (int k=0; k<pScene->Cameras.GetCount(); ++k)
				if (name == pScene->Cameras.GetAt(k)->Name)
				{
					pCamera = (FBCamera*) pScene->Cameras.GetAt(k);
					break;
				}
		}

		if (!pCamera) continue; // skip processing without assigned camera

		for (int j=0; j<pHUD->Elements.GetCount(); ++j)
		{
			FBHUDElement *elem = (FBHUDElement*) pHUD->Elements.GetAt(j);

			if (elem == pElem) {

				double x = pElem->X;
				double y = pElem->Y;
				double w = pElem->Width;
				double h = pElem->Height;

				double offsetx = mOffsetX;
				double offsety = mOffsetY;

				double minX, maxX, minY, maxY;
				// if we don't have offset values, then skip any interactivity
				if (!GetElementOffsetValues(pElem, minX, maxX, minY, maxY) ) continue;
				if ( (maxX - minX) == 0.0 || (maxY - minY) == 0.0) continue; 

				double camWidth = pCamera->CameraViewportWidth;
				double camHeight = pCamera->CameraViewportHeight;

				int invMouseY = pMouseY;
				if (pCamera) {
					invMouseY = pCamera->WindowHeight - invMouseY;
				}

				if (pElem->PositionByPercent && camWidth > 0.0 && camHeight > 0.0)
				{
					x = 100.0 * pMouseX / camWidth;
					y = 100.0 * invMouseY / camHeight;

					offsetx = 100.0 * offsetx / camWidth;
					offsety = 100.0 * offsety / camHeight;

					minX = 100.0 * minX / camWidth;
					minY = 100.0 * minY / camHeight;
					maxX = 100.0 * maxX / camWidth;
					maxY = 100.0 * maxY / camHeight;

					if (!pElem->ScaleByPercent)
					{
						w = 100.0 * w / camWidth;
						h = 100.0 * h / camHeight;
					}
				}
				else
				{
					x = pMouseX;
					y = invMouseY;

					if (pElem->ScaleByPercent)
					{
						w = 0.01 * w * camWidth;
						h = 0.01 * h * camHeight;
					}
				}

				
				pElem->X = x + offsetx;
				pElem->Y = y + offsety;

				if (pElem->X < minX) pElem->X = minX;
				if (pElem->Y < minY) pElem->Y = minY;
				if ( (pElem->X + w) > maxX) pElem->X = maxX - w;
				if ( (pElem->Y + h) > maxY) pElem->Y = maxY - h;

				// change values in reference property if exist
				FBProperty *pProp = FindProperty(elem, true);
				if (pProp && (pProp->GetPropertyType() == kFBPT_Vector3D) ) {
					FBVector3d values;

					values[0] = 100.0 * (pElem->X - minX) / (maxX - minX - w);
					values[1] = 100.0 * (pElem->Y - minY) / (maxY - minY - h);

					pProp->SetData( values );
				}

				return;
			}
		}
	}
}

bool IsMouseInside(FBHUDElement *pElement, FBCamera *pCamera, int pMouseX, int pMouseY)
{
	double x = pElement->X;
	double y = pElement->Y;
	double w = pElement->Width;
	double h = pElement->Height;

	double cx = pCamera->CameraViewportX;
	double cy = pCamera->CameraViewportY;
	double cw = pCamera->CameraViewportWidth;
	double ch = pCamera->CameraViewportHeight;

	if (pElement->ScaleByPercent)
	{
		w = 0.01 * w * cw;
		h = 0.01 * h * ch;
	}
	if (pElement->PositionByPercent)
	{
		x = 0.01 * x * cw;
		y = 0.01 * y * ch;

		if (pElement->Justification == kFBHUDRight)
		{
			x -= w;
		}
		else if (pElement->Justification == kFBHUDCenter)
		{
			x -= w * 0.5;
		}

		if (pElement->HorizontalDock == kFBHUDRight)
		{
			x += cw;
		}
		else if (pElement->HorizontalDock == kFBHUDCenter)
		{
			x += cw * 0.5;
		}


		if (pElement->VerticalDock == kFBHUDTop)
		{
			y += ch - h;
		}
		else if (pElement->VerticalDock == kFBHUDCenter)
		{
			y += ch * 0.5 - h * 0.5;
		}
	}
	
	pMouseX -= cx;
	pMouseY -= cy;

	int invMouseY = pMouseY;
	if (pCamera) {
		invMouseY = ch - invMouseY;
	}

	if (pMouseX > x && pMouseX < (x + w) 
		&& invMouseY > y && invMouseY < (y + h) )
	{
		return true;
	}

	return false;
}

FBHUDElement *Manip_HUDext::FindHUDElement(int pMouseX, int pMouseY)
{
	FBScene *pScene = FBSystem().Scene;

	for (int i=0; i<pScene->HUDs.GetCount(); ++i)
	{
		FBHUD *pHUD = (FBHUD*) pScene->HUDs.GetAt(i);
				
		FBCamera *pCamera = nullptr;
		FBProperty *prop = pHUD->PropertyList.Find("CameraName");
		if (prop) {
			FBString name = prop->AsString();
			for (int k=0; k<pScene->Cameras.GetCount(); ++k)
				if (name == pScene->Cameras.GetAt(k)->Name)
				{
					pCamera = (FBCamera*) pScene->Cameras.GetAt(k);
					break;
				}
		}

		if (!pCamera || pCamera != pScene->Renderer->CurrentCamera) continue; // skip processing without assigned camera

		for (int j=0; j<pHUD->Elements.GetCount(); ++j)
		{
			FBHUDElement *pElem = (FBHUDElement*) pHUD->Elements.GetAt(j);
			
			if ( FBIS(pElem, FBHUDRectElement) || FBIS(pElem, FBHUDTextElement) )
			{
				if (IsMouseInside( pElem, pCamera, pMouseX, pMouseY ))
					return pElem;
			}
		}
	}

	return nullptr;
}

FBProperty *Manip_HUDext::FindProperty(int pMouseX, int pMouseY)
{
	FBHUDElement *pElem = FindHUDElement(pMouseX, pMouseY);
	
	if (pElem)
	{
		// we make a click on component

		for (int l=0; l<pElem->PropertyList.GetCount(); ++l)
		{
			FBProperty *prop = pElem->PropertyList[l];
			if (prop->IsReferenceProperty() )
			{
				for (int ii=0; ii<prop->GetSrcCount(); ++ii)
				{
					FBPlug *pPlug = prop->GetSrc(ii);
										
					if (pPlug->Is( FBProperty::TypeInfo ) )
					{
						FBProperty *pSrc = (FBProperty*) pPlug;
						if (pSrc)
						{
												
							return pSrc;
						}
					}
				}
			}
			else if (prop->IsUserProperty() )
			{
				return prop;
			}
		}	
	}

	return nullptr;
}

FBProperty *Manip_HUDext::FindProperty(FBHUDElement *pElem,  bool ReferenceOnly, const char *propname)
{
	if (pElem)
	{
		// search for a reference or user property

		if (propname)
		{
			FBProperty *prop = pElem->PropertyList.Find(propname);
			if (prop)
				if (prop->IsReferenceProperty() )
				{
					for (int ii=0; ii<prop->GetSrcCount(); ++ii)
					{
						FBPlug *pPlug = prop->GetSrc(ii);
										
						if (pPlug->Is( FBProperty::TypeInfo ) )
						{
							FBProperty *pSrc = (FBProperty*) pPlug;
							if (pSrc)
							{
												
								return pSrc;
							}
						}
					}
				}
				else if (prop->IsUserProperty() && !ReferenceOnly)
				{
					return prop;
				}
		}
		else
		{
			for (int l=0; l<pElem->PropertyList.GetCount(); ++l)
			{
				FBProperty *prop = pElem->PropertyList[l];
				if (prop->IsReferenceProperty() )
				{
					for (int ii=0; ii<prop->GetSrcCount(); ++ii)
					{
						FBPlug *pPlug = prop->GetSrc(ii);
										
						if (pPlug->Is( FBProperty::TypeInfo ) )
						{
							FBProperty *pSrc = (FBProperty*) pPlug;
							if (pSrc)
							{
												
								return pSrc;
							}
						}
					}
				}
				else if (prop->IsUserProperty() && !ReferenceOnly )
				{
					return prop;
				}
			}	
		}
	}

	return nullptr;
}

bool Manip_HUDext::ViewInput(int pMouseX, int pMouseY, FBInputType pAction, int pButtonKey, int pModifier)
{
	// skip input operation if system is busy
	if (FBPlayerControl().IsPlaying || FBPlayerControl().IsPlotting || FBPlayerControl().IsRecording) 
		return false; 

	// check for the element under cursor
	if (!mDown && pAction == kFBMotionNotify)
	{
		if (mLastElement) {
			if (mLastElement->Is( FBHUDTextElement::TypeInfo ) )
			{
				((FBHUDTextElement*)mLastElement)->BackgroundColor = mLastBackground;
			}
			else if (mLastElement->Is( FBHUDRectElement::TypeInfo ) )
			{
				((FBHUDRectElement*)mLastElement)->Color = mLastBackground;
			}
			mLastElement = nullptr;
		}
	
		// search for the hud under cursor
		FBHUDElement *pText = FindHUDElement(pMouseX, pMouseY);
		if (pText ) { 
			FBProperty *pProp = FindProperty(pText, false, "Active");
			if (pProp && (pProp->AsInt() > 0) )
			{
				if (pText->Is( FBHUDTextElement::TypeInfo ) )
				{
					mLastBackground = ((FBHUDTextElement*)pText)->BackgroundColor;
					((FBHUDTextElement*)pText)->BackgroundColor = FBColorAndAlpha(1.0, 0.0, 0.0, 0.5);
				}
				else if (pText->Is( FBHUDRectElement::TypeInfo ) )
				{
					mLastBackground = ((FBHUDRectElement*)pText)->Color;\
					((FBHUDRectElement*)pText)->Color = FBColorAndAlpha(1.0, 0.0, 0.0, 0.5);
				}	

				mLastElement = pText;
			}
			else
			{
				mLastElement = nullptr;
			}
		}
	}

	if (pButtonKey == 2)
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
			if (pButtonKey == 2)
			{
				// Mouse button double-clicked - SET KEY
				FBProperty *pProp = FindProperty(mLastElement);
				if (pProp && pProp->IsAnimatable() )
				{
					FBPropertyAnimatable *pAnimProp = (FBPropertyAnimatable*) pProp;
				
					if (!pAnimProp->IsAnimated() ) pAnimProp->SetAnimated(true);
					pAnimProp->Key();
				}
			}
		}
		break;
		case kFBButtonPress:
		{
			// Mouse button clicked - CHANGE PROPERTY VALUE
			// search for the hud under cursor
			if (pButtonKey == 2)
			{

				mDown = false;
				mProperty = nullptr;

				FBProperty *pProp = FindProperty(mLastElement, false, "ScriptExecute");
				if (pProp)
				{
					FBString propName = pProp->GetName();
					if (propName == "ScriptExecute" )
					{
						FBString scriptPath = pProp->AsString();

						mScriptExecute = true;
						mScriptPath = scriptPath;
					}
				}
				else
				{
					pProp = FindProperty(mLastElement, false, "CameraSwitch");
					if (pProp)
					{
						FBString propValue = pProp->AsString();
						FBScene *pScene = FBSystem::TheOne().Scene;

						for (int i=0; i<pScene->Cameras.GetCount(); ++i)
						{
							FBCamera *pCamera = (FBCamera*) pScene->Cameras.GetAt(i);

							if ( pCamera->Name == propValue )
							{
								pScene->Renderer->CurrentCamera = pCamera;
								break;
							}
						}
					}
					else
					{
						pProp = FindProperty(mLastElement, true);
						if (pProp)
						{
							mProperty = pProp;
							mDown = true;
							mLastX = pMouseX;
							mLastY = pMouseY;

							if (mLastElement) {
								CalculateOffset(mLastElement, pMouseX, pMouseY);
							}
						}
					}
				}
			}
		}
		break;
		case kFBButtonRelease:
		{
			// Mouse button released.
			if (pButtonKey == 2)
			{
				mDown = false;
				mProperty = nullptr;
			}
		}
		break;
		case kFBMotionNotify:
		{
			// When there is mouse movement in the viewer window
			
			if (mDown && mLastElement)
			{
				if (mLastElement->Is(FBHUDRectElement::TypeInfo) )
				{
					// move rect hud
					MoveHUDElement(mLastElement, pMouseX, pMouseY, 0, 0, 0, 0);
				}
				else if (mProperty)
				{
					// change property value in a text hud

					FBPropertyType type = mProperty->GetPropertyType();
					switch(type)
					{
					case kFBPT_bool:
						{
							int value=0;
							value = mProperty->AsInt();
							if ( abs(pMouseX - mLastX) > 50 )
							{
								value = 1.0 - value;
								mProperty->SetInt(value);
							
								mDown = false;
								mProperty = nullptr;
							}
						}
						break;
					case kFBPT_double:

						double value=0.0;
						mProperty->GetData(&value, sizeof(double) );

						switch(pModifier)
						{
						case 2: // CTRL is pressed
							value += 0.1 * (pMouseX - mLastX);
							break;
						case 1: // SHIFT is pressed
							value += 10.0 * (pMouseX - mLastX);
							break;

						default:
							value += pMouseX - mLastX;
						}

						mProperty->SetData(&value);
						mLastX = pMouseX;
						break;
					}
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
