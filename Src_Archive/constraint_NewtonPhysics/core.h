
#ifndef CORE_H_
#define CORE_H_


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


#include "Newton.h"

struct RigidBodyUserData
{
	FBMatrix		initialMatrix;
	FBVector3d		initialScale;

	//HFBAnimationNode	modelPosition;
	//HFBAnimationNode	modelRotation;
};

//-- return pointer to the newton physics world object
NewtonWorld *GetWorld();

//-- make a world step
void WorldUpdate( double step );

//-- rigid body
int	AddRigidBody( NewtonBody *pBody );
int GetRigidBodyIndex( NewtonBody *pBody );
NewtonBody *GetRigidBodyNode( int index );
void RemoveRigidBody( NewtonBody *pBody );
void RemoveAllRigidBodies();

//-- joints
void RemoveRigidJoint( NewtonJoint *pJoint );


#endif