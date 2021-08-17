

/**	\file	constraintAttachment.cxx

Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declarations
#include "constraintAttachment.h"

#include <vector>
#include "pointTriangle.h"

//--- Registration defines
#define	CCONSTRAINTATTACHMENT__CLASS	CCONSTRAINTATTACHMENT__CLASSNAME
#define CCONSTRAINTATTACHMENT__NAME		"Attachment Beta (Neill3d)"
#define CCONSTRAINTATTACHMENT__LABEL	"Surface Attachment Beta (Neill3d)"
#define CCONSTRAINTATTACHMENT__DESC		"Attachment (Neill3d)"

//--- implementation and registration
FBConstraintImplementation(CCONSTRAINTATTACHMENT__CLASS);
FBRegisterConstraint(	CCONSTRAINTATTACHMENT__NAME,
						CCONSTRAINTATTACHMENT__CLASS,
						CCONSTRAINTATTACHMENT__LABEL,
						CCONSTRAINTATTACHMENT__DESC,
						FB_DEFAULT_SDK_ICON				);	// Icon filename (default=Open Reality icon)


/////////////////////////////////////////////////////////////////////////////////////
// From MoCode Library (Neill3d)

FBMatrix NormalToRotation(FBTVector normal) 
{
    // Find a vector in the plane
	FBTVector tangent0, tangent1;

	FBMult( tangent0, normal, FBTVector(1.0, 0.0, 0.0) );
    
    if (FBDot(tangent0, tangent0) < 0.001)
        FBMult(tangent0, normal, FBTVector(0.0, 1.0, 0.0));
    
	FBMult( tangent0, tangent0, 1.0 / FBLength(tangent0) );

    // Find another vector in the plane
    FBMult( tangent1, normal, tangent0 );
	FBMult( tangent1, tangent1, 1.0 / FBLength(tangent1) );
	
    // Construct a 3x3 matrix by storing three vectors in the columns of the matrix
    FBMatrix m;
	m.Identity();
	
	m[0] = tangent0[0];
	m[1] = tangent0[1];
	m[2] = tangent0[2];

	m[4] = tangent1[0];
	m[5] = tangent1[1];
	m[6] = tangent1[2];

	m[8] = normal[0];
	m[9] = normal[1];
	m[10] = normal[2];

	return m;
}

FBQuaternion getRotationQuat(const FBTVector& from, const FBTVector& to)
{     
	FBQuaternion result; 
	FBTVector H;
	FBAdd( H, from, to );
	FBMult( H, H, 1.0 / FBLength(H) );

    result[3] = FBDot(from, H);
    result[0] = from[1]*H[2] - from[2]*H[1];     
    result[1] = from[2]*H[0] - from[0]*H[2];     
    result[2] = from[0]*H[1] - from[1]*H[0];     
    return result;
}

void VectorNormalize( FBVector3d	&v )
{
	double len = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);

	if (len != 0.0)
	{
		len = 1.0 / len;
		v[0] *= len;
		v[1] *= len;
		v[2] *= len;
	}
}


/*
 If a point lies in the interior of the triangle, all of the Barycentric coordinates lie in the open interval (0,1).
 If a point lies on an edge of the triangle but not at a vertex, one of the area coordinates \lambda_{1...3} 
 (the one associated with the opposite vertex) is zero, while the other two lie in the open interval (0,1). 
 If the point lies on a vertex, the coordinate associated with that vertex equals 1 and the others equal zero. 
 Finally, if the point lies outside the triangle at least one coordinate is negative.
 */

