
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: ClusterAdvance.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
#include <fbsdk/fbsdk.h>
#include <vector>

struct LinkedVertex
{

	struct Link
	{
		FBMatrix		tm;
		FBMatrix		tmInvTranspose;
		double			weight;

		FBClusterMode	mode;

		//! a constructor
		Link(FBMatrix _tm, FBMatrix _tmInvTranspose, const double _weight, const FBClusterMode _mode)
			: tm(_tm)
			, tmInvTranspose(_tmInvTranspose)
			, weight(_weight)
			, mode(_mode)
		{}

	};

	void		SetVertex(FBVertex _pos, FBNormal _nor);
	void		AddVertexLink(FBMatrix tm, FBMatrix tmInvTranspose, const double weight, const FBClusterMode mode);

	FBMatrix	CalculateDeformedPositionMatrix();
	FBMatrix	CalculateDeformedNormalMatrix();

	FBVertex	CalculateDeformedPosition();
	FBNormal	CalculateDeformedNormal();

	void		NormalizeWeights();

private:
	
	FBVertex					pos;
	FBNormal					nor;
	std::vector<Link>			links;

};

class ClusterAdvance
{
public:
	//! a constructor
	ClusterAdvance(FBModel *pModel);

	//! a desturctor
	~ClusterAdvance();

	bool			Init(FBModel *pModel);
	void			Free();

	const int		GetVertexCount() { return count; }

	FBMatrix		CalculateDeformedPositionMatrix(const int vertIndex) { return vertices[vertIndex].CalculateDeformedPositionMatrix(); }
	FBMatrix		CalculateDeformedNormalMatrix(const int vertIndex) { return vertices[vertIndex].CalculateDeformedNormalMatrix(); }

	FBVertex		CalculateDeformedPosition(const int vertIndex) { return vertices[vertIndex].CalculateDeformedPosition(); }
	FBNormal		CalculateDeformedNormal(const int vertIndex) { return vertices[vertIndex].CalculateDeformedNormal(); }

private:

	int					count;
	LinkedVertex		*vertices;

};


/////////////////////////////
// Utilities to work with skin

// copy skin from modelList to dst model

bool SkinCopy(FBModelList &modelList, FBModel *dstModel);

// remove unused clusters

// return information about how many links was before and become after
void SkinCleanup( FBModel *pModel, const double threshold, int &linksBefore, int &linksAfter );