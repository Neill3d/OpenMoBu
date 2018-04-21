
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

/**	\file	handTool.cxx
*/

//--- Class declaration
#include "handTool.h"

//--- Registration defines
#define HANDTOOL__CLASS		HANDTOOL__CLASSNAME
#define HANDTOOL__LABEL		"Hand Controls"
#define HANDTOOL__DESC		"Hand parametric animation"

//--- FiLMBOX implementation and registration
FBToolImplementation(	HANDTOOL__CLASS	);
FBRegisterTool		(	HANDTOOL__CLASS,
						HANDTOOL__LABEL,
						HANDTOOL__DESC,
						FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)


/************************************************
 *	CFinger Constructor.
 ************************************************/
CFinger::CFinger() 
{
	mFinger[0] = NULL;
	mFinger[1] = NULL;
	mFinger[2] = NULL;

	mRotName		= "zRotation";
	mRotId			= 99;
	mRotDataType	= ANIMATIONNODE_TYPE_NUMBER;
	mRotType		= kFBPT_double;

	mDampName		= "damp";
	mDampId			= 100;
	mDampDataType	= ANIMATIONNODE_TYPE_NUMBER;
	mDampType		= kFBPT_double;

	mWeightName		= "weight";
	mWeightId		= 101;
	mWeightDataType	= ANIMATIONNODE_TYPE_NUMBER;
	mWeightType		= kFBPT_double;

	mInheritName	= "inherit";
	mInheritId		= 102;
	mInheritDataType= ANIMATIONNODE_TYPE_VECTOR;
	mInheritType	= kFBPT_Vector3D;

	mSpreadName		= "Spread";
	mSpreadId		= 103;
	mSpreadDataType	= ANIMATIONNODE_TYPE_NUMBER;
	mSpreadType		= kFBPT_double;

	for (int i=0; i<FINGER_ELEMS_COUNT; i++)
	{
		mFingerBox[i]		= NULL;
		mFingerBoxSrc[i]	= NULL;
		mVectorToNumber[i]	= NULL;
		mNumberToVector[i]	= NULL;
		mAddBox[i]			= NULL;
		mAddBox2			= NULL;
		mMultBox[i]			= NULL;
		mDampBox[i]			= NULL;
		mDampBox2			= NULL;
	}
}


void CFinger::CleanUP()
{

	for (int i=0; i<FINGER_ELEMS_COUNT; i++)
	{
		mFinger[i] = NULL;
		mSkeletFinger[i] = NULL;

		//-- UI property's
		mRotProperty.Property = 0;
		mSpreadProperty.Property = 0;
		mDampProperty.Property = 0;
		mWeightProperty.Property = 0;
		mInheritProperty.Property = 0;

		//-- relation objects
		mFingerBox[i]		= NULL;
		mFingerBoxSrc[i]	= NULL;
		mVectorToNumber[i]	= NULL;
		mNumberToVector[i]	= NULL;
		mAddBox[i]			= NULL;
		mMultBox[i]			= NULL;
		mDampBox[i]			= NULL;
		mAddBox2			= NULL;
		mDampBox2			= NULL;
	}

}


CFinger::~CFinger()
{
	/*
	if (mModel) 
	{
		mModel->FBDelete();
		mModel = NULL;
	}
	*/
}

/************************************************
 *	CFinger Add model & property
 ************************************************/
void CFinger::AddPropertyToModel( FBModel *pCtrlRigModel, FBModel *pModel )
{
	mFinger[0] = mFinger[1] = mFinger[2] = nullptr;
	mSkeletFinger[0] = mSkeletFinger[1] = mSkeletFinger[2] = nullptr;

	mFinger[0] = pCtrlRigModel;
	if (pCtrlRigModel && pCtrlRigModel->Children.GetCount() > 0)
	{
		mFinger[1] = pCtrlRigModel->Children[0];

		if (pCtrlRigModel->Children[0]->Children.GetCount() > 0)
		{
			mFinger[2] = pCtrlRigModel->Children[0]->Children[0];
		}
	}

	mSkeletFinger[0] = pModel;
	if (pModel && pModel->Children.GetCount() > 0)
	{
		mSkeletFinger[1] = pModel->Children[0];
		if (pModel->Children[0]->Children.GetCount() > 0)
		{
			mSkeletFinger[2] = pModel->Children[0]->Children[0];
		}
	}

	if (pCtrlRigModel)
	{
		//--------------------------------- add current angle property
		FBProperty *lRotProp = NULL;

		lRotProp = pCtrlRigModel->PropertyList.Find( mRotName );
		if ( !lRotProp )
		{
			lRotProp = pCtrlRigModel->PropertyCreate( mRotName, mRotType, mRotDataType, true, true );
			
			if ( lRotProp->IsAnimatable() ) {
				FBPropertyAnimatable *lRotAnimatable = (FBPropertyAnimatable*) lRotProp;
				lRotAnimatable->SetAnimated( true );

				FBAnimationNode *lAnimationNode = lRotAnimatable->GetAnimationNode();
				if ( lAnimationNode ) {
					lAnimationNode->Reference = mRotId;
				}
			}
		}

		lRotProp->SetMin( -180.0 );
		lRotProp->SetMax( 180.0 );
		mRotProperty.Property = lRotProp;


		//--------------------------------- add spread angle property
		FBProperty *lSpreadProp = NULL;

		lSpreadProp = pCtrlRigModel->PropertyList.Find( mSpreadName );
		if ( !lSpreadProp )
		{
			lSpreadProp = pCtrlRigModel->PropertyCreate( mSpreadName, mSpreadType, mSpreadDataType, true, true );
			
			if ( lSpreadProp->IsAnimatable() ) {
				FBPropertyAnimatable *lSpreadAnimatable = (FBPropertyAnimatable*) lSpreadProp;
				lSpreadAnimatable->SetAnimated( true );

				FBAnimationNode *lAnimationNode = lSpreadAnimatable->GetAnimationNode();
				if ( lAnimationNode ) {
					lAnimationNode->Reference = mSpreadId;
				}
			}
		}

		lSpreadProp->SetMin( -180.0 );
		lSpreadProp->SetMax( 180.0 );
		mSpreadProperty.Property = lSpreadProp;

		//--------------------------------- damping factor property
		FBProperty *lDampProp = NULL;

		lDampProp = pCtrlRigModel->PropertyList.Find( mDampName );
		if ( !lDampProp )
		{
			lDampProp = pCtrlRigModel->PropertyCreate( mDampName, mDampType, mDampDataType, true, true );

			if ( lDampProp->IsAnimatable() ) {
				FBPropertyAnimatable *lDampAnimatable = (FBPropertyAnimatable*) lDampProp;
				lDampAnimatable->SetAnimated( true );

				FBAnimationNode *lAnimationNode = lDampAnimatable->GetAnimationNode();
				if ( lAnimationNode ) {
					lAnimationNode->Reference = mDampId;
				}
			}
		}

		mDampProperty.Property = lDampProp;

		//--------------------------------- weight for this finger property
		FBProperty *lWeightProp = NULL;

		lWeightProp = pCtrlRigModel->PropertyList.Find( mWeightName );
		if ( !lWeightProp )
		{
			lWeightProp = pCtrlRigModel->PropertyCreate( mWeightName, mWeightType, mWeightDataType, true, true );

			if ( lWeightProp->IsAnimatable() ) {
				FBPropertyAnimatable *lWeightAnimatable = (FBPropertyAnimatable*) lWeightProp;
				lWeightAnimatable->SetAnimated( true );

				FBAnimationNode *lAnimationNode = lWeightAnimatable->GetAnimationNode();
				if ( lAnimationNode ) {
					lAnimationNode->Reference = mWeightId;
				}
			}
		}

		lWeightProp->SetMin( -100.0 );
		lWeightProp->SetMax( 100.0 );
		mWeightProperty.Property = lWeightProp;


		//--------------------------------- weight for this finger property
		FBProperty *lInheritProp = NULL;

		lInheritProp = pCtrlRigModel->PropertyList.Find( mInheritName );
		if ( !lInheritProp )
		{
			lInheritProp = pCtrlRigModel->PropertyCreate( mInheritName, mInheritType, mInheritDataType, true, true );

			if ( lInheritProp->IsAnimatable() ) {
				FBPropertyAnimatable *lInheritAnimatable = (FBPropertyAnimatable*) lInheritProp;
				lInheritAnimatable->SetAnimated( true );

				FBAnimationNode *lAnimationNode = lInheritAnimatable->GetAnimationNode();
				if ( lAnimationNode ) {
					lAnimationNode->Reference = mInheritId;
				}
			}
		}

		FBVector3d lInitialValue( 100.0, 100.0, 100.0 );

		lInheritProp->SetData( &lInitialValue );
		mInheritProperty.Property = lInheritProp;
	}
}

/************************************************
 * Remove property
 ************************************************/
void CFinger::RemovePropertyFromModel()
{
	if (mFinger[0])
	{
		//--------------------------------- remove current angle property
		FBProperty *lRotProp = NULL;

		mRotProperty.Property = 0;

		lRotProp = mFinger[0]->PropertyList.Find( mRotName );
		if ( lRotProp )
		{
			mFinger[0]->PropertyRemove( lRotProp );
			lRotProp = NULL;
		}

		//--------------------------------- remove spread property
		FBProperty *lSpreadProp = NULL;

		mSpreadProperty.Property = 0;

		lSpreadProp = mFinger[0]->PropertyList.Find( mSpreadName );
		if ( lSpreadProp )
		{
			mFinger[0]->PropertyRemove( lSpreadProp );
			lSpreadProp = NULL;
		}

		//--------------------------------- remove damping factor property
		FBProperty *lDampProp = NULL;

		mDampProperty.Property = 0;

		lDampProp = mFinger[0]->PropertyList.Find( mDampName );
		if ( lDampProp )
		{
			mFinger[0]->PropertyRemove( lDampProp );
			lDampProp = NULL;
		}

		//--------------------------------- remove weight for this finger property
		FBProperty *lWeightProp = NULL;

		mWeightProperty.Property = 0;

		lWeightProp = mFinger[0]->PropertyList.Find( mWeightName );
		if ( lWeightProp )
		{
			mFinger[0]->PropertyRemove( lWeightProp );
			lWeightProp = NULL;
		}

		//--------------------------------- remove weight for this finger property
		FBProperty *lInheritProp = NULL;

		mInheritProperty.Property = 0;

		lInheritProp = mFinger[0]->PropertyList.Find( mInheritName );
		if ( lInheritProp )
		{
			 mFinger[0]->PropertyRemove( lInheritProp );
			 lInheritProp = NULL;
		}
	}
}

/************************************************
 * find previous/next key.
 ************************************************/
bool CFinger::GetPrevKeyTime( const FBTime &startTime, const FBTime &currTime, FBTime &lTime )
{
	bool						Result	= false;
	FBTime						lKeyTime;

	FBProperty					*lProp;
	FBPropertyAnimatable		*lAnimatable;
	FBAnimationNode				*lAnimationNode;
	FBFCurve					*lFCurve;

	lTime = startTime;

	if (mFinger[0] && mFinger[1] && mFinger[2])
		if (mFinger[0]->Selected || mFinger[1]->Selected || mFinger[2]->Selected)
		{
			//-- prev from rotation keys
			lProp = mFinger[0]->PropertyList.Find( mRotName );
			if ( lProp )
			{
				if ( lProp->IsAnimatable() ) {
					lAnimatable = (FBPropertyAnimatable*) lProp;
				
					lAnimationNode = lAnimatable->GetAnimationNode();
					if ( lAnimationNode ) {
						lFCurve = lAnimationNode->FCurve;

						if ( lFCurve )
						{
							for( int i=0; i<lAnimationNode->KeyCount; i++)
							{
								lKeyTime = lFCurve->Keys[i].Time;
								if (lKeyTime >= lTime && lKeyTime < currTime)
								{
									lTime = lFCurve->Keys[i].Time;
									Result = true;
								}
							}
						}
					}
				}
			}

			//-- prev from spread keys
			lProp = mFinger[0]->PropertyList.Find( mSpreadName );
			if ( lProp )
			{
				if ( lProp->IsAnimatable() ) {
					lAnimatable = (FBPropertyAnimatable*) lProp;
				
					lAnimationNode = lAnimatable->GetAnimationNode();
					if ( lAnimationNode ) {
						lFCurve = lAnimationNode->FCurve;

						if ( lFCurve )
						{
							for( int i=0; i<lAnimationNode->KeyCount; i++)
							{
								lKeyTime = lFCurve->Keys[i].Time;
								if (lKeyTime >= lTime && lKeyTime < currTime)
								{
									lTime = lFCurve->Keys[i].Time;
									Result = true;
								}
							}
						}
					}
				}
			}

			//-- prev from weight keys
			lProp = mFinger[0]->PropertyList.Find( mWeightName );
			if ( lProp )
			{
				if ( lProp->IsAnimatable() ) {
					lAnimatable = (FBPropertyAnimatable*) lProp;
				
					lAnimationNode = lAnimatable->GetAnimationNode();
					if ( lAnimationNode ) {
						lFCurve = lAnimationNode->FCurve;

						if ( lFCurve )
						{
							for( int i=0; i<lAnimationNode->KeyCount; i++)
							{
								lKeyTime = lFCurve->Keys[i].Time;
								if (lKeyTime >= lTime && lKeyTime < currTime)
								{
									lTime = lFCurve->Keys[i].Time;
									Result = true;
								}
							}
						}
					}
				}
			}

			//-- prev from damping keys
			lProp = mFinger[0]->PropertyList.Find( mDampName );
			if ( lProp )
			{
				if ( lProp->IsAnimatable() ) {
					lAnimatable = (FBPropertyAnimatable*) lProp;
				
					lAnimationNode = lAnimatable->GetAnimationNode();
					if ( lAnimationNode ) {
						lFCurve = lAnimationNode->FCurve;

						if ( lFCurve )
						{
							for( int i=0; i<lAnimationNode->KeyCount; i++)
							{
								lKeyTime = lFCurve->Keys[i].Time;
								if (lKeyTime >= lTime && lKeyTime < currTime)
								{
									lTime = lFCurve->Keys[i].Time;
									Result = true;
								}
							}
						}
					}
				}
			}

			//-- prev from inherit keys
			lProp = mFinger[0]->PropertyList.Find( mInheritName );
			if ( lProp )
			{
				if ( lProp->IsAnimatable() ) {
					lAnimatable = (FBPropertyAnimatable*) lProp;
				
					//-- search in X node
					lAnimationNode = lAnimatable->GetAnimationNode()->Nodes[0];
					if ( lAnimationNode ) {
						lFCurve = lAnimationNode->FCurve;

						if ( lFCurve )
						{
							for( int i=0; i<lAnimationNode->KeyCount; i++)
							{
								lKeyTime = lFCurve->Keys[i].Time;
								if (lKeyTime >= lTime && lKeyTime < currTime)
								{
									lTime = lFCurve->Keys[i].Time;
									Result = true;
								}
							}
						}
					}

					//-- search in Y node
					lAnimationNode = lAnimatable->GetAnimationNode()->Nodes[1];
					if ( lAnimationNode ) {
						lFCurve = lAnimationNode->FCurve;

						if ( lFCurve )
						{
							for( int i=0; i<lAnimationNode->KeyCount; i++)
							{
								lKeyTime = lFCurve->Keys[i].Time;
								if (lKeyTime >= lTime && lKeyTime < currTime)
								{
									lTime = lFCurve->Keys[i].Time;
									Result = true;
								}
							}
						}
					}

					//-- search in Z node
					lAnimationNode = lAnimatable->GetAnimationNode()->Nodes[2];
					if ( lAnimationNode ) {
						lFCurve = lAnimationNode->FCurve;

						if ( lFCurve )
						{
							for( int i=0; i<lAnimationNode->KeyCount; i++)
							{
								lKeyTime = lFCurve->Keys[i].Time;
								if (lKeyTime >= lTime && lKeyTime < currTime)
								{
									lTime = lFCurve->Keys[i].Time;
									Result = true;
								}
							}
						}
					}
				}
			}
		}

	return Result;
}


