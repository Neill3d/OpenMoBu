
/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://code.google.com/p/motioncodelibrary/wiki/LICENSE
//
// GitHub repository - https://github.com/Neill3d/MoBu
//
// Author Sergey Solohin (Neill3d) 2014
//  e-mail to: s@neill3d.com
//		www.neill3d.com
/////////////////////////////////////////////////////////////////////////////////////////

#ifndef __BOX_RAY_INTERSECT_BOX_H__
#define __BOX_RAY_INTERSECT_BOX_H__

/**	\file	box_rayIntersect_box.h
*/

//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Registration defines
#define	BOXRAYINTERSECT__CLASSNAME		Box_RayIntersect
#define BOXRAYINTERSECT__CLASSSTR		"Box_RayIntersect"

#define	BOXSPHERECOORDS__CLASSNAME		Box_SphereCoords
#define BOXSPHERECOORDS__CLASSSTR		"Box_SphereCoords"

class RayIntersector
{
public:
	RayIntersector( FBVector3d pos, FBVector3d dir )
		: mPos(pos)
		, mDir(dir)
	{
		mGeometry = nullptr;
		mMesh = nullptr;
	}

	bool intersectModel( HFBModel pModel ); 

	struct IntersectionInfo
	{
		bool		finded;
		int			facet;	// intersection face
		FBVector3d	point;	// ray intersection point
		double		u;		// facet u coord
		double		v;		// facet v coord
	} info;

public:
	HFBGeometry		mGeometry;
	HFBMesh			mMesh;

	FBVector3d		mPos;
	FBVector3d		mDir;

	// calc intersection with a tri
	double calcIntersection( FBVector3d R0, FBVector3d R1, FBVector3d a, FBVector3d b, FBVector3d c );
	// p - point on plane
	bool checkInside( FBVertex a, FBVertex b, FBVertex c, FBVector3d p );

	int intersect_triangle( FBVector3d orig, FBVector3d dir, FBVector3d vert0, FBVector3d vert1, FBVector3d vert2,
		double &t, double &u, double &v );
};


/**	Template for FBBox class.
*/
class Box_RayIntersect : public FBBox
{
	//--- box declaration.
	FBBoxDeclare( Box_RayIntersect, FBBox );

public:
	//! creation function.
	virtual bool FBCreate();

	//! destruction function.
	virtual void FBDestroy();

	//! Overloaded FBBox real-time evaluation function.
	virtual bool AnimationNodeNotify(HFBAnimationNode pAnimationNode,HFBEvaluateInfo pEvaluateInfo);

	//! FBX Storage function
	virtual bool FbxStore( HFBFbxObject pFbxObject, kFbxObjectStore pStoreWhat );

	//! FBX Retrieval function
	virtual bool FbxRetrieve(HFBFbxObject pFbxObject, kFbxObjectStore pStoreWhat );

private:

	HFBAnimationNode	mNodeMesh;		//!> input - mesh node
	HFBAnimationNode	mRayStart;		//!> input - ray start position
	HFBAnimationNode	mRayDirection;	//!> input - ray direction point

	HFBAnimationNode	mIntersectPoint;	//!> output - mesh intersection world point
	HFBAnimationNode	mUVCoords[2];		//!> output - intersection u,v points		
};

class Box_SphereCoords : public FBBox
{
	//--- box declaration.
	FBBoxDeclare( Box_SphereCoords, FBBox );

public:
	//! creation function.
	virtual bool FBCreate();

	//! destruction function.
	virtual void FBDestroy();

	//! Overloaded FBBox real-time evaluation function.
	virtual bool AnimationNodeNotify(HFBAnimationNode pAnimationNode,HFBEvaluateInfo pEvaluateInfo);

	//! FBX Storage function
	virtual bool FbxStore( HFBFbxObject pFbxObject, kFbxObjectStore pStoreWhat );

	//! FBX Retrieval function
	virtual bool FbxRetrieve(HFBFbxObject pFbxObject, kFbxObjectStore pStoreWhat );

private:

	HFBAnimationNode	mNodeMesh;		//!> input - mesh node - use this mesh to untransform a ray and sphere coords
	HFBAnimationNode	mRayStart;		//!> input - ray start position
	HFBAnimationNode	mRayDirection;	//!> input - ray direction point

	HFBAnimationNode	mSphereCoords[2];		//!> output - inclination, azimuth
	HFBAnimationNode	mUVCoords[2];		//!> output - result u,v coords of a point
};


#endif /* __BOX_RAY_INTERSECT_BOX_H__ */