// Compute barycentric coordinates (u, v, w) for
// point p with respect to triangle (a, b, c)
void Barycentric(const FBVector3d &p, const FBVertex &a, const FBVertex &b, const FBVertex &c, double &u, double &v, double &w)
{
	FBTVector v0, v1, v2;

	FBSub(v0, FBTVector(b[0], b[1], b[2], 1.0), FBTVector(a[0], a[1], a[2], 1.0));
	FBSub(v1, FBTVector(c[0], c[1], c[2], 1.0), FBTVector(a[0], a[1], a[2], 1.0));
	FBSub(v2, FBTVector(p[0], p[1], p[2], 1.0), FBTVector(a[0], a[1], a[2], 1.0));

    double d00 = FBDot(v0, v0);
    double d01 = FBDot(v0, v1);
    double d11 = FBDot(v1, v1);
    double d20 = FBDot(v2, v0);
    double d21 = FBDot(v2, v1);
    double denom = d00 * d11 - d01 * d01;
    v = (d11 * d20 - d01 * d21) / denom;
    w = (d00 * d21 - d01 * d20) / denom;
    u = 1.0 - v - w;
}

double MyPlaneDistance(const FBVector4d &plane, const FBVector3d &point)
{
	return plane[0] * point[0] + plane[1] * point[1] + plane[2] * point[2] + plane[3];
}


//If you work with a plane defined by point and normal
void ClosestPointOnPlane( FBVector3d &result, const FBVector4d &plane, const FBVector3d &point )
{
	FBVector3d	normal(plane[0], plane[1], plane[2]);
	double distance = MyPlaneDistance(plane, point);
	result = FBVector3d(point[0] - distance*normal[0], point[1] - distance*normal[1], point[2] - distance*normal[2]);
}

// PlaneMake (3 points, hmg)
//
void PlaneMake(FBVector4d &plane, const FBVertex &p1, const FBVertex &p2, FBVertex &p3)
{
	FBVector4d	v1,v2, normal;

	FBSub( v1, FBVector4d(p2[0], p2[1], p2[2], 1.0), FBVector4d(p1[0], p1[1], p1[2], 1.0) );
	FBSub( v2, FBVector4d(p3[0], p3[1], p3[2], 1.0), FBVector4d(p1[0], p1[1], p1[2], 1.0) );
	FBMult(normal, v1, v2);
	
	double len = FBLength(normal);
	if (len != 0.0)
		FBMult(normal, normal, 1.0 / len);

	plane = FBVector4d( normal[0], normal[1], normal[2], -FBDot( FBVector4d(p1[0], p1[1], p2[2], 1.0), normal) );
}


bool PrepareTriangleList(std::vector<DistancePointTriangleExact::Triangle> &triangles, const float *modelScaling, FBMatrix &modelMatrix, FBModelVertexData *geometry)
{

	if (geometry == nullptr)
		return false;

	//dgMatrix   localMatrix = dgGetIdentityMatrix();
	
	//float localMatrix[16];
	//FBMatrix matrix(modelMatrix);
	//matrix.Identity();
	//FBMatrixTodMatrix(matrix, localMatrix);
	
	//int vertexCount = geometry->GetVertexCount();
	//int stride = sizeof(float) * 4;
	const FBVertex *vertices = (FBVertex*) geometry->GetVertexArray( kFBGeometryArrayID_Point );
	
	//unsigned int dublicateCount=0;
	//const int *dublicateMap = geometry->GetVertexArrayDuplicationMap( dublicateCount );
	//int originVertexCount = vertexCount - dublicateCount;

	//
	// iterate the entire geometry an build the collision

	const int subpatches = geometry->GetSubPatchCount();
	const int *indices = geometry->GetIndexArray();

	int totalTris = 0;

	for (int i=0; i<subpatches; ++i)
	{
		// continue if the patch size is not divisible by three
		if (geometry->GetSubPatchIndexSize(i) % 3)
			continue;
		totalTris += geometry->GetSubPatchIndexSize(i) / 3;
	}

	triangles.resize(totalTris);
	auto iter = triangles.begin();

	FBVertex vertex;

	for (int i=0; i<subpatches; ++i)
	{
		const int offset = geometry->GetSubPatchIndexOffset(i);
		const int size = geometry->GetSubPatchIndexSize(i);

		// continue if the patch size is not divisible by three
		if (geometry->GetSubPatchIndexSize(i) % 3)
			continue;

		for (int j=0; j<size; j+=3)
		{
			FBVertexMatrixMult( vertex, modelMatrix, vertices[ indices[offset + j] ] );
			iter->v[0] = vec3(vertex);
			iter->index[0] = indices[offset + j];

			//if (iter->index[0] >= originVertexCount)
			//{
			//	iter->index[0] = dublicateMap[iter->index[0]-originVertexCount];
			//}

			FBVertexMatrixMult( vertex, modelMatrix, vertices[ indices[offset + j + 1] ] );
			iter->v[1] = vec3(vertex);
			iter->index[1] = indices[offset + j + 1];
			
			//if (iter->index[1] >= originVertexCount)
			//{
			//	iter->index[1] = dublicateMap[iter->index[1]-originVertexCount];
			//}

			FBVertexMatrixMult( vertex, modelMatrix, vertices[ indices[offset + j + 2] ] );
			iter->v[2] = vec3(vertex);
			iter->index[2] = indices[offset + j + 2];
			
			//if (iter->index[2] >= originVertexCount)
			//{
			//	iter->index[2] = dublicateMap[iter->index[2]-originVertexCount];
			//}

			iter++;
		}
	}
	
	return true;
}

