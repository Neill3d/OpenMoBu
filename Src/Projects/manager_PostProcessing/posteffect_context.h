#pragma once

/**	\file	posteffect_context.h

Sergei <Neill3d> Solokhin 2025-2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- SDK include
#include <fbsdk/fbsdk.h>
#include "hashUtils.h"
#include <array>
#include <mobu_logging.h>
#include "shaderproperty_storage.h"

// forward
class PostPersistentData;
class PostEffectChain;
class StandardEffectCollection;

/**
* keep track of each folded shader name and its parents
*/
class PostEffectNameContext
{
public:
	void PushName(const char* childName) {
		VERIFY(currentLevel < MAX_LEVELS - 1);
		currentLevel = currentLevel < MAX_LEVELS - 1 ? currentLevel + 1 : currentLevel;
		strcpy_s(names[currentLevel].data(), names[currentLevel].size(), childName);
	}

	void PopName() {
		VERIFY(currentLevel >= 0);
		currentLevel = currentLevel >= 0 ? currentLevel - 1 : EMPTY_LEVEL;
	}

	const char* GetParentName() const noexcept
	{
		return (currentLevel > 0) ? names[currentLevel - 1].data() : "";
	}
	uint32_t GetParentNameHash() const noexcept
	{
		return xxhash32(GetParentName());
	}

	const char* GetName() const noexcept
	{
		return (currentLevel >= 0) ? names[currentLevel].data() : "";
	}
	uint32_t GetNameHash() const noexcept
	{
		return xxhash32(GetName());
	}
private:

	static const int32_t MAX_LEVELS{ 2 };
	static const int32_t EMPTY_LEVEL{ -1 };
	// keep track of all folded effect names we process
	std::array<std::array<char, 64>, MAX_LEVELS> names;
	int32_t currentLevel{ EMPTY_LEVEL };
};


/**
* a proxy for effect context for a given thread
* we can use it to read/write data
*/
class PostEffectContextProxy
{
public:

	struct Parameters
	{
		FBMatrix prevModelViewProjMatrix; //!< modelview-projection matrix of the previous frame

		double sysTime{ 0.0 }; //!< system time (in seconds)
		double sysTimeDT{ 0.0 };

		double localTime{ 0.0 }; //!< playback time (in seconds)
		double localTimeDT{ 0.0 };

		int x{ 0 }; //!< viewport x position
		int y{ 0 }; //!< viewport y position
		int w{ 1 }; //!< viewport width
		int h{ 1 }; //!< viewport height
		int localFrame{ 0 }; //!< playback frame number
		bool isSkipFrame{ false }; //!< in case of system camera or schematic view for the given pane
	};

	struct Cache
	{
		// playback and viewport parameters
		Parameters parameters;

		// camera matrices and position

		FBMatrix	modelView;
		FBMatrix	projection;
		FBMatrix	modelViewProj;
		FBMatrix	invModelViewProj;
		FBMatrix	prevModelViewProj;
		FBVector3d	cameraPosition;

		mutable FBComponent* userObject{ nullptr }; //!< this is a component where all ui properties are exposed
		FBCamera* camera{ nullptr }; //!< current camera that we are drawing with

		float zNear;
		float zFar;

		float modelViewF[16];
		float projectionF[16];
		float modelViewProjF[16];
		float invModelViewProjF[16];
		float prevModelViewProjF[16];
		float cameraPositionF[3];
		float iDate[4];

		bool isCameraOrtho{ false };
	};

public:

	PostEffectContextProxy(
		FBCamera* cameraIn,
		FBEvaluateInfo* pEvaluateInfoIn,
		StandardEffectCollection* effectCollectionIn,
		PostPersistentData* postProcessDataIn,
		PostEffectChain* effectChainIn,
		ShaderPropertyStorage::EffectMap* effectPropertyMapIn,
		const Cache& cacheIn)
		: camera(cameraIn)
		, pEvaluateInfo(pEvaluateInfoIn)
		, effectCollection(effectCollectionIn)
		, postProcessData(postProcessDataIn)
		, effectChain(effectChainIn)
		, effectPropertyMap(effectPropertyMapIn)
		, cache(cacheIn)
	{
	}

	// interface to query the needed data

