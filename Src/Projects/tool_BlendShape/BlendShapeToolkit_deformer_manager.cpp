
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: BlendShapeToolkit_deformer_manager.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "BlendShapeToolkit_deformer_manager.h"
#include "BlendShapeToolkit_deformer_constraint.h"
#include "math3d.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// CDeformerShape

CDeformerShape::CDeformerShape()
	: strPropName("")
{
	difCount = 0;

	origIndices = nullptr;
	difVertices = nullptr;
	difNormals = nullptr;
	//creationTM = nullptr;

	pProperty = nullptr;

	useCache = false;
	numberOfKeys = 0;
	singleTime = FBTime::Zero;
	singleValue = 0.0;
}
//! a copy constructor
CDeformerShape::CDeformerShape( const CDeformerShape &shape )
{
	difCount = shape.difCount;
	origIndices = shape.origIndices;
	difVertices = shape.difVertices;
	difNormals = shape.difNormals;
	//creationTM = shape.creationTM;
	pProperty = shape.pProperty;

	/*
	if (shape.szPropName[0] != 0)
		memcpy( szPropName, shape.szPropName, sizeof(char) * 128 );
		*/
	strPropName = shape.strPropName;

	useCache = shape.useCache;
	numberOfKeys = shape.numberOfKeys;
	singleTime = shape.singleTime;
	singleValue = shape.singleValue;
}

//! a destructor
CDeformerShape::~CDeformerShape()
{
	Free();
}

void CDeformerShape::Free()
{
	difCount = 0;
	if (origIndices)
	{
		delete [] origIndices;
		origIndices = nullptr;
	}
	if (difVertices)
	{
		delete [] difVertices;
		difVertices = nullptr;
	}
	if (difNormals)
	{
		delete [] difNormals;
		difNormals = nullptr;
	}
	/*
	if (creationTM)
	{
		delete [] creationTM;
		creationTM = nullptr;
	}
	*/
	//pProperty = nullptr;
}

void CDeformerShape::Init(const int _difCount)
{
	difCount = _difCount;

	if (difCount > 0)
	{
		origIndices = new int[difCount];
		difVertices = new FBVertex[difCount];
		difNormals = new FBNormal[difCount];
		//creationTM = new FBMatrix[difCount];
	}
}

bool CDeformerShape::InitProperty(FBConstraint *pConstraint)
{
	bool result = false;

	if (pProperty == nullptr)
	{
		if (strPropName != "")
		{
			pProperty = dynamic_cast<FBPropertyAnimatableDouble*> (pConstraint->PropertyList.Find( strPropName ));
		}

		if (pProperty != nullptr)
		{
			// TODO: need to update during property value has changed
			//UpdateCache();
			result = true;
		}
	}
	return result;
}

const double CDeformerShape::GetValue(const FBTime &time, const bool onlyOnKeyframe)
{
	double value=0.0;

	if (useCache)
	{
		if (numberOfKeys > 0 && singleTime == time)
			value = singleValue;
		
		return value;
	}

	FBAnimationNode *pAnimNode = pProperty->GetAnimationNode();
	if (pAnimNode)
		pAnimNode->Evaluate(&value, time);
	else
		pProperty->GetData( &value, sizeof(double) );

	if (onlyOnKeyframe)
	{
		if (pAnimNode==nullptr || pAnimNode->KeyCount==0)
			value = 0.0;
	}
	return value;
}

bool CDeformerShape::GetKeyTime(FBTime &time)
{
	if (useCache)
	{
		if (numberOfKeys == 0)
			return false;

		time = singleTime;
		return true;
	}

	if (pProperty == nullptr)
		return false;

	FBAnimationNode *pAnimNode = pProperty->GetAnimationNode();
	if (pAnimNode!=nullptr && pAnimNode->KeyCount>0)
	{
		FBFCurve *pCurve = pAnimNode->FCurve;
		time = pCurve->Keys[0].Time;
		return true;
	}

	return false;
}

bool CDeformerShape::GetPrevKeyTime(const FBTime &localTime, FBTime &time)
{
	if (useCache)
	{
		if (numberOfKeys == 0)
			return false;

		if (singleTime <= localTime)
		{
			time = singleTime;
			return true;
		}

		return false;
	}

	if (pProperty == nullptr)
		return false;
	
	FBAnimationNode *pAnimNode = pProperty->GetAnimationNode();
	if (pAnimNode!=nullptr && pAnimNode->KeyCount>0)
	{
		const int count = pAnimNode->KeyCount;
		FBFCurve *pCurve = pAnimNode->FCurve;

		for (int i=count-1; i>=0; --i)
		{
			FBTime keyTime = pCurve->Keys[i].Time;
			if (keyTime <= localTime)
			{
				time = keyTime;
				return true;
			}
		}
	}
	return false;
}

