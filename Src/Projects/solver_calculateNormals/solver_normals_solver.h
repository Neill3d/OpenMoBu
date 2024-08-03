
/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/MoBu/blob/master/LICENSE
//
// GitHub repository - https://github.com/Neill3d/MoBu
//
// Author Sergei Solokhin (Neill3d) 2014-2024
//  e-mail to: neill3d@gmail.com
// 
/////////////////////////////////////////////////////////////////////////////////////////


// object to blend normals and textures for a model blendshapes

#pragma once

//--- SDK include
#include <fbsdk/fbsdk.h>
#include <fbsdk/fbapplymanagerrule.h>

#include <GL\glew.h>
#include <map>

#include "nv_math.h"

#include "glslComputeShader.h"

#define NORMALSOLVERASSOCIATION__CLASSNAME	    KNormalSolverAssociation 
#define NORMALSOLVERASSOCIATION__CLASSSTR		"KNormalSolverAssociation"

#define NORMALSOLVER__CLASSNAME			SolverCalculateNormals
#define NORMALSOLVER__CLASSSTR			"Solver Calculate Normals"



//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
/**	KAMRManipulatorAssociation class.
*	Provides custom context menu rules for ORManip_Association and ORModelItem.
*/
class KNormalSolverAssociation : public FBApplyManagerRule
{
	FBApplyManagerRuleDeclare(KNormalSolverAssociation, FBApplyManagerRule);

public:


	/** Return true if pSrc is the type of custom object which this rule to be defined for. 
        *
        *    @warning you should only return true if pSrc is the type of custom object you defined.
        */
    virtual bool IsValidSrc( FBComponent* /*pSrc*/) override;

    /** Return true if pDst is the type of custom or MB object which your custom object 
    *    pSrc will be applied on. it's possible to return proper destination and return it by pDst. 
    *    For example, pSrc is RagDoll Property, pDst is a one of IK/FK bone, then you can find
    *    associated Character and replace.
    *
    *    \param    pSrc    the custom type of object you drag it to attach another object.
    *    \retval    pDst    one of the selected destinations, it could be modified and return proper destination.
    *    \retval    pAllowMultiple    return true if allow multiple objects of same type as pSrc to be connected
    *                   to pDst.
    *    \return            return true if connection between pSrc and pDst is allowed.
    */
    virtual bool IsValidConnection( FBComponent* pSrc, FBComponent*& pDst, bool& pAllowMultiple) override;

	/** Build context menu for KAMRManipulatorAssociation.
	*
	*	\param	pAMMenu	the menu to add menu option on.
	*	\param	pFocusedObject	the ORModelItem type of object you right-click on.
	*	\return	return true if menu item has been added.
	*/
	virtual bool MenuBuild( FBAMMenu* pAMMenu, FBComponent* pFocusedObject) override;

	/** Perform action if the added menu item is clicked.
	*
	*	\param	pMenuId	the menu ID right-click on.
	*	\param	pFocusedObject	the ORModelItem type of object you right-click on.
	*	\return	return true if action has been performed.
	*/
	virtual bool MenuAction( int pMenuId, FBComponent* pFocusedObject) override;

protected:
	int mAddTextureProps;
	int mAddModelProps;
	int mAddModelEvaluation;
	int mRemoveModelEvaluation;
	
	FBSystem mSystem;
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// ObjectNormalSolver - auto compute smooth normals in real-time

class SolverCalculateNormals : public FBConstraintSolver
{
	//--- declaration
	FBClassDeclare( SolverCalculateNormals, FBConstraintSolver );

public:
	//! a constructor
	SolverCalculateNormals(const char *pName = nullptr);

	virtual bool FBCreate() override;
	virtual void FBDestroy() override;		//!< FiLMBOX Destruction function.

	virtual bool PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData=nullptr, void* pDataOld=nullptr, int pDataSize=0) override;
	virtual bool PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug ) override;

	void OnPerFrameRenderingPipelineCallback    (HISender pSender, HKEvent pEvent);
	void OnSystemIdle (HISender pSender, HKEvent pEvent);

	static void SetReLoadShader(HIObject object, bool value);

public:

	FBPropertyAction						ReLoadShader;

	// model which normal buffer we will drive
	FBPropertyListObject					AffectedModels;

public:

	
	//
	// UPDATE RESOURCE FOR COMPONENTS
	//

	// update when ogl context has changed
	void ChangeContext();

	void ChangeGlobalSettings();
	
	

protected:
	bool	mNeedProgramReload{ true };

	FBSystem		mSystem;

public:
	
	void DoReloadShader();

protected:

	struct ModelSolverData
	{
		GLuint		mBufferId;	// duplicate buffer
		GLuint		duplicateCount;
		GLuint		vertexCount;
		GLuint		geomUpdateId;

		ModelSolverData()
		{
			mBufferId = 0;
			duplicateCount = 0;
			vertexCount = 0;
			geomUpdateId = 0;
		}
	};

	// this a workaround for the mobu bug
	GLuint					mBuffersId[100]{ 0 };
	GLuint					mBufferUseCount{ 0 };

	CComputeProgram			mProgramZero;
	CComputeProgram			mProgramRecomputeNormalsTris;
	CComputeProgram			mProgramRecomputeNormalsQuads;
	CComputeProgram			mProgramNorm;
	CComputeProgram			mProgramDup;

	std::unordered_map<FBModel*, ModelSolverData>		mModelData;

	void		GenerateGLBuffers();
	void		FreeGLBuffers();

	bool		LoadShaders();

	// prep duplicate buffer for each model
	bool		PrepModelData(FBModel *pModel);		

	bool		RunReComputeNormals(FBModel *pModel);

	/// <summary>
	/// verify that the model contains optimized triangulated geometry for rendering
	///  and we could bind our run-time compute shader to it
	/// </summary>
	bool	VerifyModel(FBModel* model);
};
