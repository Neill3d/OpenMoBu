#ifndef __JOINT_SOCKET_H__
#define __JOINT_SOCKET_H__

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

#define	ORJOINTSOCKET__CLASSNAME	jointSocket
#define ORJOINTSOCKET__CLASSSTR		"jointSocket"

class jointSocket : public FBConstraint
{
	//--- declaration
	FBConstraintDeclare( jointSocket, FBConstraint );

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

	bool			mActive;

	NewtonJoint		*constraint;

	// layout
	int				mGroup;

	HFBAnimationNode	mModelChild;			//!< AnimationNode: INPUT  -> Source.
	HFBAnimationNode	mModelParent;			//!< AnimationNode: INPUT  -> Source.
	HFBAnimationNode	mPivot;	//!< AnimationNode: OUTPUT -> Constraint.
};
#endif