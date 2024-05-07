
/////////////////////////////////////////////////////////////////////////////////////////
//
// boxPoseTransform.cxx
//
// Sergei <Neill3d> Solokhin 2014-2020
//
// GitHub page - https://github.com/Neill3d/OpenMoBu
// Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
/////////////////////////////////////////////////////////////////////////////////////////


//--- Class declaration
#include "boxPoseTransform.h"
#include "nv_math.h"
#include "math3d.h"

//--- Registration defines

#define BOXPOSETRANSFORM__CLASS			BOXPOSETRANSFORM__CLASSNAME
#define BOXPOSETRANSFORM__NAME			BOXPOSETRANSFORM__CLASSSTR
#define	BOXPOSETRANSFORM__LOCATION		"OpenMobu"
#define BOXPOSETRANSFORM__LABEL			"Pose Transform"
#define	BOXPOSETRANSFORM__DESC			"Grab Transform from a specified pose"

//--- implementation and registration

FBBoxImplementation	(	BOXPOSETRANSFORM__CLASS		);	// Box class name
FBRegisterBox		(	BOXPOSETRANSFORM__NAME,			// Unique name to register box.
						BOXPOSETRANSFORM__CLASS,		// Box class name
						BOXPOSETRANSFORM__LOCATION,		// Box location ('plugins')
						BOXPOSETRANSFORM__LABEL,		// Box label (name of box to display)
						BOXPOSETRANSFORM__DESC,			// Box long description.
						FB_DEFAULT_SDK_ICON			);	// Icon filename (default=Open Reality icon)


/************************************************
 *	Creation
 ************************************************/
bool CBoxPoseTransform::FBCreate()
{
	/*
	*	Create the nodes for the box.
	*/
	if( FBBox::FBCreate() )
	{
		char temp[16] = { 0 };

		// Create the input node.
		m_InTranslation = AnimationNodeInCreate( 0, "In Translation", ANIMATIONNODE_TYPE_LOCAL_TRANSLATION );
		m_InRotation = AnimationNodeInCreate(1, "In Rotation", ANIMATIONNODE_TYPE_LOCAL_ROTATION);
		
		for (int i = 0; i < NUMBER_OF_POSES; ++i)
		{
			sprintf_s(temp, sizeof(char) * 16, "Factor %d", i);
			m_Factor[i] = AnimationNodeInCreate(4+i, temp, ANIMATIONNODE_TYPE_NUMBER);
		}
		
		// Create the output nodes
		m_Translation	= AnimationNodeOutCreate( 2, "Translation", ANIMATIONNODE_TYPE_LOCAL_TRANSLATION );
		m_Rotation		= AnimationNodeOutCreate( 3, "Rotation", ANIMATIONNODE_TYPE_LOCAL_ROTATION );
		
		//
		FBPropertyPublish(this, m_Object, "Object", nullptr, nullptr);
		m_Object.SetSingleConnect(true);
		m_Object.SetFilter(FBModel::GetInternalClassId());

		for (int i = 0; i < NUMBER_OF_POSES; ++i)
		{
			sprintf_s(temp, sizeof(char) * 16, "Pose %d", i);
			FBPropertyPublish(this, m_Poses[i], temp, nullptr, nullptr);

			m_Poses[i].SetSingleConnect(true);
			m_Poses[i].SetFilter(FBObjectPose::GetInternalClassId());
		}

		return true;
	}
	return false;
}


/************************************************
 *	Destruction.
 ************************************************/
void CBoxPoseTransform::FBDestroy()
{
	/*
	*	Free any user memory associated to box.
	*/
//	FBBox::Destroy();
}

/************************************************
 *	Real-time engine evaluation
 ************************************************/