bool CDeformerShape::GetNextKeyTime(const FBTime &localTime, FBTime &time)
{
	if (useCache)
	{
		if (numberOfKeys == 0)
			return false;

		if (singleTime >= localTime)
		{
			time = singleTime;
			return true;
		}

		return false;
	}

	if (pProperty == nullptr)
		return false;

	FBAnimationNode *pAnimNode = pProperty->GetAnimationNode();
	if (pAnimNode!=nullptr && pAnimNode->KeyCount>0)
	{
		const int count = pAnimNode->KeyCount;
		FBFCurve *pCurve = pAnimNode->FCurve;

		for (int i=0; i<count; ++i)
		{
			FBTime keyTime = pCurve->Keys[i].Time;
			if (keyTime >= localTime)
			{
				time = keyTime;
				return true;
			}
		}
	}
	return false;
}

bool CDeformerShape::HasKeyframe(const FBTime &time, double &value)
{
	if (useCache)
	{
		if (numberOfKeys == 0)
			return false;

		if (singleTime == time)
		{
			value = singleValue;
			return true;
		}
		return false;
	}

	if (pProperty == nullptr)
		return false;

	FBAnimationNode *pAnimNode = pProperty->GetAnimationNode();
	if (pAnimNode!=nullptr && pAnimNode->KeyCount>0)
	{
		const int count = pAnimNode->KeyCount;
		FBFCurve *pCurve = pAnimNode->FCurve;

		for (int i=0; i<count; ++i)
		{
			FBTime keyTime = pCurve->Keys[i].Time;
			if (keyTime == time)
			{
				value = pCurve->Keys[i].Value;
				return true;
			}
		}
	}
	return false;
}

bool CDeformerShape::CheckKeyFrame(const FBTime &localTime, FBTime &prevTime, FBTime &nextTime, double &value)
{
	if (useCache)
	{
		if (numberOfKeys == 0)
			return false;

		if (singleTime == localTime)
		{
			value = singleValue;
			return true;
		}
		else if (singleTime > localTime && singleTime < nextTime)
		{
			nextTime = singleTime;
		}
		else if (singleTime < localTime && singleTime > prevTime)
		{
			prevTime = singleTime;
		}
	}
	else
	{
		FBTime time;
		
		if (GetPrevKeyTime(localTime, time) == true)
		{
			if (time > prevTime) prevTime = time;
		}
		if (GetNextKeyTime(localTime, time) == true)
		{
			if (time < nextTime) nextTime = time;
		}
		if (HasKeyframe(localTime, value) == true)
		{
			return true;
		}
	}

	return false;
}

void CDeformerShape::UpdateCache()
{
	useCache = false;
	numberOfKeys = 0;
	singleTime = FBTime::Zero;
	singleValue = 0.0;

	if (pProperty == nullptr)
		return;

	FBAnimationNode *pAnimNode = pProperty->GetAnimationNode();
	if (pAnimNode!=nullptr && pAnimNode->KeyCount>0)
	{
		const int count = pAnimNode->KeyCount;
		FBFCurve *pCurve = pAnimNode->FCurve;

		numberOfKeys = count;

		if (count == 1)
		{
			singleTime = pCurve->Keys[0].Time;
			singleValue = pCurve->Keys[0].Value;
			useCache = true;
		}
	}
}

void CDeformerShape::ClearCache()
{
	useCache = false;
	numberOfKeys = 0;
	singleTime = FBTime::Zero;
	singleValue = 0.0;
}

