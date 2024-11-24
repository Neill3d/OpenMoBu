
#pragma once

// SuperShader.h
/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "glslShader.h"
#include "SuperShader_glsl.h"
#include "GPUBuffer.h"
#include "ShaderLightManager.h"
//--- SDK include
#include <fbsdk/fbsdk.h>
#include <vector>

#include <memory>

#define kMaxDrawInstancedSize  100

namespace Graphics
{
	//
	// There are two variant to share buffer in shader - SSBO or nVidia pointer
	//

	// simple shader for render color id
	// phong shading for render shaded and textured models

	//////////////////////////////////////////////////////////////////
	// SuperShader

	class SuperShader
	{
	public:

		//! a constructor
		SuperShader();
		//! a destructor
		~SuperShader();

		// path - where to locate our effect files
		bool Initialize(const char *path);

		bool BeginShading(FBRenderOptions* pRenderOptions, FBArrayTemplate<FBLight*>* pAffectingLightList);
		void EndShading(FBRenderOptions *pRenderOptions=nullptr);

		void SwitchMaterial(FBRenderOptions* pRenderOptions, FBShaderModelInfo* pShaderModelInfo, FBMaterial* pMaterial, double pShaderTransparencyFactor, bool forceUpdate);

		void ShaderPassModelDraw(FBRenderOptions* pRenderOptions, FBRenderingPass pPass, FBShaderModelInfo* pInfo, bool forceUpdateTextures);

		void UploadModelViewMatrixArrayForDrawInstanced(const double* pModelViewMatrixArray, int pCount);

	protected:

		struct BufferIdLocations
		{

			GLint		colorId{ -1 };
			GLint		useDiffuseSampler{ -1 };

		} BufferIdShaderUniformLocations;

		struct ShadingLocations
		{
			GLint		displacementOption{ -1 };
			GLint		displacementMatrix{ -1 };

			GLint		numberOfDirLights{ -1 };
			GLint		numberOfPointLights{ -1 };
			GLint		numberOfShadows{ -1 };

			GLint		globalAmbientLight{ -1 };

			GLint		fogColor{ -1 };
			GLint		fogOptions{ -1 };

			GLint		switchAlbedoTosRGB{ -1 };
			GLint		useMatCap{ -1 };
			GLint		useLightmap{ -1 };
			GLint		rimOptions{ -1 };
			GLint		rimColor{ -1 };

		} PhongShaderUniformLocations;

		double						mAlpha;
		GLSLShader* mLastBinded{ nullptr };
		GLuint						mLastTexId{ 0 };
		GLuint						mLastLightmapId{ 0 };
		FBMaterial* mLastMaterial{ nullptr };
		FBModel* mLastModel{ nullptr };

		TTransform					mLastTransform;

		// DONE:: SSBO for transform and global settings, material, lights
		GPUBufferSSBO				mBufferTransform;
		GPUBufferSSBO				mBufferMaterial;

		GPUBufferSSBO				mBufferDirLights;
		GPUBufferSSBO				mBufferLights;

		std::unique_ptr<GLSLShader>	mShaderBufferId;
		std::unique_ptr<GLSLShader>	mShaderShading;

		CCameraInfoCache			mCameraCache;

		//
		//

		// list of used lights, could be exclusive from current composition lights list
		std::vector<FBLight*>					mUsedSceneLights;
		std::vector<FBLight*>					mUsedInfiniteLights;
		std::vector<FBLight*>					mUsedPointLights;

		std::unique_ptr<ShaderLightManager>			mGPUSceneLights;

		// last lights for a frame - default if no lights, scene lights if no composition override
		ShaderLightManager* mLastLightsBinded{ nullptr };

		//
		bool			mLastUseDisplacement{ false };
		double			mLastDispMult{ 0.0 };
		double			mLastDispCenter{ 0.0 };
		FBMatrix		mLastDispMatrix;

	public:

		bool CCameraInfoCachePrep(FBCamera *pCamera, CCameraInfoCache &cache);

		const CCameraInfoCache &GetCameraCache() const {
			return mCameraCache;
		}

		void UploadSwitchAlbedoTosRGB(bool sRGB);
		void SetLightmap(GLuint texId, double transparency);

		void UploadGlobalAmbient(double *color);
		void UploadLightingInformation(const int numdir, const int numpoint);
		void UploadShadowsInformation(const int numShadows);
		void UploadRimInformation(double useRim, double rimPower, double *rimColor);
		void UploadFogInformation(double *color, bool enable, double begin, double end, double density, FBFogMode mode);

		void SetDisplacementInfo(bool useDisp, double dispMult, double dispCenter);
		void UploadDisplacementInfo(bool useDisp);
		void UploadDisplacementInfo(bool useDisp, double dispMult, double dispCenter, const	double *dispMatrix);

		void SetMatCap(GLuint texId);

		

		// this is a GPU buffer preparation, not an actual binding
		void			PrepFBSceneLights();
		void			PrepLightsInViewSpace(ShaderLightManager* pLights) const;

		void			PrepLightsFromFBList(ShaderLightManager* pLightsManager, const CCameraInfoCache &cameraCache, std::vector<FBLight*> &lights);

		void		MapLightsOnGPU();

		// when shaderLights nullptr, we will bind all the fbscene lights
		bool		BindLights(const bool resetLastBind, const ShaderLightManager *pShaderLights = nullptr);

		const int GetNumberOfUsedLights() const {
			return (int)mUsedSceneLights.size();
		}
		std::vector<FBLight*> &GetUsedLightsVector() {
			return mUsedSceneLights;
		}

		// just prepare gpu buffer (no binding)
		bool PrepShaderLights(const bool useSceneLights, FBPropertyListObject *AffectingLights,
			std::vector<FBLight*> &shaderLightsPtr, ShaderLightManager *shaderLights);

		// TODO: bind a light buffer to the uber shader and update light count

		ShaderLightManager *GetGPUSceneLightsPtr() {
			return mGPUSceneLights.get();
		}

		GLuint GetSamplerSlotShadow() const;

	private:

		static void SetCameraTransform(TTransform& transform, FBRenderOptions* pRenderOptions);
		static void SetTransform(TTransform& transform, FBRenderOptions* pRenderOptions, FBShaderModelInfo* pInfo);
		static void SetMaterial(TMaterial& mat, FBMaterial* pMaterial);


		static GLuint GetTextureId(FBMaterial* pMaterial, const FBMaterialTextureType textureType, bool forceUpdate);
	};

};