bool CFinger::GetNextKeyTime( const FBTime &endTime, const FBTime &currTime, FBTime &lTime )
{
	bool						Result = false;
	FBTime						lKeyTime;

	FBProperty					*lProp;
	FBPropertyAnimatable		*lAnimatable;
	FBAnimationNode				*lAnimationNode;
	FBFCurve					*lFCurve;

	lTime = endTime;

	if (mFinger[0] && mFinger[1] && mFinger[2])
		if (mFinger[0]->Selected || mFinger[1]->Selected || mFinger[2]->Selected)
		{
			//-- prev from rotation keys
			lProp = mFinger[0]->PropertyList.Find( mRotName );
			if ( lProp )
			{
				if ( lProp->IsAnimatable() ) {
					lAnimatable = (FBPropertyAnimatable*) lProp;
				
					lAnimationNode = lAnimatable->GetAnimationNode();
					if ( lAnimationNode ) {
						lFCurve = lAnimationNode->FCurve;

						if ( lFCurve )
						{
							for( int i=0; i<lAnimationNode->KeyCount; i++)
							{
								lKeyTime = lFCurve->Keys[i].Time;
								if (lKeyTime <= lTime && lKeyTime > currTime)
								{
									lTime = lFCurve->Keys[i].Time;
									Result = true;
								}
							}
						}
					}
				}
			}

			//-- prev from spread keys
			lProp = mFinger[0]->PropertyList.Find( mSpreadName );
			if ( lProp )
			{
				if ( lProp->IsAnimatable() ) {
					lAnimatable = (FBPropertyAnimatable*) lProp;
				
					lAnimationNode = lAnimatable->GetAnimationNode();
					if ( lAnimationNode ) {
						lFCurve = lAnimationNode->FCurve;

						if ( lFCurve )
						{
							for( int i=0; i<lAnimationNode->KeyCount; i++)
							{
								lKeyTime = lFCurve->Keys[i].Time;
								if (lKeyTime <= lTime && lKeyTime > currTime)
								{
									lTime = lFCurve->Keys[i].Time;
									Result = true;
								}
							}
						}
					}
				}
			}

			//-- prev from weight keys
			lProp = mFinger[0]->PropertyList.Find( mWeightName );
			if ( lProp )
			{
				if ( lProp->IsAnimatable() ) {
					lAnimatable = (FBPropertyAnimatable*) lProp;
				
					lAnimationNode = lAnimatable->GetAnimationNode();
					if ( lAnimationNode ) {
						lFCurve = lAnimationNode->FCurve;

						if ( lFCurve )
						{
							for( int i=0; i<lAnimationNode->KeyCount; i++)
							{
								lKeyTime = lFCurve->Keys[i].Time;
								if (lKeyTime <= lTime && lKeyTime > currTime)
								{
									lTime = lFCurve->Keys[i].Time;
									Result = true;
								}
							}
						}
					}
				}
			}

			//-- prev from damping keys
			lProp = mFinger[0]->PropertyList.Find( mDampName );
			if ( lProp )
			{
				if ( lProp->IsAnimatable() ) {
					lAnimatable = (FBPropertyAnimatable*) lProp;
				
					lAnimationNode = lAnimatable->GetAnimationNode();
					if ( lAnimationNode ) {
						lFCurve = lAnimationNode->FCurve;

						if ( lFCurve )
						{
							for( int i=0; i<lAnimationNode->KeyCount; i++)
							{
								lKeyTime = lFCurve->Keys[i].Time;
								if (lKeyTime <= lTime && lKeyTime > currTime)
								{
									lTime = lFCurve->Keys[i].Time;
									Result = true;
								}
							}
						}
					}
				}
			}

			//-- prev from inherit keys
			lProp = mFinger[0]->PropertyList.Find( mInheritName );
			if ( lProp )
			{
				if ( lProp->IsAnimatable() ) {
					lAnimatable = (FBPropertyAnimatable*) lProp;
				
					//-- search in X node
					lAnimationNode = lAnimatable->GetAnimationNode()->Nodes[0];
					if ( lAnimationNode ) {
						lFCurve = lAnimationNode->FCurve;

						if ( lFCurve )
						{
							for( int i=0; i<lAnimationNode->KeyCount; i++)
							{
								lKeyTime = lFCurve->Keys[i].Time;
								if (lKeyTime <= lTime && lKeyTime > currTime)
								{
									lTime = lFCurve->Keys[i].Time;
									Result = true;
								}
							}
						}
					}

					//-- search in Y node
					lAnimationNode = lAnimatable->GetAnimationNode()->Nodes[1];
					if ( lAnimationNode ) {
						lFCurve = lAnimationNode->FCurve;

						if ( lFCurve )
						{
							for( int i=0; i<lAnimationNode->KeyCount; i++)
							{
								lKeyTime = lFCurve->Keys[i].Time;
								if (lKeyTime <= lTime && lKeyTime > currTime)
								{
									lTime = lFCurve->Keys[i].Time;
									Result = true;
								}
							}
						}
					}

					//-- search in Z node
					lAnimationNode = lAnimatable->GetAnimationNode()->Nodes[2];
					if ( lAnimationNode ) {
						lFCurve = lAnimationNode->FCurve;

						if ( lFCurve )
						{
							for( int i=0; i<lAnimationNode->KeyCount; i++)
							{
								lKeyTime = lFCurve->Keys[i].Time;
								if (lKeyTime <= lTime && lKeyTime > currTime)
								{
									lTime = lFCurve->Keys[i].Time;
									Result = true;
								}
							}
						}
					}
				}
			}
		}

	return Result;
}

/************************************************
 * Set key to all user properties.
 ************************************************/
void CFinger::SetKey()
{
	if (mFinger[0] && mFinger[1] && mFinger[2])
		if (mFinger[0]->Selected || mFinger[1]->Selected || mFinger[2]->Selected)
		{
			double value;
			FBVector3d v;
			
			//-- set rotation key
			FBProperty *lRotProp = mFinger[0]->PropertyList.Find( mRotName );
			if ( lRotProp )
			{
				lRotProp->GetData( &value, sizeof(double) );

				if ( lRotProp->IsAnimatable() ) {
					FBPropertyAnimatable *lRotAnimatable = (FBPropertyAnimatable*) lRotProp;
				
					FBAnimationNode *lAnimationNode = lRotAnimatable->GetAnimationNode();
					if ( lAnimationNode ) {
						lAnimationNode->KeyAdd( &value );
					}
				}
			}

			//-- set spread key
			FBProperty *lSpreadProp = mFinger[0]->PropertyList.Find( mSpreadName );
			if ( lSpreadProp )
			{
				lSpreadProp->GetData( &value, sizeof(double) );

				if ( lSpreadProp->IsAnimatable() ) {
					FBPropertyAnimatable *lSpreadAnimatable = (FBPropertyAnimatable*) lSpreadProp;
				
					FBAnimationNode *lAnimationNode = lSpreadAnimatable->GetAnimationNode();
					if ( lAnimationNode ) {
						lAnimationNode->KeyAdd( &value );
					}
				}
			}

			//-- set weight key
			FBProperty *lWeightProp = mFinger[0]->PropertyList.Find( mWeightName );
			if ( lWeightProp )
			{
				lWeightProp->GetData( &value, sizeof(double) );

				if ( lWeightProp->IsAnimatable() ) {
					FBPropertyAnimatable *lWeightAnimatable = (FBPropertyAnimatable*) lWeightProp;
				
					FBAnimationNode *lAnimationNode = lWeightAnimatable->GetAnimationNode();
					if ( lAnimationNode ) {
						lAnimationNode->KeyAdd( &value );
					}
				}
			}

			//-- set damp key
			FBProperty *lDampProp = mFinger[0]->PropertyList.Find( mDampName );
			if ( lDampProp )
			{
				lDampProp->GetData( &value, sizeof(double) );

				if ( lDampProp->IsAnimatable() ) {
					FBPropertyAnimatable *lDampAnimatable = (FBPropertyAnimatable*) lDampProp;
				
					FBAnimationNode *lAnimationNode = lDampAnimatable->GetAnimationNode();
					if ( lAnimationNode ) {
						lAnimationNode->KeyAdd( &value );
					}
				}
			}

			//-- set inherit keys
			FBProperty *lInheritProp = mFinger[0]->PropertyList.Find( mInheritName );
			if ( lInheritProp )
			{
				lInheritProp->GetData( &v, sizeof(FBVector3d) );

				if ( lInheritProp->IsAnimatable() ) {
					FBPropertyAnimatable *lInheritAnimatable = (FBPropertyAnimatable*) lInheritProp;
				
					//-- must be setUP for XYZ (three nodes)
					FBAnimationNode *lAnimationNode = lInheritAnimatable->GetAnimationNode()->Nodes[0];
					if ( lAnimationNode ) {
						lAnimationNode->KeyAdd( &v[0] );
					}
					lAnimationNode = lInheritAnimatable->GetAnimationNode()->Nodes[1];
					if ( lAnimationNode ) {
						lAnimationNode->KeyAdd( &v[1] );
					}
					lAnimationNode = lInheritAnimatable->GetAnimationNode()->Nodes[2];
					if ( lAnimationNode ) {
						lAnimationNode->KeyAdd( &v[2] );
					}
				}
			}
		}
}

/************************************************
 * Remove key to all user properties.
 ************************************************/
void CFinger::RemoveKey()
{
	if (mFinger[0] && mFinger[1] && mFinger[2])
		if (mFinger[0]->Selected || mFinger[1]->Selected || mFinger[2]->Selected)
		{
			//-- remove rotation key
			FBProperty *lRotProp = mFinger[0]->PropertyList.Find( mRotName );
			if ( lRotProp )
			{
				if ( lRotProp->IsAnimatable() ) {
					FBPropertyAnimatable *lRotAnimatable = (FBPropertyAnimatable*) lRotProp;
				
					FBAnimationNode *lAnimationNode = lRotAnimatable->GetAnimationNode();
					if ( lAnimationNode ) {
						lAnimationNode->KeyRemove();
					}
				}
			}

			//-- remove spread key
			FBProperty *lSpreadProp = mFinger[0]->PropertyList.Find( mSpreadName );
			if ( lSpreadProp )
			{
				if ( lSpreadProp->IsAnimatable() ) {
					FBPropertyAnimatable *lSpreadAnimatable = (FBPropertyAnimatable*) lSpreadProp;
				
					FBAnimationNode *lAnimationNode = lSpreadAnimatable->GetAnimationNode();
					if ( lAnimationNode ) {
						lAnimationNode->KeyRemove();
					}
				}
			}

			//-- remove weight key
			FBProperty *lWeightProp = mFinger[0]->PropertyList.Find( mWeightName );
			if ( lWeightProp )
			{
				if ( lWeightProp->IsAnimatable() ) {
					FBPropertyAnimatable *lWeightAnimatable = (FBPropertyAnimatable*) lWeightProp;
				
					FBAnimationNode *lAnimationNode = lWeightAnimatable->GetAnimationNode();
					if ( lAnimationNode ) {
						lAnimationNode->KeyRemove();
					}
				}
			}

			//-- remove damp key
			FBProperty *lDampProp = mFinger[0]->PropertyList.Find( mDampName );
			if ( lDampProp )
			{
				if ( lDampProp->IsAnimatable() ) {
					FBPropertyAnimatable *lDampAnimatable = (FBPropertyAnimatable*) lDampProp;
				
					FBAnimationNode *lAnimationNode = lDampAnimatable->GetAnimationNode();
					if ( lAnimationNode ) {
						lAnimationNode->KeyRemove();
					}
				}
			}

			//-- remove inherit keys
			FBProperty *lInheritProp = mFinger[0]->PropertyList.Find( mInheritName );
			if ( lInheritProp )
			{
				if ( lInheritProp->IsAnimatable() ) {
					FBPropertyAnimatable *lInheritAnimatable = (FBPropertyAnimatable*) lInheritProp;
				
					//-- must be setUP for XYZ (three nodes)
					FBAnimationNode *lAnimationNode = lInheritAnimatable->GetAnimationNode()->Nodes[0];
					if ( lAnimationNode ) {
						lAnimationNode->KeyRemove();
					}
					lAnimationNode = lInheritAnimatable->GetAnimationNode()->Nodes[1];
					if ( lAnimationNode ) {
						lAnimationNode->KeyRemove();
					}
					lAnimationNode = lInheritAnimatable->GetAnimationNode()->Nodes[2];
					if ( lAnimationNode ) {
						lAnimationNode->KeyRemove();
					}
				}
			}
		}
}