bool PrepareVerticesList( std::vector<vec3> &vertices, const float *modelScaling, FBMatrix &modelMatrix, FBModelVertexData *geometry )
{
	if (nullptr == geometry || 0 == geometry->GetVertexCount())
	{
		vertices.push_back(vec3((float)modelMatrix[12], (float)modelMatrix[13], (float)modelMatrix[14]));
		return true;
	}

	FBVertex *pVertices = (FBVertex*) geometry->GetVertexArray( kFBGeometryArrayID_Point );
	vertices.resize(geometry->GetVertexCount() );

	FBVertex vertex;
	for (int i=0, count=geometry->GetVertexCount(); i<count; ++i)
	{
		FBVertexMatrixMult( vertex, modelMatrix, pVertices[i] );
		vertices[i] = vec3(vertex);
	}

	return true;
}


/************************************************
 *	Creation function.
 ************************************************/

void CConstraintAttachment::SetDebug(HIObject pObject, bool value)
{
	CConstraintAttachment	*pConstraint = FBCast<CConstraintAttachment>(pObject);
	if (pObject && value)
		pConstraint->DoDebug();
}

bool CConstraintAttachment::FBCreate()
{
	Deformer = true;

	// Create reference groups
	mGroupConstrain = ReferenceGroupAdd("Constrain", 1);
	mGroupSource	= ReferenceGroupAdd( "Source Object",	1 );

	mSourceTranslation		= NULL;
	mConstrainedTranslation = NULL;

	mFirstTime = true;

	FBPropertyPublish( this, PutOnSurface, "Put On Surface", nullptr, nullptr );

	FBPropertyPublish( this, OffsetTranslation, "Offset Translation", nullptr, nullptr );
	FBPropertyPublish( this, OffsetRotation, "Offset Rotation", nullptr, nullptr );
	FBPropertyPublish( this, OffsetScaling, "Offset Scaling", nullptr, nullptr );

	FBPropertyPublish( this, BaryCoords, "Bary Coords", nullptr, nullptr );
	FBPropertyPublish( this, AttachmentVertA, "Attach Vert A", nullptr, nullptr );
	FBPropertyPublish( this, AttachmentVertB, "Attach Vert B", nullptr, nullptr );
	FBPropertyPublish( this, AttachmentVertC, "Attach Vert C", nullptr, nullptr );
	
	FBPropertyPublish( this, DebugCmd, "Debug", nullptr, SetDebug );

	PutOnSurface = false;

	OffsetTranslation = FBVector3d(0.0, 0.0, 0.0);
	OffsetRotation = FBVector3d(0.0, 0.0, 0.0);
	OffsetScaling = FBVector3d(1.0, 1.0, 1.0);

	BaryCoords = FBVector3d(0.0, 0.0, 0.0);
	AttachmentVertA = -1;
	AttachmentVertB = -1;
	AttachmentVertC = -1;

#ifndef _DEBUG
	BaryCoords.ModifyPropertyFlag( kFBPropertyFlagHideProperty, true );
	AttachmentVertA.ModifyPropertyFlag( kFBPropertyFlagHideProperty, true );
	AttachmentVertB.ModifyPropertyFlag( kFBPropertyFlagHideProperty, true );
	AttachmentVertC.ModifyPropertyFlag( kFBPropertyFlagHideProperty, true );
#endif

	return true;
}


