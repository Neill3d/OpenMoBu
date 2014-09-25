#pragma once

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
#include <fbsdk\fbsdk.h>
#ifndef OLD_OPENGL_HEADER
#include <fbsdk\fbsdk-opengl.h>
#endif

//-- image sizes
#define	HANDS_IMAGE_WIDTH		170 
#define	HANDS_IMAGE_HEIGHT		241

#define	FINGER_IMAGE_WIDTH		40
#define	FINGER_IMAGE_HEIGHT		40

#define FINGERS_COUNT			16

#define HAND_SIZE				30.0f
#define FINGER_SIZE				20.0f

struct CFingerImage
{
	//-- finger image position
	float		x;
	float		y;

	//-- is selected
	bool		selected;

	//-- texture according current weight
	GLuint		textureId;

	//-- this one finger model
	FBModel	*mModel;

	void SetSel( bool sel = true ) {
		selected = sel;

		if (mModel)
			mModel->Selected = sel;
	}

	void CleanUP();

	void UpdateTexture( FBModel *pRoot, int index, GLuint drop, GLuint drop25, GLuint drop50, GLuint drop75, GLuint drop100 );
	void UpdateTexture( double weight, FBVector3d inherit, int index, GLuint drop, GLuint drop25, GLuint drop50, GLuint drop75, GLuint drop100 );
};

/**	image of two hands (with additional fk elements selection mode)
*/
class CHandsImage : public FBView
{
public:
	CHandsImage(void);
	virtual ~CHandsImage(void);

	//! Refresh callback.
	virtual void Refresh(bool pNow=false);
	//! Expose callback.
	virtual void ViewExpose();
	//! input callback.
	virtual void ViewInput(int pMouseX,int pMouseY,FBInputType pAction,int pButtonKey,int pModifier);

public:
	FBSystem		mSystem;

	bool			mSelected;

	bool			mReDraw;				// able to redraw

	GLuint			mHandsId;
	GLuint			mRegionDropId;
	GLuint			mRegionDropId25;
	GLuint			mRegionDropId50;
	GLuint			mRegionDropId75;
	GLuint			mRegionDropId100;
	GLuint			mRegionSelectedId;

	//-- storage fingers data
	CFingerImage	mHandLeft[FINGERS_COUNT];
	CFingerImage	mHandRight[FINGERS_COUNT];

	//-- free all storage objects & relation's
	void CleanUP();

	void FillHandLeftFinger( int n, float x, float y, bool selected=false, FBModel *pModel=NULL );
	void FillHandRightFinger( int n, float x, float y, bool selected=false, FBModel *pModel=NULL );
	void FillHandLeftModels( FBModel *pHandModel );
	void FillHandRightModels( FBModel *pHandModel );
	void ResetSelection(void);									// clear scene selection
	void CheckHandsSelection(void);
	bool IsLeftBodyPartSelected(int n);
	void SelectLeftBodyPart(int n, bool sel=true);
	bool IsRightBodyPartSelected(int n);
	void SelectRightBodyPart(int n, bool sel=true);
	void DrawQuadf( float x, float y, float x2, float y2 );
	void DrawQuadWH( float x, float y, float w, float h );

	void UpdateWeight();

	//-- fingers position configuration
	int LoadFingerConfig( FBString fileName );
	int SaveFingerConfig( FBString fileName );
};


struct CCharacterBodyPart
{
	FBVector2d						mCoords[2];		// body part line
	FBArrayTemplate<FBModel* >		mModels;		// pointers to fk models
};

/**	image of the character (only for selection elements)
*/
class CCharacterImage : public FBView
{
public:
	CCharacterImage(void);
	virtual ~CCharacterImage(void);

	//! Refresh callback.
	virtual void Refresh(bool pNow=false);
	//! Expose callback.
	virtual void ViewExpose();
	//! input callback.
	virtual void ViewInput(int pMouseX,int pMouseY,int pAction,int pButtonKey,int pModifier);

public:
	FBSystem		mSystem;

	bool			mSelected;

	GLuint			mCharacterId;						// background character image
	GLuint			mRegionDropId;						// fk element marker on image
	GLuint			mRegionSelectedId;					// selection round rect

	//-- internal data
	FBArrayTemplate<CCharacterBodyPart>		mBodyParts;			// character body parts with pointers to fk models

	void DrawQuadf( float x, float y, float x2, float y2 );
	void DrawQuadWH( float x, float y, float w, float h );
};