bool CDeformerShape::FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	if (pStoreWhat & kAttributes)
	{
		pFbxObject->FieldWriteI( "count", difCount );

		for (int i=0; i<difCount; ++i)
		{
			pFbxObject->FieldWriteI( "index", origIndices[i] );
			
			pFbxObject->FieldWriteD( "posX", (double) difVertices[i][0] );
			pFbxObject->FieldWriteD( "posY", (double) difVertices[i][1] );
			pFbxObject->FieldWriteD( "posZ", (double) difVertices[i][2] );

			pFbxObject->FieldWriteD( "norX", (double) difNormals[i][0] );
			pFbxObject->FieldWriteD( "norY", (double) difNormals[i][1] );
			pFbxObject->FieldWriteD( "norZ", (double) difNormals[i][2] );
		}

		if (pProperty)
			pFbxObject->FieldWriteC( "propName", pProperty->GetName() );
		else
			pFbxObject->FieldWriteC( "propName", "EMPTY" );
	}

	return true;
}
bool CDeformerShape::FbxRetrieve	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	if (pStoreWhat & kAttributes)
	{
		Free();

		int count = pFbxObject->FieldReadI( "count" );
		Init(count);

		for (int i=0; i<count; ++i)
		{
			origIndices[i] = pFbxObject->FieldReadI( "index" );

			difVertices[i][0] = (float) pFbxObject->FieldReadD( "posX" );
			difVertices[i][1] = (float) pFbxObject->FieldReadD( "posY" );
			difVertices[i][2] = (float) pFbxObject->FieldReadD( "posZ" );

			difNormals[i][0] = (float) pFbxObject->FieldReadD( "norX" );
			difNormals[i][1] = (float) pFbxObject->FieldReadD( "norY" );
			difNormals[i][2] = (float) pFbxObject->FieldReadD( "norZ" );
		}

		const char *propName = pFbxObject->FieldReadC( "propName" );
		if ( strcmp(propName, "EMPTY") == 0 )
			pProperty = nullptr;
		else
		{
			strPropName = propName;
		}
	}

	return true;
}