/************************************************
 * Finger ROTATION property setUP.
 ************************************************/
void CFinger::SetRotation( double value )
{

	if (mFinger[0] && mFinger[1] && mFinger[2])
		if (mFinger[0]->Selected || mFinger[1]->Selected || mFinger[2]->Selected)
		{
			FBProperty *lRotProp = mFinger[0]->PropertyList.Find( mRotName );
			if ( lRotProp )
			{
				if ( lRotProp->IsAnimatable() ) {
					FBPropertyAnimatable *lRotAnimatable = (FBPropertyAnimatable*) lRotProp;
				
					lRotAnimatable->SetData( &value );
					lRotAnimatable->Key();
					FBAnimationNode *lAnimationNode = lRotAnimatable->GetAnimationNode();
					if ( lAnimationNode ) {
						lAnimationNode->KeyAdd( &value );
					}
				}
			
				//lRotProp->SetInt( (int) value );
				lRotProp->SetData( &value );
			}
		}
}

//-- relative mode
void CFinger::AddRotation( double value )
{
	double temp;

	if (mFinger[0] && mFinger[1] && mFinger[2])
		if (mFinger[0]->Selected || mFinger[1]->Selected || mFinger[2]->Selected)
		{
			FBProperty *lRotProp = mFinger[0]->PropertyList.Find( mRotName );
			if ( lRotProp )
			{
				lRotProp->GetData( &temp, sizeof(double) );
				temp += value;

				if ( lRotProp->IsAnimatable() ) {
					FBPropertyAnimatable *lRotAnimatable = (FBPropertyAnimatable*) lRotProp;
				
					lRotAnimatable->SetData( &temp );
					lRotAnimatable->Key();
					FBAnimationNode *lAnimationNode = lRotAnimatable->GetAnimationNode();
					if ( lAnimationNode ) {
						lAnimationNode->KeyAdd( &temp );
					}
				}
			
				//lRotProp->SetInt( (int) temp );
				lRotProp->SetData( &temp );
			}
		}
}

double CFinger::GetRotation()
{
	double value = 0.0;

	if (mFinger[0])
	{
		FBProperty *lRotProp = mFinger[0]->PropertyList.Find( mRotName );
			if ( lRotProp )
			{
				lRotProp->GetData( &value, sizeof(double) );
			}
	}

	return value;
}

/************************************************
 * finger SPREAD property setUp.
 ************************************************/
void CFinger::SetSpread( double value )
{

	if (mFinger[0] && mFinger[1] && mFinger[2])
		if (mFinger[0]->Selected || mFinger[1]->Selected || mFinger[2]->Selected)
		{
			FBProperty *lSpreadProp = mFinger[0]->PropertyList.Find( mSpreadName );
			if ( lSpreadProp )
			{
				if ( lSpreadProp->IsAnimatable() ) {
					FBPropertyAnimatable *lSpreadAnimatable = (FBPropertyAnimatable*) lSpreadProp;
				
					lSpreadAnimatable->SetData( &value );
					lSpreadAnimatable->Key();
					FBAnimationNode *lAnimationNode = lSpreadAnimatable->GetAnimationNode();
					if ( lAnimationNode ) {
						lAnimationNode->KeyAdd( &value );
					}
				}
			
				//lSpreadProp->SetInt( (int) value );
				lSpreadProp->SetData( &value );
			}
		}
}

void CFinger::AddSpread( double value )
{
	double temp;

	if (mFinger[0] && mFinger[1] && mFinger[2])
		if (mFinger[0]->Selected || mFinger[1]->Selected || mFinger[2]->Selected)
		{
			FBProperty *lSpreadProp = mFinger[0]->PropertyList.Find( mSpreadName );
			if ( lSpreadProp )
			{
				lSpreadProp->GetData( &temp, sizeof(double) );
				temp += value;

				if ( lSpreadProp->IsAnimatable() ) {
					FBPropertyAnimatable *lSpreadAnimatable = (FBPropertyAnimatable*) lSpreadProp;
				
					lSpreadAnimatable->SetData( &temp );
					lSpreadAnimatable->Key();
					FBAnimationNode *lAnimationNode = lSpreadAnimatable->GetAnimationNode();
					if ( lAnimationNode ) {
						lAnimationNode->KeyAdd( &temp );
					}
				}
			
				//lSpreadProp->SetInt( (int) temp );
				lSpreadProp->SetData( &temp );
			}
		}
}

double CFinger::GetSpread()
{
	double value = 0.0;

	if (mFinger[0])
	{
		FBProperty *lSpreadProp = mFinger[0]->PropertyList.Find( mSpreadName );
			if ( lSpreadProp )
			{
				lSpreadProp->GetData( &value, sizeof(double) );
			}
	}

	return value;
}

/************************************************
 * finger WEIGHT property setUp.
 ************************************************/
void CFinger::SetWeight( double value )
{
	if (mFinger[0] && mFinger[1] && mFinger[2])
		if (mFinger[0]->Selected || mFinger[1]->Selected || mFinger[2]->Selected)
		{
			FBProperty *lWeightProp = mFinger[0]->PropertyList.Find( mWeightName );
			if ( lWeightProp )
			{
				if ( lWeightProp->IsAnimatable() ) {
					FBPropertyAnimatable *lWeightAnimatable = (FBPropertyAnimatable*) lWeightProp;
				
					lWeightAnimatable->SetData( &value );
					lWeightAnimatable->Key();
					FBAnimationNode *lAnimationNode = lWeightAnimatable->GetAnimationNode();
					if ( lAnimationNode ) {
						lAnimationNode->KeyAdd( &value );
					}
				}

				//lWeightProp->SetInt( (int) value );
				lWeightProp->SetData( &value );
			}
		}
}

void CFinger::AddWeight( double value )
{
	double temp;

	if (mFinger[0] && mFinger[1] && mFinger[2])
		if (mFinger[0]->Selected || mFinger[1]->Selected || mFinger[2]->Selected)
		{
			FBProperty *lWeightProp = mFinger[0]->PropertyList.Find( mWeightName );
			if ( lWeightProp )
			{
				lWeightProp->GetData( &temp, sizeof(double) );
				temp += value;

				if ( lWeightProp->IsAnimatable() ) {
					FBPropertyAnimatable *lWeightAnimatable = (FBPropertyAnimatable*) lWeightProp;
				
					lWeightAnimatable->SetData( &temp );
					lWeightAnimatable->Key();
					FBAnimationNode *lAnimationNode = lWeightAnimatable->GetAnimationNode();
					if ( lAnimationNode ) {
						lAnimationNode->KeyAdd( &temp );
					}
				}

				//lWeightProp->SetInt( (int) temp );
				lWeightProp->SetData( &value );
			}
		}
}

double CFinger::GetWeight()
{
	double value = 0.0;

	if (mFinger[0])
	{
		FBProperty *lWeightProp = mFinger[0]->PropertyList.Find( mWeightName );
			if ( lWeightProp )
			{
				lWeightProp->GetData( &value, sizeof(double) );
			}
	}

	return value;
}


/************************************************
 * finger DAMPING property setUp.
 ************************************************/
void CFinger::SetDamp( double value )
{
	if (mFinger[0] && mFinger[1] && mFinger[2])
		if (mFinger[0]->Selected || mFinger[1]->Selected || mFinger[2]->Selected)
		{
			FBProperty *lDampProp = mFinger[0]->PropertyList.Find( mDampName );
			if ( lDampProp )
			{
				if ( lDampProp->IsAnimatable() ) {
					FBPropertyAnimatable *lDampAnimatable = (FBPropertyAnimatable*) lDampProp;
				
					lDampAnimatable->SetData( &value );
					lDampAnimatable->Key();
					FBAnimationNode *lAnimationNode = lDampAnimatable->GetAnimationNode();
					if ( lAnimationNode ) {
						lAnimationNode->KeyAdd( &value );
					}
				}

				//lDampProp->SetInt( (int) value );
				lDampProp->SetData( &value );
			}
		}
}

void CFinger::AddDamp( double value )
{
	double temp;

	if (mFinger[0] && mFinger[1] && mFinger[2])
		if (mFinger[0]->Selected || mFinger[1]->Selected || mFinger[2]->Selected)
		{
			FBProperty *lDampProp = mFinger[0]->PropertyList.Find( mDampName );
			if ( lDampProp )
			{
				lDampProp->GetData( &temp, sizeof(double) );
				temp += value;

				if ( lDampProp->IsAnimatable() ) {
					FBPropertyAnimatable *lDampAnimatable = (FBPropertyAnimatable*) lDampProp;
				
					lDampAnimatable->SetData( &temp );
					lDampAnimatable->Key();
					FBAnimationNode *lAnimationNode = lDampAnimatable->GetAnimationNode();
					if ( lAnimationNode ) {
						lAnimationNode->KeyAdd( &temp );
					}
				}

				//lDampProp->SetInt( (int) temp );
				lDampProp->SetData( &temp );
			}
		}
}

double CFinger::GetDamp()
{	
	double value = 0.0;

	if (mFinger[0])
	{
		FBProperty *lDampProp = mFinger[0]->PropertyList.Find( mDampName );
			if ( lDampProp )
			{
				lDampProp->GetData( &value, sizeof(double) );
			}
	}

	return value;
}

/************************************************
 * finger INHERIT property setUp.
 ************************************************/
void CFinger::SetInherit( FBVector3d value )
{
	if (mFinger[0] && mFinger[1] && mFinger[2])
		if (mFinger[0]->Selected || mFinger[1]->Selected || mFinger[2]->Selected)
		{
			FBProperty *lInheritProp = mFinger[0]->PropertyList.Find( mInheritName );
			if ( lInheritProp )
			{
				if ( lInheritProp->IsAnimatable() ) {
					FBPropertyAnimatable *lInheritAnimatable = (FBPropertyAnimatable*) lInheritProp;
				
					//-- must be setUP for XYZ (three nodes)
					FBAnimationNode *lAnimationNode = lInheritAnimatable->GetAnimationNode()->Nodes[0];
					if ( lAnimationNode ) {
						lAnimationNode->KeyAdd( &value[0] );
					}
					lAnimationNode = lInheritAnimatable->GetAnimationNode()->Nodes[1];
					if ( lAnimationNode ) {
						lAnimationNode->KeyAdd( &value[1] );
					}
					lAnimationNode = lInheritAnimatable->GetAnimationNode()->Nodes[2];
					if ( lAnimationNode ) {
						lAnimationNode->KeyAdd( &value[2] );
					}
				}

				lInheritProp->SetData( &value );
			}
		}
}

void CFinger::AddInherit( FBVector3d value )
{
	FBVector3d	temp( 100.0, 100.0, 100.0 );

	if (mFinger[0] && mFinger[1] && mFinger[2])
		if (mFinger[0]->Selected || mFinger[1]->Selected || mFinger[2]->Selected)
		{
			FBProperty *lInheritProp = mFinger[0]->PropertyList.Find( mInheritName );
			if ( lInheritProp )
			{
				lInheritProp->GetData( &temp, sizeof(FBVector3d) );
				temp[0] += value[0];
				temp[1] += value[1];
				temp[2] += value[2];

				if ( lInheritProp->IsAnimatable() ) {
					FBPropertyAnimatable *lInheritAnimatable = (FBPropertyAnimatable*) lInheritProp;
				
					//-- must be setUP for XYZ (three nodes)
					FBAnimationNode *lAnimationNode = lInheritAnimatable->GetAnimationNode()->Nodes[0];
					if ( lAnimationNode ) {
						lAnimationNode->KeyAdd( &temp[0] );
					}
					lAnimationNode = lInheritAnimatable->GetAnimationNode()->Nodes[1];
					if ( lAnimationNode ) {
						lAnimationNode->KeyAdd( &temp[1] );
					}
					lAnimationNode = lInheritAnimatable->GetAnimationNode()->Nodes[2];
					if ( lAnimationNode ) {
						lAnimationNode->KeyAdd( &temp[2] );
					}
				}

				lInheritProp->SetData( &temp );
			}
		}
}

FBVector3d CFinger::GetInherit()
{
	FBVector3d	value( 100.0, 100.0, 100.0 );

	if (mFinger[0])
	{
		FBProperty *lInheritProp = mFinger[0]->PropertyList.Find( mInheritName );
			if ( lInheritProp )
			{
				lInheritProp->GetData( &value, sizeof(FBVector3d) );
			}
	}

	return value;
}



void CFinger::SelectFinger()
{
	for (int i=0; i<FINGER_ELEMS_COUNT; i++)
		if (mFinger[i])
			mFinger[i]->Selected = true;
}

void CFinger::DeSelectFinger()
{
	for (int i=0; i<FINGER_ELEMS_COUNT; i++)
		if (mFinger[i])
			mFinger[i]->Selected = false;
}

bool CFinger::IsSelected()
{
	if (mFinger[0] && mFinger[1] && mFinger[2])
		return (mFinger[0]->Selected || mFinger[1]->Selected || mFinger[2]->Selected);

	return false;
}


/************************************************
 *	CFinger Relation constraint init
 ************************************************/
