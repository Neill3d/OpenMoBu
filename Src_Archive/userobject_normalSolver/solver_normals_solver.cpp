
/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/MoBu/blob/master/LICENSE
//
// GitHub repository - https://github.com/Neill3d/MoBu
//
// Author Sergey Solokhin (Neill3d) 2014-2017
//  e-mail to: s@neill3d.com
//		www.neill3d.com
/////////////////////////////////////////////////////////////////////////////////////////


#include "solver_normals_solver.h"
#include "nv_math.h"
//#include "graphics\checkglerror_MOBU.h"
#include <Windows.h>
#include "resource.h"
#include "ResourceUtils.h"

//--- Registration defines
#define	NORMALSOLVER__CLASS		NORMALSOLVER__CLASSNAME
#define NORMALSOLVER__NAME		"SolverCalculateNormals"
#define NORMALSOLVER__LABEL		"SolverCalculateNormals"
#define NORMALSOLVER__DESC		"SolverCalculateNormals"

FBConstraintSolverImplementation (	NORMALSOLVER__CLASS		);
FBRegisterConstraintSolver		(	NORMALSOLVER__NAME,
									NORMALSOLVER__CLASS,
									NORMALSOLVER__LABEL,
									NORMALSOLVER__DESC,
									"character_solver.png"			);	// Icon filename (default=Open Reality icon)

FBApplyManagerRuleImplementation(KNormalSolverAssociation);
FBRegisterApplyManagerRule( KNormalSolverAssociation, "KNormalSolverAssociation", "Apply Manager Rule for Normal Solver");


//extern void DebugOGL_Callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, const void*userParam);



/////////////////////////////////////////////////////////////////////////////////////////////////////////
//

bool KNormalSolverAssociation::IsValidSrc( FBComponent *pSrc)
{
	return FBIS(pSrc, FBModel);
}

bool KNormalSolverAssociation::IsValidConnection( FBComponent* pSrc, FBComponent*& pDst, bool& pAllowMultiple)
{
	pAllowMultiple = true;
	return FBIS(pDst, SolverCalculateNormals);		// add src into the solver components !!
}

bool KNormalSolverAssociation::MenuBuild( FBAMMenu* pAMMenu, FBComponent* pFocusedObject)
{
	mAddTextureProps  = -1;
	mAddModelProps = -1;
	mAddModelEvaluation = -1;
	mRemoveModelEvaluation = -1;

	if(pFocusedObject)
	{
		if( FBIS(pFocusedObject, FBTexture) )
		{
			pAMMenu->AddOption( "" );
			mAddTextureProps = pAMMenu->AddOption("Add Sprite Sheet Properties", -1, true);
			
			pAMMenu->AddOption( "" );
			return true;
		}
		else if (FBIS(pFocusedObject, FBModel) )
		{
			FBModelVertexData *pData = ((FBModel*) pFocusedObject)->ModelVertexData;

			if (nullptr != pData && pData->IsDrawable() )
			{
				pAMMenu->AddOption( "" );
				mAddModelEvaluation = pAMMenu->AddOption( "Add To Normal Solver", -1, true);
				pAMMenu->AddOption( "" );
			}
			return true;
		}
	}
	return false;
}

