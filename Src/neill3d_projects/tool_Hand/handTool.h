#ifndef __ORTOOL_TEMPLATE_TOOL_H__
#define __ORTOOL_TEMPLATE_TOOL_H__

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

/**	\file	handTool.h
*	Template for tool development.
*/

//--- SDK include
#include <fbsdk/fbsdk.h>

//-- additional UI classes (hands FK control)
#include "handsImage.h"

//--- Registration define
#define HANDTOOL__CLASSNAME	HandCtrlTool
#define HANDTOOL__CLASSSTR	"Hand Controller"

#define HAND_FINGERS_COUNT				5
#define FINGER_ELEMS_COUNT				3


//-- struct declaration
struct CFinger;
struct CFingerMultiplyBox;
struct CHand;


/** one finger data struct
*/
struct CFinger
{
	//------------------ each finger connected to model (finger property)
	// finger model
	FBModel			*mFinger[FINGER_ELEMS_COUNT];	// three elemetns in finger

	FBModel			*mSkeletFinger[FINGER_ELEMS_COUNT];

	// user property ( relative rotation ) animatable
	FBString				mRotName;				
	int						mRotId;
	FBString				mRotDataType;
	FBPropertyType			mRotType;
	
	// user property ( damping ) animatable
	FBString				mDampName;				
	int						mDampId;
	FBString				mDampDataType;
	FBPropertyType			mDampType;

	// user property ( weight ) not animated
	FBString				mWeightName;				
	int						mWeightId;
	FBString				mWeightDataType;
	FBPropertyType			mWeightType;

	// user property ( weight ) not animated
	FBString				mInheritName;				
	int						mInheritId;
	FBString				mInheritDataType;
	FBPropertyType			mInheritType;

	// user property ( weight ) not animated
	FBString				mSpreadName;				
	int						mSpreadId;
	FBString				mSpreadDataType;
	FBPropertyType			mSpreadType;


	//------------------ UI controls
	FBLabel					mLabelRot;				// rotation label
	FBLabel					mLabelDamp;				// damping label
	FBLabel					mLabelWeight;			// weight label
	FBLabel					mLabelInherit;			// inherit label
	FBLabel					mLabelSpread;			// inherit label

	FBEditPropertyModern	mRotProperty;			// finger rotation
	FBEditPropertyModern	mDampProperty;			// damping of this rotation
	FBEditPropertyModern	mWeightProperty;		// weight property for all elements in finger
	FBEditPropertyModern	mInheritProperty;		// inherit for all elements
	FBEditPropertyModern	mSpreadProperty;		// inherit for all elements

	//------------------ Relation boxes (for each finger joint)
	FBBox					*mFingerBoxSrc[FINGER_ELEMS_COUNT];			// finger as source
	FBBox					*mSkeletFingerSrc[FINGER_ELEMS_COUNT];
	FBBox					*mFingerBox[FINGER_ELEMS_COUNT];				// constraint finger
	FBBox					*mVectorToNumber[FINGER_ELEMS_COUNT];
	FBBox					*mNumberToVector[FINGER_ELEMS_COUNT];
	FBBox					*mAddBox[FINGER_ELEMS_COUNT];	// for rotation
	FBBox					*mAddBox2;						// for spread
	FBBox					*mDampBox[FINGER_ELEMS_COUNT];	// for rotation
	FBBox					*mDampBox2;						// for spread
	FBBox					*mMultBox[FINGER_ELEMS_COUNT];
	FBBox					*mWeightBox[FINGER_ELEMS_COUNT];

	//-- finger model
	FBAnimationNode *mRotationSrc[FINGER_ELEMS_COUNT];
	FBAnimationNode *mRotation[FINGER_ELEMS_COUNT];
	FBAnimationNode *mZRotation;										//-- use this ONLY form base joint (for whole finger)
	FBAnimationNode *mSpread;
	FBAnimationNode *mDamp;
	FBAnimationNode *mWeight;
	FBAnimationNode *mInherit;
	//-- number to vector nodes
	FBAnimationNode *mNumberToVectorIn[FINGER_ELEMS_COUNT];
	FBAnimationNode *mNumberToVectorOut[FINGER_ELEMS_COUNT];
	FBAnimationNode *mNumberToVectorIn2;							// spread (y rotation
	FBAnimationNode *mNumberToVectorIn3;							// X rotation
	//-- vector to number nodes
	FBAnimationNode *mVectorToNumberIn[FINGER_ELEMS_COUNT];
	FBAnimationNode *mVectorToNumberOut[FINGER_ELEMS_COUNT];
	FBAnimationNode *mVectorToNumberOut2;
	FBAnimationNode *mVectorToNumberOut3;							// X rotation out animation node
	
