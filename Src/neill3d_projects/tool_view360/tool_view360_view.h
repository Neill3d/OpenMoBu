
#pragma once

/**	\file	ortoolview360_view.h
*	Tool with 3D viewer.

	Author Sergey Solokhin (Neill3d)
	 e-mail to: s@neill3d.com
	  www.neill3d.com

*/

//--- SDK include
#include <fbsdk/fbsdk.h>

#include <vector>


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**	View 3D class.
*/
class ORView360 : public FBView
{
	FBClassDeclare( ORView360, FBView );
  private:
    FBSystem    mSystem;
	FBRenderer*	mRender;	//!< Renderer object.

  public:
	//! Constructor.
	ORView360();
	//! Destructor
	virtual void FBDestroy();

	//! Refresh callback.
	virtual void Refresh(bool pNow=false);
	//! Expose callback.
	virtual void ViewExpose();

	/** Input callback function. */
    virtual void ViewInput(int pMouseX,int pMouseY,FBInputType pAction,int pButtonKey,int pModifier);

private:
	
	int				mPaneId;
	bool			mIsMaximized;
	int				mScaleDown;
};
