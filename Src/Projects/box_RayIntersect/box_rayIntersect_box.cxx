
/////////////////////////////////////////////////////////////////////////////////////////
//
// box_rayIntersect_box.cxx
//
// Sergei <Neill3d> Solokhin 2014-2018
//
// GitHub page - https://github.com/Neill3d/OpenMoBu
// Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
/////////////////////////////////////////////////////////////////////////////////////////


//--- Class declaration
#include "box_rayIntersect_box.h"
#include "math3d.h"

//--- Registration defines
#define BOXRAYINTERSECT__CLASS			BOXRAYINTERSECT__CLASSNAME
#define BOXRAYINTERSECT__NAME			BOXRAYINTERSECT__CLASSSTR
#define	BOXRAYINTERSECT__LOCATION		"Neill3d"
#define BOXRAYINTERSECT__LABEL			"Ray Intersection Beta"
#define	BOXRAYINTERSECT__DESC			"Intersect mesh with a given ray and return u,v coords"

#define BOXSPHERECOORDS__CLASS			BOXSPHERECOORDS__CLASSNAME
#define BOXSPHERECOORDS__NAME			BOXSPHERECOORDS__CLASSSTR
#define	BOXSPHERECOORDS__LOCATION		"Neill3d"
#define BOXSPHERECOORDS__LABEL			"Sphere Coords"
#define	BOXSPHERECOORDS__DESC			"Convert given direction into sphere coords"


//--- implementation and registration
FBBoxImplementation	(	BOXRAYINTERSECT__CLASS		);	// Box class name
FBRegisterBox		(	BOXRAYINTERSECT__NAME,			// Unique name to register box.
						BOXRAYINTERSECT__CLASS,			// Box class name
						BOXRAYINTERSECT__LOCATION,		// Box location ('plugins')
						BOXRAYINTERSECT__LABEL,			// Box label (name of box to display)
						BOXRAYINTERSECT__DESC,			// Box long description.
						FB_DEFAULT_SDK_ICON			);	// Icon filename (default=Open Reality icon)

FBBoxImplementation	(	BOXSPHERECOORDS__CLASS		);	// Box class name
FBRegisterBox		(	BOXSPHERECOORDS__NAME,			// Unique name to register box.
						BOXSPHERECOORDS__CLASS,			// Box class name
						BOXSPHERECOORDS__LOCATION,		// Box location ('plugins')
						BOXSPHERECOORDS__LABEL,			// Box label (name of box to display)
						BOXSPHERECOORDS__DESC,			// Box long description.
						FB_DEFAULT_SDK_ICON			);	// Icon filename (default=Open Reality icon)


/************************************************
 *	Creation
 ************************************************/
bool Box_SphereCoords::FBCreate()
{
	/*
	*	Create the nodes for the box.
	*/
	if( FBBox::FBCreate() )
	{
		// Create the input node.
		mNodeMesh = AnimationNodeInCreate( 0, "TM Node", ANIMATIONNODE_TYPE_VECTOR );
		mRayStart = AnimationNodeInCreate( 1, "RayStart", ANIMATIONNODE_TYPE_VECTOR );
		mRayDirection = AnimationNodeInCreate( 2, "RayDirection", ANIMATIONNODE_TYPE_VECTOR );
		
		// Create the output nodes
		mSphereCoords[0]	= AnimationNodeOutCreate(	4, "Inclination", ANIMATIONNODE_TYPE_NUMBER );
		mSphereCoords[1]	= AnimationNodeOutCreate(	5, "Azimuth", ANIMATIONNODE_TYPE_NUMBER );

		mUVCoords[0]	= AnimationNodeOutCreate(	6, "U", ANIMATIONNODE_TYPE_NUMBER );
		mUVCoords[1]	= AnimationNodeOutCreate(	7, "V", ANIMATIONNODE_TYPE_NUMBER );

		return true;
	}
	return false;
}


/************************************************
 *	Destruction.
 ************************************************/
void Box_SphereCoords::FBDestroy()
{
	/*
	*	Free any user memory associated to box.
	*/
//	FBBox::Destroy();
}

/************************************************
 *	Real-time engine evaluation
 ************************************************/
void ExtractSphereCoords(FBVector3d v, const FBMatrix &m, double &inclination, double &azimuth)
{
	FBMatrix m2(m);
	FBMatrixInverse( m2, m2 );
	
	VectorTransform33(v, m2, v);

	double R = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
	inclination = (R != 0.0) ? acos(v[1] / R) : 0.0;
	azimuth = (v[0] != 0.0) ? atan2(-v[2], -v[0]) : 0.0;
	azimuth += PI;

	//if (azimuth > PI) azimuth = 2*PI - azimuth;
}