bool KNormalSolverAssociation::MenuAction( int pMenuId, FBComponent* pFocusedObject)
{
	bool result = false;

	
	if (mAddModelEvaluation >= 0 && pMenuId == mAddModelEvaluation)
	{
		// DONE: automaticaly add properties to texture and model

		// try to find if model is already in some solver
		SolverCalculateNormals *pSolver = nullptr;
		FBScene *pScene = mSystem.Scene;
		for (int i=0, count=pScene->ConstraintSolvers.GetCount(); i<count; ++i)
		{	
			if (FBIS(pScene->ConstraintSolvers[i], SolverCalculateNormals) )
			{
				int index = pScene->ConstraintSolvers[i]->Components.Find(pFocusedObject);
				if (index >= 0)
				{
					pSolver = (SolverCalculateNormals*) pScene->ConstraintSolvers[i];
					break;
				}
			}
		}

		if (pSolver)
		{
			// model is already in the solver, let's do Unregister operation
			bool activeState = pSolver->Active;
			pSolver->Active = false;

			bool isExist = false;
			for (int i=0, count=pSolver->AffectedModels.GetCount(); i<count; ++i)
			{
				if ( pSolver->AffectedModels[i] == pFocusedObject )
				{
					isExist = true;
					break;
				}
			}

			if (false == isExist)
				pSolver->AffectedModels.Add(pFocusedObject);
			pSolver->Active = activeState;
		}
		else
		{
			// register model for the first model in the scene solvers
			FBModel *pModel = (FBModel*) pFocusedObject;
			pSolver = (SolverCalculateNormals*) FBCreateObject( "Browsing/Templates/Solvers", "SolverCalculateNormals", "Normal Solver" );

			if (nullptr != pSolver)
			{
				pSolver->AffectedModels.Add(pFocusedObject);
			}
		}
	}

	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////
// ObjectBlendSolver

void SolverCalculateNormals::SetReLoadShader(HIObject object, bool value)
{
	SolverCalculateNormals *pBase = FBCast<SolverCalculateNormals>(object);
	if (pBase && value) 
	{
		pBase->DoReloadShader();
	}
}

SolverCalculateNormals::SolverCalculateNormals(const char *pName)
		: FBConstraintSolver( pName )
{
	FBClassInit;
	
	mNeedProgramReload = false;

	for (int i=0; i<100; ++i)
		mBuffersId[i] = 0;
	
}

bool SolverCalculateNormals::FBCreate()
{
	
#ifdef _DEBUG
	FBPropertyPublish(this, ReLoadShader, "Reload shader", nullptr, SetReLoadShader);
#endif

	FBPropertyPublish(this, AffectedModels, "Affected Models", nullptr, nullptr);
	
	Active = true;
	
	AffectedModels.SetSingleConnect(false);
	AffectedModels.SetFilter(FBModel::GetInternalClassId() );
	
	mSystem.OnUIIdle.Add( this, (FBCallback) &SolverCalculateNormals::OnSystemIdle );

	return ParentClass::FBCreate();
}

void SolverCalculateNormals::FBDestroy()
{
	FreeGLBuffers();
	
	//

	FBEvaluateManager::TheOne().OnRenderingPipelineEvent .Remove(this, (FBCallback)&SolverCalculateNormals::OnPerFrameRenderingPipelineCallback);
	
	ParentClass::FBDestroy();
}


bool SolverCalculateNormals::PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData,void* pDataOld,int pDataSize)
{
	return ParentClass::PlugDataNotify(pAction, pThis, pData, pDataOld, pDataSize);
}

bool SolverCalculateNormals::PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug )
{
	return ParentClass::PlugNotify(pAction, pThis, pIndex, pPlug, pConnectionType, pNewPlug);
}

void SolverCalculateNormals::OnSystemIdle (HISender pSender, HKEvent pEvent)
{
	FBEvaluateManager::TheOne().OnRenderingPipelineEvent .Add(this, (FBCallback)&SolverCalculateNormals::OnPerFrameRenderingPipelineCallback);

	mSystem.OnUIIdle.Remove( this, (FBCallback) &SolverCalculateNormals::OnSystemIdle );
}

void SolverCalculateNormals::OnPerFrameRenderingPipelineCallback    (HISender pSender, HKEvent pEvent)
{
	if (false == Active || AffectedModels.GetCount() == 0)
		return;

	static bool firstTime = true;

	if (true == firstTime)
	{
		/*
#ifdef _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback( DebugOGL_Callback, nullptr );
#endif
	*/
		firstTime = false;
	}

	//HGLRC context = wglGetCurrentContext();

	
	FBEventEvalGlobalCallback lFBEvent(pEvent);

	switch(lFBEvent.GetTiming() )
	{
	case kFBGlobalEvalCallbackBeforeRender:
		{
		
			for (int i=0, count=AffectedModels.GetCount(); i<count; ++i)
			{
				FBModel *pModel = (FBModel*) AffectedModels.GetAt(i);

				PrepModelData(pModel);
				RunReComputeNormals(pModel);
			}
		
		}
		break;
	case kFBGlobalEvalCallbackAfterRender:
		
		break;
	}
}

// update when ogl context has changed
void SolverCalculateNormals::ChangeContext()
{
	FreeGLBuffers();
}

void SolverCalculateNormals::ChangeGlobalSettings()
{
}

void SolverCalculateNormals::DoReloadShader()
{
	mNeedProgramReload = true;
}

void SolverCalculateNormals::GenerateGLBuffers()
{
	FreeGLBuffers();
	glGenBuffers(100, mBuffersId);
}

void SolverCalculateNormals::FreeGLBuffers()
{
	if (mBuffersId[0] > 0)
		glDeleteBuffers(100, &mBuffersId[0] );

	for (auto iter=begin(mModelData); iter!=end(mModelData); ++iter)
	{
		ModelSolverData &data = iter->second;

		if (data.mBufferId > 0)
		{
			glDeleteBuffers(1, &data.mBufferId);
			data.mBufferId = 0;
		}
	}
}

