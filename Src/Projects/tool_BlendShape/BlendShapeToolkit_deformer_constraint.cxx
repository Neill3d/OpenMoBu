
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: BlendShapeToolkit_deformer_constraint.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- Class declarations
#include "blendshapetoolkit_deformer_constraint.h"
#include "blendshapetoolkit_deformer_layout.h"
#include "BlendShapeToolkit_Helper.h"
#include "BlendShapeToolkit_brushesBase.h"
#include "BlendShapeToolkit_manip.h"

//--- Registration defines
#define BLENDSHAPEDEFORMER__CLASS		BLENDSHAPEDEFORMER__CLASSNAME
#define BLENDSHAPEDEFORMER__NAME		"BlendShape Deformer"
#define BLENDSHAPEDEFORMER__LABEL		"BlendShape Deformer"
#define BLENDSHAPEDEFORMER__DESC		"BlendShape Deformation Constraint"

//--- implementation and registration
FBConstraintImplementation	(	BLENDSHAPEDEFORMER__CLASS		);
FBRegisterConstraint		(	BLENDSHAPEDEFORMER__NAME,
								BLENDSHAPEDEFORMER__CLASS,
								BLENDSHAPEDEFORMER__LABEL,
								BLENDSHAPEDEFORMER__DESC,
								FB_DEFAULT_SDK_ICON				);	// Icon filename (default=Open Reality icon)




/////////////////////////////////////////////////////////////////////////////////
//

FBClassImplementation(FBDeformerCorrective);

///////////////////////////////////////////////////////////////////////////////////////////////////
// FBDeformerCorrective

