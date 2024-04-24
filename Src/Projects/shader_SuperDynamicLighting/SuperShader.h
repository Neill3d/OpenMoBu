
#pragma once

// SuperShader.h
/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "glslShader.h"
#include "SuperShader_glsl.h"
//--- SDK include
#include <fbsdk/fbsdk.h>
#include <vector>

#include <memory>

#define kMaxDrawInstancedSize  100

namespace Graphics
{
	typedef std::vector<TLight>			lights_vector;

	//
	// There are two variant to share buffer in shader - SSBO or nVidia pointer
	//

	class CGPUBuffer
	{
	public:

		//! a constructor
		CGPUBuffer();

		// a destructor
		virtual ~CGPUBuffer();

		virtual void Free();

		// size in bytes, data is a pointer to the data struct
		virtual void UpdateData(const size_t elemSize, const size_t count, const void *buffer)
		{}

		virtual void UpdateData(const size_t elemSize, const size_t count1, const void *buffer1,
			const size_t count2, const void *buffer2)
		{
		}

		virtual void Bind(const GLuint unitId) const
		{}
		virtual void UnBind() const
		{}

		const size_t	GetSize() const
		{
			return mBufferSize;
		}
		const size_t	GetCount() const
		{
			return mBufferCount;
		}

		const GLuint GetBufferId() const {
			return mBuffer;
		}

	protected:
		// SSBO for texture addresses
		GLuint							mBuffer; // TODO: SSBO or texture buffer pointer

		size_t			mBufferSize;	// one element size
		size_t			mBufferCount;	// number of elements in the buffer
	};

	////////////////////////////////////////////////////////////////////////////////////////////////
	// ssbo array
	class CGPUBufferSSBO : public CGPUBuffer
	{
	public:

		//! a constructor
		CGPUBufferSSBO();

		// a destructor
		virtual ~CGPUBufferSSBO();

		// size in bytes, data is a pointer to the data struct
		virtual void UpdateData(const size_t elemSize, const size_t count, const void *buffer) override;

		virtual void Bind(const GLuint unitId) const override;
		virtual void UnBind() const override
		{}

	};

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	//

	struct CCameraInfoCache
	{
		//FBCamera *pCamera;
		void				*pUserData;

		//
		int					offsetX;
		int					offsetY;
		int					width;
		int					height;

		//
		double				fov;

		double				farPlane;
		double				nearPlane;
		double				realFarPlane;

		vec4				pos;	// camera eye pos

		mat4				mv4;
		mat4				mvInv4; // mv inverse
		mat4				p4;	// projection matrix
		mat4				proj2d;

		double				mv[16];

		/*
		// pre-loaded data from camera
		FBMatrix			mv;
		FBMatrix			mvInv;
		FBMatrix			p;

		static void Prep(FBCamera *pCamera, CCameraInfoCache &cache);
		*/
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// this is a light version of lights manager !
	

	class CGPUShaderLights
	{

	public:
		//! a constructor
		CGPUShaderLights();
		//! a destructor
		~CGPUShaderLights();

		//void Build (CCameraInfoCache &cameraCache, std::vector<LightDATA> &lights);

		void MapOnGPU();
		void PrepGPUPtr();

		void Bind(const GLuint programId, const GLuint dirLightsLoc, const GLuint lightsLoc) const;
		void UnBind() const;

		const int GetNumberOfDirLights() const
		{
			return (int)mDirLights.size();
		}
		const int GetNumberOfLights() const
		{
			return (int)mLights.size();
		}

		lights_vector		&GetLightsVector()
		{
			return mLights;
		}
		lights_vector		&GetDirLightsVector()
		{
			return mDirLights;
		}

		lights_vector		&GetTransformedLightsVector()
		{
			return mTransformedLights;
		}
		lights_vector		&GetTransformedDirLightsVector()
		{
			return mTransformedDirLights;
		}

		// prepare lights in a view space
		void UpdateTransformedLights(const mat4 &modelview, const mat4 &rotation, const mat4 &scaling);

	protected:
		//
		// lights scene data

		lights_vector					mDirLights;	// dir lights stored in viewSpace !!
		lights_vector					mLights;	// point/spot lights stored in viewSpace !!

		lights_vector					mTransformedDirLights;
		lights_vector					mTransformedLights;

		/// vars for parallel evaluation
		//int								mNumberOfDirLights;
		//int								mNumberOfLights;

		//
		// uniform buffer object (UBO) for lights data

		CGPUBufferSSBO			mBufferLights;
		CGPUBufferSSBO			mBufferDirLights;

	};

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