	//-- add box nodes
	FBAnimationNode *mAddA[FINGER_ELEMS_COUNT];
	FBAnimationNode *mAddB[FINGER_ELEMS_COUNT];
	FBAnimationNode *mAddR[FINGER_ELEMS_COUNT];
	//-- add box nodes (for SPREAD)
	FBAnimationNode *mAdd2A;
	FBAnimationNode *mAdd2B;
	FBAnimationNode *mAdd2R;
	//-- damping box nodes
	FBAnimationNode *mDampA[FINGER_ELEMS_COUNT];
	FBAnimationNode *mDampF[FINGER_ELEMS_COUNT];
	FBAnimationNode *mDampR[FINGER_ELEMS_COUNT];
	//-- damping box nodes (for SPREAD)
	FBAnimationNode *mDamp2A;
	FBAnimationNode *mDamp2F;
	FBAnimationNode *mDamp2R;
	//-- multS box nodes
	FBAnimationNode *mMultA[FINGER_ELEMS_COUNT];
	FBAnimationNode *mMultB[FINGER_ELEMS_COUNT];
	FBAnimationNode *mMultR[FINGER_ELEMS_COUNT];
	//-- weight box nodes
	FBAnimationNode *mWeightW[FINGER_ELEMS_COUNT];
	FBAnimationNode *mWeightI[FINGER_ELEMS_COUNT];
	FBAnimationNode *mWeightR[FINGER_ELEMS_COUNT];

	bool					mLocalMode;

	CFinger();
	~CFinger();

	//-- cleanUP all usage objects and relation
	void CleanUP();

	//-- add some user property to each finger model
	void AddPropertyToModel( FBModel *pCtrlRigModel, FBModel *pModel );
	//-- remove all propertys
	void RemovePropertyFromModel();
	//-- add UI property
	void AddUI( FBLayout *pTool );
	//-- select/deselect finger model
	void SelectFinger();
	void DeSelectFinger();
	//-- check for seleciton
	bool IsSelected();
	//-- add relation boxes for this and connect it
	void AddRelationBoxes( int n, int ThumbAxis, FBConstraintRelation *pConstraint, bool LocalMode );
	void MakeConnections();

	//-- working with keys
	void SetKey();
	void RemoveKey();
	bool GetPrevKeyTime( const FBTime &startTime, const FBTime &currTime, FBTime &lTime );
	bool GetNextKeyTime( const FBTime &endTime,	const FBTime &currTime, FBTime &lTime );

	//-- set/add weight value
	void SetWeight( double value );
	void AddWeight( double value );
	//-- set/add rotation value
	void SetRotation( double value );
	void AddRotation( double value );
	//-- set/add spread value
	void SetSpread( double value );
	void AddSpread( double value );
	//-- damp/add value
	void SetDamp( double value );
	void AddDamp( double value );
	//-- inherit for joints
	void SetInherit( FBVector3d value );
	void AddInherit( FBVector3d value );

	//-- value from first joint
	double GetWeight();
	double GetRotation();
	double GetSpread();
	double GetDamp();
	FBVector3d GetInherit();
};


/** Hand
*/
struct CHand
{
	// hand model
	FBModel			*mModel;


	//------------------ Fingers
	CFinger					mFingers[HAND_FINGERS_COUNT];

	CHand();
	~CHand();

	//-- free all usage objects and relation
	void CleanUP();

	//-- user property for hand
	void AddPropertyToModel( FBModel *pCtrlRigModel, FBModel *pModel );
	//-- remove all propertys
	void RemovePropertyFromModel();
	//-- property on tool
	void AddUI( FBLayout *pTool );
	//-- select/deselect hand model
	void SelectHand();
	bool IsSelected();
	void DeSelectHand();
	//-- relation boxes for constraint
	void AddRelationBoxes( int ThumbAxis, FBConstraintRelation *pConstraint );
	void MakeConnections();

	//-- working with keys
	void SetKey();
	void RemoveKey();
	bool GetPrevKeyTime( const FBTime &startTime, const FBTime &currTime, FBTime &lTime );
	bool GetNextKeyTime( const FBTime &endTime,	const FBTime &currTime, FBTime &lTime );

	//-- set new weight value
	void SetWeight( double value );
	void AddWeight( double value ); 
	//-- set new rotation value
	void SetRotation( double value );
	void AddRotation( double value );
	//-- set spread value
	void SetSpread( double value );
	void AddSpread( double value );
	//-- damp value
	void SetDamp( double value );
	void AddDamp( double value );
	//-- inherit for finger joints
	void SetInherit( FBVector3d value );
	void AddInherit( FBVector3d value );

	//-- value from selected fingers
	double GetWeight();
	double GetRotation();
	double GetSpread();
	double GetDamp();
	FBVector3d GetInherit();
};



/**	Tool template.
*/
class HandCtrlTool : public FBTool
{
	//--- FiLMBOX Tool declaration.
	FBToolDeclare( HandCtrlTool, FBTool );

public:
	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate();		//!< FiLMBOX Creation function.
	virtual void FBDestroy();		//!< FiLMBOX Destruction function.