/************************************************
 *	Destruction function.
 ************************************************/
void CConstraintAttachment::FBDestroy()
{
}


/************************************************
 *	Setup all of the animation nodes.
 ************************************************/
void CConstraintAttachment::SetupAllAnimationNodes()
{
	
	if (ReferenceGet( mGroupSource, 0 ) && ReferenceGet( mGroupConstrain, 0 ) )
	{
		mSourceTranslation		= AnimationNodeOutCreate( 0, ReferenceGet( mGroupSource,	0 ), ANIMATIONNODE_TYPE_TRANSLATION );
		mSourceRotation		= AnimationNodeOutCreate( 1, ReferenceGet( mGroupSource,	0 ), ANIMATIONNODE_TYPE_ROTATION );
		mSourceScaling		= AnimationNodeOutCreate( 2, ReferenceGet( mGroupSource,	0 ), ANIMATIONNODE_TYPE_SCALING );

		mConstrainedTranslation = AnimationNodeInCreate	( 3, ReferenceGet( mGroupConstrain, 0 ), ANIMATIONNODE_TYPE_TRANSLATION );
		mConstrainedRotation = AnimationNodeInCreate( 4, ReferenceGet( mGroupConstrain, 0 ), ANIMATIONNODE_TYPE_ROTATION );

		DeformerBind( ReferenceGet(mGroupSource, 0) );
	}
	
}


/************************************************
 *	Removed all of the animation nodes.
 ************************************************/
void CConstraintAttachment::RemoveAllAnimationNodes()
{
}


/************************************************
 *	FBX storage of constraint parameters.
 ************************************************/
bool CConstraintAttachment::FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	return true;
}


/************************************************
 *	FBX retrieval of constraint parameters.
 ************************************************/
bool CConstraintAttachment::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	return true;
}

/************************************************
 *	Suggest a snap.
 ************************************************/
