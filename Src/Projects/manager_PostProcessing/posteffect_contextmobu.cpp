/**	\file	posteffect_contextmobu.cpp

Sergei <Neill3d> Solokhin 2025-2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

#include "posteffect_contextmobu.h"

#include <limits>
#include <ctime>

#include "posteffect_chain.h"
#include "shaderproperty_storage.h"
#include "standardeffectcollection.h"


PostEffectContextMoBu::PostEffectContextMoBu(FBCamera* cameraIn,
	FBComponent* userObjectIn, 
	PostPersistentData* postProcessDataIn, 
	FBEvaluateInfo* pEvaluateInfoIn, 
	StandardEffectCollection* effectCollectionIn,
	const PostEffectContextProxy::Parameters& parametersIn)
		: postProcessData(postProcessDataIn)
		, standardEffects(effectCollectionIn)
		, effectChain(postProcessDataIn)
{
	UpdateContextParameters(mCache[0], cameraIn, pEvaluateInfoIn, parametersIn);
	mCache[1] = mCache[0];
}

StandardEffectCollection* PostEffectContextMoBu::GetEffectCollection() const noexcept
{
	return standardEffects;
}

const PostEffectContextProxy::Cache& PostEffectContextMoBu::GetReadCache() const
{
	const PostEffectContextProxy::Cache& c = mCache[mReadIndex.load(std::memory_order_acquire)];
	return c;
}
PostEffectContextProxy::Cache& PostEffectContextMoBu::GetWriteCache()
{
	const uint32_t writeIndex = 1 - mReadIndex.load(std::memory_order_acquire);
	return mCache[writeIndex];
}
void PostEffectContextMoBu::SwapCacheIndices()
{
	mReadIndex.fetch_xor(1, std::memory_order_acq_rel);
}

PostPersistentData* PostEffectContextMoBu::GetPostProcessData() const
{ 
	return postProcessData; 
}

const PostEffectChain* PostEffectContextMoBu::GetFXChain() const 
{ 
	return &effectChain; 
}
PostEffectChain* PostEffectContextMoBu::GetFXChain() { 
	return &effectChain; 
}
const ShaderPropertyStorage* PostEffectContextMoBu::GetShaderPropertyStorage() const 
{ 
	return &shaderPropertyStorage; 
}
ShaderPropertyStorage* PostEffectContextMoBu::GetShaderPropertyStorage() 
{ 
	return &shaderPropertyStorage; 
}

void PostEffectContextMoBu::UpdateContextParameters(PostEffectContextProxy::Cache& cacheOut, FBCamera* cameraIn, FBEvaluateInfo* pEvaluateInfoIn, const PostEffectContextProxy::Parameters& parametersIn)
{
	cacheOut.camera = cameraIn;
	cacheOut.parameters = parametersIn;
	PrepareCache(cacheOut, cameraIn, pEvaluateInfoIn);
}

void PostEffectContextMoBu::Evaluate(
	FBEvaluateInfo* pEvaluateInfoIn, 
	FBCamera* cameraIn, 
	const PostEffectContextProxy::Parameters& parametersIn)
{
	UpdateContextParameters(GetWriteCache(), cameraIn, pEvaluateInfoIn, parametersIn);
	
	PostEffectContextProxy proxy(
		cameraIn,
		pEvaluateInfoIn,
		standardEffects,
		postProcessData,
		&effectChain,
		&shaderPropertyStorage.GetWriteEffectMap(),
		GetWriteCache());

	effectChain.Evaluate(&proxy);
}

void PostEffectContextMoBu::Synchronize()
{
	SwapCacheIndices();
	effectChain.Synchronize();
	shaderPropertyStorage.CommitWrite(0);
}

void PostEffectContextMoBu::ChangeContext()
{
	effectChain.ChangeContext();
	shaderPropertyStorage.Clear();
}

bool PostEffectContextMoBu::IsReadyToRender() const
{
	return effectChain.IsReadyToRender();
}

bool PostEffectContextMoBu::IsAnyReloadShadersRequested() const
{
	if (!standardEffects || !postProcessData)
	{
		return false;
	}

	return (standardEffects->IsNeedToReloadShaders()
		|| postProcessData->IsNeedToReloadShaders(false)
		|| postProcessData->IsExternalReloadRequested());
}

bool PostEffectContextMoBu::ReloadShaders()
{
	if (!standardEffects || !postProcessData)
	{
		return false;
	}

	// standard effects
	constexpr const bool propagateToUserEffects = false;
	if (postProcessData->IsNeedToReloadShaders(propagateToUserEffects)
		|| standardEffects->IsNeedToReloadShaders())
	{
		postProcessData->DoReloadShaders();
		standardEffects->ChangeContext();
		effectChain.ChangeContext();
		if (!standardEffects->ReloadShaders())
		{
			return false;
		}
	}

	// user effects

	if (postProcessData->IsExternalReloadRequested())
	{
		for (int i = 0; i < postProcessData->UserEffects.GetCount(); ++i)
		{
			FBComponent* component = postProcessData->UserEffects.GetAt(i);
			EffectShaderUserObject* userEffect = FBCast<EffectShaderUserObject>(component);
			if (userEffect)
			{
				if (userEffect->IsNeedToReloadShaders())
				{
					if (!userEffect->DoReloadShaders())
					{
						return false;
					}
				}
			}
		}
	}

	return true;
}

bool PostEffectContextMoBu::Render(FBEvaluateInfo* pEvaluateInfoIn, PostEffectBuffers* buffers)
{
	if (!effectChain.IsReadyToRender())
	{
		return false;
	}

	const PostEffectContextProxy::Cache& cache = GetReadCache();
	const double time = cache.parameters.localTime;

	PostEffectContextProxy proxy(
		cache.camera,
		pEvaluateInfoIn,
		standardEffects,
		postProcessData,
		&effectChain,
		&shaderPropertyStorage.GetReadEffectMap(),
		cache);

	return effectChain.Render(buffers, time, &proxy);
}


void PostEffectContextMoBu::PrepareCache(PostEffectContextProxy::Cache& cacheOut, FBCamera* camera, FBEvaluateInfo* pEvaluateInfoIn)
{
	if (!camera)
		return;
	
	cacheOut.zNear = static_cast<float>(camera->NearPlaneDistance);
	cacheOut.zFar = static_cast<float>(camera->FarPlaneDistance);
		
	cacheOut.isCameraOrtho = (camera->Type == FBCameraType::kFBCameraTypeOrthogonal);

	camera->GetVector(cacheOut.cameraPosition, kModelTranslation, true, pEvaluateInfoIn);
	for (int i = 0; i < 3; ++i)
		cacheOut.cameraPositionF[i] = static_cast<float>(cacheOut.cameraPosition[i]);

	camera->GetCameraMatrix(cacheOut.modelView, FBCameraMatrixType::kFBModelView, pEvaluateInfoIn);
	camera->GetCameraMatrix(cacheOut.projection, FBCameraMatrixType::kFBProjection, pEvaluateInfoIn);
	camera->GetCameraMatrix(cacheOut.modelViewProj, FBCameraMatrixType::kFBModelViewProj, pEvaluateInfoIn);
	FBMatrixInverse(cacheOut.invModelViewProj, cacheOut.modelViewProj);
	cacheOut.prevModelViewProj = cacheOut.parameters.prevModelViewProjMatrix;

	for (int i = 0; i < 16; ++i)
	{
		cacheOut.modelViewF[i] = static_cast<float>(cacheOut.modelView[i]);
		cacheOut.projectionF[i] = static_cast<float>(cacheOut.projection[i]);
		cacheOut.modelViewProjF[i] = static_cast<float>(cacheOut.modelViewProj[i]);
		cacheOut.invModelViewProjF[i] = static_cast<float>(cacheOut.invModelViewProj[i]);
		cacheOut.prevModelViewProjF[i] = static_cast<float>(cacheOut.prevModelViewProj[i]);
	}
		
	std::time_t now = std::time(nullptr);
	std::tm localTime;
	localtime_s(&localTime, &now);  // now should be of type std::time_t

	const float secondsSinceMidnight = static_cast<float>(localTime.tm_hour * 3600 + localTime.tm_min * 60 + localTime.tm_sec);
	cacheOut.iDate[0] = static_cast<float>(localTime.tm_year + 1900);
	cacheOut.iDate[1] = static_cast<float>(localTime.tm_mon + 1);
	cacheOut.iDate[2] = static_cast<float>(localTime.tm_mday);
	cacheOut.iDate[3] = secondsSinceMidnight;
}