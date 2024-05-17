#ifndef __BLENDSHAPE_MANIP_H__
#define __BLENDSHAPE_MANIP_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: BlendShapeToolkit_manip.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
#include <fbsdk/fbsdk.h>
#include <fbsdk\fbundomanager.h>
#include "BlendShapeToolkit_brushes.h"
#include "BlendShapeToolkit_deformer_constraint.h"
#include <vector>
#include <stack>
#include <map>

#include "WGLFONT.h"

#include "Delegate.h"

//--- Registration defines
#define ORMANIPASSOCIATION__CLASSNAME	ORManip_Sculpt
#define ORMANIPASSOCIATION__CLASSSTR	"ORManip_Sculpt"
#define ORMANIPASSOCIATION__LABEL		"Sculpting brush"

//! Manipulator association
FB_FORWARD(ORManip_Sculpt);

enum FBBrushAffect
{
	kFBBrushAffectOnSurface,
	kFBBrushAffectOnVolume
};



const char * FBPropertyBaseEnum<FBBrushAffect>::mStrings[] = {
	"Surface",
	"Volume",
	0
};

const char * FBPropertyBaseEnum<FBBrushDirection>::mStrings[] = {
	"Screen",
	"First Normal",
	"Average Normal",
	"Vertex Normal",
	"X",
	"Y",
	"Z",
	0
};

struct	PolyShell
{
	FBVector3d			center;
	double				radius;

	std::vector <int>	polys;
};

struct	PolyMesh
{
	std::vector <PolyShell*>	shells;

