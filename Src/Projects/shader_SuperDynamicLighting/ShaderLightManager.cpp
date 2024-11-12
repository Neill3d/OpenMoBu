
// SuperShader.cpp
/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "ShaderLightManager.h"
#include "CheckGLError.h"

namespace Graphics {



	

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// FBShaderLights

	ShaderLightManager::ShaderLightManager()
	{
		//mNumberOfLights = 0;
		//mNumberOfDirLights = 0;
	}

	ShaderLightManager::~ShaderLightManager()
	{
	}

	void ShaderLightManager::UpdateTransformedLights(const glm::mat4& modelview, const glm::mat4& rotation, const glm::mat4& scaling)
	{
		// Ensure transformed vectors are the right size, if needed
		if (transformedLights.size() != lights.size()) {
			transformedLights.resize(lights.size());
		}
		if (transformedDirLights.size() != dirLights.size()) {
			transformedDirLights.resize(dirLights.size());
		}

		// Transform point/spot lights
		for (std::size_t i = 0; i < lights.size(); ++i) {
			transformedLights[i] = lights[i];  // Copy source light to destination
			transformedLights[i].position = modelview * glm::vec4(lights[i].position, 1.0f);  // Transform position
			transformedLights[i].dir = rotation * glm::vec4(lights[i].dir, 0.0f);  // Transform direction
		}

		// Transform directional lights
		for (std::size_t i = 0; i < dirLights.size(); ++i) {
			transformedDirLights[i] = dirLights[i];  // Copy source light to destination
			transformedDirLights[i].dir = rotation * glm::vec4(dirLights[i].dir, 0.0f);  // Transform direction only
			// Note: No position transform for directional lights as they represent directions.
		}
	}

	void ShaderLightManager::MapOnGPU()
	{
		// dir lights
		if (!transformedDirLights.empty())
		{
			bufferDirLights.UpdateData(sizeof(TLight), transformedDirLights.size(), transformedDirLights.data());
		}
		
		// point / spot lights
		if (!transformedLights.empty())
		{
			bufferLights.UpdateData(sizeof(TLight), transformedLights.size(), transformedLights.data());
		}
		
		//
		//mNumberOfDirLights = (int) mTransformedDirLights.size();
		//mNumberOfLights = (int) mTransformedLights.size();
	}


	void ShaderLightManager::Resize(std::size_t dirLightsCount, std::size_t lightsCount)
	{
		if (dirLightsCount > 0 && dirLightsCount != dirLights.size())
		{
			dirLights.resize(dirLightsCount);
		}
		else if (dirLightsCount == 0)
		{
			dirLights.clear();
		}

		if (lightsCount > 0 && lightsCount != lights.size())
		{
			lights.resize(lightsCount);
		}
		else if (lightsCount == 0)
		{
			lights.clear();
		}
	}

	TLight& ShaderLightManager::GetLightRef(std::size_t index)
	{
		assert(index < lights.size());
		return lights[index];
	}

	TLight& ShaderLightManager::GetDirLightRef(std::size_t index)
	{
		assert(index < dirLights.size());
		return dirLights[index];
	}


	void ShaderLightManager::PrepGPUPtr()
	{
		/*
		// dir lights
		if ( mBufferDirLights.GetCount() > 0 )
			mBufferDirLights.UpdateGPUPtr();

		// point / spot
		if ( mBufferLights.GetCount() > 0 )
		mBufferLights.UpdateGPUPtr();

		mNumberOfDirLights = (int) mBufferDirLights.GetCount();
		mNumberOfLights = (int) mBufferLights.GetCount();
		*/


	}

	/*
	void ShaderLightManager::Build(CCameraInfoCache &cameraCache, std::vector<FBLight*> &lights)
	{
	if (lights.size() == 0)
	{
	mDirLights.clear();
	mLights.clear();

	return;
	}

	FBMatrix pCamMatrix(cameraCache.mv);
	//pCamera->GetCameraMatrix( pCamMatrix, kFBModelView );

	FBMatrix lViewMatrix( pCamMatrix );

	FBRVector lViewRotation;
	FBMatrixToRotation(lViewRotation, lViewMatrix);

	FBMatrix lViewRotationMatrix;
	FBRotationToMatrix(lViewRotationMatrix, lViewRotation);

	//
	const int numLights = (int) lights.size();

	// process scene lights and enable clustering if some point/spot light exist

	int numDirLights = 0;
	int numPointLights = 0;
	//int numLightCasters = 0;

	for (int i=0; i<numLights; ++i)
	{
	FBLight *pLight = lights[i];

	if (pLight->CastLightOnObject)
	{
	if (pLight->LightType == kFBLightTypeInfinite) numDirLights++;
	else numPointLights++;
	}

	}


	mDirLights.resize(numDirLights);
	mLights.resize(numPointLights);

	numDirLights = 0;
	numPointLights = 0;
	//numLightCasters = 0;

	for (int i=0; i<numLights; ++i)
	{
	FBLight *pLight = lights[i];
	LightDATA *pLightData = nullptr;

	if (pLight->CastLightOnObject)
	{
	if (pLight->LightType != kFBLightTypeInfinite)
	{
	LightDATA::ConstructFromFBLight( false, lViewMatrix, lViewRotationMatrix, pLight, mLights[numPointLights] );
	pLightData = &mLights[numPointLights];
	numPointLights++;
	}
	else
	{
	LightDATA::ConstructFromFBLight( true, lViewMatrix, lViewRotationMatrix, pLight, mDirLights[numDirLights] );
	pLightData = &mDirLights[numDirLights];
	numDirLights++;
	}
	}

	}

	//
	//

	mat4 modelrotation;

	for (int i=0; i<16; ++i)
	{
	modelrotation.mat_array[i] = (float) lViewRotationMatrix[i];
	}

	for (size_t i=0; i<mLights.size(); ++i)
	{
	mLights[i].position = cameraCache.mv4 * mLights[i].position;
	mLights[i].dir = modelrotation * mLights[i].dir;
	}
	}
	*/
	void ShaderLightManager::Bind(const GLuint programId, const GLuint dirLightsLoc, const GLuint lightsLoc) const
	{
		// bind dir lights uniforms
		if (programId > 0)
		{
			bufferDirLights.Bind(dirLightsLoc);
			bufferLights.Bind(lightsLoc);
		}
	}

	void ShaderLightManager::UnBind() const
	{
	}
};