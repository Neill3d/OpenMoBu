
#ifndef __RIGID_BODIES_H__
#define __RIGID_BODIES_H__

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
//-- Newton SDK
#include "Newton.h"

#include "core.h"

#define	ORRIGIDBODIES__CLASSNAME		RigidBodies
#define ORRIGIDBODIES__CLASSSTR		"RigidBodies"

class RigidBodies : public FBConstraint
{
	//--- declaration
	FBConstraintDeclare( RigidBodies, FBConstraint );

public:
	//--- Creation & Destruction
	virtual bool			FBCreate();							//!< Constructor.
	virtual void			FBDestroy();						//!< Destructor.

	//--- Animation node management
	virtual void			RemoveAllAnimationNodes();			//!< Remove animation nodes.
	virtual void			SetupAllAnimationNodes();			//!< Setup animation nodes.


	virtual bool			SaveState			();				//!< Save current state.
	virtual void			RestoreState		();				//!< Restore current state.
	virtual void			SnapSuggested		();				//!< Suggest 'snap'.
	virtual void			FreezeSuggested		();				//!< Suggest 'freeze'.

	//--- Constraint Status interface
	virtual bool			Disable		( HFBModel pModel );	//!< Disable the constraint on a model.
	virtual void			Snap		();						//!< 'Snap' constraint.

	virtual bool			ReferenceAddNotify		( int pGroupIndex, HFBModel pModel );					//!< Reference added: Callback.
	virtual bool			ReferenceRemoveNotify	( int pGroupIndex, HFBModel pModel );					//!< Reference removed: Callback.

	//--- Real-Time Engine
	//! Real-time evaluation engine function.
	virtual bool			AnimationNodeNotify		( HFBAnimationNode pAnimationNode, HFBEvaluateInfo pEvaluateInfo, HFBConstraintInfo pConstraintInfo );

private:

	FBMatrix			bodyMatrix;
	RigidBodyUserData	*userData;
	FBVector3d			boxSize;
	double				lastTimeDouble;

	bool				mFirstTime;
	bool				mSetupTime;

	// layout
	int		mGroupSource;
	int		mGroupConstrain;

	HFBAnimationNode	mSourceTranslation;			//!< AnimationNode: INPUT  -> Source.
	HFBAnimationNode	mConstrainedTranslation;	//!< AnimationNode: OUTPUT -> Constraint.

	HFBModel				GetSourceModel(int index);
	HFBModel				GetConstraintModel();

	void				PrepareBodies();
};

#endif