	void ComputeShells( FBMesh *pMesh )
	{
		//int polyCount = pMesh->PolygonCount();


	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class ModelStateHolder
{
public:
	std::vector<float>		flags;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class SculptUndo : public FBUndo
{
public:
	//! a constructor
	SculptUndo(const OperationBuffer &buffer, ORManip_Sculpt *pManip);

	/** Destructor.
    */
    ~SculptUndo();

	bool CalculateDifference( const OperationBuffer &buffer );

    /** Callback function for undo custom action.
    */
    virtual void	Undo();

    /** Callback function for redo custom action.
    */
    virtual void	Redo();
    
    /** Get Custom action name.
    */
	const char*	GetName() { return "Sculpt action"; }

private:

	ORManip_Sculpt			*mManip;

	OperationBuffer			mBuffer;
	// store different to redo
	FBVector3<float>		*mDifference;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class ORManip_Sculpt : public FBManipulator
{
	//--- FiLMBOX declaration.
	FBManipulatorDeclare( ORManip_Sculpt, FBManipulator );

public:
	//! FiLMBOX Constructor.
	virtual bool FBCreate();

	//!< FiLMBOX Destructor.
	virtual void FBDestroy();

	//!	Deal with manipulator input
	virtual bool ViewInput(int pMouseX, int pMouseY, FBInputType pAction, int pButtonKey,int pModifier);

	//! Manipulator expose function
	virtual void ViewExpose();

	void		EventSceneChange ( HISender pSender, HKEvent pEvent );
	void		EventFileNewOrOpen ( HISender pSender, HKEvent pEvent ); // reset freeze states from that event

	void SetModel(FBComponent* pFBModel, OperationBuffer *bufferZero);
	const char *GetModelName();
	FBModel *GetModelPtr();

	const bool IsModelDeformed() { return mDeformed; }

public:
	bool								mRenderInModelSpace;
	bool								mActivated;
	bool								mDeformed;					//!< direct mesh sculpting or using a deform constraint and buffer

	//
	// Common brush properties
	//
	FBPropertyBool						UseConstraint;

	FBPropertyColor						Color;

	FBPropertyDouble					Radius;
	FBPropertyDouble					Strength;

	FBPropertyBool						Inverted;		// inverted strength action;

	FBPropertyDouble					RadiusSens;
	FBPropertyDouble					StrengthSens;

	FBPropertyColor						BrushColor;

	FBPropertyBaseEnum<FBBrushDirection> DirectionMode;
	FBPropertyBaseEnum<FBBrushAffect>	AffectMode;
	FBPropertyBool						DisplayAffect;	// display interactive vertices weights

	FBPropertyBool						ScreenInfluence;
	FBPropertyDouble					ScreenInfluenceRadius;

	//
	// Wacom tablet properties
	//
	FBPropertyBool						UseTablet;
	FBPropertyBool						UsePenPressure;
	FBPropertyBool						UsePenEraser;		// TODO: switching to eraser brush when using such a pen
	FBPropertyInt						MaxPenPressure;
	FBPropertyBool						DisplayPressure;

	//
	// Deformer properties
	//

	FBPropertyBool						AutoBlendshape;
	FBPropertyBool						AutoKeyframe;
	FBPropertyBool						UseKeyframeRange;
	FBPropertyDouble					KeyframeIn;
	FBPropertyDouble					KeyframeLen;
	FBPropertyDouble					KeyframeOut;

	Gallant::Delegate0<void>			OnDataUpdate;

	
	//
	///
	//

	BrushData		*GetBrushDataPtr() { return &mBrushData; }
	BrushManager	*GetBrushManagerPtr() { return &mBrushManager; }

	void			SetCurrentBrush( const int index );
	void			SetCurrentFalloff( const int index );

	//void			AssignInputDevice( FBDevice *pDevice );
	void			SetUseTabletPenPressure( bool useflag );
	void			AssignDeformerConstraint( BlendShapeDeformerConstraint *pDeformer );

	static void			CopyGeometryToBuffer(FBModel *pModel, OperationBuffer &buffer);
	static void			CopyBufferToGeometry(const OperationBuffer &buffer, FBModel *pModel, const FBVector3<float> *difference = nullptr);
	static void			ApplyBufferDifferenceToGeometry(const OperationBuffer &bufferA, const OperationBuffer &bufferB, FBModel *pModel);

	OperationBuffer		&GetBuffer() { return mBuffer; }
	OperationBuffer		*GetBufferPtr() { return &mBuffer; }

	void			FreezeAll();
	void			FreezeInvert();
	void			FreezeNone();

	void			StoreFreezeState();
	void			RestoreFreezeState();
	void			FreeFreezeState();

	void			Fill();

	// reset method will reset all the mBuffer difference from mBufferZero
	void			Reset();

	// this function will use difference between mBuffer and mBufferZero to make a new blendshape for the pModel
	//	if setKeyframe is true, then for this blendshape will be automatically created 3 keyframes
	void			AddBlendShape(const FBTime &curTime, const bool useContraint, const bool setKeyframe, const double timeIn, const double timeLen, const double timeOut, const bool replaceExisting);

	void			AddBlendShapeDynamic(const FBTime &curTime, const bool setKeyframe, const double timeIn, const double timeLen, const double timeOut, const bool replaceExisting);
	void			AddBlendShapeConstraint(const FBTime &curTime, BlendShapeDeformerConstraint *pConstraint, const bool setKeyframe, const double timeIn, const double timeLen, const double timeOut, const bool replaceExisting);

	// if we sculpt for change base geometry either adding a new blendshape
	void			ApplyGeometryChanges();

	void SetCurrentConstraint( BlendShapeDeformerConstraint *pConstraint )
	{
		mCurrentConstraint = pConstraint;
	}
	BlendShapeDeformerConstraint *GetCurrentConstraint()
	{
		if ( mCurrentConstraint.Ok() )
			return mCurrentConstraint;
		return nullptr;
	}

	static void GlobalAddBlendShape(ORManip_Sculpt *pManipulator, const bool overrideExisting);

private:

	FBApplication		mTheApplication;
	FBSystem			mTheSystem;

	int					lViewport[4];
	FBMatrix			modelview;
	FBMatrix			projection;

	//! perspective or ortho projection
	bool				mPerspective;
	bool				mSystem; //! user camera or default views
	FBVector3d			mCamera;
	FBVector3d			mInterest;	//! importang to have camera target
	int					mScreenWidth;
	int					mScreenHeight;
	FBVector4d			mViewPlane;
	FBVector3d			mViewRight;
	FBVector3d			mViewUp;
	double				mFOV;
	double				mNearDist;
	double				mFarDist;

	FBTVector							orig;
	FBTVector							dir;

	FBVector3d							mPointInViewSpace;
	FBVector3d							mPrevPointInViewSpace;

	int									mSmoothBrushIndex;

	HdlFBPlugTemplate<FBCamera>			mInputCamera;

	BrushData							mBrushData;
	BrushManager						mBrushManager;		// hold all brushes and their ui

	HdlFBPlugTemplate<BlendShapeDeformerConstraint>	mDeformer;

	// store scene model states (freeze)
	std::map<FBModel*, ModelStateHolder*>		mFreezeStates;

	//! buffer works only with static geometry !!
	
	OperationBuffer						mBufferZero;		// initial buffer (used in erase brush)
	OperationBuffer						mBuffer;			// current mesh buffer

	FBUndoManager						mUndoManager;
	SculptUndo							*mUndo;

	//HdlFBPlugTemplate<FBDevice>			mDevice;			// input device (like wacom tablet for mult input pressure)
	bool								mTabletSupported;	
	bool								mUsePenPressure;
	float								mLastPressure;

	WGLFont								mGLFont;

	bool								mPrepStage;

	void				ConfigBrushManager();

	void				ChangeNotify();		// recalculate edge distances (for iteractive real-time cursor)

	void				CalculateBufferWeights(BrushCameraData *pCameraData);
	void				DistributeSurfaceWeights( std::stack<VertEdge> &stack, const float lRadius );		// lenght in segments
	void				RenderWeights();
	
	//! calcualte viewplane for current camera
	void BuildViewPlane(const FBTVector &pos);

	//! update viewport
	/*!
		\param currentCamera - current viewport camera (manipulator variable)
	*/
	void UpdateViewPlane(FBCamera *currentCamera, BrushCameraData *pCameraData);

	//! draw debug lines
	void DrawViewPlane(BrushCameraData *pCameraData);

	FBVector3d	SpaceToScreen( const FBVector3d	&v );

private:

	HdlFBPlugTemplate<BlendShapeDeformerConstraint>		mCurrentConstraint;

};

#endif /* __BLENDSHAPE_MANIP_H__ */
