#pragma once

/**	\file	posteffect_contextmobu.h

Sergei <Neill3d> Solokhin 2025-2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

#include "posteffect_context.h"

//--- SDK include
#include <fbsdk/fbsdk.h>

#include <limits>
#include <ctime>
#include <stdint.h>

#include "posteffect_chain.h"
#include "shaderproperty_storage.h"

// forward declarations
class StandardEffectCollection;

// TODO: mark to delete in case context is not in use any more !
/**
* effect context, thread-safe
* implementation class is used to read from UI and build the data
* interface read methods can be used to read from render thread in a safe manner
*/
class PostEffectContextMoBu
{
public:

	PostEffectContextMoBu(FBCamera* cameraIn,
		FBComponent* userObjectIn,
		PostPersistentData* postProcessDataIn,
		FBEvaluateInfo* pEvaluateInfoIn,
		StandardEffectCollection* effectCollectionIn,
		const PostEffectContextProxy::Parameters& parametersIn);

	
	StandardEffectCollection* GetEffectCollection() const noexcept;
	PostPersistentData* GetPostProcessData() const;
	const PostEffectChain* GetFXChain() const;
	PostEffectChain* GetFXChain();
	const ShaderPropertyStorage* GetShaderPropertyStorage() const;
	ShaderPropertyStorage* GetShaderPropertyStorage();

	const PostEffectContextProxy::Cache& GetReadCache() const;
	PostEffectContextProxy::Cache& GetWriteCache();

	// evaluate thread to read UI and prepare cache for render
	void Evaluate(
		FBEvaluateInfo* pEvaluateInfoIn,
		FBCamera* cameraIn,
		const PostEffectContextProxy::Parameters& parametersIn);

	// synchronize between evaluate and render threads
	void Synchronize();

	// notify about graphics context change, clear all hardware resources
	void ChangeContext();

	bool ReloadShaders();

	bool IsAnyReloadShadersRequested() const;

	// check if we have needed connected data
	bool IsReadyToRender() const;

	bool Render(FBEvaluateInfo* pEvaluateInfoIn, 
		PostEffectBuffers* buffers);

	void SetPostProcessData(PostPersistentData* postProcessDataIn)
	{
		postProcessData = postProcessDataIn;
		effectChain.SetPersistenceData(postProcessData);
	}

private:

	PostEffectContextProxy::Cache mCache[2];
	std::atomic<uint32_t> mReadIndex{ 0 };
	
	// that is a key to the context and have to be the same in any thread	
	PostPersistentData* postProcessData{ nullptr }; //!< this is a main post process object for common effects properties
	
	StandardEffectCollection* standardEffects{ nullptr }; //!< standard effects collection to use

	PostEffectChain effectChain; //!< build a chain from a postProcessData
	
	// NOTE: this class is already thread-safe, call shaderPropertyStorage.CommitWrite(0);
	ShaderPropertyStorage shaderPropertyStorage; //!< storage for shader properties

	void SwapCacheIndices();

	// update parameters for write cache
	void UpdateContextParameters(PostEffectContextProxy::Cache& cacheOut, FBCamera* cameraIn, FBEvaluateInfo* pEvaluateInfoIn, const PostEffectContextProxy::Parameters& parametersIn);

	void PrepareCache(PostEffectContextProxy::Cache& cacheOut, FBCamera* camera, FBEvaluateInfo* pEvaluateInfoIn);
};