void CDeformerShape::CatchDifference(const FBVertex*  pSrcVertex,const FBVertex* pSrcNormal,int pCount,FBVertex*  pDstVertex,FBVertex*  pDstNormal)
{
	int newDifCount = 0;

	for (int i=0; i<pCount; ++i)
	{
		if (pSrcVertex[i][0]!=pDstVertex[i][0] || pSrcVertex[i][1]!=pDstVertex[i][1] || pSrcVertex[i][2]!=pDstVertex[i][2]
			|| pSrcNormal[i][0]!=pDstNormal[i][0] || pSrcNormal[i][1]!=pDstNormal[i][1] || pSrcNormal[i][2]!=pDstNormal[i][2] )
		{
			newDifCount += 1;
		}
	}

	if (newDifCount == 0)
		return;

	if (newDifCount != difCount)
	{
		Free();
		Init(newDifCount);
	}

	newDifCount = 0;
	for (int i=0; i<pCount; ++i)
	{
		if (pSrcVertex[i][0]!=pDstVertex[i][0] || pSrcVertex[i][1]!=pDstVertex[i][1] || pSrcVertex[i][2]!=pDstVertex[i][2]
			|| pSrcNormal[i][0]!=pDstNormal[i][0] || pSrcNormal[i][1]!=pDstNormal[i][1] || pSrcNormal[i][2]!=pDstNormal[i][2] )
		{

			origIndices[newDifCount] = i;
			difVertices[newDifCount] = FBVertex( pDstVertex[i][0]-pSrcVertex[i][0], pDstVertex[i][1]-pSrcVertex[i][1], pDstVertex[i][2]-pSrcVertex[i][2] );
			difNormals[newDifCount] = FBNormal( pDstVertex[i][0]-pSrcVertex[i][0], pDstVertex[i][1]-pSrcVertex[i][1], pDstVertex[i][2]-pSrcVertex[i][2] );

			newDifCount += 1;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// CDeformerChannel

CDeformerChannel::CDeformerChannel(const char *_name, const int verticesCount)
	: name(_name)
	, numberOfVertices(verticesCount)
{}

CDeformerChannel::~CDeformerChannel()
{
	Free();
}

void CDeformerChannel::Free()
{
	for (auto iter=shapes.begin(); iter!=shapes.end(); ++iter)
		delete (*iter);
	shapes.clear();
}

void CDeformerChannel::RemoveProperties(FBConstraint *pConstraint)
{
	for (auto iter=shapes.begin(); iter!=shapes.end(); ++iter)
	{
		FBProperty *pProp = (*iter)->pProperty;
		pConstraint->PropertyRemove(pProp);
		(*iter)->pProperty = nullptr;
	}
}

bool CDeformerChannel::FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	if (pStoreWhat & kAttributes)
	{
		pFbxObject->FieldWriteC( "name", name );
		pFbxObject->FieldWriteI( "vertices", numberOfVertices );
		pFbxObject->FieldWriteI( "count", (int)shapes.size() );

		for (auto iter=shapes.begin(); iter!=shapes.end(); ++iter)
			(*iter)->FbxStore(pFbxObject, pStoreWhat);
	}

	return true;
}
bool CDeformerChannel::FbxRetrieve	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	if (pStoreWhat & kAttributes)
	{
		name = pFbxObject->FieldReadC( "name" );
		numberOfVertices = pFbxObject->FieldReadI( "vertices" );
		int count = pFbxObject->FieldReadI( "count" );

		shapes.resize(count, nullptr);

		for (int i=0; i<count; ++i)
		{
			CDeformerShape *pNewShape = new CDeformerShape();
			pNewShape->FbxRetrieve(pFbxObject, pStoreWhat);
			shapes[i] = pNewShape;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// CDeformerManager

CDeformerManager::CDeformerManager()
{
}

CDeformerManager::~CDeformerManager()
{
	Free();
}

void CDeformerManager::Free()
{
	for (auto iter=mChannels.begin(); iter!=mChannels.end(); ++iter)
	{
		delete (*iter);
	}
	mChannels.clear();
}

const int CDeformerManager::GetNumberOfChannels() const
{
	return (int)mChannels.size();
}
const char *CDeformerManager::GetChannelName(const int index) const
{
	return mChannels[index]->name;
}
CDeformerChannel *CDeformerManager::GetChannelPtr(const int index)
{
	return mChannels[index];
}

const int CDeformerManager::GetNumberOfChannelVertices(const int index) const
{
	return mChannels[index]->numberOfVertices;
}

const int CDeformerManager::GetNumberOfShapes(const int channel) const
{
	return (int) mChannels[channel]->shapes.size();
}
const char *CDeformerManager::GetShapeName(const int channel, const int shape) const
{
	return mChannels[channel]->shapes[shape]->strPropName;
}

CDeformerShape *CDeformerManager::GetShapePtr(const int channel, const int shape)
{
	return mChannels[channel]->shapes[shape];
}

FBPropertyAnimatableDouble *CDeformerManager::GetShapeProperty(const int channel, const int shape)
{
	return mChannels[channel]->shapes[shape]->pProperty;
}

bool CDeformerManager::AddShape( const FBTime &curTime, FBConstraint *pConstraint, FBModel *pModel, FBPropertyAnimatableDouble *pProp, CDeformerShape *shape, const bool replaceExisting )
{
	if (pModel == nullptr || pConstraint == nullptr)
		return false;

	FBDeformerCorrective *deformer = nullptr;
	int channel = -1;

	for (int i=0; i<pModel->Deformers.GetCount(); ++i)
		if (FBIS(pModel->Deformers[i], FBDeformerCorrective) )
		{
			FBDeformerCorrective *pDeformer = (FBDeformerCorrective*) pModel->Deformers[i];
			if (pDeformer->Constraint.GetCount() > 0 && pDeformer->Constraint.GetAt(0) == pConstraint)
			{
				deformer = pDeformer;

				if (pDeformer->Active)
					channel = pDeformer->Channel;
			}
		}

	if (deformer == nullptr)
	{
		deformer = new FBDeformerCorrective(pConstraint->Name);
		deformer->Constraint.Add(pConstraint);
		pModel->Deformers.Add(deformer);
	}

	if (channel < 0)
	{
		FBGeometry *pGeometry = pModel->Geometry;
		int verticesCount = 0;
		if (pGeometry)
			verticesCount = pGeometry->VertexCount();

		CDeformerChannel *pNewChannel = new CDeformerChannel(pModel->Name, verticesCount);
		channel = (int) mChannels.size();
		mChannels.push_back(pNewChannel);
	}
	
	// finally everything is assigned and we can add a new shape
	bool result = AddShape( curTime, mChannels[channel], pProp, shape, replaceExisting );

	return result;
}

bool CDeformerManager::AddShape( const FBTime &currentTime, CDeformerChannel *pChannel, FBPropertyAnimatableDouble *pProp, CDeformerShape *shape, const bool replaceExisting )
{
	if (pChannel == nullptr)
		return false;

	// shape add or rewrite ?!
	//  and remove property in that case!

	if (false == replaceExisting)
	{
		pChannel->shapes.push_back(shape);
	}
	else
	{
		// try to find a shape to override

		for (auto iter=pChannel->shapes.begin(); iter!=pChannel->shapes.end(); ++iter)
		{
			if ((*iter)->pProperty == nullptr)
				continue;

			double value;
			FBAnimationNode *pNode = (*iter)->pProperty->GetAnimationNode();
			if (pNode)
			{
				pNode->Evaluate( &value, currentTime );
			}
			else
			{
				(*iter)->pProperty->GetData( &value, sizeof(double) );
			}

			if (value == 100.0)
			{
				// we find that shape !
				MergeShapes( *iter, shape );
				return false;		// we don't need new animatable property and we should free shape memory
			}
		}

		// we are here, means failed to find anything for merging
		shape->UpdateCache();
		pChannel->shapes.push_back(shape);
	}


	return true;
}

void CDeformerManager::AddShapes( CDeformerChannel *pChannel, std::vector<CDeformerShape*> &shapes )
{
	pChannel->shapes.resize(shapes.size() );

	for (size_t i=0; i<pChannel->shapes.size(); ++i)
		pChannel->shapes[i] = shapes[i];
}

void CDeformerManager::MergeShapes( CDeformerShape *shape, const CDeformerShape *mergeShape )
{
	// compute total amoung of differences

	std::vector<int>		origVertices;
	std::vector<FBVertex>	difPositions;
	std::vector<FBNormal>	difNormals;

	origVertices.reserve(shape->difCount + mergeShape->difCount);
	difPositions.reserve(shape->difCount + mergeShape->difCount);
	difNormals.reserve(shape->difCount + mergeShape->difCount);

	for (int i=0; i<shape->difCount; ++i)
	{
		origVertices.push_back(shape->origIndices[i]);
		difPositions.push_back(shape->difVertices[i]);
		difNormals.push_back(shape->difNormals[i]);
	}

	FBVertex mergeVertex, vertex;
	FBNormal mergeNormal, normal;

	for (int i=0; i<mergeShape->difCount; ++i)
	{
		int index = mergeShape->origIndices[i];

		mergeVertex = mergeShape->difVertices[i];
		mergeNormal = mergeShape->difNormals[i];

		bool merged = false;
		for (int j=0; j<shape->difCount; ++j)
			if (index == shape->origIndices[j])
			{
				vertex = difPositions[j];
				normal = difNormals[j];
				
				difPositions[j] = FBVertex(vertex[0]+mergeVertex[0], vertex[1]+mergeVertex[1], vertex[2]+mergeVertex[2], 1.0);
				difNormals[j] = FBNormal(normal[0]+mergeNormal[0], normal[1]+mergeNormal[1], normal[2]+mergeNormal[2], 1.0);

				merged = true;
				break;
			}

		if (merged == false)
		{
			origVertices.push_back(index);
			difPositions.push_back(mergeVertex);
			difNormals.push_back(mergeNormal);
		}
	}

	// finalyze
	shape->Free();
	shape->Init( (int) origVertices.size() );

	for (int i=0; i<shape->difCount; ++i)
	{
		shape->origIndices[i] = origVertices[i];
		shape->difVertices[i] = difPositions[i];
		shape->difNormals[i] = difNormals[i];
	}
}

bool CDeformerManager::Process(FBConstraint *pConstraint, CDeformerChannel *pChannel, const FBTime &localTime, const FBVertex*  pSrcVertex,const FBVertex* pSrcNormal,int pCount,FBVertex*  pDstVertex,FBVertex*  pDstNormal, const bool applyOnKeyframe, const bool exclusiveMode)
{
	if (pChannel == nullptr || pConstraint == nullptr)
		return false;

	FBMatrix tm;
	double globalWeight = 0.01 * pConstraint->Weight;

	if (exclusiveMode && pChannel->shapes.size() > 0)
	{
		// apply only prev and next shapes
		CDeformerShape *shape = nullptr;

		for (auto iter=pChannel->shapes.begin(); iter!=pChannel->shapes.end(); ++iter)
		{
			(*iter)->InitProperty(pConstraint);
		}

		if (pChannel->shapes.size() == 1)
		{
			shape = pChannel->shapes[0];
			shape->InitProperty(pConstraint);
			

			double value = shape->GetValue(localTime, applyOnKeyframe);
			
			if (value != 0.0)
			{
				value = 0.01 * value * globalWeight;	// [0; 100] -> [0; 1]

				// finally we can apply our shape
				ApplyShape(shape, tm, value, pSrcVertex, pSrcNormal, pCount, pDstVertex, pDstNormal);
			}
		}
		else
		{
			
			FBTime prevTime = FBTime::MinusInfinity;
			FBTime nextTime = FBTime::Infinity;

			double value;
			FBTime time;

			bool hasLocalKey = false;

			for (auto iter=pChannel->shapes.begin(); iter!=pChannel->shapes.end(); ++iter)
			{
				if ((*iter)->CheckKeyFrame(localTime, prevTime, nextTime, value) == true)
				{
					hasLocalKey = true;
					break;
				}
			}

			if (hasLocalKey == true)
			{
				// apply only current keyframe
				for (auto iter=pChannel->shapes.begin(); iter!=pChannel->shapes.end(); ++iter)
				{
					if ((*iter)->HasKeyframe(localTime, value) )
					{
						value = 0.01 * value * globalWeight;	// [0; 100] -> [0; 1]

						// finally we can apply our shape
						ApplyShape(*iter, tm, value, pSrcVertex, pSrcNormal, pCount, pDstVertex, pDstNormal);
					}
				}
			}
			else if (prevTime == FBTime::MinusInfinity)
			{
				// apply only next keyframes
				for (auto iter=pChannel->shapes.begin(); iter!=pChannel->shapes.end(); ++iter)
				{
					// if shape has keyframe in nextTime, apply from nextTime keyframe
					if ((*iter)->HasKeyframe(nextTime, value) )
					{
						value = 0.01 * value * globalWeight;	// [0; 100] -> [0; 1]

						// finally we can apply our shape
						ApplyShape(*iter, tm, value, pSrcVertex, pSrcNormal, pCount, pDstVertex, pDstNormal);
					}
				}
			}
			else if (nextTime == FBTime::Infinity)
			{
				// apply only prev keyframes
				for (auto iter=pChannel->shapes.begin(); iter!=pChannel->shapes.end(); ++iter)
				{
					// if shape has keyframe in prevTime, apply from prevTime keyframe
					if ((*iter)->HasKeyframe(prevTime, value) )
					{
						value = 0.01 * value * globalWeight;	// [0; 100] -> [0; 1]

						// finally we can apply our shape
						ApplyShape(*iter, tm, value, pSrcVertex, pSrcNormal, pCount, pDstVertex, pDstNormal);
					}
				}
			}
			else
			{
				// interpolate between prev and next keyframes
				double len = 1.0 * nextTime.Get() - prevTime.Get();

				if (len != 0.0)
				{
					double v1, v2;
					double f = (1.0 * localTime.Get() - prevTime.Get()) / len;
					
					for (auto iter=pChannel->shapes.begin(); iter!=pChannel->shapes.end(); ++iter)
					{
						
						if ((*iter)->HasKeyframe(prevTime, v1) == false)
							v1 = 0.0;
						if ((*iter)->HasKeyframe(nextTime, v2) == false)
							v2 = 0.0;

						value = smootherstep( 0.01*v1, 0.01*v2, 0.01 * (v1 + (v2-v1)*f) );
						value = v1 + (v2-v1) * value;
						value = 0.01 * value * globalWeight;	// [0; 100] -> [0; 1]

						// finally we can apply our shape
						if (value != 0.0)
							ApplyShape(*iter, tm, value, pSrcVertex, pSrcNormal, pCount, pDstVertex, pDstNormal);
					}
					
				}
			}
		}
	}
	else
	{
		// apply all active shapes according to their weight
		for (size_t i=0; i<pChannel->shapes.size(); ++i)
		{
			CDeformerShape *shape = pChannel->shapes[i];
			shape->InitProperty(pConstraint);

			double value = shape->GetValue(localTime, applyOnKeyframe);
			if (value != 0.0)
			{
				value = 0.01 * value * globalWeight;	// [0; 100] -> [0; 1]

				// finally we can apply our shape
				ApplyShape(shape, tm, value, pSrcVertex, pSrcNormal, pCount, pDstVertex, pDstNormal);
			}
		}
	}
	return true;
}

void CDeformerManager::ApplyShape( const CDeformerShape *shape, const FBMatrix &modelTM, const double value, const FBVertex*  pSrcVertex,const FBVertex* pSrcNormal,int pCount,FBVertex*  pDstVertex,FBVertex*  pDstNormal )
{
	for (int i=0; i<shape->difCount; ++i)
	{
		int index = shape->origIndices[i];

		//FBMatrix tm;
		//FBMatrixMult( tm, modelTM, shape.creationTM[i] );

		FBVertex vertex = shape->difVertices[i];
		FBNormal normal = shape->difNormals[i];

		//FBVertexMatrixMult(vertex, tm, vertex);

		FBVertex &outVertex = pDstVertex[index];
		FBNormal &outNormal = pDstNormal[index];

		outVertex[0] += value * vertex[0];
		outVertex[1] += value * vertex[1];
		outVertex[2] += value * vertex[2];

		outNormal[0] += value * normal[0];
		outNormal[1] += value * normal[1];
		outNormal[2] += value * normal[2];
	}
}

bool CDeformerManager::FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	if (pStoreWhat & kAttributes)
	{
		pFbxObject->FieldWriteI( "channels", (int)mChannels.size() );

		for (auto iter=mChannels.begin(); iter!=mChannels.end(); ++iter)
			(*iter)->FbxStore(pFbxObject, pStoreWhat);
	}

	return true;
}
bool CDeformerManager::FbxRetrieve	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	if (pStoreWhat & kAttributes)
	{
		Free();

		int count = pFbxObject->FieldReadI( "channels" );

		mChannels.resize(count, nullptr);

		for (int i=0; i<count; ++i)
		{
			CDeformerChannel *pNewChannel = new CDeformerChannel("Channel", 0);
			pNewChannel->FbxRetrieve(pFbxObject, pStoreWhat);
			mChannels[i] = pNewChannel;
		}
	}

	return true;
}

void CDeformerManager::ZeroAll()
{
	double zeroValue = 0.0;
	
	for (auto iter=mChannels.begin(); iter!=mChannels.end(); ++iter)
		for (auto itShape=(*iter)->shapes.begin(); itShape!=(*iter)->shapes.end(); ++itShape)
		{
			if ((*itShape)->pProperty)
			{
				FBAnimationNode *pNode = (*itShape)->pProperty->GetAnimationNode();

				(*itShape)->pProperty->SetData( &zeroValue );
				(*itShape)->pProperty->SetCandidate( &zeroValue, sizeof(double) );

				if (pNode)
				{
					pNode->KeyAdd( &zeroValue, kFBInterpolationCubic, kFBTangentModeUser );
				}
			}
		}
}

void CDeformerManager::KeyAllShapes( const FBTime &curTime, FBConstraint *pConstraint, FBModel *pModel )
{
	if (pModel == nullptr || pConstraint == nullptr)
		return;

	FBDeformerCorrective *deformer = nullptr;
	int channel = -1;

	for (int i=0; i<pModel->Deformers.GetCount(); ++i)
		if (FBIS(pModel->Deformers[i], FBDeformerCorrective) )
		{
			FBDeformerCorrective *pDeformer = (FBDeformerCorrective*) pModel->Deformers[i];
			if (pDeformer->Constraint.GetCount() > 0 && pDeformer->Constraint.GetAt(0) == pConstraint)
			{
				deformer = pDeformer;

				if (pDeformer->Active)
					channel = pDeformer->Channel;
			}
		}

	if (channel >= 0)
	{

		CDeformerShape *shape = nullptr;
		CDeformerChannel *pChannel = mChannels[channel];

		if (pChannel->shapes.size() == 1)
		{
			shape = pChannel->shapes[0];
			shape->InitProperty(pConstraint);
			
			if (shape->pProperty)
				shape->pProperty->Key();
		}
		else
		{
			
			FBTime localTime = FBSystem::TheOne().LocalTime;
			FBTime prevTime = FBTime::MinusInfinity;
			FBTime nextTime = FBTime::Infinity;

			double value;
			FBTime time;

			bool hasLocalKey = false;

			for (auto iter=pChannel->shapes.begin(); iter!=pChannel->shapes.end(); ++iter)
			{
				if ((*iter)->GetPrevKeyTime(localTime, time) == true)
				{
					if (time > prevTime) prevTime = time;
				}
				if ((*iter)->GetNextKeyTime(localTime, time) == true)
				{
					if (time < nextTime) nextTime = time;
				}
				if ((*iter)->HasKeyframe(localTime, value) == true)
				{
					hasLocalKey = true;
					break;
				}
			}

			if (hasLocalKey == true)
			{
				// apply only current keyframe
				for (auto iter=pChannel->shapes.begin(); iter!=pChannel->shapes.end(); ++iter)
				{
					if ((*iter)->pProperty)
					{
						(*iter)->pProperty->GetData( &value, sizeof(double) );
						if (value > 0.0)
							(*iter)->pProperty->Key();
						else
							(*iter)->pProperty->KeyRemoveAt(localTime);
					}
				}
			}
			else if (prevTime == FBTime::MinusInfinity)
			{
				// apply only next keyframes
				for (auto iter=pChannel->shapes.begin(); iter!=pChannel->shapes.end(); ++iter)
				{
					(*iter)->pProperty->GetData( &value, sizeof(double) );

					if ((*iter)->HasKeyframe(nextTime, value) == false)
							value = 0.0;

					if (value > 0.0)
						(*iter)->pProperty->Key();
				}
			}
			else if (nextTime == FBTime::Infinity)
			{
				// apply only prev keyframes
				for (auto iter=pChannel->shapes.begin(); iter!=pChannel->shapes.end(); ++iter)
				{
					(*iter)->pProperty->GetData( &value, sizeof(double) );

					if ((*iter)->HasKeyframe(prevTime, value) == false)
							value = 0.0;

					if (value > 0.0)
						(*iter)->pProperty->Key();
				}
			}
			else
			{

				// curve - prevTime2, prevTime, curTime, nextTime, nextTime2


				// interpolate between prev and next keyframes
				double len = nextTime.GetSecondDouble() - prevTime.GetSecondDouble();

				if (len != 0.0)
				{
					double v1, v2;
					double f = (localTime.GetSecondDouble() - prevTime.GetSecondDouble()) / len;
					//v1 = 1.0 - v2;

					//v2 = smootherstep( 0.0, 1.0, f );
					//v1 = 1.0 - v2;

					for (auto iter=pChannel->shapes.begin(); iter!=pChannel->shapes.end(); ++iter)
					{
						if ((*iter)->HasKeyframe(prevTime, v1) == false)
							v1 = 0.0;
						if ((*iter)->HasKeyframe(nextTime, v2) == false)
							v2 = 0.0;

						value = smootherstep( 0.01*v1, 0.01*v2, 0.01 * (v1 + (v2-v1)*f) );
						value = v1 + (v2-v1) * value;

						if ((*iter)->pProperty)
						{
							FBAnimationNode *pNode = (*iter)->pProperty->GetAnimationNode();

							if (pNode)
							{
								//value *= 100.0;
								if (value > 0.0)
									pNode->KeyAdd(curTime, &value, kFBInterpolationCubic, kFBTangentModeUser );
							}
						}
					}
				}
			}
		}
		/*
		for (auto iter=mChannels[channel]->shapes.begin(); iter!=mChannels[channel]->shapes.end(); ++iter)
			if (iter->pProperty != nullptr)
			{
				FBAnimationNode *pNode = iter->pProperty->GetAnimationNode();
				if (pNode)
				{
					pNode->KeyCandidate(curTime);
				}
			}
			*/
	}
}

void CDeformerManager::SelectShapes(FBConstraint *pConstraint, FBModel *pModel)
{
	int channel = -1;

	for (int i=0; i<pModel->Deformers.GetCount(); ++i)
		if (FBIS(pModel->Deformers[i], FBDeformerCorrective) )
		{
			FBDeformerCorrective *pDeformer = (FBDeformerCorrective*) pModel->Deformers[i];
			if (pDeformer->Constraint.GetCount() > 0 && pDeformer->Constraint.GetAt(0) == pConstraint)
			{
				if (pDeformer->Active)
					channel = pDeformer->Channel;
			}
		}

	if (channel >= 0 )
	{
		for (auto itShape=mChannels[channel]->shapes.begin(); itShape!=mChannels[channel]->shapes.end(); ++itShape)
		{
			if ((*itShape)->pProperty)
			{
				(*itShape)->pProperty->SetFocus(true);
			}
		}
	}
	else if (pModel == nullptr)
	{
		for (auto iter=mChannels.begin(); iter!=mChannels.end(); ++iter)
			for (auto itShape=(*iter)->shapes.begin(); itShape!=(*iter)->shapes.end(); ++itShape)
			{
				if ((*itShape)->pProperty)
				{
					(*itShape)->pProperty->SetFocus(true);
				}
			}
	}
}

void CDeformerManager::DeselectShapes()
{
	for (auto iter=mChannels.begin(); iter!=mChannels.end(); ++iter)
		for (auto itShape=(*iter)->shapes.begin(); itShape!=(*iter)->shapes.end(); ++itShape)
		{
			if ((*itShape)->pProperty)
			{
				(*itShape)->pProperty->SetFocus(false);
			}
		}
}