	[[nodiscard]] int GetViewWidth() const noexcept { return cache.parameters.w; }
	[[nodiscard]] int GetViewHeight() const noexcept { return cache.parameters.h; }

	[[nodiscard]] int GetLocalFrame() const noexcept { return cache.parameters.localFrame; }
	[[nodiscard]] double GetSystemTime() const noexcept { return cache.parameters.sysTime; }
	[[nodiscard]] double GetLocalTime() const noexcept { return cache.parameters.localTime; }

	[[nodiscard]] double GetLocalTimeDT() const noexcept { return cache.parameters.localTimeDT; }
	[[nodiscard]] double GetSystemTimeDT() const noexcept { return cache.parameters.sysTimeDT; }

	[[nodiscard]] double* GetCameraPosition() const noexcept { return cache.cameraPosition; }
	[[nodiscard]] const float* GetCameraPositionF() const noexcept { return cache.cameraPositionF; }

	[[nodiscard]] float GetCameraNearDistance() const noexcept { return cache.zNear; }
	[[nodiscard]] float GetCameraFarDistance() const noexcept { return cache.zFar; }

	[[nodiscard]] bool IsCameraOrthogonal() const noexcept { return cache.isCameraOrtho; }

	[[nodiscard]] double* GetModelViewMatrix() const noexcept { return cache.modelView; }
	[[nodiscard]] const float* GetModelViewMatrixF() const noexcept { return cache.modelViewF; }
	[[nodiscard]] double* GetProjectionMatrix() const noexcept { return cache.projection; }
	[[nodiscard]] const float* GetProjectionMatrixF() const noexcept { return cache.projectionF; }
	[[nodiscard]] double* GetModelViewProjMatrix() const noexcept { return cache.modelViewProj; }
	[[nodiscard]] const float* GetModelViewProjMatrixF() const noexcept { return cache.modelViewProjF; }
	// returns the modelview-projection matrix of the previous frame
	[[nodiscard]] const float* GetPrevModelViewProjMatrixF() const noexcept { return cache.prevModelViewProjF; }
	// returns the inverse of the modelview-projection matrix
	[[nodiscard]] const float* GetInvModelViewProjMatrixF() const noexcept { return cache.invModelViewProjF; }

	// 4 floats in format - year + 1900, month + 1, day, seconds since midnight
	[[nodiscard]] const float* GetIDate() const noexcept { return cache.iDate; }

	[[nodiscard]] StandardEffectCollection* GetEffectCollection() const noexcept { return effectCollection; }
	[[nodiscard]] FBCamera* GetCamera() const { return camera; }
	[[nodiscard]] PostPersistentData* GetPostProcessData() const { return postProcessData; }
	[[nodiscard]] const PostEffectChain* GetFXChain() const { return effectChain; }
	[[nodiscard]] PostEffectChain* GetFXChain() { return effectChain; }
	
	FBEvaluateInfo* GetEvaluateInfo() const { return pEvaluateInfo; }

	// read/write shader property values
	const ShaderPropertyStorage::EffectMap* GetEffectPropertyMap() const { return effectPropertyMap; }
	ShaderPropertyStorage::EffectMap* GetEffectPropertyMap() { return effectPropertyMap; }

	ShaderPropertyStorage::PropertyValueMap* GetEffectPropertyValueMap(uint32_t effectHash)
	{
		return const_cast<ShaderPropertyStorage::PropertyValueMap*>(
			std::as_const(*this).GetEffectPropertyValueMap(effectHash));
	}

	const ShaderPropertyStorage::PropertyValueMap* GetEffectPropertyValueMap(uint32_t effectHash) const
	{
		if (effectPropertyMap)
		{
			const auto it = effectPropertyMap->find(effectHash);
			if (it != effectPropertyMap->cend())
			{
				return &it->second;
			}
		}
		return nullptr;
	}

private:

	FBCamera* camera{ nullptr };
	FBEvaluateInfo* pEvaluateInfo{ nullptr };
	StandardEffectCollection* effectCollection{ nullptr };
	PostPersistentData* postProcessData{ nullptr };
	PostEffectChain* effectChain{ nullptr };
	ShaderPropertyStorage::EffectMap* effectPropertyMap{ nullptr };
	const Cache& cache;
	
};