bool CBoxPoseTransform::AnimationNodeNotify( FBAnimationNode *pAnimationNode, FBEvaluateInfo *pEvaluateInfo )
{
	/*
	*	1. Read the data from the in connector
	*	2. Treat the data as required
	*	3. Write the data to the out connector
	*	4. Return the status (LIVE/DEAD) of the box.
	*/

	FBVector3d		translation, rotation;

	// Read the input nodes.
	bool rotation_status{ false };

	m_InTranslation->ReadData(translation, pEvaluateInfo);
	rotation_status = m_InRotation->ReadData(rotation, pEvaluateInfo);
	
	FBModel* model = (m_Object.GetCount() > 0) ? (FBModel*)m_Object.GetAt(0) : nullptr;

	if (!model)
	{
		// try to get model from input connection
		if (rotation_status)
		{
			int count = m_InRotation->GetSrcCount();
			for (int i = 0; i < count; ++i)
			{
				FBPlug* pPlug = m_InRotation->GetSrc(i);
				pPlug = pPlug->GetOwner();

				if (pPlug->Is(FBModelPlaceHolder::TypeInfo))
				{
					FBModelPlaceHolder* place_holder = (FBModelPlaceHolder*)pPlug;
					model = place_holder->Model;
				}
			}
		}
	}

	if (!model)
		return false;

    const char* modelName = model->Name;

	for (int i = 0; i < NUMBER_OF_POSES; ++i)
	{
		FBObjectPose* pose = (m_Poses[i].GetCount() > 0) ? (FBObjectPose*)m_Poses[i].GetAt(0) : nullptr;

        if (pose == nullptr)
            continue;

        FBStringList storedNames = pose->GetStoredObjectNames();

        int nameIndex = -1;

        for (int j = 0, storedNamesCount = storedNames.GetCount(); j < storedNamesCount; ++j)
        {
            const char* name{ storedNames.GetAt(j) };
            char* lastDelim = strrchr((char*)name, ':');

            if (lastDelim != nullptr)
            {
                name = lastDelim + 1;    
            }
            
            if (strcmpi(name, modelName) == 0)
            {
                nameIndex = j;
                break;
            }
        }

        if (nameIndex < 0)
            continue;

        const char* name{ storedNames.GetAt(nameIndex) };
		double factor{ 0.0f };
		m_Factor[i]->ReadData(&factor, pEvaluateInfo);

		factor *= 0.01;

		FBMatrix tm, scl;
		FBTVector t;
		FBRVector r;
		FBQuaternion q;

		bool is_valid = false;

		if (pose->GetTransform(t, tm, scl, name, FBPoseTransformType::kFBPoseTransformLocal))
		{
			FBMatrixToQuaternion(q, tm);
			is_valid = true;
		}
		else if (pose->IsPropertyStored(name, "Lcl Translation") && pose->IsPropertyStored(name, "Lcl Rotation"))
		{
			pose->GetPropertyValue(t, sizeof(double) * 3, name, "Lcl Translation");
			pose->GetPropertyValue(r, sizeof(double) * 3, name, "Lcl Rotation");

			FBRotationToQuaternion(q, r);
			is_valid = true;
		}

		if (is_valid)
		{
			if (factor >= 1.0)
			{
				translation = FBVector3d(t);
				FBQuaternionToRotation(rotation, q);
			}
			else
			{
				FBQuaternion p;
				FBRotationToQuaternion(p, rotation);

				FBQuaternion result;
				FBInterpolateRotation(result, p, q, factor);
				FBQuaternionToRotation(rotation, result);

				for (int j = 0; j < 3; ++j)
					translation[j] = factor * (t[j] - translation[j]) + translation[j];
			}
		}
	}

	// 
	m_Translation->WriteGlobalData(translation, pEvaluateInfo);
	m_Rotation->WriteGlobalData(rotation, pEvaluateInfo);
	
	return true;	
}


/************************************************
 *	FBX Storage.
 ************************************************/
bool CBoxPoseTransform::FbxStore( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Store box parameters.
	*/

	if (pStoreWhat & kAttributes)
	{
		FBModel* model = (m_Object.GetCount() > 0) ? (FBModel*)m_Object.GetAt(0) : nullptr;

		if (model)
		{
			pFbxObject->FieldWriteC("CBoxPoseTransformObject", model->LongName);
		}

		char temp[32] = { 0 };
		for (int i = 0; i < NUMBER_OF_POSES; ++i)
		{
			FBObjectPose* pose = (m_Poses[i].GetCount() > 0) ? (FBObjectPose*)m_Poses[i].GetAt(0) : nullptr;

			if (!pose)
				continue;

			sprintf_s(temp, sizeof(char) * 32, "CBoxPoseTransformPose%d", i);
			pFbxObject->FieldWriteC(temp, pose->LongName);
		}
		
	}

	return true;
}


/************************************************
 *	FBX Retrieval.
 ************************************************/
bool CBoxPoseTransform::FbxRetrieve( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Retrieve box parameters.
	*/

	if (pStoreWhat & kAttributes)
	{
		m_ModelName = pFbxObject->FieldReadC("CBoxPoseTransformObject");

		char temp[32] = { 0 };
		for (int i = 0; i < NUMBER_OF_POSES; ++i)
		{
			sprintf_s(temp, sizeof(char) * 32, "CBoxPoseTransformPose%d", i);
			m_PoseNames[i] = pFbxObject->FieldReadC(temp);
		}

		m_System.OnUIIdle.Add(this, (FBCallback)&CBoxPoseTransform::OnSystemIdle);
	}

	return true;
}


void CBoxPoseTransform::OnSystemIdle(HISender pSender, HKEvent pEvent)
{
	m_System.OnUIIdle.Remove(this, (FBCallback)&CBoxPoseTransform::OnSystemIdle);

	if (!m_ModelName.IsEmpty())
	{
		if (FBModel* model = FBFindModelByLabelName(m_ModelName))
		{
			m_Object.Add(model);
		}
	}

	for (int i = 0; i < NUMBER_OF_POSES; ++i)
	{
		if (m_PoseNames[i].IsEmpty())
			continue;

		FBScene* scene = FBSystem::TheOne().Scene;

		for (int j = 0, count = scene->ObjectPoses.GetCount(); j < count; ++j)
		{
			if (scene->ObjectPoses[j]->LongName == m_PoseNames[i])
			{
				m_Poses[i].Add(scene->ObjectPoses[j]);
			}
		}
	}
}