void CFinger::AddRelationBoxes( int n, int ThumbAxis, FBConstraintRelation *pConstraint, bool LocalMode )
{
	
	mLocalMode = LocalMode;

	//---------------------------- create boxes needed for this finger
	for (int i=0; i<FINGER_ELEMS_COUNT; i++)
	{
		if (mFinger[i] == nullptr || mSkeletFinger[i] == nullptr)
			continue;

		mFingerBoxSrc[i]	= pConstraint->SetAsSource( mFinger[i] );
		mSkeletFingerSrc[i] = pConstraint->SetAsSource( mSkeletFinger[i] );
		mFingerBox[i]		= pConstraint->ConstrainObject( mFinger[i] );

		
		if (mLocalMode)
		{
			if( mFingerBoxSrc[i]->Is( FBModelPlaceHolder::TypeInfo ))
			{
				FBModelPlaceHolder *lPlaceHolderModel = (FBModelPlaceHolder*) mFingerBoxSrc[i] ;
				lPlaceHolderModel->UseGlobalTransforms = false;
			}
			/*
			HFBProperty		lProperty = mFingerBoxSrc[i]->PropertyList.Find( "UseGlobalTransforms" );
			if (lProperty)
			{
				FBPropertyBool		*lPropertyBool	= (FBPropertyBool*) lProperty;
				*lPropertyBool = false;
			}
			*/
		}
		


		mNumberToVector[i] = pConstraint->CreateFunctionBox("Converters", "Number to Vector");
		mVectorToNumber[i] = pConstraint->CreateFunctionBox("Converters", "Vector to Number");

		mAddBox[i]		= pConstraint->CreateFunctionBox("Number", "Add (a + b)");
		mDampBox[i]		= pConstraint->CreateFunctionBox("Number", "Damp");
		mMultBox[i]		= pConstraint->CreateFunctionBox("Number", "Multiply (a x b)");

		switch( i )
		{
		case 0: mWeightBox[i] = pConstraint->CreateFunctionBox( "HandTool", "Weight1" );
			break;
		case 1: mWeightBox[i] = pConstraint->CreateFunctionBox( "HandTool", "Weight2" );
			break;
		case 2: mWeightBox[i] = pConstraint->CreateFunctionBox( "HandTool", "Weight3" );
			break;
		};

		//-- finger model
		mRotationSrc[i]	= mFingerBoxSrc[i]->AnimationNodeOutGet()->Nodes.Find( "Rotation" );
		//mRotationSrc[i]		= mSkeletFingerSrc[i]->AnimationNodeOutGet()->Nodes.Find( "Rotation" );
		mRotation[i]		= mFingerBox[i]->AnimationNodeInGet()->Nodes.Find( "Rotation" );

		//-- Thumb must around around another local system
		if ( (n==0) && (ThumbAxis==0) )
		{
			//-- number to vector nodes
			mNumberToVectorIn[i] = mNumberToVector[i]->AnimationNodeInGet()->Nodes.Find("Y");
			mNumberToVectorOut[i] = mNumberToVector[i]->AnimationNodeOutGet()->Nodes.Find("Result");
			//-- vector to number nodes
			mVectorToNumberIn[i] = mVectorToNumber[i]->AnimationNodeInGet()->Nodes.Find("V");
			mVectorToNumberOut[i] = mVectorToNumber[i]->AnimationNodeOutGet()->Nodes.Find("Y");
		}
		else
		{
			//-- number to vector nodes
			mNumberToVectorIn[i] = mNumberToVector[i]->AnimationNodeInGet()->Nodes.Find("Z");
			mNumberToVectorOut[i] = mNumberToVector[i]->AnimationNodeOutGet()->Nodes.Find("Result");
			//-- vector to number nodes
			mVectorToNumberIn[i] = mVectorToNumber[i]->AnimationNodeInGet()->Nodes.Find("V");
			mVectorToNumberOut[i] = mVectorToNumber[i]->AnimationNodeOutGet()->Nodes.Find("Z");
		}

		//-- add box nodes
		if (mAddBox[i]) 
		{
			mAddA[i]	= mAddBox[i]->AnimationNodeInGet()->Nodes.Find("a");
			mAddB[i]	= mAddBox[i]->AnimationNodeInGet()->Nodes.Find("b");
			mAddR[i]	= mAddBox[i]->AnimationNodeOutGet()->Nodes.Find("Result");
		}

		//-- add box nodes
		if (mMultBox[i]) 
		{
			mMultA[i]	= mMultBox[i]->AnimationNodeInGet()->Nodes.Find("a");
			mMultB[i]	= mMultBox[i]->AnimationNodeInGet()->Nodes.Find("b");
			mMultR[i]	= mMultBox[i]->AnimationNodeOutGet()->Nodes.Find("Result");
		}
	
		mWeightW[i]		= mWeightBox[i]->AnimationNodeInGet()->Nodes.Find("Weight");
		mWeightI[i]		= mWeightBox[i]->AnimationNodeInGet()->Nodes.Find("Inherit");
		mWeightR[i]		= mWeightBox[i]->AnimationNodeOutGet()->Nodes.Find("Result");

		//-- damping box nodes
		mDampA[i]	= mDampBox[i]->AnimationNodeInGet()->Nodes.Find("a");
		mDampF[i]	= mDampBox[i]->AnimationNodeInGet()->Nodes.Find("Damping Factor");
		mDampR[i]	= mDampBox[i]->AnimationNodeOutGet()->Nodes.Find("Result");
	}

	mAddBox2		= pConstraint->CreateFunctionBox("Number", "Add (a + b)");
	mDampBox2		= pConstraint->CreateFunctionBox("Number", "Damp");

	//-- damping box nodes
	mAdd2A	= mAddBox2->AnimationNodeInGet()->Nodes.Find("a");
	mAdd2B	= mAddBox2->AnimationNodeInGet()->Nodes.Find("b");
	mAdd2R	= mAddBox2->AnimationNodeOutGet()->Nodes.Find("Result");

	mDamp2A	= mDampBox2->AnimationNodeInGet()->Nodes.Find("a");
	mDamp2F	= mDampBox2->AnimationNodeInGet()->Nodes.Find("Damping Factor");
	mDamp2R	= mDampBox2->AnimationNodeOutGet()->Nodes.Find("Result");

	if ( (n==0) && (ThumbAxis==0) )
	{
		//-- number to vector nodes
		mNumberToVectorIn2 = mNumberToVector[0]->AnimationNodeInGet()->Nodes.Find("Z");
		mNumberToVectorIn3 = mNumberToVector[0]->AnimationNodeInGet()->Nodes.Find("X");
		//-- vector to number nodes
		mVectorToNumberOut2 = mVectorToNumber[0]->AnimationNodeOutGet()->Nodes.Find("Z");
		mVectorToNumberOut3 = mVectorToNumber[0]->AnimationNodeOutGet()->Nodes.Find("X");
	}
	else
	{
		//-- number to vector nodes
		mNumberToVectorIn2 = mNumberToVector[0]->AnimationNodeInGet()->Nodes.Find("Y");
		mNumberToVectorIn3 = mNumberToVector[0]->AnimationNodeInGet()->Nodes.Find("X");
		//-- vector to number nodes
		mVectorToNumberOut2 = mVectorToNumber[0]->AnimationNodeOutGet()->Nodes.Find("Y");
		mVectorToNumberOut3 = mVectorToNumber[0]->AnimationNodeOutGet()->Nodes.Find("X");
	}

	mZRotation		= mFingerBoxSrc[0]->AnimationNodeOutGet()->Nodes.Find( mRotName );
	mSpread			= mFingerBoxSrc[0]->AnimationNodeOutGet()->Nodes.Find( mSpreadName );
	mDamp			= mFingerBoxSrc[0]->AnimationNodeOutGet()->Nodes.Find( mDampName );
	mWeight			= mFingerBoxSrc[0]->AnimationNodeOutGet()->Nodes.Find( mWeightName );
	mInherit		= mFingerBoxSrc[0]->AnimationNodeOutGet()->Nodes.Find( mInheritName );
}

void CFinger::MakeConnections()
{
	for (int i=0; i<FINGER_ELEMS_COUNT; i++)
	{
		if (mFinger[i] == nullptr)
			continue;

		if (mLocalMode)
		{
			mRotationSrc[i] = NULL;
			if (mFingerBoxSrc[i])
				mRotationSrc[i] = mFingerBoxSrc[i]->AnimationNodeOutGet()->Nodes.Find( "Lcl Rotation" );
			if (!mRotationSrc[i]) continue;	// skip if failed to find a rotation source
		}
		

		//-- damping box ( current inherit angle damp according damping factor )
		FBConnect( mZRotation, mDampA[i] );
		FBConnect( mDamp, mDampF[i] );

		//-- mult box ( angle * weight )
		FBConnect( mWeight, mWeightW[i] );
		FBConnect( mInherit, mWeightI[i] );
		FBConnect( mWeightR[i], mMultA[i] );
		FBConnect( mDampR[i], mMultB[i] );

		//-- add box ( plot rotation + current inherit angle )
		FBConnect( mMultR[i], mAddA[i] );
		FBConnect( mRotationSrc[i], mVectorToNumberIn[i] );
		FBConnect( mVectorToNumberOut[i], mAddB[i] );

		if (i==0)
		{
			FBConnect( mSpread, mDamp2A );
			FBConnect( mDamp, mDamp2F );

			FBConnect( mVectorToNumberOut2, mAdd2B );
			FBConnect( mDamp2R, mAdd2A );

			FBConnect( mAdd2R, mNumberToVectorIn2 );

			//-- transfer x rotation
			FBConnect( mVectorToNumberOut3, mNumberToVectorIn3 );
		}

		//-- result (as converting to vector)
		FBConnect( mAddR[i], mNumberToVectorIn[i] );
		
		if (mLocalMode)
		{
			if( mFingerBox[i]->Is( FBModelPlaceHolder::TypeInfo ))
			{
				FBModelPlaceHolder *lPlaceHolderModel = (FBModelPlaceHolder*) mFingerBox[i] ;
				lPlaceHolderModel->UseGlobalTransforms = false;
			}
			/*
			HFBProperty		lProperty = mFingerBox[i]->PropertyList.Find( "UseGlobalTransforms" );
			if (lProperty)
			{
				FBPropertyBool		*lPropertyBool	= (FBPropertyBool*) lProperty;
				*lPropertyBool = false;
			}
			*/
			mRotation[i] = NULL;
			mRotation[i] = mFingerBox[i]->AnimationNodeInGet()->Nodes.Find( "Lcl Rotation" );
		}

		if (mRotation[i])
		{	
			FBConnect( mNumberToVectorOut[i], mRotation[i] );
		}
	}
}



/************************************************
 *	CHand Constructor.
 ************************************************/
CHand::CHand()
{
	mModel = NULL;
}

CHand::~CHand()
{

}

/************************************************
 *	free usage objects and relation's.
 ************************************************/
void CHand::CleanUP()
{
	mModel = NULL;

	for (int i=0; i<HAND_FINGERS_COUNT; i++)
	{
		mFingers[i].CleanUP();
	}
}

/************************************************
 *	AddPropertyToModel
 ************************************************/
void CHand::AddPropertyToModel( FBModel *pCtrlRigModel, FBModel *pModel )
{
	mModel = pCtrlRigModel;

	if (mModel && mModel->Children.GetCount() != 0)
	{
		//----------------------------------------------------
		//-- fingers property list (hold in first fk marker)
		for (int i=0; i<HAND_FINGERS_COUNT; i++)
		{
			if (mModel->Children.GetCount() > i && pModel->Children.GetCount() > i)
				mFingers[i].AddPropertyToModel( mModel->Children[i], pModel->Children[i] );
		}
	}
	else
	{
		mModel = NULL;
	}
}

/************************************************
 *	Remove property
 ************************************************/
void CHand::RemovePropertyFromModel()
{
	if (mModel)
	{
		//----------------------------------------------------
		//-- fingers property list (hold in first fk marker)
		for (int i=0; i<HAND_FINGERS_COUNT; i++)
		{
			mFingers[i].RemovePropertyFromModel();
		}
	} 

	mModel = NULL;
}


/************************************************
 *	AddUI
 ************************************************/