bool Box_SphereCoords::AnimationNodeNotify( FBAnimationNode *pAnimationNode, FBEvaluateInfo *pEvaluateInfo )
{
	/*
	*	1. Read the data from the in connector
	*	2. Treat the data as required
	*	3. Write the data to the out connector
	*	4. Return the status (LIVE/DEAD) of the box.
	*/

	FBVector3d		lMeshVector, lPos, lDir;


	//bool			lStatus;	// Status of input node

	// Read the input nodes.
	if (!mNodeMesh->ReadData( lMeshVector, pEvaluateInfo ) ) return false;
	if (!mRayStart->ReadData( lPos, pEvaluateInfo ) ) return false;
	if (!mRayDirection->ReadData( lDir, pEvaluateInfo ) ) return false;

	// If the read was not from a dead node.
	int count = mNodeMesh->GetSrcCount();
	for (int i=0; i<count; ++i)
	{
		FBPlug *pPlug = mNodeMesh->GetSrc(i);
		pPlug = pPlug->GetOwner();
			
		if (pPlug->Is( FBModelPlaceHolder::TypeInfo ) )
		{
			FBModelPlaceHolder *pPlaceHolder = (FBModelPlaceHolder*) pPlug;
			FBModel *pModel = pPlaceHolder->Model;

			FBMatrix modelMatrix;
			pModel->GetMatrix(modelMatrix);

			FBVector3d vec;
			vec = VectorSubtract(lDir, lPos);

			double inclination, azimuth;
			ExtractSphereCoords(vec, modelMatrix, inclination, azimuth);

			double u,v;
			u = azimuth / (2.0 * PI);
			v = 2.0 * inclination / (2.0 * PI);

			mSphereCoords[0]->WriteData( &inclination, pEvaluateInfo );
			mSphereCoords[1]->WriteData( &azimuth, pEvaluateInfo );

			mUVCoords[0]->WriteData( &u, pEvaluateInfo );
			mUVCoords[1]->WriteData( &v, pEvaluateInfo );
			return true;
		}
	}
	

	return false;
}


/************************************************
 *	FBX Storage.
 ************************************************/
bool Box_SphereCoords::FbxStore( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Store box parameters.
	*/
	return true;
}


/************************************************
 *	FBX Retrieval.
 ************************************************/
bool Box_SphereCoords::FbxRetrieve( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Retrieve box parameters.
	*/
	return true;
}

/************************************************
 *	Creation
 ************************************************/
bool Box_RayIntersect::FBCreate()
{
	/*
	*	Create the nodes for the box.
	*/
	if( FBBox::FBCreate() )
	{
		// Create the input node.
		mNodeMesh = AnimationNodeInCreate( 0, "Mesh", ANIMATIONNODE_TYPE_VECTOR );
		mRayStart = AnimationNodeInCreate( 1, "RayStart", ANIMATIONNODE_TYPE_VECTOR );
		mRayDirection = AnimationNodeInCreate( 2, "RayDirection", ANIMATIONNODE_TYPE_VECTOR );
		
		// Create the output nodes
		mIntersectPoint	= AnimationNodeOutCreate(	3, "Point", ANIMATIONNODE_TYPE_VECTOR );
		mIntersectNormal= AnimationNodeOutCreate(	4, "Normal", ANIMATIONNODE_TYPE_VECTOR);
		mUVCoords[0]	= AnimationNodeOutCreate(	5, "U", ANIMATIONNODE_TYPE_NUMBER );
		mUVCoords[1]	= AnimationNodeOutCreate(	6, "V", ANIMATIONNODE_TYPE_NUMBER );

		return true;
	}
	return false;
}

/************************************************
 *	Destruction.
 ************************************************/
void Box_RayIntersect::FBDestroy()
{
	/*
	*	Free any user memory associated to box.
	*/
//	FBBox::Destroy();
}


// intersect_RayTriangle(): intersect a ray with a 3D triangle
//    Input:  a ray R, and a triangle T
//    Output: *I = intersection point (when it exists)
//    Return: -1 = triangle is degenerate (a segment or point)
//             0 = disjoint (no intersect)
//             1 = intersect in unique point I1
//             2 = are in the same plane

#define SMALL_NUM  0.00000001 // anything that avoids division overflow