	// UI Management
	void	UICreate				();			//!< Build the UI elements.
	void		UICreateLayout0		();
	void		UICreateLayout1		();
	void		UICreateLayout2		();
	void		UICreateLayout3		();
	void	UIConfigure				();			//!< Configure the UI elements.
	void		UIConfigureLayout0	();
	void		UIConfigureLayout1	();
	void		UIConfigureLayout2	();
	void		UIConfigureLayout3	();

	virtual bool FbxStore		( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat );
	virtual bool FbxRetrieve	( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat );

	//-- delete all object property's that take part in tool
	void Disconnect();

	//-- cleanUP (free all usage objects and relation)
	void	CleanUP();

private:
	void		EventButtonConnectClick( HISender pSender, HKEvent pEvent );
	void		EventButtonUseClick( HISender pSender, HKEvent pEvent );
	void		EventButtonApplyClick( HISender pSender, HKEvent pEvent );

	void		EventButtonSwitchClick( HISender pSender, HKEvent pEvent );

	void		EventButtonPrevClick( HISender pSender, HKEvent pEvent );
	void		EventButtonKeyClick( HISender pSender, HKEvent pEvent );
	void		EventButtonKeyRemoveClick( HISender pSender, HKEvent pEvent );
	void		EventButtonNextClick( HISender pSender, HKEvent pEvent );
	void		EventButtonAbsoluteClick( HISender pSender, HKEvent pEvent );

	void		EventToolIdle		( HISender pSender, HKEvent pEvent );
	void		EventToolShow		( HISender pSender, HKEvent pEvent );
	void		EventToolPaint		( HISender pSender, HKEvent pEvent );
	void		EventToolResize		( HISender pSender, HKEvent pEvent );
	void		EventToolInput		( HISender pSender, HKEvent pEvent );

	void		EventTabPanelChange			( HISender pSender, HKEvent pEvent );

	void		EventIdleAnim		( HISender pSender, HKEvent pEvent );
	void		EventPropertyChange ( HISender pSender, HKEvent pEvent );

	//-- events for finger manipulations
	void		EventRotationChange ( HISender pSender, HKEvent pEvent );
	void		EventSpreadChange ( HISender pSender, HKEvent pEvent );
	void		EventWeightChange ( HISender pSender, HKEvent pEvent );
	void		EventDampChange ( HISender pSender, HKEvent pEvent );
	void		EventInheritChange ( HISender pSender, HKEvent pEvent );

	void		EventNew( HISender pSender, HKEvent pEvent );
	void		EventNewCompleted( HISender pSender, HKEvent pEvent );
	void		EventOpenCompleted( HISender pSender, HKEvent pEvent );
	void		EventFileExit( HISender pSender, HKEvent pEvent );

	void		EventSceneChange( HISender pSender, HKEvent pEvent );

private:
	FBApplication		mApp;
	FBCharacter			*mLastCharacter;
	FBSystem			mSystem;

	FBConstraintRelation	*mConstraintLeft;			// for left hand
	FBConstraintRelation	*mConstraintRight;			// for right hand

	//-- main tool states
	bool				mConnectState;
	bool				mUseState;
	bool				mImageState;

	//-------------------------- Main UI
	FBButton			mButtonConnect;
	FBButton			mButtonUse;
	FBButton			mButtonApply;

	FBLayout			mLayout			[4 ];
	FBTabPanel			mTabPanel;

	FBLabel				mLabelAbout;

	//-- layout 0
	FBButton			mImageSwitcher;				// switch between hands/character images

	CHandsImage			mHandsImage;
	CCharacterImage		mCharacterImage;

	FBButton			mButtonKey;
	FBButton			mButtonKeyRemove;
	FBButton			mButtonPrev;
	FBButton			mButtonNext;
	FBButton			mButtonAbsolute;

	FBLabel				mLabelRotation;
	FBLabel				mLabelSpread;
	FBLabel				mLabelDamping;
	FBLabel				mLabelWeight;
	FBLabel				mLabelInherit;

	FBEditNumber		mEditRotation;
	FBEditNumber		mEditSpread;
	FBEditNumber		mEditDamping;
	FBEditNumber		mEditWeight;
	FBEditVector		mEditInherit;

	int					mLastWeight;
	FBVector3d			mLastInherit;

	//-- variables for relative mode
	double				mPrevRotation;
	double				mPrevSpread;
	double				mPrevWeight;
	double				mPrevDamping;
	FBVector3d			mPrevInherit;

	//-- layout 1,2
	CHand				mHandLeft;
	CHand				mHandRight;

	//-- layout 3
	FBLabel				mLabelThumbAxis;
	FBList				mListThumbAxis;

	double				GetRotation();
	double				GetSpread();
	double				GetDamping();
	double				GetWeight();
	FBVector3d			GetInherit();
};

#endif /* __FB_TOOL_TEMPLATE_TOOL_H__ */