void CHand::AddUI( FBLayout *pTool )
{ 
	int lB = 6;
	int lW = 150;
	int lH = 14;
	int lS = 4;

	//-- add regions
	pTool->AddRegion( "ThumbAngle",  "ThumbAngle",
                                                lB,	kFBAttachLeft,	"",	1.0	,
												lS,	kFBAttachTop,	"",	1.0,
												lW,	kFBAttachNone,	"",	1.0,
												lH,	kFBAttachNone,	"",	1.0 );
	pTool->AddRegion( "ThumbSpread", "ThumbSpread",
												lB,     kFBAttachLeft,      "",					1.0,
                                                lS,     kFBAttachBottom,    "ThumbAngle",		1.0,
                                                lW,		kFBAttachNone,		"",					1.0,
                                                lH,     kFBAttachNone,		NULL,				1.0 );
	pTool->AddRegion( "ThumbDamp", "ThumbDamp",
												lB,     kFBAttachLeft,      "",					1.0,
                                                lS,     kFBAttachBottom,    "ThumbSpread",		1.0,
                                                lW,		kFBAttachNone,		"",					1.0,
                                                lH,     kFBAttachNone,		NULL,				1.0 );
	pTool->AddRegion( "ThumbWeight",  "ThumbWeight",
                                                lB,     kFBAttachLeft,      "",					1.0,
                                                lS,     kFBAttachBottom,    "ThumbDamp",		1.0,
                                                lW,		kFBAttachNone,		"",					1.0,
                                                lH,     kFBAttachNone,		NULL,				1.0  );
	pTool->AddRegion( "ThumbInherit", "ThumbInherit",
												lB,     kFBAttachLeft,      "",					1.0,
                                                lS,     kFBAttachBottom,    "ThumbWeight",		1.0,
                                                lW,		kFBAttachNone,		"",					1.0,
                                                lH,     kFBAttachNone,		NULL,				1.0 );
	
	//-- index
	pTool->AddRegion( "IndexAngle",  "IndexAngle",
                                                lB,     kFBAttachLeft,      "",					1.0,
                                                lS+5,   kFBAttachBottom,    "ThumbInherit",		1.0,
                                                lW,		kFBAttachNone,		"",					1.0,
                                                lH,     kFBAttachNone,		NULL,				1.0 );
	pTool->AddRegion( "IndexSpread", "IndexSpread",
												lB,     kFBAttachLeft,      "",					1.0,
                                                lS,     kFBAttachBottom,    "IndexAngle",		1.0,
                                                lW,		kFBAttachNone,		"",					1.0,
                                                lH,     kFBAttachNone,		NULL,				1.0 );
	pTool->AddRegion( "IndexDamp", "IndexDamp",
												lB,     kFBAttachLeft,      "",					1.0,
                                                lS,     kFBAttachBottom,    "IndexSpread",		1.0,
                                                lW,		kFBAttachNone,		"",					1.0,
                                                lH,     kFBAttachNone,		NULL,				1.0 );
	pTool->AddRegion( "IndexWeight",  "IndexWeight",
                                                lB,     kFBAttachLeft,      "",					1.0,
                                                lS,     kFBAttachBottom,    "IndexDamp",		1.0,
                                                lW,		kFBAttachNone,		"",					1.0,
                                                lH,     kFBAttachNone,		NULL,				1.0  );
	pTool->AddRegion( "IndexInherit", "IndexInherit",
												lB,     kFBAttachLeft,      "",					1.0,
                                                lS,     kFBAttachBottom,    "IndexWeight",		1.0,
                                                lW,		kFBAttachNone,		"",					1.0,
                                                lH,     kFBAttachNone,		NULL,				1.0 );
	
	//-- middle
	pTool->AddRegion( "MiddleAngle",  "MiddleAngle",
                                                lB,     kFBAttachLeft,      "",					1.0,
                                                lS+5,   kFBAttachBottom,    "IndexInherit",		1.0,
                                                lW,		kFBAttachNone,		"",					1.0,
                                                lH,     kFBAttachNone,		NULL,				1.0 );
	pTool->AddRegion( "MiddleSpread", "MiddleSpread",
												lB,     kFBAttachLeft,      "",					1.0,
                                                lS,     kFBAttachBottom,    "MiddleAngle",		1.0,
                                                lW,		kFBAttachNone,		"",					1.0,
                                                lH,     kFBAttachNone,		NULL,				1.0 );
	pTool->AddRegion( "MiddleDamp", "MiddleDamp",
												lB,     kFBAttachLeft,      "",					1.0,
                                                lS,     kFBAttachBottom,    "MiddleSpread",		1.0,
                                                lW,		kFBAttachNone,		"",					1.0,
                                                lH,     kFBAttachNone,		NULL,				1.0 );
	pTool->AddRegion( "MiddleWeight",  "MiddleWeight",
                                                lB,     kFBAttachLeft,      "",					1.0,
                                                lS,     kFBAttachBottom,    "MiddleDamp",		1.0,
                                                lW,		kFBAttachNone,		"",					1.0,
                                                lH,     kFBAttachNone,		NULL,				1.0  );
	pTool->AddRegion( "MiddleInherit", "MiddleInherit",
												lB,     kFBAttachLeft,      "",					1.0,
                                                lS,     kFBAttachBottom,    "MiddleWeight",		1.0,
                                                lW,		kFBAttachNone,		"",					1.0,
                                                lH,     kFBAttachNone,		NULL,				1.0 );

	//-- ring
	pTool->AddRegion( "RingAngle",  "RingAngle",
                                                lB,     kFBAttachLeft,      "",					1.0,
                                                lS+5,     kFBAttachBottom,    "MiddleInherit",	1.0,
                                                lW,		kFBAttachNone,		"",					1.0,
                                                lH,     kFBAttachNone,		NULL,				1.0 );
	pTool->AddRegion( "RingSpread", "RingSpread",
												lB,     kFBAttachLeft,      "",					1.0,
                                                lS,     kFBAttachBottom,    "RingAngle",		1.0,
                                                lW,		kFBAttachNone,		"",					1.0,
                                                lH,     kFBAttachNone,		NULL,				1.0 );
	pTool->AddRegion( "RingDamp", "RingDamp",
												lB,     kFBAttachLeft,      "",					1.0,
                                                lS,     kFBAttachBottom,    "RingSpread",		1.0,
                                                lW,		kFBAttachNone,		"",					1.0,
                                                lH,     kFBAttachNone,		NULL,				1.0 );
	pTool->AddRegion( "RingWeight",  "RingWeight",
                                                lB,     kFBAttachLeft,      "",					1.0,
                                                lS,     kFBAttachBottom,    "RingDamp",		1.0,
                                                lW,		kFBAttachNone,		"",					1.0,
                                                lH,     kFBAttachNone,		NULL,				1.0  );
	pTool->AddRegion( "RingInherit", "RingInherit",
												lB,     kFBAttachLeft,      "",					1.0,
                                                lS,     kFBAttachBottom,    "RingWeight",		1.0,
                                                lW,		kFBAttachNone,		"",					1.0,
                                                lH,     kFBAttachNone,		NULL,				1.0 );

	//-- pinky
	pTool->AddRegion( "PinkyAngle",  "PinkyAngle",
                                                lB,     kFBAttachLeft,      "",					1.0,
                                                lS+5,     kFBAttachBottom,    "RingInherit",		1.0,
                                                lW,		kFBAttachNone,		"",					1.0,
                                                lH,     kFBAttachNone,		NULL,				1.0 );
	pTool->AddRegion( "PinkySpread", "PinkySpread",
												lB,     kFBAttachLeft,      "",					1.0,
                                                lS,     kFBAttachBottom,    "PinkyAngle",		1.0,
                                                lW,		kFBAttachNone,		"",					1.0,
                                                lH,     kFBAttachNone,		NULL,				1.0 );
	pTool->AddRegion( "PinkyDamp", "PinkyDamp",
												lB,     kFBAttachLeft,      "",					1.0,
                                                lS,     kFBAttachBottom,    "PinkySpread",		1.0,
                                                lW,		kFBAttachNone,		"",					1.0,
                                                lH,     kFBAttachNone,		NULL,				1.0 );
	pTool->AddRegion( "PinkyWeight",  "PinkyWeight",
                                                lB,     kFBAttachLeft,      "",					1.0,
                                                lS,     kFBAttachBottom,    "PinkyDamp",		1.0,
                                                lW,		kFBAttachNone,		"",					1.0,
                                                lH,     kFBAttachNone,		NULL,				1.0  );
	pTool->AddRegion( "PinkyInherit", "PinkyInherit",
												lB,     kFBAttachLeft,      "",					1.0,
                                                lS,     kFBAttachBottom,    "PinkyWeight",		1.0,
                                                lW,		kFBAttachNone,		"",					1.0,
                                                lH,     kFBAttachNone,		NULL,				1.0 );

	//---------------------------------------- assign regions
	//-- thumb finger
	pTool->SetControl( "ThumbAngle", mFingers[0].mRotProperty );
	pTool->SetControl( "ThumbSpread", mFingers[0].mSpreadProperty );
	pTool->SetControl( "ThumbDamp", mFingers[0].mDampProperty );
	pTool->SetControl( "ThumbWeight", mFingers[0].mWeightProperty );
	pTool->SetControl( "ThumbInherit", mFingers[0].mInheritProperty );
	//-- index finger
	pTool->SetControl( "IndexAngle", mFingers[1].mRotProperty );
	pTool->SetControl( "IndexSpread", mFingers[1].mSpreadProperty );
	pTool->SetControl( "IndexDamp", mFingers[1].mDampProperty );
	pTool->SetControl( "IndexWeight", mFingers[1].mWeightProperty );
	pTool->SetControl( "IndexInherit", mFingers[1].mInheritProperty );
	//-- middle finger
	pTool->SetControl( "MiddleAngle", mFingers[2].mRotProperty );
	pTool->SetControl( "MiddleSpread", mFingers[2].mSpreadProperty );
	pTool->SetControl( "MiddleDamp", mFingers[2].mDampProperty );
	pTool->SetControl( "MiddleWeight", mFingers[2].mWeightProperty );
	pTool->SetControl( "MiddleInherit", mFingers[2].mInheritProperty );
	//-- ring finger
	pTool->SetControl( "RingAngle", mFingers[3].mRotProperty );
	pTool->SetControl( "RingSpread", mFingers[3].mSpreadProperty );
	pTool->SetControl( "RingDamp", mFingers[3].mDampProperty );
	pTool->SetControl( "RingWeight", mFingers[3].mWeightProperty );
	pTool->SetControl( "RingInherit", mFingers[3].mInheritProperty );
	//-- pinky finger
	pTool->SetControl( "PinkyAngle", mFingers[4].mRotProperty );
	pTool->SetControl( "PinkySpread", mFingers[4].mSpreadProperty );
	pTool->SetControl( "PinkyDamp", mFingers[4].mDampProperty );
	pTool->SetControl( "PinkyWeight", mFingers[4].mWeightProperty );
	pTool->SetControl( "PinkyInherit", mFingers[4].mInheritProperty );


	for (int i=0; i<HAND_FINGERS_COUNT; i++)
	{
		mFingers[i].mRotProperty.Property = 0;
		mFingers[i].mRotProperty.SetBackgroundColorIndex( kFBColorIndexLtGray );
		mFingers[i].mSpreadProperty.Property = 0;
		mFingers[i].mSpreadProperty.SetBackgroundColorIndex( kFBColorIndexLtGray );
		mFingers[i].mDampProperty.Property = 0;
		mFingers[i].mDampProperty.SetBackgroundColorIndex( kFBColorIndexDkGray );
		mFingers[i].mWeightProperty.Property = 0;
		mFingers[i].mWeightProperty.SetBackgroundColorIndex( kFBColorIndexDkGray );
		mFingers[i].mInheritProperty.Property = 0;
		mFingers[i].mInheritProperty.SetBackgroundColorIndex( kFBColorIndexDkGray );
	}
}


bool CHand::GetPrevKeyTime( const FBTime &startTime, const FBTime &currTime, FBTime &lTime )
{
	bool		Result	= false;
	bool		bTemp;
	FBTime		lTemp;

	lTime	= startTime;

	for( int i=0; i<HAND_FINGERS_COUNT; i++)
	{
		bTemp = mFingers[i].GetPrevKeyTime( startTime, currTime, lTemp );

		if (bTemp && (lTemp >= lTime) && (lTemp < currTime))
		{
			lTime = lTemp;
			Result = true;
		}
	}
	
	return Result;
}

bool CHand::GetNextKeyTime( const FBTime &endTime, const FBTime &currTime, FBTime &lTime )
{
	bool	Result	= false; 
	bool	bTemp;
	FBTime	lTemp;

	lTime	= endTime;

	for( int i=0; i<HAND_FINGERS_COUNT; i++)
	{
		bTemp = mFingers[i].GetNextKeyTime( endTime, currTime, lTemp );

		if (bTemp && (lTemp <= lTime) && (lTemp > currTime))
		{
			lTime = lTemp;
			Result = true;
		}
	}
	
	return Result;
}

void CHand::SetKey()
{
	for( int i=0; i<HAND_FINGERS_COUNT; i++)
		mFingers[i].SetKey();
}

void CHand::RemoveKey()
{
	for( int i=0; i<HAND_FINGERS_COUNT; i++)
		mFingers[i].RemoveKey();
}


void CHand::SetRotation( double value )
{
	for( int i=0; i<HAND_FINGERS_COUNT; i++)
		mFingers[i].SetRotation( value );
}

void CHand::AddRotation( double value )
{
	for( int i=0; i<HAND_FINGERS_COUNT; i++)
		mFingers[i].AddRotation( value );
}

void CHand::SetSpread( double value )
{
	for( int i=0; i<HAND_FINGERS_COUNT; i++)
		mFingers[i].SetSpread( value );
}

void CHand::AddSpread( double value )
{
	for( int i=0; i<HAND_FINGERS_COUNT; i++)
		mFingers[i].AddSpread( value );
}

void CHand::SetWeight( double value )
{
	for( int i=0; i<HAND_FINGERS_COUNT; i++)
		mFingers[i].SetWeight( value );
}

void CHand::AddWeight( double value )
{
	for( int i=0; i<HAND_FINGERS_COUNT; i++)
		mFingers[i].AddWeight( value );
}

void CHand::SetDamp( double value )
{
	for( int i=0; i<HAND_FINGERS_COUNT; i++)
		mFingers[i].SetDamp( value );
}

void CHand::AddDamp( double value )
{
	for( int i=0; i<HAND_FINGERS_COUNT; i++)
		mFingers[i].AddDamp( value );
}

void CHand::SetInherit( FBVector3d value )
{
	for( int i=0; i<HAND_FINGERS_COUNT; i++)
		mFingers[i].SetInherit( value );
}

void CHand::AddInherit( FBVector3d value )
{
	for( int i=0; i<HAND_FINGERS_COUNT; i++)
		mFingers[i].AddInherit( value );
}

void CHand::SelectHand()
{
	mModel->Selected = true;
	
	for( int i=0; i<HAND_FINGERS_COUNT; i++ )
		mFingers[i].SelectFinger();
}

bool CHand::IsSelected()
{
	if (mModel) 
	{
		for( int i=0; i<HAND_FINGERS_COUNT; i++ )
			if (mFingers[i].IsSelected() )
				return true;
	}

	return false;
}

void CHand::DeSelectHand()
{
	if (mModel)
	{
		mModel->Selected = false;
		
		for( int i=0; i<HAND_FINGERS_COUNT; i++ )
			mFingers[i].DeSelectFinger();
	}
}

double CHand::GetDamp()
{
	int selCount = 0;
	double value = 0.0;

	for (int i=0; i<HAND_FINGERS_COUNT; i++)
	{
		if (mFingers[i].IsSelected() )
		{
			value += mFingers[i].GetDamp();
			selCount++;
		}
	}

	value /= selCount;
	return value;
}

double CHand::GetRotation()
{
	int selCount = 0;
	double value = 0.0;

	for (int i=0; i<HAND_FINGERS_COUNT; i++)
	{
		if (mFingers[i].IsSelected() )
		{
			value += mFingers[i].GetRotation();
			selCount++;
		}
	}

	value /= selCount;
	return value;
}

double CHand::GetSpread()
{
	int selCount = 0;
	double value = 0.0;

	for (int i=0; i<HAND_FINGERS_COUNT; i++)
	{
		if (mFingers[i].IsSelected() )
		{
			value += mFingers[i].GetSpread();
			selCount++;
		}
	}

	value /= selCount;
	return value;
}

double CHand::GetWeight()
{
	int selCount = 0;
	double value = 0.0;

	for (int i=0; i<HAND_FINGERS_COUNT; i++)
	{
		if (mFingers[i].IsSelected() )
		{
			value += mFingers[i].GetWeight();
			selCount++;
		}
	}

	value /= selCount;
	return value;
}

FBVector3d CHand::GetInherit()
{
	FBVector3d value(100.0, 100.0, 100.0);

	for (int i=0; i<HAND_FINGERS_COUNT; i++)
	{
		if (mFingers[i].IsSelected() )
		{
			value = mFingers[i].GetInherit();
		}
	}

	return value;
}

/************************************************
 *	CHand relation constraint init
 ************************************************/