int intersect_RayTriangle( FBVector3d R0, FBVector3d R1, FBVector3d V0, FBVector3d V1, FBVector3d V2, FBVector3d &I )
{
    FBVector3d    u, v, n;             // triangle vectors
    FBVector3d    dir, w0, w;          // ray vectors
    double			r, a, b;             // params to calc ray-plane intersect

    // get triangle edge vectors and plane normal
    u =  VectorSubtract(V1, V0);
	v = VectorSubtract(V2, V0);
    n = CrossProduct(u, v);             // cross product
    if ( VectorLength(n) == 0.0)            // triangle is degenerate
        return -1;                 // do not deal with this case

	dir = VectorSubtract(R1, R0);             // ray direction vector
	w0 = VectorSubtract( R0, V0 );
    a = -DotProduct(n,w0);
    b = DotProduct(n,dir);
    if (fabs(b) < SMALL_NUM) {     // ray is parallel to triangle plane fabs(b) < SMALL_NUM
        if (a == 0)                // ray lies in triangle plane
            return 2;
        else return 0;             // ray disjoint from plane
    }
	//else return 0;

    // get intersect point of ray with triangle plane
    r = a / b;
    if (r < 0.0)                   // ray goes away from triangle
        return 0;                  // => no intersect
    // for a segment, also test if (r > 1.0) => no intersect
	I = dir;
	//VectorNormalize(I);
	VectorMult( I, r );
	I = VectorAdd( R0, I );           // intersect point of ray and plane

    // is I inside T?
    double    uu, uv, vv, wu, wv, D;
    uu = DotProduct(u,u);
    uv = DotProduct(u,v);
    vv = DotProduct(v,v);
	w = VectorSubtract( I, V0 );
    wu = DotProduct(w,u);
    wv = DotProduct(w,v);
    D = uv * uv - uu * vv;

    // get and test parametric coords
    double s, t;
    s = (uv * wv - vv * wu) / D;
    if (s < 0.0 || s > 1.0)        // I is outside S
        return 0;
    t = (uv * wu - uu * wv) / D;
    if (t < 0.0 || (s + t) > 1.0)  // I is outside T
        return 0;

    return 1;                      // I is in T
}


//-----------------------------------------------------------------------------
//-- function Is point inside polygon
//-- numVertices, vertices - polygon data
//-- p - point for test
//-- use on 2D plane ONLY! (Z coord not used)
/*
bool IsInside(int numVertices, FBVector3d vertices, p
{ 
    count = 0; // number of ray/edge intersections

    for(i=1; i<=numVertices; i++) {
        j = i % numVertices + 1;
        
        if (vertices[i].y==vertices[j].y)
            continue;
        if (vertices[i].y > p.y && vertices[j].y > p.y)
            continue;
        if (vertices[i].y < p.y && vertices[j].y < p.y)
            continue;
        if ( max(vertices[i].y, vertices[j].y) == p.y )
            count++;
        else
        if ( min(vertices[i].y, vertices[j].y) == p.y)
            continue;
        else
        {
            t = (p.y - vertices[i].y) / (vertices[j].y - vertices[i].y);
            if (vertices[i].x + t * (vertices[j].x - vertices[i].x) >= p.x)
                count++;
        }
    
    }
        
    //-- if (count) point sit. inside polygon
    return (count & 1);
}
*/

double AngleBetween(const FBVector3d &a, const FBVector3d &b)
{
	
	return acos( DotProduct(a, b) );
}

bool InsidePolygon( int verticeCount, FBVector3d poly_verts[], FBVector3d Point)
{
	
#define MATCH_FACTOR  0.99
				
	double Angle = 0.0;
	FBVector3d vA(0.0, 0.0, 0.0);
	FBVector3d vB(0.0, 0.0, 0.0);
	for (int i = 0; i<verticeCount; ++i)
	{
		vA = VectorSubtract(poly_verts[i], Point);
		 
		int reminder = i % verticeCount;
		reminder++;
		
		vB = VectorSubtract(poly_verts[reminder], Point);
		VectorNormalize(vA);
		VectorNormalize(vB);
		Angle +=  AngleBetween(vA, vB);
	}
	//Angle = degToRad Angle
	if (Angle >= (MATCH_FACTOR * (2*PI)) ) return true;
	else return false;
}

#define MAX_POLY_VERTS	4
#define	TOO_FAR		1000000.0