void CConstraintAttachment::SnapSuggested()
{
	FBVector3d srcContactPoint, dstContactPoint;

	FBModel *modelA = ReferenceGet( mGroupSource, 0);
	FBModel *modelB = ReferenceGet( mGroupConstrain, 0);

	if (Active == true || modelA == nullptr || modelB == nullptr)
		return;

	int vertA, vertB, vertC;
	FBVector3d baryCoords;

	if (true == CalculateClosestPoints3(modelA, modelB, srcContactPoint, dstContactPoint, vertA, vertB, vertC, baryCoords) )
	{
		AttachmentVertA = vertA;
		AttachmentVertB = vertB;
		AttachmentVertC = vertC;
		BaryCoords = baryCoords;

		FBMatrix constrainMatrix;
		FBVector3d constrainPos;
		modelB->GetMatrix(constrainMatrix, kModelRotation);
		modelB->GetVector(constrainPos);

		constrainMatrix[12] = constrainPos[0];
		constrainMatrix[13] = constrainPos[1];
		constrainMatrix[14] = constrainPos[2];

		FBMatrix modelMatrix, normalMatrix;

		modelA->GetMatrix(modelMatrix);
		modelA->GetMatrix(normalMatrix, kModelRotation);
		normalMatrix = normalMatrix.Transpose();
		normalMatrix = normalMatrix.Inverse();
		normalMatrix.Identity();

		FBModelVertexData *pVertexData = modelA->ModelVertexData;
		const int pCount = pVertexData->GetVertexCount();
		FBVertex *pSrcVertex = (FBVertex*) pVertexData->GetVertexArray( kFBGeometryArrayID_Point );
		FBNormal *pSrcNormal = (FBNormal*) pVertexData->GetVertexArray( kFBGeometryArrayID_Normal );

		FBVector3d pos, nor;
		BaryCoordsToTM(modelMatrix, normalMatrix, pCount, pSrcVertex, pSrcNormal, vertA, vertB, vertC, baryCoords, pos, nor );

		ComputeAttachmentMatrix(modelMatrix, pSrcVertex, vertA, vertB, vertC, pos);

		// compute constrain local matrix
		FBMatrix localMatrix;

		if (PutOnSurface)
		{
			constrainMatrix[12] = srcContactPoint[0] + constrainMatrix[12]-dstContactPoint[0];
			constrainMatrix[13] = srcContactPoint[1] + constrainMatrix[13]-dstContactPoint[1];
			constrainMatrix[14] = srcContactPoint[2] + constrainMatrix[14]-dstContactPoint[2];
		}

		FBGetLocalMatrix( localMatrix, mAttachmentMatrix, constrainMatrix );

		FBTVector tvec;
		FBRVector rvec;
		FBSVector svec;

		FBMatrixToTRS( tvec, rvec, svec, localMatrix );

		OffsetTranslation = FBVector3d( tvec[0], tvec[1], tvec[2] );
		OffsetRotation = FBVector3d( rvec[0], rvec[1], rvec[2] );
		OffsetScaling = FBVector3d( svec[0], svec[1], svec[2] );
	}

}

/************************************************
 *	Suggest a freeze.
 ************************************************/
void CConstraintAttachment::FreezeSuggested()
{
	mFirstTime = true;
	FBConstraint::FreezeSuggested();

	if( ReferenceGet( mGroupSource,0 ) )
	{
		FreezeSRT( (FBModel*)ReferenceGet( mGroupSource, 0), true, true, true );
	}
}


/************************************************
 *	Real-Time Engine Evaluation
 ************************************************/
bool CConstraintAttachment::DeformerNotify(FBModel* pModel, const FBVertex*  pSrcVertex, const FBVertex* pSrcNormal,
									int pCount, 
									FBVertex*  pDstVertex,FBVertex*  pDstNormal)
{
	// compute attachment matrix here
	int vertA = AttachmentVertA;

	if (vertA >= 0)
	{
		int vertB = AttachmentVertB;
		int vertC = AttachmentVertC;
		FBVector3d bary = BaryCoords;

		FBMatrix modelMatrix, normalMatrix;

		pModel->GetMatrix(modelMatrix);
		pModel->GetMatrix(normalMatrix, kModelRotation);

		FBVector3d pos, nor;
		BaryCoordsToTM(modelMatrix, normalMatrix, pCount, pSrcVertex, pSrcNormal, vertA, vertB, vertC, bary, pos, nor );

		ComputeAttachmentMatrix(modelMatrix, pSrcVertex, vertA, vertB, vertC, pos);
	}

	return false;
}