void CHand::AddRelationBoxes( int ThumbAxis, FBConstraintRelation *pConstraint )
{
	//-- for each finger - create relation boxes
	for ( int i=0; i<HAND_FINGERS_COUNT; i++ )
		mFingers[i].AddRelationBoxes( i, ThumbAxis, pConstraint, true );
}

void CHand::MakeConnections()
{
	//-- connect finger( current angle, damping factor, inherit value for this finger )
	for ( int i =0; i<HAND_FINGERS_COUNT; i++ ) 
		mFingers[i].MakeConnections();
}


/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool HandCtrlTool::FBCreate()
{
	// Tool options
	StartSize[0] = 240;
	StartSize[1] = 450;	

	// UI Management
	UICreate	();
	UIConfigure	();

	// Add tool callbacks
	OnShow.Add	( this, (FBCallback) &HandCtrlTool::EventToolShow		);
	OnIdle.Add	( this, (FBCallback) &HandCtrlTool::EventToolIdle		);
	OnResize.Add( this, (FBCallback) &HandCtrlTool::EventToolResize	);
	OnPaint.Add	( this, (FBCallback) &HandCtrlTool::EventToolPaint		);
	OnInput.Add	( this, (FBCallback) &HandCtrlTool::EventToolInput		);

	//-- register new & load callbacks( for setting hand FK control of character )
	mApp.OnFileNew.Add( this, (FBCallback) &HandCtrlTool::EventNew );
	mApp.OnFileNewCompleted.Add( this, (FBCallback) &HandCtrlTool::EventNewCompleted );
	mApp.OnFileOpenCompleted.Add( this, (FBCallback) &HandCtrlTool::EventOpenCompleted );

	mLastCharacter = NULL;
	mLastWeight = 0;
	mLastInherit = FBVector3d(100.0, 100.0, 100.0);

	mPrevRotation	= 0.0;
	mPrevSpread		= 0.0;
	mPrevWeight		= 0.0;
	mPrevDamping	= 0.0;
	mPrevInherit	= FBVector3d( 100.0, 100.0, 100.0 );

	if (mSystem.Scene)
	{
		mSystem.Scene->OnChange.Add( this, (FBCallback) &HandCtrlTool::EventSceneChange );
	}

	mConstraintLeft = NULL;
	mConstraintRight = NULL;

	mConnectState = false;
	mUseState = false;
	mImageState = false;

	//mSystem.OnUIIdle.Add( this, (FBCallback)&HandCtrlTool::EventIdleAnim );

	return true;
}

void HandCtrlTool::CleanUP()
{
	mHandLeft.CleanUP();
	mHandRight.CleanUP();
	mHandsImage.CleanUP();

	//-- we reset connection
	mConnectState = false;
	mUseState = false;

	mButtonConnect.State = false;
	mButtonUse.State = false;
}



/************************************************
 *	FiLMBOX Destruction function.
 ************************************************/
void HandCtrlTool::FBDestroy()
{
	// If the idle anim callback is still in the system, remove it.
	//mSystem.OnUIIdle.Remove( this, (FBCallback) &HandCtrlTool::EventIdleAnim );

	if (mSystem.Scene)
	{
		mSystem.Scene->OnChange.Remove( this, (FBCallback) &HandCtrlTool::EventSceneChange );
	}

	//-- UNregister new & load callbacks( for setting hand FK control of character )
	mApp.OnFileNew.Remove( this, (FBCallback) &HandCtrlTool::EventNew );
	mApp.OnFileNewCompleted.Remove( this, (FBCallback) &HandCtrlTool::EventNewCompleted );
	mApp.OnFileOpenCompleted.Remove( this, (FBCallback) &HandCtrlTool::EventOpenCompleted );

	// Remove tool callbacks
	OnShow.Remove	( this, (FBCallback) &HandCtrlTool::EventToolShow	);
	OnIdle.Remove	( this, (FBCallback) &HandCtrlTool::EventToolIdle	);
	OnPaint.Remove	( this, (FBCallback) &HandCtrlTool::EventToolPaint );
	OnInput.Remove	( this, (FBCallback) &HandCtrlTool::EventToolInput );
	OnResize.Remove	( this, (FBCallback) &HandCtrlTool::EventToolResize);

	// Free user allocated memory
	CleanUP();
}


/************************************************
 *	Create the UI.
 ************************************************/
void HandCtrlTool::UICreate()
{
	int lB = 10;
	int lW = 90;
	int lH = 18;
	int lS = 4;

	// Add regions
	AddRegion( "ButtonConnect", "ButtonConnect",
										lB,	kFBAttachLeft,	"",	1.0	,
										lS,	kFBAttachTop,	"",	1.0,
										lW-10,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );
	AddRegion( "ButtonUse", "ButtonUse",
										lB,	kFBAttachRight,		"ButtonConnect",	1.0	,
										lS,	kFBAttachTop,		"",					1.0,
										20,	kFBAttachNone,		"",					1.0,
										lH,	kFBAttachNone,		"",					1.0 );
	AddRegion( "ButtonApply", "ButtonApply",
										-lW,	kFBAttachRight,		"",					1.0	,
										lS,	kFBAttachTop,		"",					1.0,
										-lB,	kFBAttachRight,		"",					1.0,
										lH,	kFBAttachNone,		"",					1.0 );
	AddRegion( "LabelAbout", "LabelAbout",
										lB,		kFBAttachLeft,		"",					1.0	,
										-14,	kFBAttachBottom,	"",					1.0,
										-lB,	kFBAttachRight,		"",					1.0,
										-lS,	kFBAttachBottom,	"",					1.0 );
	AddRegion( "TabPanel", "TabPanel",
										lB,	kFBAttachLeft,		"",					1.0	,
										lH,	kFBAttachTop,		"ButtonApply",		1.0,
										-lB,kFBAttachRight,		"",					1.0,
										lH,	kFBAttachNone,		"",					1.0 );
	
	AddRegion( "Layout", "Layout",
										0,		kFBAttachLeft,		"TabPanel",		1.0,
										0,		kFBAttachBottom,	"TabPanel",		1.0,
										0,		kFBAttachRight,		"TabPanel",		1.0,
										-4,		kFBAttachTop,		"LabelAbout",	1.0 );
	

	//-- assign regions
	SetControl( "ButtonConnect", mButtonConnect );
	SetControl( "ButtonUse", mButtonUse );
	SetControl( "ButtonApply", mButtonApply );
	SetControl( "LabelAbout", mLabelAbout );
	SetControl( "TabPanel", mTabPanel );
	SetControl( "Layout", mLayout[0] );

	//-- create sub-layouts
	UICreateLayout0();
	UICreateLayout1();
	UICreateLayout2();
	UICreateLayout3();

/*
	//mHand.AddUI( this );

	
	*/
}

/************************************************
 *	Create the UI layout 0.
 ************************************************/
void HandCtrlTool::UICreateLayout0()
{
	int lB = 5;
	int lH = 16;
	int lS = 14;

	//-- add regions
	mLayout[0].AddRegion( "HandsImage", "HandsImage", 
										lS,	kFBAttachLeft,	"",						1.0	,
										lB,	kFBAttachTop,	"Layout",				1.0,
										HANDS_IMAGE_WIDTH,	kFBAttachNone,	"",		1.0,
										HANDS_IMAGE_HEIGHT,	kFBAttachNone,	"",		1.0 );
	
	mLayout[0].AddRegion( "ImageSwitcher", "ImageSwitcher",
										2,	kFBAttachRight,		"HandsImage",		1.0	,
										lB,	kFBAttachTop,		"Layout",			1.0,
										25,	kFBAttachNone,		"",					1.0,
										lH,	kFBAttachNone,		"",					1.0 );

	//-- key controls regions
	mLayout[0].AddRegion( "ButtonPrev", "ButtonPrev", 
										lH,	kFBAttachLeft,		"",					1.0	,
										lB,	kFBAttachBottom,	"HandsImage",		1.0,
										40,	kFBAttachNone,		"",					1.0,
										lS,	kFBAttachNone,		"",					1.0 );
	mLayout[0].AddRegion( "ButtonKey", "ButtonKey", 
										lB,	kFBAttachRight,		"ButtonPrev",		1.0	,
										lB,	kFBAttachBottom,	"HandsImage",		1.0,
										30,	kFBAttachNone,		"",					1.0,
										lS,	kFBAttachNone,		"",					1.0 );
	mLayout[0].AddRegion( "ButtonNext", "ButtonNext", 
										lB,	kFBAttachRight,		"ButtonKey",		1.0	,
										lB,	kFBAttachBottom,	"HandsImage",		1.0,
										40,	kFBAttachNone,		"",					1.0,
										lS,	kFBAttachNone,		"",					1.0 );
	mLayout[0].AddRegion( "ButtonRemove", "ButtonRemove", 
										lB,	kFBAttachRight,		"ButtonNext",		1.0	,
										lB,	kFBAttachBottom,	"HandsImage",		1.0,
										30,	kFBAttachNone,		"",					1.0,
										lS,	kFBAttachNone,		"",					1.0 );
	mLayout[0].AddRegion( "ButtonAbsolute", "ButtonAbsolute", 
										-40,	kFBAttachRight,		"Layout",		1.0	,
										lB,	kFBAttachBottom,	"HandsImage",		1.0,
										-lB,	kFBAttachRight,		"Layout",		1.0,
										lS,	kFBAttachNone,		"",					1.0 );

	mLayout[0].AddRegion( "LabelRotation", "LabelRotation", 
										lB,	kFBAttachLeft,		"",					1.0	,
										lB,	kFBAttachBottom,		"ButtonPrev",		1.0,
										60,kFBAttachNone,		"",					1.0,
										lH,	kFBAttachNone,		"",					1.0 );
	mLayout[0].AddRegion( "LabelSpread", "LabelSpread", 
										lB,	kFBAttachLeft,		"",					1.0	,
										lS,	kFBAttachTop,		"EditRotation",		1.0,
										60,kFBAttachNone,		"",					1.0,
										lH,	kFBAttachNone,		"",					1.0 );
	mLayout[0].AddRegion( "LabelDamping", "LabelDamping", 
										lB,	kFBAttachLeft,		"",					1.0	,
										lS,	kFBAttachTop,		"EditSpread",		1.0,
										60,kFBAttachNone,		"",					1.0,
										lH,	kFBAttachNone,		"",					1.0 );
	mLayout[0].AddRegion( "LabelWeight", "LabelWeight", 
										lB,	kFBAttachLeft,		"",					1.0	,
										lS,	kFBAttachTop,		"EditDamping",		1.0,
										60,kFBAttachNone,		"",					1.0,
										lH,	kFBAttachNone,		"",					1.0 );
	mLayout[0].AddRegion( "LabelInherit", "LabelInherit", 
										lB,	kFBAttachLeft,		"",					1.0	,
										lS,	kFBAttachTop,		"LabelWeight",		1.0,
										60,kFBAttachNone,		"",					1.0,
										lH,	kFBAttachNone,		"",					1.0 );

	mLayout[0].AddRegion( "EditRotation", "EditRotation", 
										60,	kFBAttachLeft,		"LabelRotation",	1.0	,
										lB,	kFBAttachBottom,	"ButtonPrev",		1.0,
										-lB,kFBAttachRight,		"",					1.0,
										lH,	kFBAttachNone,		"",					1.0 );
	mLayout[0].AddRegion( "EditSpread", "EditSpread", 
										60,	kFBAttachLeft,		"LabelSpread",					1.0	,
										lS,	kFBAttachTop,		"EditRotation",		1.0,
										-lB,kFBAttachRight,		"",					1.0,
										lH,	kFBAttachNone,		"",					1.0 );
	mLayout[0].AddRegion( "EditDamping", "EditDamping", 
										60,	kFBAttachLeft,		"LabelDamping",					1.0	,
										lS,	kFBAttachTop,		"EditSpread",		1.0,
										-lB,kFBAttachRight,		"",					1.0,
										lH,	kFBAttachNone,		"",					1.0 );
	mLayout[0].AddRegion( "EditWeight", "EditWeight", 
										60,	kFBAttachLeft,		"LabelWeight",					1.0	,
										lS,	kFBAttachTop,		"EditDamping",		1.0,
										-lB,kFBAttachRight,		"",					1.0,
										lH,	kFBAttachNone,		"",					1.0 );
	mLayout[0].AddRegion( "EditInherit", "EditInherit", 
										60,	kFBAttachLeft,		"LabelInherit",					1.0	,
										lS,	kFBAttachTop,		"EditWeight",		1.0,
										-lB,kFBAttachRight,		"",					1.0,
										lH,	kFBAttachNone,		"",					1.0 );

	//-- assign regions
	mLayout[0].SetView( "HandsImage", mHandsImage );
	//mLayout[0].SetControl( "ImageSwitcher", mImageSwitcher );
	mLayout[0].SetControl( "ButtonPrev",	mButtonPrev );
	mLayout[0].SetControl( "ButtonKey",		mButtonKey );
	mLayout[0].SetControl( "ButtonNext",	mButtonNext );
	mLayout[0].SetControl( "ButtonRemove",	mButtonKeyRemove );
	mLayout[0].SetControl( "ButtonAbsolute",mButtonAbsolute );
	mLayout[0].SetControl( "LabelRotation",	mLabelRotation );
	mLayout[0].SetControl( "LabelSpread",	mLabelSpread );
	mLayout[0].SetControl( "LabelDamping",	mLabelDamping );
	mLayout[0].SetControl( "LabelWeight",	mLabelWeight );
	mLayout[0].SetControl( "LabelInherit",	mLabelInherit );
	mLayout[0].SetControl( "EditRotation",	mEditRotation );
	mLayout[0].SetControl( "EditSpread",	mEditSpread );
	mLayout[0].SetControl( "EditDamping",	mEditDamping );
	mLayout[0].SetControl( "EditWeight",	mEditWeight );
	mLayout[0].SetControl( "EditInherit",	mEditInherit );
}

/************************************************
 *	Create the UI layout 1.
 ************************************************/
void HandCtrlTool::UICreateLayout1()
{
	mHandLeft.AddUI( &mLayout[1] );
}

/************************************************
 *	Create the UI layout 2.
 ************************************************/
void HandCtrlTool::UICreateLayout2()
{
	mHandRight.AddUI( &mLayout[2] );
}