bool SolverCalculateNormals::PrepModelData(FBModel *pModel)
{
	auto iter = mModelData.find(pModel);

	ModelSolverData data;

	if (iter != end(mModelData) )
		data = iter->second;

	FBGeometry *pGeometry = pModel->Geometry;
	FBModelVertexData *pData = pModel->ModelVertexData;

	if (nullptr == pGeometry || nullptr == pData 
		|| false == pData->IsDrawable() )
		return false;

	const int vertexCount = pData->GetVertexCount();
	const int geomUpdateId = pModel->GeometryUpdateId;

	if (geomUpdateId != data.geomUpdateId || vertexCount != data.vertexCount)
	{
		// let's update duplicate buffer

		if ( 0 == data.mBufferId )
			glGenBuffers( 1, &data.mBufferId );

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, data.mBufferId);

		data.duplicateCount = 0;
		const int *duplicates = pData->GetVertexArrayDuplicationMap(data.duplicateCount);

		const size_t size = sizeof(int) * data.duplicateCount;

		glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_STREAM_READ);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, duplicates, GL_STREAM_READ);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		data.geomUpdateId = geomUpdateId;
		data.vertexCount = vertexCount;

		if (iter != end(mModelData) )
			iter->second = data;
		else
			mModelData.insert( std::make_pair(pModel, data) );
	}
	return true;
}



bool SolverCalculateNormals::LoadShaders()
{
	if (true == mNeedProgramReload)
	{
		mProgramZero.Clear();
		mProgramNorm.Clear();
		mProgramRecomputeNormals.Clear();
		mNeedProgramReload = false;
	}

	// check for a compute shader
	/*
	if (false == mProgramZero.PrepProgram("\\GLSL_CS\\recomputeNormalsZero.cs") )
		return false;

	if (false == mProgramRecomputeNormals.PrepProgram("\\GLSL_CS\\recomputeNormals.cs") )
		return false;
	
	if (false == mProgramNorm.PrepProgram("\\GLSL_CS\\recomputeNormalsNorm.cs") )
		return false;
		*/

	
	if (0 == mProgramZero.GetProgramId() )
	{
		bool res = false;
		char* data = nullptr;
		HINSTANCE hInst = GetThisModuleHandle();
		
		HRSRC hRes = FindResourceEx(hInst,
                           "TEXT",
                           MAKEINTRESOURCE(IDR_TEXT1),
                           MAKELANGID(LANG_NEUTRAL,
                           SUBLANG_NEUTRAL));
						   
		if(nullptr != hRes)
		{
			HGLOBAL hData = LoadResource(hInst, hRes);
			if (hData)
			{
				DWORD dataSize = SizeofResource(hInst, hRes);
				data = (char*)LockResource(hData);
				
				char *shaderData = new char[dataSize+1];
				memset( shaderData, 0, sizeof(char) * (dataSize+1) );
				memcpy( shaderData, data, sizeof(char) * dataSize );

				res = mProgramZero.PrepProgramFromBuffer(shaderData, "recomputeNormalsZero");
				
				delete [] shaderData;

				UnlockResource(hData);
			}
		}

		if (false == res)
		{
			Active =false;
			return false;
		}
	}
	if (0 == mProgramNorm.GetProgramId() )
	{
		bool res = false;
		char* data = nullptr;
		HINSTANCE hInst = GetThisModuleHandle();
		
		HRSRC hRes = FindResourceEx(hInst,
                           "TEXT",
                           MAKEINTRESOURCE(IDR_TEXT2),
                           MAKELANGID(LANG_NEUTRAL,
                           SUBLANG_NEUTRAL));
		if(nullptr != hRes)
		{
			HGLOBAL hData = LoadResource(hInst, hRes);
			if (hData)
			{
				DWORD dataSize = SizeofResource(hInst, hRes);
				data = (char*)LockResource(hData);

				char *shaderData = new char[dataSize+1];
				memset( shaderData, 0, sizeof(char) * (dataSize+1) );
				memcpy( shaderData, data, sizeof(char) * dataSize );

				res = mProgramNorm.PrepProgramFromBuffer(shaderData, "recomputeNormalsNorm");
				
				delete [] shaderData;

				UnlockResource(hData);
			}
		}

		if (false == res)
		{
			Active =false;
			return false;
		}
	}
	if (0 == mProgramRecomputeNormals.GetProgramId() )
	{
		bool res = false;
		char* data = nullptr;
		HINSTANCE hInst = GetThisModuleHandle();
		
		HRSRC hRes = FindResourceEx(hInst,
                           "TEXT",
                           MAKEINTRESOURCE(IDR_TEXT3),
                           MAKELANGID(LANG_NEUTRAL,
                           SUBLANG_NEUTRAL));
		if(nullptr != hRes)
		{
			HGLOBAL hData = LoadResource(hInst, hRes);
			if (hData)
			{
				DWORD dataSize = SizeofResource(hInst, hRes);
				data = (char*)LockResource(hData);

				char *shaderData = new char[dataSize+1];
				memset( shaderData, 0, sizeof(char) * (dataSize+1) );
				memcpy( shaderData, data, sizeof(char) * dataSize );

				res = mProgramRecomputeNormals.PrepProgramFromBuffer(shaderData, "recomputeNormals");
				
				delete [] shaderData;

				UnlockResource(hData);
			}
		}

		if (false == res)
		{
			Active =false;
			return false;
		}
	}

	return true;
}