void CConstraintAttachment::ComputeAttachmentMatrix(const FBMatrix &modelMatrix, const FBVertex *pSrcVertex, const int vertA, const int vertB, const int vertC, const FBVector3d &pos)
{
	
	// triangle TBN

	FBVertex p1, p2, p3;
	FBVertexMatrixMult( p1, modelMatrix, pSrcVertex[vertA] ); 
	FBVertexMatrixMult( p2, modelMatrix, pSrcVertex[vertB] );
	FBVertexMatrixMult( p3, modelMatrix, pSrcVertex[vertC] );

	vec3 v1(p1);
	vec3 v2(p2);
	vec3 v3(p3);

	vec3 n;
	cross(n, v2-v1, v3-v1);
	normalize(n);

	mDebugA = p1;
	mDebugB = p2;
	mDebugC = p3;
	mDebugNormal = FBVector3d( (double)n[0], (double)n[1], (double)n[2] );

	mat3 TBN(array9_id);
	tangent_basis(TBN, v1, v2, v3, vec2(0.0f, 0.0f), vec2(1.0f, 0.0f), vec2(0.0f, 1.0f), n);
		
	mAttachmentMatrix.Identity();
	mAttachmentMatrix[0] = TBN.a00; 
	mAttachmentMatrix[1] = TBN.a01; 
	mAttachmentMatrix[2] = TBN.a02; 

	mAttachmentMatrix[4] = TBN.a10; 
	mAttachmentMatrix[5] = TBN.a11; 
	mAttachmentMatrix[6] = TBN.a12; 

	mAttachmentMatrix[8] = TBN.a20; 
	mAttachmentMatrix[9] = TBN.a21; 
	mAttachmentMatrix[10] = TBN.a22; 
		
	mAttachmentMatrix[12] = pos[0];
	mAttachmentMatrix[13] = pos[1];
	mAttachmentMatrix[14] = pos[2];
}

bool CConstraintAttachment::AnimationNodeNotify(FBAnimationNode* pConnector, FBEvaluateInfo* pEvaluateInfo, FBConstraintInfo* pConstraintInfo)
{
	double lPosition[3], lRotation[3], lScaling[3];

	if( mSourceTranslation && mConstrainedTranslation )
	{
		mSourceTranslation->ReadData	( lPosition, pEvaluateInfo );
		mSourceRotation->ReadData		( lRotation, pEvaluateInfo );
		mSourceScaling->ReadData		( lScaling, pEvaluateInfo );

		FBMatrix constrainMatrix;
		
		int vertA = AttachmentVertA;
		//int vertB = AttachmentVertB;
		//int vertC = AttachmentVertC;
		//FBVector3d bary = BaryCoords;

		if (vertA >= 0)
		{
			FBMatrix localMatrix;

			FBVector3d t, r, s;
			t = OffsetTranslation;
			r = OffsetRotation;
			s = OffsetScaling;

			FBTRSToMatrix( localMatrix, FBTVector(t[0], t[1], t[2], 1.0), FBRVector(r[0], r[1], r[2]), FBSVector(s[0], s[1], s[2]) );

			//
			FBGetGlobalMatrix( constrainMatrix, mAttachmentMatrix, localMatrix );
		}

		FBTVector tvector;
		FBRVector rvector;
		FBSVector svector;

		FBMatrixToTRS( tvector, rvector, svector, constrainMatrix );
		
		mConstrainedTranslation->WriteData	( tvector, pEvaluateInfo );
		mConstrainedRotation->WriteData( rvector, pEvaluateInfo );
	}
	mPosition = lPosition;

	return true;
}


bool CConstraintAttachment::CalculateClosestPoints3(FBModel *srcModel, FBModel *dstModel, FBVector3d &srcContactPoint, FBVector3d &dstContactPoint, int &outVertA, int &outVertB, int &outVertC, FBVector3d &outBaryCoords)
{

	if (srcModel == nullptr || dstModel == nullptr)
		return false;

	float modelScaling[3] = {1.0f, 1.0f, 1.0f};

	FBMatrix matrixA, matrixB;

	srcModel->GetMatrix(matrixA);
	dstModel->GetMatrix(matrixB);

	// 1 - get source list of triangles
	std::vector<DistancePointTriangleExact::Triangle>	triangles;
	PrepareTriangleList(triangles, modelScaling, matrixA, srcModel->ModelVertexData);

	// 2 - get constrain list of vertices
	std::vector<vec3>	vertices;
	PrepareVerticesList(vertices, modelScaling, matrixB, dstModel->ModelVertexData);

	// 3 - find closest distance to triangle
	//	brute force - for each point find closest triangle, then sort and find pair point-triangle

	vec3 holdContactA(0.0f, 0.0f, 0.0f);
	vec3 holdContactB(0.0f, 0.0f, 0.0f);

	bool firstPoint = true;
	float minDistance = 0.0f;

	DistancePointTriangleExact	distOp;

	for (vec3& point : vertices)
	{
		for (DistancePointTriangleExact::Triangle& tri : triangles)
		{
			auto result = distOp(point, tri);

			if ( result.sqrDistance > 0.0f &&  (firstPoint || minDistance > result.sqrDistance) )
			{
				firstPoint = false;
				minDistance = result.sqrDistance;
				outBaryCoords = FBVector3d(result.parameter[0], result.parameter[1], result.parameter[2]);

				outVertA = tri.index[0];
				outVertB = tri.index[1];
				outVertC = tri.index[2];

				holdContactA = result.closest;
				holdContactB = point;
			}
		}
	}

	//
	// move constraint model to the contact point A

	srcContactPoint = FBVector3d((double)holdContactA[0], (double)holdContactA[1], (double)holdContactA[2]);
	dstContactPoint = FBVector3d((double)holdContactB[0], (double)holdContactB[1], (double)holdContactB[2]);

	return true;
}