FBDeformerCorrective::FBDeformerCorrective(const char* pName, HIObject pObject)
	: FBDeformer(pName, pObject)
{
	FBPropertyPublish(this, Constraint, "Constraint", nullptr, nullptr);
	FBPropertyPublish(this, Active, "Active", nullptr, nullptr);
	FBPropertyPublish(this, Channel, "Channel", nullptr, nullptr);

	Constraint.SetSingleConnect(true);
	Active = true;
	Channel = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////// BlendShapeDeformerConstraint

/************************************************
 *	Creation function.
 ************************************************/
bool BlendShapeDeformerConstraint::FBCreate()
{
	// Create reference groups
	//mGroupSource		= ReferenceGroupAdd( "Source Object",	1 );
	mGroupDeform		= ReferenceGroupAdd( "Deform",			100 );

	Temp = false;

//	FBPropertyPublish( this, AffectedModels, "Affected Models", nullptr, nullptr );

	FBPropertyPublish( this, ApplyOnlyOnKeyframe, "Apply On Keyframe", nullptr, nullptr );
	FBPropertyPublish( this, ApplyTransformation, "Apply Transformation", nullptr, nullptr );
	FBPropertyPublish( this, ExclusiveMode, "Exclusive Mode", nullptr, nullptr );
	ApplyOnlyOnKeyframe = true;
	ApplyTransformation = true;
	ExclusiveMode = false;

	// Constraint options
	Deformer			= true;
	HasLayout			= true;

	// Member variables
	mSourceTranslation	= NULL;
	mFirstTime			= true;
	mResetCount			= true;

	mBufferDst = nullptr;

	//
	//

	
	const int numberOfManipulators = mSystem.Manipulators.GetCount();

	for (int i=0; i<numberOfManipulators; ++i)
	{
		if (FBIS(mSystem.Manipulators[i], ORManip_Sculpt) )
		{
			mManipulator = mSystem.Manipulators[i];
			break;
		}
	}

	mMutex = CreateMutex( 0, FALSE, 0 );
	
	mSystem.OnUIIdle.Add(this, (FBCallback) &BlendShapeDeformerConstraint::EventSystemIdle );

	return true;
}


/************************************************
 *	Destruction function.
 ************************************************/
void BlendShapeDeformerConstraint::FBDestroy()
{
	CloseHandle(mMutex);

	mSystem.OnUIIdle.Remove(this, (FBCallback) &BlendShapeDeformerConstraint::EventSystemIdle );
}

/************************************************
 *	Deformer Binding.
 ************************************************/

bool BlendShapeDeformerConstraint::ReferenceAddNotify( int pGroupIndex, FBModel* pModel )
{
	// Create node for deform
	if ( pGroupIndex==mGroupDeform )
	{
		bool exist = false;
		for (int i=0; i<mAffectedModels.size(); ++i)
			if (pModel == mAffectedModels[i] )
			{
				exist = true;
				break;
			}

		if (exist == false)
			mAffectedModels.push_back(pModel);

		DeformerBind(pModel);
	}
	return true;
}

bool BlendShapeDeformerConstraint::ReferenceRemoveNotify( int pGroupIndex, FBModel* pModel )
{
	// Unbind deform node from deformer callback
	if (pGroupIndex==mGroupDeform)
	{
		for (int i=mAffectedModels.size()-1; i>=0; --i)
			if (pModel == mAffectedModels.at(i))
				mAffectedModels.erase(mAffectedModels.begin() + i);

		DeformerUnBind(pModel);
	}
	return true;
}

/************************************************
 *	Setup all of the animation nodes.
 ************************************************/
void BlendShapeDeformerConstraint::SetupAllAnimationNodes()
{
	/*
	// Create node for source
	if ( ReferenceGet( mGroupSource, 0 ) )
	{
		mSourceTranslation	= AnimationNodeOutCreate( 0, ReferenceGet( mGroupSource, 0 ), ANIMATIONNODE_TYPE_TRANSLATION );
		mGhostNode			= AnimationNodeInCreate ( 0, ReferenceGet( mGroupSource, 0 ), ANIMATIONNODE_TYPE_TRANSLATION );
	}
	*/
	// Create node for deform
	if ( ReferenceGet( mGroupDeform, 0 ) )
	{
		mDeformTranslation = AnimationNodeOutCreate( 1, ReferenceGet( mGroupDeform,	0 ), ANIMATIONNODE_TYPE_TRANSLATION );
	}
}


/************************************************
 *	Removed all of the animation nodes.
 ************************************************/
void BlendShapeDeformerConstraint::RemoveAllAnimationNodes()
{
}


/************************************************
 *	Suggest a freeze.
 ************************************************/
void BlendShapeDeformerConstraint::FreezeSuggested()
{
	mFirstTime = true;

	if( ReferenceGet( 0, 0) )
	{
		FreezeSRT( (FBModel*)ReferenceGet( 0, 0), true, true, true );
	}
}


/************************************************
 *	Real-Time Engine Evaluation.
 ************************************************/
bool BlendShapeDeformerConstraint::AnimationNodeNotify(FBAnimationNode* pConnector,FBEvaluateInfo* pEvaluateInfo,FBConstraintInfo* pConstraintInfo)
{
	/*
	double lPosition[3];

	if( mSourceTranslation )
	{
		mSourceTranslation->ReadData	( lPosition, pEvaluateInfo );
		mGhostNode->WriteData			( lPosition, pEvaluateInfo );
		mPosition = lPosition;
	}
	*/
	
	

	return true;
}


void BlendShapeDeformerConstraint::EventSystemIdle( HISender pSender, HKEvent pEvent )
{
	if ( (mBufferDst == nullptr) || (mBufferDst->pModel.Ok() == false) || (mBufferDst->vertices.size() <= 0) )
		return;

	if ( FBIS(mBufferDst->pModel.GetPlug(), FBModelPath3D) )
	{
			
		FBModelPath3D *pCurve = (FBModelPath3D*) mBufferDst->pModel.GetPlug();

		FBMatrix tm;
		pCurve->GetMatrix(tm, kModelInverse_Transformation_Geometry);
		FBVector4d lVertex;

		const int numberOfVertices = (int) mBufferDst->vertices.size();
		const int numberOfKeys = pCurve->PathKeyGetCount();

		for (int i=0; i<numberOfKeys; ++i)
		{
			FBVector4d pos;
			pos = pCurve->PathKeyGet(i);

			double *d;
			
			if (i == 0)
			{
				d = mBufferDst->vertices[0].position;
			}
			else if (i == numberOfKeys-1)
			{
				d = mBufferDst->vertices[numberOfVertices-1].position;
			}
			else
			{
				d = mBufferDst->vertices[2 + (i-1) * 3].position;
			}

			lVertex = FBVector4d( d[0], d[1], d[2], 1.0 );
			FBVectorMatrixMult( pos, tm, lVertex );

			if (i!=0)
				pCurve->PathKeySet(i, pos, true);
		}
	}

}

/************************************************
 *	Real-Time Deformer Evaluation.
 ************************************************/
bool BlendShapeDeformerConstraint::DeformerNotify(FBModel* pModel,const FBVertex*  pSrcVertex,const FBVertex* pSrcNormal,int pCount,FBVertex*  pDstVertex,FBVertex*  pDstNormal)
{
	if (Temp == false)
	{
		WaitForSingleObject( mMutex, INFINITE );

		FBTime localTime = FBSystem::TheOne().LocalTime;

		int channel = -1;

		for (int i=0; i<pModel->Deformers.GetCount(); ++i)
			if (FBIS(pModel->Deformers[i], FBDeformerCorrective) )
			{
				FBDeformerCorrective *pDeformer = (FBDeformerCorrective*) pModel->Deformers[i];
				if (pDeformer->Constraint.GetCount() > 0 && pDeformer->Constraint.GetAt(0) == this)
				{
					if (pDeformer->Active)
						channel = pDeformer->Channel;
				}
			}

		bool res = false;
		if (channel >= 0)
		{
			res = mManager.Process(this, mManager.GetChannelPtr(channel), localTime, pSrcVertex, pSrcNormal, pCount, pDstVertex, pDstNormal, ApplyOnlyOnKeyframe, ExclusiveMode);
		}

		ReleaseMutex( mMutex );

		return res;
	}

	// make a test for changes
	/*
	if (mManipulator.Ok() )
	{
		ORManip_Sculpt *pManip = (ORManip_Sculpt*) mManipulator.GetPlug();
		
		if (pModel != pManip->GetModelPtr() )
		{
			printf( "DEFORMER CONSTRAINT ERROR - manipulator model and constraint model are different" );
			return false;
		}
	}
	*/
	if (pCount != mLastPoints.size() )
	{
		mFirstTime = true;
		mResetCount = true;
	}
	else if (pCount > 0)
	{
		if (mFirstTime == false && ((pSrcVertex[0][0] != mLastPoints[0][0]) || (pSrcVertex[0][1] != mLastPoints[0][1]) || (pSrcVertex[0][2] != mLastPoints[0][2])) )
		{
			mFirstTime = true;
			Active = false;
			mLastPoints.clear();

			if (mManipulator.Ok() )
				((ORManip_Sculpt*) mManipulator.GetPlug())->StoreFreezeState();

			return false;
		}
	}


	// we can change firstTime when we have different incoming (pCount, srcVertex data coords)
	FBPlayerControl &lPlayerControl = FBPlayerControl::TheOne();

	if (mFirstTime && lPlayerControl.IsPlaying == false && lPlayerControl.IsPlotting == false && lPlayerControl.IsRecording == false)
	{

		if (mBufferDst == nullptr || mBufferDst->vertices.size() != mBufferSrc.vertices.size() )
			mResetCount = true;

		mLastPoints.resize(pCount);
		for (int i=0; i<pCount; ++i)
			mLastPoints[i] = pSrcVertex[i];

		// prepare base and zero buffers from incoming data

		mBufferSrc.pModel = pModel;
		mBufferSrc.vertices.resize( pCount );
		OperationVertex *pOperationVertex = mBufferSrc.vertices.data();

		FBMatrix tm;
		pModel->GetMatrix(tm, kModelTransformation_Geometry);
		FBVertex lVertex;

		for (int i=0; i<pCount; ++i)
		{
			FBVertexMatrixMult( lVertex, tm, pSrcVertex[i] );
			//lVertex = pSrcVertex[i];
			pOperationVertex->position = FBVector3d( (double) lVertex[0], (double) lVertex[1], (double) lVertex[2] );
			pOperationVertex->normal = pSrcNormal[i];
			if (mResetCount)
				pOperationVertex->freeze = 0.0f;
			else
				pOperationVertex->freeze = mBufferDst->vertices[i].freeze;
			pOperationVertex->weight = 0.0f;

			pOperationVertex++;
		}

		// find manipulator and setup data there
		
		if (mManipulator.Ok() )
		{
			ORManip_Sculpt *pManip = (ORManip_Sculpt*) mManipulator.GetPlug();
			pManip->SetModel( pModel, &mBufferSrc );
			mBufferDst = pManip->GetBufferPtr();	
		}

		mFirstTime = false;
		mResetCount = false;
	}

	if (lPlayerControl.IsPlaying == true)
	{
		for (int Count=0; Count<pCount; Count++)
		{
			pDstVertex[Count][0] = pSrcVertex[Count][0];
			pDstVertex[Count][1] = pSrcVertex[Count][1];
			pDstVertex[Count][2] = pSrcVertex[Count][2];

			pDstNormal[Count][0] = pSrcNormal[Count][0];
			pDstNormal[Count][1] = pSrcNormal[Count][1];
			pDstNormal[Count][2] = pSrcNormal[Count][2];
		}
	}
	else
	if (mBufferDst != nullptr && (mBufferDst->vertices.size() == pCount) )
	{
		OperationVertex *pOperationVertex = mBufferDst->vertices.data();

		FBMatrix tm;
		pModel->GetMatrix(tm, kModelInverse_Transformation_Geometry);
		FBVertex lVertex;

		for (int Count=0; Count<pCount; Count++)
		{
			lVertex = FBVertex( (float) pOperationVertex->position[0], (float) pOperationVertex->position[1], (float) pOperationVertex->position[2], 1.0 );
			FBVertexMatrixMult( pDstVertex[Count], tm, lVertex );

			pDstNormal[Count][0] = pOperationVertex->normal[0];
			pDstNormal[Count][1] = pOperationVertex->normal[1];
			pDstNormal[Count][2] = pOperationVertex->normal[2];

			pOperationVertex++;
		}

		return true;
	}

	return false;
}

int BlendShapeDeformerConstraint::ComputeDifference( const OperationBuffer &mBufferSrc, const OperationBuffer &mBufferDst, CDeformerShape *shape  )
{
	if (mBufferSrc.pModel.Ok() == false)
		return 0;

	shape->Free();

	FBModel *pModel = mBufferSrc.pModel;
	FBMatrix tmInv;
	pModel->GetMatrix(tmInv, kModelInverse_Transformation_Geometry);

	int difCount = 0;
	int count = mBufferSrc.vertices.size();

	//FBVector3d *srcPos, *dstPos;

	for (int i=0; i<count; ++i)
	{
		const FBVector3d *srcPos = &mBufferSrc.vertices[i].position;
		const FBVector3d *dstPos = &mBufferDst.vertices[i].position;

		if ( (*srcPos)[0] != (*dstPos)[0] || (*srcPos)[1] != (*dstPos)[1] || (*srcPos)[2] != (*dstPos)[2] )
			difCount += 1;
	}

	if (difCount == 0)
		return 0;

	//
	shape->Init(difCount);

	FBVertex lVertex, lSrcVertex, lDstVertex;
	//FBNormal *lSrcNormal, *lDstNormal;

	difCount = 0;
	for (int i=0; i<count; ++i)
	{
		const FBVector3d *srcPos = &mBufferSrc.vertices[i].position;
		const FBVector3d *dstPos = &mBufferDst.vertices[i].position;

		if ( (*srcPos)[0] != (*dstPos)[0] || (*srcPos)[1] != (*dstPos)[1] || (*srcPos)[2] != (*dstPos)[2] )
		{
			lVertex = FBVertex( (float) mBufferSrc.vertices[i].position[0], (float) mBufferSrc.vertices[i].position[1], (float) mBufferSrc.vertices[i].position[2], 1.0 );
			FBVertexMatrixMult( lSrcVertex, tmInv, lVertex );
			//lSrcVertex = lVertex;
			
			lVertex = FBVertex( (float) mBufferDst.vertices[i].position[0], (float) mBufferDst.vertices[i].position[1], (float) mBufferDst.vertices[i].position[2], 1.0 );
			FBVertexMatrixMult( lDstVertex, tmInv, lVertex );
			//lDstVertex = lVertex;

			shape->difVertices[difCount] = FBVertex( lDstVertex[0]-lSrcVertex[0], lDstVertex[1]-lSrcVertex[1], lDstVertex[2]-lSrcVertex[2], 0.0 );
			
			//
			
			const FBNormal *lSrcNormal = &mBufferSrc.vertices[i].normal;
			const FBNormal *lDstNormal = &mBufferDst.vertices[i].normal;

			shape->difNormals[difCount] = FBNormal( (*lDstNormal)[0]-(*lSrcNormal)[0], (*lDstNormal)[1]-(*lSrcNormal)[1], (*lDstNormal)[2]-(*lSrcNormal)[2], 0.0 );

			//
			shape->origIndices[difCount] = i;

			//shape.creationTM[difCount] = tmInv;

			difCount += 1;
		}
	}

	return difCount;
}

void BlendShapeDeformerConstraint::Reset()
{
	mFirstTime = true;
}

/************************************************
 *	FBX Storage.
 ************************************************/
bool BlendShapeDeformerConstraint::FbxStore( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	ParentClass::FbxStore(pFbxObject, pStoreWhat);

	if (pStoreWhat == kAttributes)
	{
		int count = mAffectedModels.size();
		pFbxObject->FieldWriteI("count", count);

		for (int i=0; i<count; ++i)
		{
			FBModel *pModel = (FBModel*) mAffectedModels.at(i);
			
			int channel = -1;

			for (int j=0; j<pModel->Deformers.GetCount(); ++j)
				if ( FBIS(pModel->Deformers[j], FBDeformerCorrective) )
				{
					FBDeformerCorrective *pDeformer = (FBDeformerCorrective*) pModel->Deformers[j];
					if (pDeformer->Constraint.GetCount() && pDeformer->Constraint.GetAt(0) == this)
					{
						channel = pDeformer->Channel;
						break;
					}
				}

			pFbxObject->FieldWriteObjectReference("model", pModel);
			pFbxObject->FieldWriteI("id", channel);
		}
		
		mManager.FbxStore(pFbxObject, pStoreWhat);
	}

	return true;
}


/************************************************
 *	FBX Retrieval.
 ************************************************/
bool BlendShapeDeformerConstraint::FbxRetrieve( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	ParentClass::FbxRetrieve(pFbxObject, pStoreWhat);

	if (pStoreWhat & kInit)
	{
		ReferenceRemoveAll();
	}
	else
	if (pStoreWhat & kAttributes)
	{
		mAffectedModels.clear();

		int count = pFbxObject->FieldReadI("count");
		if (count > 0)
		{
			mChannelList.resize(count);

			for (int i=0; i<count; ++i)
			{
				FBModel *pModel = FBCast<FBModel>( pFbxObject->FieldReadObjectReference("model") );
				if (pModel)
					mAffectedModels.push_back(pModel);

				mChannelList[i] = pFbxObject->FieldReadI("id");
			}
		}
		else
		{
			mChannelList.clear();
		}

		//
		mManager.FbxRetrieve(pFbxObject, pStoreWhat);
	}
	else
	if (pStoreWhat & kCleanup)
	{
		for (int i=0; i<mAffectedModels.size(); ++i)
		{
			FBModel *pModel = (FBModel*) mAffectedModels.at(i);

			if (mChannelList.size() > i && mChannelList[i] >= 0)
			{
				FBDeformerCorrective *pDeformer = new FBDeformerCorrective(Name);
				pDeformer->Constraint.Add(this);
				pDeformer->Channel = mChannelList[i];
				pModel->Deformers.Add(pDeformer);
			}

			//ReferenceAdd(0, pModel);
		}
	}


	return true;
}

#define BLENDSHAPEDEFORMER_FAILED_CREATE_PROPERTY	((HRESULT)11L)
#define BLENDSHAPEDEFORMER_EMPTY_TEMP_CONSTRAINT	((HRESULT)12L)
#define	BLENDSHAPEDEFORMER_NEWSHAPE_NO_DIFFERENCE	((HRESULT)13L)

int BlendShapeDeformerConstraint::AddBlendShape(	const FBTime &curTime, 
										FBModel *pModel, 
										const OperationBuffer &bufferSrc, 
										const OperationBuffer &bufferDst, 
										const bool setKeyframe, 
										const double timeIn, 
										const double timeLen, 
										const double timeOut, 
										const bool replaceExisting )
{

	WaitForSingleObject( mMutex, INFINITE );

	// 1 - add new animatable property
	
	FBString propName(pModel->Name, ((FBTime&)curTime).GetTimeString( kFBTimeModeDefault, FBTime::eFrame ) );
	CDeformerShape *newShape = new CDeformerShape();
	
	FBPropertyAnimatableDouble *pProp = dynamic_cast<FBPropertyAnimatableDouble*> (PropertyCreate( propName, kFBPT_double, ANIMATIONNODE_TYPE_NUMBER, true, false ));
	if (pProp == nullptr) 
		return BLENDSHAPEDEFORMER_FAILED_CREATE_PROPERTY;

	pProp->SetMinMax(0.0, 100.0);
	double value = 100.0;
	pProp->SetData(&value);

	PropertyAdd( pProp );
	newShape->pProperty = pProp;
	newShape->strPropName = propName;
	
	try
	{
		//
		int difCount = BlendShapeDeformerConstraint::ComputeDifference( bufferSrc, bufferDst, newShape);

		if (0 == difCount)
			throw BLENDSHAPEDEFORMER_NEWSHAPE_NO_DIFFERENCE;

		//
		newShape->pProperty = pProp;
		if (false == mManager.AddShape( curTime, this, pModel, pProp, newShape, replaceExisting ) )
			throw S_FALSE;
		
		// 3 - animate property if needed

		if (ExclusiveMode && setKeyframe)
		{
			mManager.KeyAllShapes( curTime, this, pModel );
		}

		if (setKeyframe)
		{
			double ValueOne = 100.0;
			double ValueZero = 0.0;

			FBTime lTimeIn(0,0,0, timeIn);
			FBTime lTimeOut(0,0,0, timeOut);
			FBTime lA(curTime);
			FBTime lB(curTime);
			FBTime lLen(0,0,0, timeLen * 0.5);

			lA -= lLen;
			lB += lLen;

			lTimeIn = lA - lTimeIn;
			lTimeOut = lB+ lTimeOut;

			pProp->SetAnimated(true);
			FBAnimationNode *pNode = pProp->GetAnimationNode();

			if (pNode)
			{
				pNode->KeyAdd(lA, &ValueOne, kFBInterpolationCubic, kFBTangentModeUser);

				if (lA != lB)
					pNode->KeyAdd(lB, &ValueOne, kFBInterpolationCubic, kFBTangentModeUser);
				if (lA != lTimeIn)
					pNode->KeyAdd(lTimeIn, &ValueZero, kFBInterpolationCubic, kFBTangentModeUser);
				if (lA != lTimeOut)
					pNode->KeyAdd(lTimeOut, &ValueZero, kFBInterpolationCubic, kFBTangentModeUser);
			}	

			pProp->SetData( &ValueOne );
			pProp->SetFocus(true);
		}

	}
	catch (const HRESULT error_code)
	{
		PropertyRemove(pProp);
		delete newShape;

		ReleaseMutex(mMutex);

		return error_code;
	}

	ReleaseMutex(mMutex);

	return S_OK;
}

const int BlendShapeDeformerConstraint::GetNumberOfChannels() const
{
	return mManager.GetNumberOfChannels();
}

const char *BlendShapeDeformerConstraint::GetChannelName(const int index) const
{
	return mManager.GetChannelName(index);
}

CDeformerChannel *BlendShapeDeformerConstraint::GetChannelPtr(const int index)
{
	return mManager.GetChannelPtr(index);
}

const int BlendShapeDeformerConstraint::GetNumberOfShapes(const int channel) const
{
	return mManager.GetNumberOfShapes(channel);
}

const char *BlendShapeDeformerConstraint::GetShapeName(const int channel, const int shape) const
{
	return mManager.GetShapeName(channel, shape);
}

FBPropertyAnimatableDouble *BlendShapeDeformerConstraint::GetShapeProperty(const int channel, const int shape)
{
	return mManager.GetShapeProperty(channel, shape);
}

void BlendShapeDeformerConstraint::ZeroAll()
{
	mManager.ZeroAll();
}

void BlendShapeDeformerConstraint::SelectShapes(FBModel *pModel)
{
	mManager.SelectShapes(this, pModel);
}

void BlendShapeDeformerConstraint::DeselectShapes()
{
	mManager.DeselectShapes();
}

void BlendShapeDeformerConstraint::Plot( CDeformerChannel *pChannel, const FBTime &startTime, const FBTime &stopTime, const FBTime &stepTime )
{
	if (pChannel == nullptr || pChannel->numberOfVertices <= 0)
		return;

	//FBPlayerControl &lPlayerControl = FBPlayerControl::TheOne();
	//FBScene *pScene = FBSystem::TheOne().Scene;

	FBTime currTime(startTime);

	// compute new number of shapes
	int numberOfNewShapes = 0;

	while(currTime <= stopTime)
	{
		numberOfNewShapes += 1;
		currTime += stepTime;
	}

	if (numberOfNewShapes == 0)
		return;

	std::vector<CDeformerShape*>		newShapes;
	newShapes.resize(numberOfNewShapes);

	// now lets bake total frame transform into each new shape
	int pCount = pChannel->numberOfVertices;

	FBVertex *pSrcVertex = new FBVertex[pCount];
	FBNormal *pSrcNormal = new FBNormal[pCount];
	FBVertex *pDstVertex = new FBVertex[pCount];
	FBNormal *pDstNormal = new FBNormal[pCount];

	memset( pSrcVertex, 0, sizeof(FBVertex) * pCount );
	memset( pSrcNormal, 0, sizeof(FBNormal) * pCount );

	currTime = startTime;
	numberOfNewShapes = 0;	// prepare for indexing
	while(currTime <= stopTime)
	{
		// prepare shade for deform capturing
		
		CDeformerShape *pNewShape = new CDeformerShape();
		
		
		// capture deformations at currTime

		memset( pDstVertex, 0, sizeof(FBVertex) * pCount );
		memset( pDstNormal, 0, sizeof(FBNormal) * pCount );

		if (mManager.Process( this, pChannel, currTime, pSrcVertex, pSrcNormal, pCount, pDstVertex, pDstNormal, ApplyOnlyOnKeyframe, ExclusiveMode ) )
		{
			pNewShape->CatchDifference(pSrcVertex, pSrcNormal, pCount, pDstVertex, pDstNormal);
		}

		//
		
		newShapes[numberOfNewShapes] = pNewShape;
		numberOfNewShapes += 1;
		
		
		currTime += stepTime;
	}

	// free mem
	delete [] pSrcVertex;
	delete [] pSrcNormal;
	delete [] pDstVertex;
	delete [] pDstNormal;

	// udpate shapes
	WaitForSingleObject( mMutex, INFINITE );

	pChannel->RemoveProperties(this);
	pChannel->Free();

	// add new properties
	numberOfNewShapes = 0;	// prepare for indexing
	currTime = startTime;

	char propName[128];
	memset(propName, 0, sizeof(char)*128);

	while(currTime <= stopTime)
	{
		CDeformerShape *pNewShape = newShapes[numberOfNewShapes];

		sprintf_s(propName, sizeof(char)*128, "bakedShape %u", static_cast<unsigned int>(currTime.GetFrame()) ); 
		FBPropertyAnimatableDouble *pProp = dynamic_cast<FBPropertyAnimatableDouble*> (PropertyCreate( propName, kFBPT_double, ANIMATIONNODE_TYPE_NUMBER, true, false ));
		
		if (pProp)
		{
			pProp->SetMinMax(0.0, 100.0);
			double value = 100.0;
			pProp->SetData(&value);

			PropertyAdd( pProp );
			
			pNewShape->pProperty = pProp;
			pNewShape->strPropName = propName;

			pProp->SetAnimated(true);
			FBAnimationNode *pNode = pProp->GetAnimationNode();
			if (pNode)
				pNode->KeyAdd( currTime, &value, kFBInterpolationConstant );
		}

		pNewShape->UpdateCache();

		numberOfNewShapes += 1;
		currTime += stepTime;
	}

	mManager.AddShapes( pChannel, newShapes );

	ReleaseMutex( mMutex );

}