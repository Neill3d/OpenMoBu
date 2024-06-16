
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: ClusterAdvance.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "ClusterAdvance.h"
#include "math3d.h"
#include "Logger.h"

// Scale all the elements of a matrix.
void MatrixScale(FBMatrix& pMatrix, double pValue)
{
    int i,j;

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            pMatrix(i,j) *= pValue;
        }
    }
}


// Add a value to all the elements in the diagonal of the matrix.
void MatrixAddToDiagonal(FBMatrix& pMatrix, double pValue)
{
    pMatrix(0,0) += pValue;
    pMatrix(1,1) += pValue;
    pMatrix(2,2) += pValue;
    pMatrix(3,3) += pValue;
}

// Sum two matrices element by element.
void MatrixAdd(FBMatrix& pDstMatrix, FBMatrix& pSrcMatrix)
{
    int i,j;

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            pDstMatrix(i,j) += pSrcMatrix(i,j);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// LinkedVertex

void LinkedVertex::SetVertex( FBVertex _pos, FBNormal _nor )
{
	pos = _pos;
	nor = _nor;
}

void LinkedVertex::AddVertexLink(FBMatrix tm, FBMatrix tmInvTranspose, const double weight, const FBClusterMode mode)
{
	links.push_back( Link(tm, tmInvTranspose, weight, mode) );
}


void LinkedVertex::NormalizeWeights()
{
	double total=0.0;

	for (size_t i=0; i<links.size(); ++i)
		total += links[i].weight;

	// normalize
	for (size_t i=0; i<links.size(); ++i)
		links[i].weight = links[i].weight / total;
}
	
FBMatrix LinkedVertex::CalculateDeformedPositionMatrix()
{
	/*
	FBMatrix tm;

	for (int i=0; i<links.GetCount(); ++i)
	{	
		FBMatrix temp(links[i].tm);

		for (int ii=0; ii<4; ++ii)
			for (int jj=0; jj<4; ++jj)
				tm(ii, jj) += temp(ii, jj) * links[i].weight;

 	}
	return tm;
	*/

	double total=0.0;

	FBMatrix lClusterDeformation;
	//lClusterDeformation.Identity();

	for (size_t i=0; i<links.size(); ++i)
	{
		
		double lWeight = links[i].weight;
		const FBClusterMode mode = links[i].mode;

		if (lWeight == 0.0)
		{
			continue;
		}
		
		// Compute the influence of the link on the vertex
		FBMatrix lInfluence = links[i].tm;
		
		const double lScale = lWeight * 100000.0; // (total + 0.000001);
		MatrixScale(lInfluence, lScale);

		switch(mode)
		{
		case kFBClusterAdditive:
			{
				// Multiply with the product of the deformations on the vertex
				MatrixAddToDiagonal( lInfluence, 1.0 - lWeight );
				FBMatrixMult( lClusterDeformation, lInfluence, lClusterDeformation );
				total = 1.0;
			} break;
		case kFBClusterNormalize:
			{
				
				MatrixAdd( lClusterDeformation, lInfluence );
				
				total += lWeight;
			} break;
		default:
			LOGI( "only additive and normalize is supported!\n" );
			break;
		}
 	}
	
	return lClusterDeformation;
}



FBVertex LinkedVertex::CalculateDeformedPosition()
{

	FBVertex vertex, linkVertex;

	FBMatrix tm = CalculateDeformedPositionMatrix();
	FBMatrixInverse( tm, tm );
	//tm = MatrixInvert(tm);

	FBVertexMatrixMult( vertex, tm, pos );
	//vertex = pos;
	return vertex;
}

FBMatrix LinkedVertex::CalculateDeformedNormalMatrix()
{
	FBMatrix tm;

	for (size_t i=0; i<links.size(); ++i)
	{	
		FBMatrix temp(links[i].tmInvTranspose);

		for (int ii=0; ii<4; ++ii)
			for (int jj=0; jj<4; ++jj)
				tm(ii, jj) += temp(ii, jj) * links[i].weight;

 	}
	return tm;
}

FBNormal LinkedVertex::CalculateDeformedNormal()
{
	FBNormal normal, linkNormal;
	/*
	for (int i=0; i<links.GetCount(); ++i)
	{
		FBVertexMatrixMult( linkNormal, links[i].tmInvTranspose, nor );
			
		normal[0] += links[i].weight * linkNormal[0];
		normal[1] += links[i].weight * linkNormal[1];
		normal[2] += links[i].weight * linkNormal[2];
 	}
	*/

	FBMatrix tm = CalculateDeformedNormalMatrix();

	FBVertexMatrixMult( normal, tm, nor );

	return normal;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
//

ClusterAdvance::ClusterAdvance(FBModel *pModel)
	: count(0)
	, vertices(nullptr)
{
	Init(pModel);
}

ClusterAdvance::~ClusterAdvance()
{
	Free();
}

bool ClusterAdvance::Init(FBModel *pModel)
{
	if (pModel == nullptr) return false;

	FBGeometry *pGeometry = pModel->Geometry;
	FBCluster *pCluster = pModel->Cluster;
	FBModelVertexData *pVertexData = pModel->ModelVertexData;

	if ( pGeometry == nullptr || pCluster == nullptr || pModel->SkeletonDeformable == false ) return false;

	int vertCount, normCount;
	
	vertCount = pVertexData->GetVertexCount();
	//FBVertex *pPositions = pGeometry->GetPositionsArray(vertCount);
	FBVertex *pPositions = (FBVertex*) pVertexData->GetVertexArray( kFBGeometryArrayID_Point, true );
	FBNormal *pNormals = pGeometry->GetNormalsDirectArray(normCount);

	vertices = new LinkedVertex[vertCount];
	count = vertCount;

	FBMatrix tm;
	pModel->GetMatrix(tm);

	for (int i=0; i<vertCount; ++i)
	{
		FBVertex transformed;
		FBVertexMatrixMult( transformed, tm, pPositions[i] );
		vertices[i].SetVertex( transformed, pNormals[i] );
	}

	if (pCluster && pCluster->LinkGetCount())
	{
		FBString linkName;
		FBModel* linkModel;
		FBModel		*linkAssociateModel;
		int numLinks, numVerts, vertIndex;
		double vertWeight;

		FBMatrix m, m2, tm, tmInvTranspose, pLM;
		FBTVector pos;
		FBRVector rot, pDof;
		FBSVector scale;
		
		numLinks = pCluster->LinkGetCount();

		for (int n=0; n < numLinks; n++) 
		{
			pCluster->ClusterBegin(n);			// Set the current cluster index

			linkName = pCluster->LinkGetName(n);
			linkModel = pCluster->LinkGetModel(n);

			if (linkModel == nullptr)
			{
				continue;
			}

			linkAssociateModel = pCluster->LinkGetAssociateModel(n);
			FBClusterMode mode = static_cast<FBClusterMode>(pCluster->ClusterMode.AsInt());
			/*
			if (mode != kFBClusterAdditive)
			{
				printf ("only additive skinning is supported!\n" );
			}
			*/
			FBVector3d temp;
			pCluster->VertexGetTransform(temp, rot, scale);
			pos = FBTVector(temp[0], temp[1], temp[2], 1.0);
			//scale = FBSVector(1.0, 1.0, 1.0);
			FBTRSToMatrix( tm, pos, rot, scale );
			//FBMatrixInverse( tm, tm );

			FBString linkModelName = linkModel->Name.AsString();
			linkModel->GetMatrix( m, kModelTransformation_Geometry );
			
			if (linkAssociateModel)
			{
				linkAssociateModel->GetMatrix( m2, kModelTransformation_Geometry );
				FBMatrixMult( m, m2, m );
			}

			FBMatrixMult( tm, m, tm );

			FBMatrixInverse( tmInvTranspose, tm );
			FBMatrixTranspose( tmInvTranspose, tmInvTranspose );

			numVerts = pCluster->VertexGetCount();	// Using the current cluster index
			for (int v=0; v < numVerts; v++) 
			{
				vertIndex = pCluster->VertexGetNumber(v);		// Using the current cluster index
				vertWeight = pCluster->VertexGetWeight(v);	// Using the current cluster index

				vertices[vertIndex].AddVertexLink( tm, tmInvTranspose, vertWeight, mode );

			}
			pCluster->ClusterEnd();			
		}
		/*
		for (int i=0; i<count; ++i)
			vertices[i].NormalizeWeights();
			*/
	}

	return true;
}

void ClusterAdvance::Free()
{
	if (vertices)
	{
		delete [] vertices;
		vertices = nullptr;
	}
	count = 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// Skin utilities

bool SkinCopy( FBModelList &modelList, FBModel *pDstModel )
{
	FBCluster *pDstCluster = pDstModel->Cluster;
	if (pDstCluster == nullptr)
	{
		return false;
	}

	int startVertexIndex = 0;
	FBVector3d pos, rot, scale;

	for (int nModel = 0; nModel<modelList.GetCount(); ++nModel)
	{
		FBModel *pSrcModel = modelList.GetAt(nModel);

		FBCluster *pSrcCluster = pSrcModel->Cluster;
		if (pSrcCluster == nullptr) 
		{
			continue;
		}
		
		const int numLinks = pSrcCluster->LinkGetCount();

		// if model has moved by parent and we have this cluster, lets simple attach vertices to that's link

		if (numLinks == 0)
		{
			FBModel *pSrcLinkModel = pSrcModel->Parent;
			
			// find if cluster is already has been added
			int dstClusterIndex = -1;	// by default we will add a new link
			for (int j=0; j<pDstCluster->LinkGetCount(); ++j)
			{
				FBModel *pDstLinkModel = pDstCluster->LinkGetModel(j);
				
				if (pSrcLinkModel == pDstLinkModel)
				{
					dstClusterIndex = j;
					break;
				}
			}

			if (dstClusterIndex >= 0)
			{
				const int numVerts = pSrcModel->Geometry->VertexCount();

				pDstCluster->ClusterBegin(dstClusterIndex);

				for (int i=0; i<numVerts; ++i)
				{
					// SHIFT INDEX
					pDstCluster->VertexAdd( startVertexIndex + i, 1.0 );
				}

				pDstCluster->ClusterEnd();

			}

		}
		else
		{
			for (int i=0; i<numLinks; ++i)
			{
				pSrcCluster->ClusterBegin(i);
			
				const char *linkName = pSrcCluster->LinkGetName(i);
				FBModel *pSrcLinkModel = pSrcCluster->LinkGetModel(i);
				const FBClusterMode mode = static_cast<FBClusterMode>(pSrcCluster->ClusterMode.AsInt());
				const double accuracy = pSrcCluster->ClusterAccuracy;
				pSrcCluster->VertexGetTransform(pos, rot, scale);

				// find if cluster is already has been added
				int dstClusterIndex = pDstCluster->LinkGetCount();	// by default we will add a new link
				for (int j=0; j<pDstCluster->LinkGetCount(); ++j)
				{
					FBModel *pDstLinkModel = pDstCluster->LinkGetModel(j);
				
					if (pSrcLinkModel == pDstLinkModel)
					{
						FBVector3d pos2, rot2, scale2;
						pDstCluster->ClusterBegin(j);
						pDstCluster->VertexGetTransform(pos2, rot2, scale2);
						pDstCluster->ClusterEnd();

						if( VectorLength(VectorSubtract(pos2, pos)) < 0.01 && VectorLength(VectorSubtract(rot2, rot)) < 0.01 && VectorLength(VectorSubtract(scale2, scale)) < 0.01)
						{
							dstClusterIndex = j;
							break;
						}
					}
				}
				
				// DONE: select distination cluster index (find existing or create a new one)
				pDstCluster->ClusterBegin(dstClusterIndex);

				pDstCluster->LinkSetName(linkName, dstClusterIndex);
				pDstCluster->LinkSetModel(pSrcLinkModel);
				pDstCluster->ClusterMode.SetPropertyValue(mode);
				pDstCluster->ClusterAccuracy = accuracy;
				pDstCluster->VertexSetTransform(pos, rot, scale);

				const int numSrcVerts = pSrcCluster->VertexGetCount();

				for (int j=0; j<numSrcVerts; ++j)
				{
					const int index = pSrcCluster->VertexGetNumber(j);
					const double weight = pSrcCluster->VertexGetWeight(j);

					// SHIFT INDEX
					pDstCluster->VertexAdd( startVertexIndex + index, weight );
				}

				pDstCluster->ClusterEnd();
				pSrcCluster->ClusterEnd();
			}
		}

		//
		// update vertex index for next model
		startVertexIndex += pSrcModel->Geometry->VertexCount();
	}

	return true;
}


void SkinCleanup( FBModel *pModel, const double threshold, int &linksBefore, int &linksAfter )
{
	FBCluster *pCluster = pModel->Cluster;

	linksBefore = pCluster->LinkGetCount();
	pCluster->LinkClearUnused( threshold );
	linksAfter = pCluster->LinkGetCount();
}