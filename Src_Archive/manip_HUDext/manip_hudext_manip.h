#ifndef __MANIP_HUDEXT_MANIP_H__
#define __MANIP_HUDEXT_MANIP_H__

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


//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Registration defines
#define ORMANIPTEMPLATE__CLASSNAME	Manip_HUDext
#define ORMANIPTEMPLATE__CLASSSTR	"Manip_HUDext"

//! Manipulator template
class Manip_HUDext : public FBManipulator
{
	//--- FiLMBOX declaration.
	FBManipulatorDeclare( Manip_HUDext, FBManipulator );

public:
	//! FiLMBOX Constructor.
	virtual bool FBCreate();

	//!< FiLMBOX Destructor.
	virtual void FBDestroy();

	//!	Deal with manipulator input
	virtual bool ViewInput(int pMouseX, int pMouseY, FBInputType pAction, int pButtonKey,int pModifier);

	//! Manipulator expose function
	virtual void ViewExpose();

	void	EventIdle						( HISender pSender, HKEvent pEvent );

public:
	int			mTestState;
	FBModel*	mModel;
	bool		mRenderInModelSpace;
	bool		mDrawSquare;

private:

	bool			mDown;			//!< is mouse button pressed
	int				mLastX;			//!< last mouse x 
	int				mLastY;			//!< last mouse y
	FBProperty		*mProperty;		// we are controlling value of this property

	bool			mScriptExecute;
	FBString		mScriptPath;

	FBHUDElement		*mLastElement;
	FBColorAndAlpha		mLastBackground;
	double				mOffsetX;
	double				mOffsetY;

	FBHUDElement *FindHUDElement(int pMouseX, int pMouseY);
	FBProperty *FindProperty(int pMouseX, int pMouseY);
	FBProperty *FindProperty(FBHUDElement *pElem, bool ReferenceOnly=false, const char *propname = 0);

	void CalculateOffset(FBHUDElement *pElem, int pMouseX, int pMouseY);
	void MoveHUDElement(FBHUDElement *pElem, int pMouseX, int pMouseY, int minX, int minY, int maxX, int MaxY);
	bool GetElementOffsetValues(FBHUDElement *pElem, double &minx, double &maxx, double &miny, double &maxy);
	void UpdateRectElements();
};

#endif /* __MANIP_HUDEXT_MANIP_H__ */
