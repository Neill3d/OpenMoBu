
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: BlendShapeToolkit_deformer_manager.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
#include <fbsdk/fbsdk.h>
#include <vector>

//////////////////////////////////////////////////////////////////////////////////////////
// most of shapes can be just one frame corrections, we must cache them instead of looping into the property each time
struct	CDeformerShape
{
	int				difCount;

	int				*origIndices;
	FBVertex		*difVertices;
	FBNormal		*difNormals;

	//FBMatrix		*creationTM;	// matrix for each vertex during creation

	FBString		strPropName;
	FBPropertyAnimatableDouble	*pProperty;		// this property handle shape activity

	// caching a deformer area
	int				numberOfKeys;
	FBTime			singleTime;		// pre cached time and value
	double			singleValue;

	bool			useCache;

	//! a constructor
	CDeformerShape();
	//! a copy constructor
	CDeformerShape( const CDeformerShape &shape );
	

	//! a destructor
	~CDeformerShape();

	void Free();
	
	void Init(const int _difCount);

	bool InitProperty(FBConstraint *pConstraint);

	const double GetValue(const FBTime &localTime, const bool onlyOnKeyframe);

	bool GetKeyTime(FBTime &time);

	bool GetPrevKeyTime(const FBTime &localTime, FBTime &time);
	bool GetNextKeyTime(const FBTime &localTime, FBTime &time);
	bool HasKeyframe(const FBTime &time, double &value);

	// return true is has keyframe at local time
	bool CheckKeyFrame(const FBTime &localTime, FBTime &prevTime, FBTime &nextTime, double &value);

	// update time range and single frame status
	void	UpdateCache();
	void	ClearCache();

	void	CatchDifference(const FBVertex*  pSrcVertex,const FBVertex* pSrcNormal,int pCount,FBVertex*  pDstVertex,FBVertex*  pDstNormal);

	//--- FBX Interface
	bool			FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );	//!< FBX Storage.
	bool			FbxRetrieve	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );	//!< FBX Retrieval.
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// CDeformerManager

class CDeformerChannel
{
public:

	//! a constructor
	CDeformerChannel(const char *_name, const int verticesCount);

	//! a destructor
	~CDeformerChannel();
	
	void Free();
	void RemoveProperties(FBConstraint *pConstraint);

	FBString						name;		// should be associated with a model name for recognition
	int								numberOfVertices;	// should be the same with the associate model
	std::vector<CDeformerShape*>		shapes;		// shapes that was added to current channel (model)

	//--- FBX Interface
	bool			FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );	//!< FBX Storage.
	bool			FbxRetrieve	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );	//!< FBX Retrieval.
};

////////////////////////////////////////////////////////////////////////////////
//
class CDeformerManager
{
public:
	//! a constructor
	CDeformerManager();
	//! a destructor
	~CDeformerManager();

	void Free();

	const int GetNumberOfChannels() const;
	const char *GetChannelName(const int index) const;
	CDeformerChannel *GetChannelPtr(const int index);

	const int GetNumberOfChannelVertices(const int index) const;

	const int GetNumberOfShapes(const int channel) const;
	const char *GetShapeName(const int channel, const int shape) const;

	CDeformerShape *GetShapePtr(const int channel, const int shape);

	FBPropertyAnimatableDouble *GetShapeProperty(const int channel, const int shape);


	bool AddShape( const FBTime &currentTime, FBConstraint *pConstraint, FBModel *pModel, FBPropertyAnimatableDouble *pProp, CDeformerShape *shape, const bool overrideExisting );
	bool AddShape( const FBTime &currentTime, CDeformerChannel *pChannel, FBPropertyAnimatableDouble *pProp, CDeformerShape *shape, const bool overrideExisting );

	void AddShapes( CDeformerChannel *pChannel, std::vector<CDeformerShape*> &shapes );

	void RemoveShape( const char *modelName, const int shapeIndex );
	void RemoveShape( const int channelIndex, const int shapeIndex );

	void RemoveAllShapes( const int channelIndex );

	void ZeroAll();
	void KeyAllShapes( const FBTime &currentTime, FBConstraint *pConstraint, FBModel *pModel );

	void SelectShapes(FBConstraint *pConstraint, FBModel *pModel);
	void DeselectShapes();

	void RenameModel( const char *oldname, const char *newname );

	bool Process(FBConstraint *pConstraint, CDeformerChannel *pChannel, const FBTime &localTime, const FBVertex*  pSrcVertex,const FBVertex* pSrcNormal,int pCount,FBVertex*  pDstVertex,FBVertex*  pDstNormal, const bool applyOnKeyframe, const bool exclusiveMode);

	//--- FBX Interface
	bool			FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );	//!< FBX Storage.
	bool			FbxRetrieve	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );	//!< FBX Retrieval.

protected:

	std::vector<CDeformerChannel*>		mChannels;

	void ApplyShape(const CDeformerShape *shape, const FBMatrix &modelTM, const double value, const FBVertex*  pSrcVertex,const FBVertex* pSrcNormal,int pCount,FBVertex*  pDstVertex,FBVertex*  pDstNormal);
	void MergeShapes( CDeformerShape *shape, const CDeformerShape *mergeShape );

};