void HandCtrlTool::UICreateLayout3()
{
	int lB = 10;
	int lW = 100;
	int lH = 16;
	int lS = 14;

	//-- adding regions
	mLayout[3].AddRegion( "LabelThumbAxis", "LabelThumbAxis", 
										lS,	kFBAttachLeft,	"",	1.0	,
										lB,	kFBAttachTop,	"Layout",	1.0,
										lW,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );
	
	mLayout[3].AddRegion( "ListThumbAxis", "ListThumbAxis", 
										lH,	kFBAttachRight,		"LabelThumbAxis",	1.0	,
										lB,	kFBAttachTop,		"Layout",			1.0,
										40,	kFBAttachNone,		"",					1.0,
										lS,	kFBAttachNone,		"",					1.0 );

	//-- assign controls
	mLayout[3].SetControl( "LabelThumbAxis", mLabelThumbAxis );
	mLayout[3].SetControl( "ListThumbAxis", mListThumbAxis );
}

void HandCtrlTool::UIConfigure()
{
	// Configure button
	mButtonConnect.Style = kFB2States;
	mButtonConnect.OnClick.Add( this, (FBCallback) &HandCtrlTool::EventButtonConnectClick );
	mButtonConnect.Caption = "Connect";

	mButtonUse.Style = kFB2States;
	mButtonUse.OnClick.Add( this, (FBCallback) &HandCtrlTool::EventButtonUseClick );
	mButtonUse.Caption = "";

	FBColor lColorRed( 1.0, 0.0, 0.0 );
    mButtonUse.SetStateColor( kFBButtonState0, lColorRed );

    FBColor lColorGreen( 0.0, 1.0, 0.0 );
    mButtonUse.SetStateColor( kFBButtonState1, lColorGreen );


	mButtonApply.OnClick.Add( this, (FBCallback) &HandCtrlTool::EventButtonApplyClick );
	mButtonApply.Caption = "Apply";

	mLabelAbout.Caption = "Copyright (C) 2011 Sergey Solohin (Neill3d)";

	// Configure main layout
	mTabPanel.Items.SetString	("Basic~Left Hand~Right Hand~Setup");
	mTabPanel.OnChange.Add		( this, (FBCallback)&HandCtrlTool::EventTabPanelChange );

	SetBorder ("Layout", kFBStandardBorder, false,true, 1, 0,90,0);

	// Configure sub layouts
	UIConfigureLayout0();
	UIConfigureLayout1();
	UIConfigureLayout2();
	UIConfigureLayout3();
}

void HandCtrlTool::UIConfigureLayout0()
{
	//mImageSwitcher.Caption	= "<->";
	//mImageSwitcher.PropertyList.Find( "Hint" )->SetString( "Switch between character and hands images" );
	//mImageSwitcher.Visible = false;

	//-- setUp properties
	mButtonPrev.Caption		= "<";
	mButtonPrev.PropertyList.Find( "Hint" )->SetString( "Go to previous keys" );
	mButtonNext.Caption		= ">";
	mButtonNext.PropertyList.Find( "Hint" )->SetString( "Go to next keys" );
	mButtonKey.Caption		= "Key";
	mButtonKeyRemove.Caption= "X";
	mButtonKeyRemove.PropertyList.Find( "Hint" )->SetString( "Remove keys" );
	mButtonAbsolute.Style	= kFB2States;
	mButtonAbsolute.Caption = "A";
	mButtonAbsolute.PropertyList.Find( "Hint" )->SetString( "Absolute/Relative" );
	mButtonAbsolute.State	= true;

	mLabelRotation.Caption	= "Rotation";
	mLabelSpread.Caption	= "Spread";
	mLabelDamping.Caption	= "Damping";
	mLabelWeight.Caption	= "Weight";
	mLabelInherit.Caption	= "Inherit";

	mEditRotation.Min		= -360.0;
	mEditRotation.Max		= 360.0;

	mEditSpread.Min			= -360.0;
	mEditSpread.Max			= 360.0;
	mEditSpread.LargeStep	= 0.5;
	mEditSpread.SmallStep	= 0.1;

	mEditWeight.Min			= -100.0;
	mEditWeight.Max			= 100.0;

	mEditDamping.Min		= 0.0;
	mEditDamping.Max		= 100.0;
	mEditDamping.LargeStep	= 0.5;
	mEditDamping.SmallStep	= 0.1;

	//-- setUP callbacks
	mImageSwitcher.OnClick.Add( this, (FBCallback) &HandCtrlTool::EventButtonSwitchClick );

	mButtonPrev.OnClick.Add( this, (FBCallback) &HandCtrlTool::EventButtonPrevClick );
	mButtonNext.OnClick.Add( this, (FBCallback) &HandCtrlTool::EventButtonNextClick );
	mButtonKey.OnClick.Add( this, (FBCallback) &HandCtrlTool::EventButtonKeyClick );
	mButtonKeyRemove.OnClick.Add( this, (FBCallback) &HandCtrlTool::EventButtonKeyRemoveClick );
	mButtonAbsolute.OnClick.Add( this, (FBCallback) &HandCtrlTool::EventButtonAbsoluteClick );

	mEditRotation.OnChange.Add( this, (FBCallback) &HandCtrlTool::EventRotationChange );
	mEditSpread.OnChange.Add( this, (FBCallback) &HandCtrlTool::EventSpreadChange );
	mEditWeight.OnChange.Add( this, (FBCallback) &HandCtrlTool::EventWeightChange );
	mEditDamping.OnChange.Add( this, (FBCallback) &HandCtrlTool::EventDampChange );
	mEditInherit.OnChange.Add( this, (FBCallback) &HandCtrlTool::EventInheritChange );
}

void HandCtrlTool::UIConfigureLayout1()
{

}

void HandCtrlTool::UIConfigureLayout2()
{

}

void HandCtrlTool::UIConfigureLayout3()
{
	mLabelThumbAxis.Caption = "Thumb rotation axis: ";

	mListThumbAxis.Items.SetString("Y~Z"		);	
	mListThumbAxis.Style = kFBDropDownList;
}


void HandCtrlTool::EventRotationChange( HISender pSender, HKEvent pEvent )
{
	if (mButtonAbsolute.State)
	{
		mHandLeft.SetRotation( mEditRotation.Value );
		mHandRight.SetRotation( mEditRotation.Value );
	}
	else
	{
		mHandLeft.AddRotation( mEditRotation.Value - mPrevRotation );
		mHandRight.AddRotation( mEditRotation.Value - mPrevRotation );

		mPrevRotation = mEditRotation.Value;
	}
}

void HandCtrlTool::EventSpreadChange( HISender pSender, HKEvent pEvent )
{
	if (mButtonAbsolute.State)
	{
		mHandLeft.SetSpread( mEditSpread.Value );
		mHandRight.SetSpread( mEditSpread.Value );
	}
	else
	{
		mHandLeft.AddSpread( mEditSpread.Value - mPrevSpread );
		mHandRight.AddSpread( mEditSpread.Value - mPrevSpread );

		mPrevSpread = mEditSpread.Value;
	}
}

void HandCtrlTool::EventWeightChange( HISender pSender, HKEvent pEvent )
{
	if (mButtonAbsolute.State)
	{
		mHandLeft.SetWeight( mEditWeight.Value );
		mHandRight.SetWeight( mEditWeight.Value );
	}
	else
	{
		mHandLeft.AddWeight( mEditWeight.Value - mPrevWeight );
		mHandRight.AddWeight( mEditWeight.Value - mPrevWeight );

		mPrevWeight = mEditWeight.Value;
	}
}

void HandCtrlTool::EventDampChange( HISender pSender, HKEvent pEvent )
{
	if (mButtonAbsolute.State)
	{
		mHandLeft.SetDamp( mEditDamping.Value );
		mHandRight.SetDamp( mEditDamping.Value );
	}
	else
	{
		mHandLeft.AddDamp( mEditDamping.Value - mPrevDamping );
		mHandRight.AddDamp( mEditDamping.Value - mPrevDamping );

		mPrevDamping = mEditDamping.Value;
	}
}

void HandCtrlTool::EventInheritChange( HISender pSender, HKEvent pEvent )
{
	if (mButtonAbsolute.State)
	{
		mHandLeft.SetInherit( mEditInherit.Value );
		mHandRight.SetInherit( mEditInherit.Value );
	}
	else
	{
		FBVector3d delta = mEditInherit.Value;
		delta[0] -= mPrevInherit[0];
		delta[1] -= mPrevInherit[1];
		delta[2] -= mPrevInherit[2];
			
		mHandLeft.AddInherit( delta );
		mHandRight.AddInherit( delta );

		mPrevInherit = mEditInherit.Value;
	}
}

double HandCtrlTool::GetRotation()
{
	int count=0;
	double value = 0.0;

	//-- setUp rotation only in absolute mode
	if (mButtonAbsolute.State)
	{
		if (mHandLeft.IsSelected() )
		{
			value += mHandLeft.GetRotation();
			count++;
		}
		if (mHandRight.IsSelected() )
		{
			value += mHandRight.GetRotation();
			count++;
		}

		if (count > 0)
			value /= count;
	}

	return value;
}

double HandCtrlTool::GetSpread()
{
	int count=0;
	double value = 0.0;

	//-- setup spread only in absolute mode
	if (mButtonAbsolute.State)
	{
		if (mHandLeft.IsSelected() )
		{
			value += mHandLeft.GetSpread();
			count++;
		}
		if (mHandRight.IsSelected() )
		{
			value += mHandRight.GetSpread();
			count++;
		}

		if (count > 0)
			value /= count;
	}

	return value;
}

double HandCtrlTool::GetDamping()
{
	int count=0;
	double value = 0.0;

	//-- setup damping value only on absolute mode
	if (mButtonAbsolute.State)
	{
		if (mHandLeft.IsSelected() )
		{
			value += mHandLeft.GetDamp();
			count++;
		}
		if (mHandRight.IsSelected() )
		{
			value += mHandRight.GetDamp();
			count++;
		}

		if (count > 0)
			value /= count;
	}

	return value;
}

double HandCtrlTool::GetWeight()
{
	int count=0;
	double value = 0.0;

	//-- setup weight value only on absolute mode
	if (mButtonAbsolute.State)
	{
		if (mHandLeft.IsSelected() )
		{
			value += mHandLeft.GetWeight();
			count++;
		}
		if (mHandRight.IsSelected() )
		{
			value += mHandRight.GetWeight();
			count++;
		}

		if (count > 0)
			value /= count;
	}

	return value;
}

FBVector3d HandCtrlTool::GetInherit()
{
	FBVector3d value(100.0, 100.0, 100.0);

	//-- setup inherit value only on absolute mode
	if (mButtonAbsolute.State)
	{
		if (mHandLeft.IsSelected() )
		{
			value = mHandLeft.GetInherit();
		}
		if (mHandRight.IsSelected() )
		{
			value = mHandRight.GetInherit();
		}
	}

	return value;
}

/************************************************
 *	Tab panel change callback.
 ************************************************/
void HandCtrlTool::EventTabPanelChange ( HISender pSender, HKEvent pEvent )
{
	switch( mTabPanel.ItemIndex )
	{
		case 0:	SetControl( "Layout", mLayout[0] );		
			//-- image weight output
			mHandsImage.UpdateWeight();
			//-- UI channels
			mEditRotation.Value = GetRotation();
			mEditSpread.Value	= GetSpread();
			mEditDamping.Value	= GetDamping();
			mEditWeight.Value	= GetWeight();
			mEditInherit.Value	= GetInherit();

			mPrevRotation	= mEditRotation.Value;
			mPrevSpread		= mEditSpread.Value;
			mPrevWeight		= mEditDamping.Value;
			mPrevDamping	= mEditWeight.Value;
			mPrevInherit	= mEditInherit.Value;
			break;
		case 1:	SetControl( "Layout", mLayout[1] );		break;
		case 2: SetControl( "Layout", mLayout[2] );		break;
		case 3: SetControl( "Layout", mLayout[3] );		break;
	}
}

/************************************************
 *	Button Switch image callback.
 ************************************************/
void HandCtrlTool::EventButtonSwitchClick( HISender pSender, HKEvent pEvent )
{
	mImageState = !mImageState;

	if (mImageState)
	{
		mLayout[0].SetView( "HandsImage", mCharacterImage );
	}
	else
	{
		mLayout[0].SetView( "HandsImage", mHandsImage );
	}
}

/************************************************
 *	Button Apply callback.
 ************************************************/
void HandCtrlTool::EventButtonApplyClick( HISender pSender, HKEvent pEvent )
{
	//-- if left hand exist then plot animation from this one
	if (mHandLeft.mModel)
	{
		//-- select elements to plot
		mHandRight.DeSelectHand();
		mHandLeft.SelectHand();

		//-- set up plot period ( FPS )
		FBTime			lTime;
		lTime.SetSecondDouble( 1.0 / 30.0 );	// 30 fps

		mSystem.CurrentTake->PlotTakeOnSelected( lTime );
	}

	//-- if right hand exist then plot animation from this one
	if (mHandRight.mModel)
	{
		//-- select elements to plot
		mHandLeft.DeSelectHand();
		mHandRight.SelectHand();

		//-- set up plot period ( FPS )
		FBTime			lTime;
		lTime.SetSecondDouble( 1.0 / 30.0 );	// 30 fps

		mSystem.CurrentTake->PlotTakeOnSelected( lTime );
	}

	//-- disconnect hand tool
	Disconnect();
}

/************************************************
 *	Use switcher callback.
 ************************************************/
void HandCtrlTool::EventButtonUseClick( HISender pSender, HKEvent pEvent )
{
	if (!mConnectState)
	{
		mUseState = false;
		mButtonUse.State = false;
	}
	else
	{
		if (mConstraintLeft != NULL)
		{
			mConstraintLeft->Active =  mButtonUse.State != 0;
			mUseState = mButtonUse.State != 0;
		}
		if (mConstraintRight != NULL)
		{
			mConstraintRight->Active = mButtonUse.State != 0;
			mUseState = mButtonUse.State != 0;
		}
	}
}

/************************************************
 *	Go to prev key.
 ************************************************/