bool RayIntersector::intersectModel(FBModel *pModel)
{
	if (!pModel) return false;

	FBMatrix m, invm;
	pModel->GetMatrix(m);
	mGeometry = pModel->Geometry;
	mMesh = pModel->TessellatedMesh;
	
	FBVector3d dir;
	dir = VectorSubtract(mDir, mPos);
	//VectorNormalize(dir);

	// we have to pass normalized ray direction
	FBVector3d R0, R1;
	//invm = m;
	//MatrixFastInverse(invm);
	FBMatrixInverse(invm, m);
	VectorTransform(mPos, invm, R0);
	VectorTransform(mDir, invm, R1);

	FBVector3d a,b,c,d, P;
	double cloDist = TOO_FAR;
	int cloId = 0;
	info.finded = false;

	FBVertex	polyverts[MAX_POLY_VERTS];
	int facetn = mMesh->PolygonCount();
	for (int i=0; i<facetn; ++i)
	{
		int count = mMesh->PolygonVertexCount(i);
		if ((count < 3) || (count > MAX_POLY_VERTS)) continue; // skip innormal polys

		double di=0.0, u, v;

		for (int j=0; j<count; ++j)
			polyverts[j] = mGeometry->VertexGet( mMesh->PolygonVertexIndex(i,j) );
		
		if (count == 3)
		{
			a = FBVector3d(polyverts[2][0], polyverts[2][1], polyverts[2][2]);
			b = FBVector3d(polyverts[0][0], polyverts[0][1], polyverts[0][2]);
			c = FBVector3d(polyverts[1][0], polyverts[1][1], polyverts[1][2]);

			int test = intersect_RayTriangle(R0, R1, a, b, c, P );

			if (test > 0) di = VectorLength( VectorSubtract(P, mPos) );

			if (test > 0 && (di < cloDist) ) {
				// we found new closest intersection point
				cloDist = di;
				cloId = i;

				info.facet = i;
				VectorTransform(P, m, info.point);
			}
		}
		else if (count == 4) 
		{
			a = FBVector3d(polyverts[0][0], polyverts[0][1], polyverts[0][2]);
			b = FBVector3d(polyverts[1][0], polyverts[1][1], polyverts[1][2]);
			c = FBVector3d(polyverts[2][0], polyverts[2][1], polyverts[2][2]);
			d = FBVector3d(polyverts[3][0], polyverts[3][1], polyverts[3][2]);
			

			// test second tri in quad
			di = calcIntersection(R0, dir, a, b, c); 

			if (di >= 0.0 && di < cloDist) {

				FBVector3d poly_verts[4] = { a,b,c,d };
				P = dir;
				VectorMult( P, di );
				P = VectorAdd( P, R0 );
				
				if (InsidePolygon(4, poly_verts, P) )
				{
					// we found new closest intersection point
					cloDist = di;
					cloId = i;

					info.facet = i;
					VectorTransform(P, m, info.point);
				}
			}
		}
	}

	// for result let's calculate
	if (cloDist < TOO_FAR)
	{
		info.finded = true;
		/*
		// calculate intersection point
		int count = mMesh->PolygonVertexCount(info.facet);

		for (int j=0; j<count; ++j)
			polyverts[j] = mGeometry->VertexGet( mMesh->PolygonVertexIndex(info.facet,j) );
		FBVector3d a,b,c;
		a = FBVector3d(polyverts[0][0], polyverts[0][1], polyverts[0][2]);
		b = FBVector3d(polyverts[1][0], polyverts[1][1], polyverts[1][2]);
		c = FBVector3d(polyverts[2][0], polyverts[2][1], polyverts[2][2]);
		*/
		
		return true;
	}

	return false;
}

double RayIntersector::calcIntersection( FBVector3d R0, FBVector3d R1, FBVector3d a, FBVector3d b, FBVector3d c )
{
	FBVector4d plane;
	double di;

	plane = PlaneMake( a, b, c ); 

	if (!RayIntersection( plane, R0, R1, di ) )
		return TOO_FAR;

	return di;
}

bool RayIntersector::checkInside( FBVertex a, FBVertex b, FBVertex c, FBVector3d p )
{
	double u0, u1, u2;
	double v0, v1, v2;

	return false;
}

#ifndef EPSILON
#define EPSILON 0.000001
#endif