			GLint		colorId;
			GLint		useDiffuseSampler;

			//
			BufferIdLocations()
			{
				colorId = -1;
				useDiffuseSampler = -1;
			}

		} mBufferIdLoc;

		struct ShadingLocations
		{
			GLint		displacementOption;
			GLint		displacementMatrix;

			GLint		numberOfDirLights;
			GLint		numberOfPointLights;

			GLint		globalAmbientLight;

			GLint		fogColor;
			GLint		fogOptions;

			GLint		switchAlbedoTosRGB;
			GLint		useMatCap;
			GLint		useLightmap;
			GLint		rimOptions;
			GLint		rimColor;

			//
			ShadingLocations()
			{
				displacementOption = -1;
				displacementMatrix = -1;

				numberOfDirLights = -1;
				numberOfPointLights = -1;

				globalAmbientLight = -1;

				fogColor = -1;
				fogOptions = -1;

				rimOptions = -1;
				rimColor = -1;

				switchAlbedoTosRGB = -1;
				useMatCap = -1;
				useLightmap = -1;
			}

		} mShadingLoc;

		double						mAlpha;
		GLSLShader					*mLastBinded;
		GLuint						mLastTexId;
		GLuint						mLastLightmapId;
		FBMaterial					*mLastMaterial;
		FBModel						*mLastModel;

		TTransform					mLastTransform;

		// DONE:: SSBO for transform and global settings, material, lights
		CGPUBufferSSBO				mBufferTransform;
		CGPUBufferSSBO				mBufferMaterial;

		CGPUBufferSSBO				mBufferDirLights;
		CGPUBufferSSBO				mBufferLights;

		std::unique_ptr<GLSLShader>	mShaderBufferId;
		std::unique_ptr<GLSLShader>	mShaderShading;

		CCameraInfoCache			mCameraCache;

		//
		//

		// list of used lights, could be exclusive from current composition lights list
		std::vector<FBLight*>					mUsedSceneLights;
		std::vector<FBLight*>					mUsedInfiniteLights;
		std::vector<FBLight*>					mUsedPointLights;

		std::unique_ptr<CGPUShaderLights>			mGPUSceneLights;

		// last lights for a frame - default if no lights, scene lights if no composition override
		CGPUShaderLights					*mLastLightsBinded;

		//
		bool			mLastUseDisplacement;
		double			mLastDispMult;
		double			mLastDispCenter;
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
		void UploadRimInformation(double useRim, double rimPower, double *rimColor);
		void UploadFogInformation(double *color, bool enable, double begin, double end, double density, FBFogMode mode);

		void SetDisplacementInfo(bool useDisp, double dispMult, double dispCenter);
		void UploadDisplacementInfo(bool useDisp);
		void UploadDisplacementInfo(bool useDisp, double dispMult, double dispCenter, const	double *dispMatrix);

		void SetMatCap(GLuint texId);

		static void ConstructDefaultLight0(bool inEyeSpace, const mat4 &lViewMatrix, const mat4 &lViewRotationMatrix, TLight &light);
		static void ConstructDefaultLight1(bool inEyeSpace, const mat4 &lViewMatrix, const mat4 &lViewRotationMatrix, TLight &light);
		static void ConstructFromFBLight(const bool ToEyeSpace, const mat4 &lViewMatrix,
			const mat4 &lViewRotationMatrix, FBLight *pLight, TLight &light);

		// this is a GPU buffer preparation, not an actual binding
		void			PrepFBSceneLights();
		void			PrepLightsInViewSpace(CGPUShaderLights *pLights);

		void			PrepLightsFromFBList(CGPUShaderLights *pLightsManager, const CCameraInfoCache &cameraCache, std::vector<FBLight*> &lights);

		void		MapLightsOnGPU();

		// when shaderLights nullptr, we will bind all the fbscene lights
		bool		BindLights(const bool resetLastBind, const CGPUShaderLights *pShaderLights = nullptr);

		const int GetNumberOfUsedLights() const {
			return (int)mUsedSceneLights.size();
		}
		std::vector<FBLight*> &GetUsedLightsVector() {
			return mUsedSceneLights;
		}

		// just prepare gpu buffer (no binding)
		bool PrepShaderLights(const bool useSceneLights, FBPropertyListObject *AffectingLights,
			std::vector<FBLight*> &shaderLightsPtr, CGPUShaderLights *shaderLights);

		// TODO: bind a light buffer to the uber shader and update light count

		CGPUShaderLights			*GetGPUSceneLightsPtr() {
			return mGPUSceneLights.get();
		}
	};

};