bool CConstraintAttachment::BaryCoordsToTM(FBMatrix &modelMatrix, FBMatrix &normalMatrix,
	int numVerts, const FBVertex *vertices, const FBNormal *normals, int vertA, int vertB, int vertC, const FBVector3d &baryCoords, FBVector3d &pos, FBVector3d &nor )
{
	if (vertA >= numVerts || vertB >= numVerts || vertC >= numVerts || vertA < 0 || vertB < 0 || vertC < 0)
		return false;

	FBVertex v1, v2, v3;
	
	FBVertexMatrixMult( v1, modelMatrix, vertices[vertA] );
	FBVertexMatrixMult( v2, modelMatrix, vertices[vertB] );
	FBVertexMatrixMult( v3, modelMatrix, vertices[vertC] );

	FBNormal n1, n2, n3;

	FBVertexMatrixMult( n1, normalMatrix, normals[vertA] );
	FBVertexMatrixMult( n2, normalMatrix, normals[vertB] );
	FBVertexMatrixMult( n3, normalMatrix, normals[vertC] );

	double b2 = baryCoords[0];
	double b3 = baryCoords[1];

	pos[0] = ( (1.0 - b2 - b3) * v1[0] ) + (b2 * v2[0]) + (b3 * v3[0]);
    pos[1] = ( (1.0 - b2 - b3) * v1[1] ) + (b2 * v2[1]) + (b3 * v3[1]);
    pos[2] = ( (1.0 - b2 - b3) * v1[2] ) + (b2 * v2[2]) + (b3 * v3[2]);

	nor[0] = ( (1.0 - b2 - b3) * n1[0] ) + (b2 * n2[0]) + (b3 * n3[0]);
    nor[1] = ( (1.0 - b2 - b3) * n1[1] ) + (b2 * n2[1]) + (b3 * n3[1]);
    nor[2] = ( (1.0 - b2 - b3) * n1[2] ) + (b2 * n2[2]) + (b3 * n3[2]);
	
	VectorNormalize(nor);

	return true;
}

FBModel *NewNull(const char *name, const FBVertex &pos)
{
	FBModelNull *pNull = new FBModelNull(name);
	pNull->SetVector( FBVector3d( (double)pos[0], (double)pos[1], (double)pos[2] ) );
	pNull->Show = true;
	return pNull;
}

FBModel *NewNull(const char *name, const FBVector3d &pos, const bool global = true, FBModel *pParent=nullptr)
{
	FBModelNull *pNull = new FBModelNull(name);
	if (pParent)
		pNull->Parent = pParent;
	pNull->SetVector( pos, kModelTranslation, global );
	pNull->Show = true;
	return pNull;
}

void CConstraintAttachment::DoDebug()
{
	FBModel *pObj = NewNull( "DebugA", mDebugA );
	NewNull( "DebugB", mDebugB );
	NewNull( "DebugC", mDebugC );

	NewNull( "DebugNormal", mDebugNormal, false, pObj );
}