int RayIntersector::intersect_triangle( FBVector3d orig, FBVector3d dir, FBVector3d vert0, FBVector3d vert1, FBVector3d vert2,
		double &t, double &u, double &v )
{
	FBVector3d edge1, edge2, tvec, pvec, qvec;
	double det, inv_det;

	// find vectors for two edges sharing vert0
	edge1 = VectorSubtract(vert1, vert0);
	edge2 = VectorSubtract(vert2, vert0);

	// begin calculation determinant - also used to calculate U parameter
	pvec = CrossProduct(dir, edge2);

	// if determinant is near zero, ray lies in plane of tringle
	det = DotProduct(edge1, pvec);

#ifdef TEST_CULL			// test_cull for culling single side tris
	if (det < EPSILON) return 0;

	// calculate distance from vert0 to ray origin
	tvec = VectorSubtract( orig, vert0 );

	// calculate U parameter and test bounds
	u = DotProduct( tvec, pvec );
	if (u < 0.0 || u > det) return 0;

	// prepare to test v parameter
	qvec = CrossProduct(tvec, edge1);

	// calculate v parameter and test bounds
	v = DotProduct(dir, qvec);
	if (v < 0.0 || (u+v) > det) return 0;

	// calculate t, scale parameters, ray intersects triangle
	t = DotProduct( edge2, qvec );
	inv_det = 1.0 / det;
	t *= inv_det;
	u *= inv_det;
	v *= inv_det;
#else
	if (det > -EPSILON && det < EPSILON) return 0;
	inv_det = 1.0 / det;

	// calculate distance from vert0 to ray origin
	tvec = VectorSubtract( orig, vert0 );

	// calculate U parameter and test bounds
	u = DotProduct( tvec, pvec ) * inv_det;
	if (u < 0.0 || u > 1.0) return 0;

	// prepare to test v parameter
	qvec = CrossProduct(tvec, edge1);

	// calculate v parameter and test bounds
	v = DotProduct(dir, qvec) * inv_det;
	if (v < 0.0 || (u+v) > 1.0) return 0;

	// calculate t, scale parameters, ray intersects triangle
	t = DotProduct( edge2, qvec ) * inv_det;
#endif

	return 1;
}


/************************************************
 *	Real-time engine evaluation
 ************************************************/
bool Box_RayIntersect::AnimationNodeNotify( FBAnimationNode *pAnimationNode, FBEvaluateInfo *pEvaluateInfo )
{
	/*
	*	1. Read the data from the in connector
	*	2. Treat the data as required
	*	3. Write the data to the out connector
	*	4. Return the status (LIVE/DEAD) of the box.
	*/


	FBVector3d		lMeshVector, lPos, lDir;

	// Read the input nodes.
	if (!mNodeMesh->ReadData( lMeshVector, pEvaluateInfo ) ) 
		return false;
	if (!mRayStart->ReadData( lPos, pEvaluateInfo ) ) 
		return false;
	if (!mRayDirection->ReadData( lDir, pEvaluateInfo ) ) 
		return false;

	// If the read was not from a dead node.
	const int count = mNodeMesh->GetSrcCount();
	for (int i=0; i<count; ++i)
	{
		FBPlug *pPlug = mNodeMesh->GetSrc(i);
		pPlug = pPlug->GetOwner();
			
		if (pPlug->Is( FBModelPlaceHolder::TypeInfo ) )
		{
			FBModelPlaceHolder *pPlaceHolder = (FBModelPlaceHolder*) pPlug;
			FBModel *pModel = pPlaceHolder->Model;

			// let's use a native mobu method for a ray casting
			FBTVector pos4(lPos[0], lPos[1], lPos[2]);
			FBTVector dir4(lDir[0], lDir[1], lDir[2]);
			FBTVector intersectPos, intersectNormal;

			FBMatrix tm, invtm;
			pModel->GetMatrix(tm, kModelTransformation_Geometry);
			pModel->GetMatrix(invtm, kModelInverse_Transformation_Geometry);
			FBVectorMatrixMult(pos4, invtm, pos4);
			FBVectorMatrixMult(dir4, invtm, dir4);

			if (nullptr != pModel && true == pModel->ClosestRayIntersection(pos4, dir4, intersectPos, (FBNormal&)intersectNormal))
			{
				FBVectorMatrixMult(intersectPos, tm, intersectPos);

				tm[12] = tm[13] = tm[14] = 0.0;
				FBVectorMatrixMult(intersectNormal, tm, intersectNormal);
				if (FBLength(intersectNormal) > 0.0)
				{
					FBMult(intersectNormal, intersectNormal, 1.0 / FBLength(intersectNormal));
				}

				mIntersectPoint->WriteData(intersectPos, pEvaluateInfo);
				mIntersectNormal->WriteData(intersectNormal, pEvaluateInfo);
				return true;
			}

			/*
			FBGeometry *pGeom = pModel->Geometry;

			RayIntersector ray(lPos, lDir);
			if (ray.intersectModel(pModel) )
			{
				mIntersectPoint->WriteData( ray.info.point, pEvaluateInfo );
				return true;
			}
			*/
		}
	}
	
	return false;
}


/************************************************
 *	FBX Storage.
 ************************************************/
bool Box_RayIntersect::FbxStore( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Store box parameters.
	*/
	return true;
}


/************************************************
 *	FBX Retrieval.
 ************************************************/
bool Box_RayIntersect::FbxRetrieve( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Retrieve box parameters.
	*/
	return true;
}
