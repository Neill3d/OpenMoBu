
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
#include <fbsdk/fbarray.h>

#ifdef KARCH_ENV_WIN
	#include <windows.h>
#endif

#include "core.h"


NewtonWorld							*g_pWorld;
FBArrayTemplate <NewtonBody*>		g_pRigidBodys;


//----------------------------------------------------- GLOBAL FUNCTIONS
NewtonWorld *GetWorld() {
	return g_pWorld;
}



//-- make a world step
void WorldUpdate( double step )
{
	NewtonUpdate( g_pWorld, step );
}


int	AddRigidBody( NewtonBody *pBody )
{
	return g_pRigidBodys.Add(pBody);
}

NewtonBody *GetRigidBodyNode( int index )
{
	if (index > -1) 
		return g_pRigidBodys[index];

	return NULL;
}

int GetRigidBodyIndex( NewtonBody *pBody )
{
	return g_pRigidBodys.Find( pBody );
}

//-- remove rigid body
void RemoveRigidBody( NewtonBody *pBody )
{
	g_pRigidBodys.Remove(pBody);
	NewtonDestroyBody( g_pWorld, pBody );
}

void RemoveAllRigidBodies()
{
	for (int i=0; i<g_pRigidBodys.GetCount(); ++i)
	{
		if (g_pRigidBodys[i])
		{
			RigidBodyUserData *pUserData;
			pUserData = (RigidBodyUserData*)NewtonBodyGetUserData( g_pRigidBodys[i] );
			NewtonBodySetUserData( g_pRigidBodys[i], NULL );
			delete pUserData;
			pUserData = NULL;

			NewtonDestroyBody( g_pWorld, g_pRigidBodys[i] );
		}
	}
	g_pRigidBodys.Clear();
}

void RemoveRigidJoint( NewtonJoint *pJoint )
{
	NewtonDestroyJoint( g_pWorld, pJoint );
}


//--- Library declaration
FBLibraryDeclare( CRigidBody )
{
	//FBLibraryRegister( CRigidBody );
	//FBLibraryRegister( CRigidJoint );
	FBLibraryRegister( RigidBodyTool );
	FBLibraryRegister( RigidBodies );

	FBLibraryRegister(jointSocket);
}
FBLibraryDeclareEnd;

/************************************************
 *	Library functions.
 ************************************************/
bool FBLibrary::LibInit()		
{ 
	
	g_pWorld = NewtonCreate( NULL, NULL );

	return true; 
}
bool FBLibrary::LibOpen()		{ return true; }
bool FBLibrary::LibReady()		{ return true; }
bool FBLibrary::LibClose()		{ return true; }
bool FBLibrary::LibRelease()	
{ 
	
	if (g_pWorld){
		RemoveAllRigidBodies();
		NewtonDestroy( g_pWorld );
	}

	return true; 
}

/**
*	\mainpage	Box Template
*	\section	intro	Introduction
*	Template showing what needs to be done
*	in order to create a new box/operator.
*/
