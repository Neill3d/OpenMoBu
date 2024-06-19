
#pragma once

/**	\file	ortoolview3d_view.h
*	Tool with 3D viewer.

// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/MoBu/blob/master/LICENSE

	GitHub repo - https://github.com/Neill3d/MoBu

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
class ORViewPane : public FBView
{
	FBClassDeclare( ORViewPane, FBView );
  private:
    FBSystem    mSystem;
	FBRenderer*	mRender;	//!< Renderer object.

  public:
	//! Constructor.
	ORViewPane(const int paneId);
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