bool SolverCalculateNormals::RunReComputeNormals(FBModel *pModel)
{
	if (false == LoadShaders() )
		return false;
	
	//
	// prepare number of blendshapes

	FBGeometry *pGeometry = pModel->Geometry;
	FBModelVertexData *pData = pModel->ModelVertexData;

	if ( nullptr == pGeometry || nullptr == pData || false == pData->IsDrawable() )
		return false;

	const int numberOfVertices = pData->GetVertexCount();
	
	if (0 == numberOfVertices)
		return false;


	int numberOfIndices = 0;
	
	for (int i=0, count=pData->GetSubPatchCount(); i<count; ++i)
	{
		int offset = pData->GetSubPatchIndexOffset(i);
		int size = pData->GetSubPatchIndexSize(i);

		int localCount = offset+size;
		if ( localCount > numberOfIndices )
			numberOfIndices = localCount;
	}

	const int numberOfTriangles = numberOfIndices / 3;

	//
	// run a compute program

	const GLuint posId = pData->GetVertexArrayVBOId( kFBGeometryArrayID_Point, true );
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posId );

	const GLuint indId = pData->GetIndexArrayVBOId();
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, indId );

	const GLuint deformId = pData->GetVertexArrayVBOId( kFBGeometryArrayID_Normal, true );
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, deformId );
	
	// duplicate buffer if allocated
	int duplicateCount = 0;

	auto iter = mModelData.find( pModel );
	if (iter != end(mModelData) )
	{
		duplicateCount = iter->second.duplicateCount;
		
		if (iter->second.mBufferId > 0)
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, iter->second.mBufferId );
	}

	

//	CHECK_GL_ERROR_MOBU();

	//
	// ZERO NORMALS
	{
	const GLuint programId = mProgramZero.GetProgramId();
	if (programId == 0)
		return false;

	mProgramZero.Bind();

	GLint loc = glGetUniformLocation( programId, "numberOfNormals" );
	if (loc >= 0)
		glProgramUniform1i( programId, loc, numberOfVertices );

	const int computeLocalX = 1024;
	const int x = numberOfVertices / computeLocalX + 1;

	mProgramZero.DispatchPipeline( x, 1, 1 );

	mProgramZero.UnBind();

//	CHECK_GL_ERROR_MOBU();
	}
	
	glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT ); //GL_ALL_BARRIER_BITS

	//
	// ACCUM NORMALS
	{
	const GLuint programId = mProgramRecomputeNormals.GetProgramId();
	if (programId == 0)
		return false;

	mProgramRecomputeNormals.Bind();

	GLint loc = glGetUniformLocation( programId, "numberOfTriangles" );
	if (loc >= 0)
		glProgramUniform1i( programId, loc, numberOfTriangles );
	
	const int computeLocalX = 1024;
	const int x = numberOfTriangles / computeLocalX + 1;

	mProgramRecomputeNormals.DispatchPipeline( x, 1, 1 );

	mProgramRecomputeNormals.UnBind();

//	CHECK_GL_ERROR_MOBU();
	}

	glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );

	//
	// ZERO NORMALS


	{
	const GLuint programId = mProgramNorm.GetProgramId();
	if (programId == 0)
		return false;

	mProgramNorm.Bind();

	GLint loc = glGetUniformLocation( programId, "numberOfNormals" );
	if (loc >= 0)
		glProgramUniform1i( programId, loc, numberOfVertices );

	loc = glGetUniformLocation( programId, "duplicateStart" );
	if (loc >= 0)
		glProgramUniform1i( programId, loc, numberOfVertices - duplicateCount );
	
	const int computeLocalX = 1024;
	const int x = numberOfVertices / computeLocalX + 1;

	mProgramNorm.DispatchPipeline( x, 1, 1 );

	mProgramNorm.UnBind();
	}
//	CHECK_GL_ERROR_MOBU();

	return true;
}