void HandCtrlTool::EventButtonPrevClick( HISender pSender, HKEvent pEvent )
{
	FBPlayerControl		lPlayer;

	FBTime		startTime	= lPlayer.LoopStart;
	FBTime		currTime	= mSystem.LocalTime;
	FBTime		stopTime	= lPlayer.LoopStop;
	
	FBTime		lLeftHand	= currTime;
	FBTime		lRightHand  = currTime;

	bool		bLeftHand;
	bool		bRightHand;

	bLeftHand=mHandLeft.GetPrevKeyTime( startTime, currTime, lLeftHand );
	if (false == bLeftHand)
		lLeftHand = currTime;

	bRightHand=mHandRight.GetPrevKeyTime( startTime, currTime, lRightHand );
	if (false == bRightHand)
		lRightHand = currTime;

	if (bLeftHand)
	{
		if (bRightHand && lRightHand > lLeftHand)
			lLeftHand = lRightHand;
	}
	else
	{
		if (bRightHand)
			lLeftHand = lRightHand;
	}

	if ( lLeftHand != currTime )
	{
		lPlayer.Goto( lLeftHand );
	}
}

/************************************************
 *	Go to next key.
 ************************************************/
void HandCtrlTool::EventButtonNextClick( HISender pSender, HKEvent pEvent )
{
	FBPlayerControl		lPlayer;

	FBTime		startTime	= lPlayer.LoopStart;
	FBTime		currTime	= mSystem.LocalTime;
	FBTime		stopTime	= lPlayer.LoopStop;

	FBTime lLeftHand	= currTime;
	FBTime lRightHand	= currTime;

	bool	bLeftHand;
	bool	bRightHand;

	bLeftHand=mHandLeft.GetNextKeyTime( stopTime, currTime, lLeftHand );
	if (false == bLeftHand)
		lLeftHand = currTime;

	bRightHand=mHandRight.GetNextKeyTime( stopTime, currTime, lRightHand );
	if (false == bRightHand)
		lRightHand = currTime;

	if (bLeftHand)
	{
		if (bRightHand && lRightHand < lLeftHand)
			lLeftHand = lRightHand;
	}
	else
	{
		if (bRightHand)
			lLeftHand = lRightHand;
	}

	if ( lLeftHand != currTime )
	{
		lPlayer.Goto( lLeftHand );
	}
}

/************************************************
 *	Set key to all user properties.
 ************************************************/
void HandCtrlTool::EventButtonKeyClick( HISender pSender, HKEvent pEvent )
{
	mHandLeft.SetKey();
	mHandRight.SetKey();
}

/************************************************
 * Remove key in all user properties.
 ************************************************/
void HandCtrlTool::EventButtonKeyRemoveClick( HISender pSender, HKEvent pEvent )
{
	mHandLeft.RemoveKey();
	mHandRight.RemoveKey();
}

/************************************************
 *	absolute/relative mode.
 ************************************************/
void HandCtrlTool::EventButtonAbsoluteClick( HISender pSender, HKEvent pEvent )
{
	//-- UI channels
	mEditRotation.Value = GetRotation();
	mEditSpread.Value	= GetSpread();
	mEditDamping.Value	= GetDamping();
	mEditWeight.Value	= GetWeight();
	mEditInherit.Value	= GetInherit();

	mPrevRotation	= mEditRotation.Value;
	mPrevSpread		= mEditSpread.Value;
	mPrevWeight		= mEditDamping.Value;
	mPrevDamping	= mEditWeight.Value;
	mPrevInherit	= mEditInherit.Value;
}

/************************************************
 *	Disconnect.
 ************************************************/
void HandCtrlTool::Disconnect()
{
	if (mApp.CurrentCharacter && mApp.CurrentCharacter->IsCtrlSetReady() )
	{
		//-- disconnect LEFT hand
		FBModel *pCtrlRigModel = mApp.CurrentCharacter->GetCtrlRigModel( kFBLeftWristNodeId );
		FBModel *pModel	= mApp.CurrentCharacter->GetModel( kFBLeftWristNodeId );
/*
#ifdef OLD_CONSTRAINT_MANAGER
		FBConstraintManager lConstraintManager;
#else
		FBConstraintManager	&lConstraintManager = FBConstraintManager::TheOne();
#endif
		*/
		if ( pCtrlRigModel && pModel ) 
		{

			// Create Relation Constraint
			if (mConstraintLeft)
			{

				mConstraintLeft->Active = false;

				FBSystem().Scene->Constraints.Remove( mConstraintLeft );
				//lConstraintManager.ConstraintRemove( mConstraintLeft );
				mConstraintLeft = NULL;
			}

			//-- remove all addition property's
			mHandLeft.RemovePropertyFromModel();

			//-- we have done it!
			mConnectState	= false;
			mUseState		= false;
			mButtonConnect.State = false;
			mButtonUse.State = false;
		}

		//-- disconnect RIGHT hand
		pCtrlRigModel = mApp.CurrentCharacter->GetCtrlRigModel( kFBRightWristNodeId );
		pModel	= mApp.CurrentCharacter->GetModel( kFBRightWristNodeId );

		if ( pCtrlRigModel && pModel ) 
		{

			// Create Relation Constraint
			if (mConstraintRight)
			{
				
				mConstraintRight->Active = false;

				FBSystem().Scene->Constraints.Remove( mConstraintRight );
				//lConstraintManager.ConstraintRemove( mConstraintRight );
				mConstraintRight = NULL;
			}

			//-- remove all addition property's
			mHandRight.RemovePropertyFromModel();

			//-- we have done it!
			mConnectState	= false;
			mUseState		= false;
			mButtonConnect.State = false;
			mButtonUse.State = false;
		}

		mHandsImage.UpdateWeight();
		mHandsImage.Refresh( true );
	}
}

/************************************************
 *	Button Connect Click.
 ************************************************/
void HandCtrlTool::EventButtonConnectClick( HISender pSender, HKEvent pEvent )
{
	if (mButtonConnect.State)
	{
		if (mApp.CurrentCharacter  && mApp.CurrentCharacter->IsCtrlSetReady() )
		{
			//-- connect LEFT hand
			FBModel *pCtrlRigModel = mApp.CurrentCharacter->GetCtrlRigModel( kFBLeftWristNodeId );
			FBModel *pModel	= mApp.CurrentCharacter->GetModel( kFBLeftWristNodeId );
#ifdef OLD_CONSTRAINT_MANAGER
			FBConstraintManager lConstraintManager;
#else
			FBConstraintManager	&lConstraintManager = FBConstraintManager::TheOne();
#endif
			if ( pCtrlRigModel && pModel && pCtrlRigModel->Children.GetCount() != 0 ) 
			{
				mHandLeft.AddPropertyToModel( pCtrlRigModel, pModel );

				
				// Create Relation Constraint
				int i, c = lConstraintManager.TypeGetCount();
				for( i = 0; i < c; i++ )
				{
					if( strstr(lConstraintManager.TypeGetName(i), "Relation") )
					{
						mConstraintLeft = (FBConstraintRelation*)lConstraintManager.TypeCreateConstraint(i);
						break;
					}
				}

				mHandLeft.AddRelationBoxes( mListThumbAxis.ItemIndex, mConstraintLeft );
				mHandLeft.MakeConnections();

			
				// Activate constraint
				mConstraintLeft->Active = true;

				//-- we have done it!
				mConnectState	= true;
				mUseState		= true;
				mButtonUse.State = true;
			}

			//-- connect RIGHT hand
			pCtrlRigModel = mApp.CurrentCharacter->GetCtrlRigModel( kFBRightWristNodeId );
			pModel	= mApp.CurrentCharacter->GetModel( kFBRightWristNodeId );

			if ( pCtrlRigModel && pModel && pCtrlRigModel->Children.GetCount() != 0 ) 
			{
				mHandRight.AddPropertyToModel( pCtrlRigModel, pModel );

				
				// Create Relation Constraint
				int i, c = lConstraintManager.TypeGetCount();
				for( i = 0; i < c; i++ )
				{
					if( strstr(lConstraintManager.TypeGetName(i), "Relation") )
					{
						mConstraintRight = (FBConstraintRelation*)lConstraintManager.TypeCreateConstraint(i);
						break;
					}
				}

				mHandRight.AddRelationBoxes( mListThumbAxis.ItemIndex, mConstraintRight );
				mHandRight.MakeConnections();

			
				// Activate constraint
				mConstraintRight->Active = true;

				//-- we have done it!
				mConnectState	= true;
				mUseState		= true;
				mButtonUse.State = true;
			}

			mHandsImage.UpdateWeight();
			mHandsImage.Refresh( true );
		}
	} else {
		Disconnect();
	}

	if (!mConnectState) 
	{
		mButtonUse.State = false;
		mButtonConnect.State = false;
	}
}

void HandCtrlTool::EventIdleAnim( HISender pSender, HKEvent pEvent )
{
	/*
	if ( !mHand.mModel ) return;

	FBTime		lTime;

	lTime = mSystem.LocalTime;

	FBProperty	*lProperty	= mHand.mModel->PropertyList.Find( mHand.mRotName );
	if (lProperty)
	{
		FBPropertyAnimatable *lAnimatableProperty = (FBPropertyAnimatable*) lProperty;

		FBAnimationNode *lAnimationNode = lAnimatableProperty->GetAnimationNode();
		if (lAnimationNode) 
		{
			FBFCurve	*lCurve	= lAnimationNode->FCurve;
			
			float value = lCurve->Evaluate( lTime );

			for (int i=0; i<mHand.mModel->Children.GetCount(); i++)
			{
				FBVector3d	rot;
				rot[2] = value;
				rot[0] = rot[1] = 0.0f;
				mHand.mModel->Children[i]->Rotation = rot;

				
				FBProperty *lRotProperty = mHand.mModel->Children[i]->PropertyList.Find( "currAngle" );
				if (lRotProperty)
				{
					double dValue = (double) value;

					lRotProperty->SetData( &dValue );
				}
				
			}
		}
	}
	*/
}

void HandCtrlTool::EventNew( HISender pSender, HKEvent pEvent )
{
	mHandsImage.FillHandModels( NULL, true );
	mHandsImage.FillHandModels( NULL, false );

	mLastCharacter = NULL;

	CleanUP();
}

void HandCtrlTool::EventFileExit( HISender pSender, HKEvent pEvent )
{
	mHandsImage.FillHandModels( NULL, true );
	mHandsImage.FillHandModels( NULL, false );

	mLastCharacter = NULL;

	CleanUP();
}

void HandCtrlTool::EventNewCompleted( HISender pSender, HKEvent pEvent )
{
	/*
	mHandsImage.FillHandLeftModels( NULL );
	mHandsImage.FillHandRightModels( NULL );

	mLastCharacter = NULL;

	CleanUP();
	*/
}


/************************************************
 *	Scene selection change.
 ************************************************/
void HandCtrlTool::EventSceneChange( HISender pSender, HKEvent pEvent )
{
	FBEventSceneChange lEvent( pEvent );

	switch ( lEvent.Type )
	{
	case kFBSceneChangeSelect:
	case kFBSceneChangeUnselect:
	case kFBSceneChangeReSelect:

		//-- fill UI elements
		if (mTabPanel.ItemIndex == 0)
		{
			mHandsImage.CheckHandsSelection();

			mEditRotation.Value = GetRotation();
			mEditSpread.Value	= GetSpread();
			mEditDamping.Value	= GetDamping();
			mEditWeight.Value	= GetWeight();
			mEditInherit.Value	= GetInherit();

			mPrevRotation	= mEditRotation.Value;
			mPrevSpread		= mEditSpread.Value;
			mPrevWeight		= mEditDamping.Value;
			mPrevDamping	= mEditWeight.Value;
			mPrevInherit	= mEditInherit.Value;
		}

		break;
	}
}

void HandCtrlTool::EventOpenCompleted( HISender pSender, HKEvent pEvent )
{
	if (mApp.CurrentCharacter && mApp.CurrentCharacter->IsCtrlSetReady() )
	{
		mHandsImage.FillHandModels( mApp.CurrentCharacter->GetCtrlRigModel( kFBLeftWristNodeId ), true );
		mHandsImage.FillHandModels( mApp.CurrentCharacter->GetCtrlRigModel( kFBRightWristNodeId ), false );

		mLastCharacter = mApp.CurrentCharacter;
	} 
	else
	{
		CleanUP();
	}
}


/************************************************
 *	UI Idle callback.
 ************************************************/
void HandCtrlTool::EventToolIdle( HISender pSender, HKEvent pEvent )
{
	int value = (int) mEditWeight.Value;

	if ((mTabPanel.ItemIndex == 0) && (value != mLastWeight))
	{
		mHandsImage.UpdateWeight();
		mHandsImage.Refresh(true);

		mLastWeight = mEditWeight.Value;
	}

	FBCharacter *pChar = mApp.CurrentCharacter;
	if ( pChar && pChar->IsCtrlSetReady() )
	if ( pChar != mLastCharacter)
	{
		mLastCharacter = pChar;

		if (!mLastCharacter)
		{
			CleanUP();
		}
		else
		{
			mHandsImage.FillHandModels( pChar->GetCtrlRigModel( kFBLeftWristNodeId ), true );
			mHandsImage.FillHandModels( pChar->GetCtrlRigModel( kFBRightWristNodeId ), false );
		}
	}
}


/************************************************
 *	Handle tool activation (selection/unselection).
 ************************************************/
void HandCtrlTool::EventToolShow( HISender pSender, HKEvent pEvent )
{
	FBEventShow lEvent( pEvent );

	if( lEvent.Shown )
	{
		// Reset the UI here.
	}
	else
	{
	}
}


/************************************************
 *	Paint callback for tool (on expose).
 ************************************************/
void HandCtrlTool::EventToolPaint( HISender pSender, HKEvent pEvent )
{
	mHandsImage.Refresh(true);	
}


/************************************************
 *	Tool resize callback.
 ************************************************/
void HandCtrlTool::EventToolResize( HISender pSender, HKEvent pEvent )
{
}


/************************************************
 *	Handle input into the tool.
 ************************************************/
void HandCtrlTool::EventToolInput( HISender pSender, HKEvent pEvent )
{
	
}


/************************************************
 *	FBX Storage.
 ************************************************/
bool HandCtrlTool::FbxStore	( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat )
{
	
	return true;
}

/************************************************
 *	FBX Retrieval.
 ************************************************/
bool HandCtrlTool::FbxRetrieve( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat )
{

	return true;
}


