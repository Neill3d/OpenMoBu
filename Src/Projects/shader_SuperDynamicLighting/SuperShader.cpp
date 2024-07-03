
// SuperShader.cpp
/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "SuperShader.h"
#include "SuperShader_glsl.h"
#include "CheckGLError.h"

#define SHADER_BUFFERID_VERTEX		"scene_bufferid.vsh"
#define SHADER_BUFFERID_FRAGMENT	"scene_bufferid.fsh"

#define SHADER_SHADING_VERTEX		"scene_shading.vsh"
#define SHADER_SHADING_FRAGMENT		"scene_shading.fsh"

#define SAMPLER_SLOT_DIFFUSE		0
#define SAMPLER_SLOT_TRANSPARENCY	1
#define SAMPLER_SLOT_SPECULAR		2
#define SAMPLER_SLOT_NORMAL			3
#define SAMPLER_SLOT_REFLECT		4
#define SAMPLER_SLOT_DISPLACE		5
#define SAMPLER_SLOT_MATCAP			6
#define SAMPLER_SLOT_DETAIL			7	// possible with a second UV for lightmaps

namespace Graphics {


	//////////////////////////////////////////////////////////////////////////////////////////////////
	
	// return false if transformation matrix is reflected (rendering reflection pass)

	void SetCameraTransform(TTransform &transform, FBRenderOptions* pRenderOptions)
	{
		using namespace nv;
		FBCamera *pCamera = pRenderOptions->GetRenderingCamera();

		FBVector3d	eyePos;
		FBMatrix lCameraMVMatrix, lCameraVPMatrix, lWorldMatrix;
		if (nullptr != pCamera)
		{
			pCamera->GetVector(eyePos);
			pCamera->GetCameraMatrix(lCameraMVMatrix, kFBModelView);
			pCamera->GetCameraMatrix(lCameraVPMatrix, kFBProjection);
		}
		lWorldMatrix.Identity();

		transform.eyePos = vec4((float)eyePos[0], (float)eyePos[1], (float)eyePos[2], 1.0f);

		FBSVector scl;

		FBMatrixToScaling(scl, lCameraMVMatrix);
		if (scl[1] < 0.0)
		{
			transform.eyePos[3] = -1.0f;
		}

		for (int i = 0; i < 16; ++i)
		{
			transform.m4_World.mat_array[i] = (float)lWorldMatrix[i];
			transform.m4_View.mat_array[i] = (float)lCameraMVMatrix[i];
			transform.m4_Proj.mat_array[i] = (float)lCameraVPMatrix[i];
		}
	}

	void SetTransform(TTransform &transform, FBRenderOptions* pRenderOptions, FBShaderModelInfo* pInfo)
	{
		using namespace nv;
		FBMatrix lModelMatrix;
		if (pInfo->GetFBModel())
			pInfo->GetFBModel()->GetMatrix(lModelMatrix, kModelTransformation_Geometry);

		for (int i = 0; i < 16; ++i)
		{
			transform.m4_Model.mat_array[i] = (float)lModelMatrix[i];
		}
		// DONE: assign a normal matrix !
		const mat4 tm = transform.m4_View * transform.m4_Model;
		invert(transform.normalMatrix, tm);
		transpose(transform.normalMatrix);
	}

	void SetMaterial(TMaterial &mat, FBMaterial *pMaterial)
	{
		if (nullptr == pMaterial)
			return;

		FBColor lEmissiveColor = pMaterial->Emissive;
		FBColor lDiffuseColor = pMaterial->Diffuse;
		FBColor lAmbientColor = pMaterial->Ambient;
		FBColor lSpecularColor = pMaterial->Specular;
		FBColor lReflectionColor = pMaterial->Reflection;
		double shin = pMaterial->Shininess;

		double lEmissiveFactor = pMaterial->EmissiveFactor;
		double lDiffuseFactor = pMaterial->DiffuseFactor;
		double lAmbientFactor = pMaterial->AmbientFactor;
		double lSpecularFactor = pMaterial->SpecularFactor;
		double lReflectionFactor = pMaterial->ReflectionFactor;
		double lTransparencyFactor = pMaterial->TransparencyFactor;
		double lDisplacementFactor = pMaterial->DisplacementFactor;
		double lBumpFactor = pMaterial->BumpFactor;

		for (int i = 0; i < 3; ++i)
		{
			mat.emissiveColor.vec_array[i] = (float)lEmissiveColor[i];
			mat.diffuseColor.vec_array[i] = (float)lDiffuseColor[i];
			mat.ambientColor.vec_array[i] = (float)lAmbientColor[i];
			mat.specularColor.vec_array[i] = (float)lSpecularColor[i];
			mat.reflectColor.vec_array[i] = (float)lReflectionColor[i];
		}

		mat.specExp = shin;

		mat.emissiveColor[3] = (float)lEmissiveFactor;
		mat.diffuseColor[3] = (float)lDiffuseFactor;
		mat.ambientColor[3] = (float)lAmbientFactor;
		mat.specularColor[3] = (float)lSpecularFactor;
		mat.reflectColor[3] = (float)lReflectionFactor;
		mat.transparencyColor[3] = (float)lTransparencyFactor;

		mat.useDiffuse = 0.0f;
		mat.useTransparency = 0.0f;
		mat.useSpecular = 0.0f;
		mat.useNormalmap = 0.0f;
		mat.useDisplacement = 0.0f;
		mat.useReflect = 0.0f;

		if (nullptr != pMaterial->GetTexture() && lDiffuseFactor > 0.0)
		{
			mat.useDiffuse = 1.0f;

			const double *tm = pMaterial->GetTexture()->GetMatrix();
			for (int i = 0; i < 16; ++i)
				mat.diffuseTransform.mat_array[i] = (float)tm[i];
		}
		if (nullptr != pMaterial->GetTexture(kFBMaterialTextureTransparent) && lTransparencyFactor > 0.0)
		{
			mat.useTransparency = 1.0f;

			const double *tm = pMaterial->GetTexture(kFBMaterialTextureTransparent)->GetMatrix();
			for (int i = 0; i < 16; ++i)
				mat.transparencyTransform.mat_array[i] = (float)tm[i];
		}
		if (nullptr != pMaterial->GetTexture(kFBMaterialTextureDisplacementColor))
		{
			mat.useDisplacement = (float) lDisplacementFactor;
			/*
			const double *tm = pMaterial->GetTexture(kFBMaterialTextureDisplacementColor)->GetMatrix();
			for (int i = 0; i < 16; ++i)
				mat.displaceTransform.mat_array[i] = (float)tm[i];
				*/
		}
		if (nullptr != pMaterial->GetTexture(kFBMaterialTextureSpecular) && lSpecularFactor > 0.0)
		{
			mat.useSpecular = 1.0f;

			const double *tm = pMaterial->GetTexture(kFBMaterialTextureSpecular)->GetMatrix();
			for (int i = 0; i < 16; ++i)
				mat.specularTransform.mat_array[i] = (float)tm[i];
		}
		if (nullptr != pMaterial->GetTexture(kFBMaterialTextureReflection) && lReflectionFactor > 0.0)
		{
			mat.useReflect = 1.0f;

			const double *tm = pMaterial->GetTexture(kFBMaterialTextureReflection)->GetMatrix();
			for (int i = 0; i < 16; ++i)
				mat.reflectTransform.mat_array[i] = (float)tm[i];
		}
		if (nullptr != pMaterial->GetTexture(kFBMaterialTextureNormalMap) && lBumpFactor > 0.0)
		{
			mat.useNormalmap = 1.0f * (float) lBumpFactor;

			const double *tm = pMaterial->GetTexture(kFBMaterialTextureNormalMap)->GetMatrix();
			for (int i = 0; i < 16; ++i)
				mat.normalTransform.mat_array[i] = (float)tm[i];
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////

	CGPUBuffer::CGPUBuffer()
	{
		mBuffer = 0;
		mBufferSize = 0;
		mBufferCount = 0;
	}

	void CGPUBuffer::Free()
	{
		if (mBuffer > 0)
		{
			glDeleteBuffers(1, &mBuffer);
			mBuffer = 0;
		}
	}

	CGPUBuffer::~CGPUBuffer()
	{
		Free();
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// SSBO

	CGPUBufferSSBO::CGPUBufferSSBO()
		: CGPUBuffer()
	{
	}

	CGPUBufferSSBO::~CGPUBufferSSBO()
	{
		Free();
	}

	void CGPUBufferSSBO::UpdateData(const size_t _size, const size_t _count, const void *data)
	{

		if (mBuffer == 0)
		{
			glGenBuffers(1, &mBuffer);
			CHECK_GL_ERROR();
		}

		// update data in SSBO
		if (mBuffer > 0 && _size > 0 && _count > 0)
		{
			GLsizeiptr size = (GLsizeiptr)(_size * _count);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, mBuffer);
			glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_STATIC_DRAW);
			glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_STATIC_DRAW);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			CHECK_GL_ERROR();
		}

		mBufferSize = _size;
		mBufferCount = _count;
	}

	void CGPUBufferSSBO::Bind(const GLuint unitId) const
	{
		if (mBuffer > 0)
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, unitId, mBuffer);
		}
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// FBShaderLights

	CGPUShaderLights::CGPUShaderLights()
	{
		//mNumberOfLights = 0;
		//mNumberOfDirLights = 0;
	}

	CGPUShaderLights::~CGPUShaderLights()
	{
	}

	void CGPUShaderLights::UpdateTransformedLights(const nv::mat4 &modelview, const nv::mat4 &rotation, const nv::mat4 &scaling)
	{
		mTransformedLights.resize(mLights.size());
		mTransformedDirLights.resize(mDirLights.size());

		auto srcLight = begin(mLights);
		auto dstLight = begin(mTransformedLights);

		for (; srcLight != end(mLights); ++srcLight, ++dstLight)
		{
			dstLight->position = modelview * srcLight->position;
			dstLight->dir = rotation * srcLight->dir;

			dstLight->type = srcLight->type;
			dstLight->spotAngle = srcLight->spotAngle;

			dstLight->color = srcLight->color;
			dstLight->radius = srcLight->radius;

			dstLight->attenuations = srcLight->attenuations;
		}

		auto srcDirLight = begin(mDirLights);
		auto dstDirLight = begin(mTransformedDirLights);

		for (; srcDirLight != end(mDirLights); ++srcDirLight, ++dstDirLight)
		{
			dstDirLight->position = srcDirLight->position;
			dstDirLight->dir = rotation * srcDirLight->dir;

			dstDirLight->type = srcDirLight->type;
			dstDirLight->spotAngle = srcDirLight->spotAngle;

			dstDirLight->color = srcDirLight->color;
			dstDirLight->radius = srcDirLight->radius;

			dstDirLight->attenuations = srcDirLight->attenuations;
		}
	}

	void CGPUShaderLights::MapOnGPU()
	{
		// dir lights
		mBufferDirLights.UpdateData(sizeof(TLight), mTransformedDirLights.size(), mTransformedDirLights.data());

		// point / spot lights
		mBufferLights.UpdateData(sizeof(TLight), mTransformedLights.size(), mTransformedLights.data());

		//
		//mNumberOfDirLights = (int) mTransformedDirLights.size();
		//mNumberOfLights = (int) mTransformedLights.size();
	}



	void CGPUShaderLights::PrepGPUPtr()
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
	void CGPUShaderLights::Build(CCameraInfoCache &cameraCache, std::vector<FBLight*> &lights)
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
	void CGPUShaderLights::Bind(const GLuint programId, const GLuint dirLightsLoc, const GLuint lightsLoc) const
	{
		// bind dir lights uniforms
		if (programId > 0)
		{
			mBufferDirLights.Bind(dirLightsLoc);
			mBufferLights.Bind(lightsLoc);
		}
	}

	void CGPUShaderLights::UnBind() const
	{
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	//

	SuperShader::SuperShader()
	{
		mAlpha = 1.0;
		mLastBinded = nullptr;
		mLastMaterial = nullptr;
		mLastModel = nullptr;
		mLastLightsBinded = nullptr;
		mGPUSceneLights.reset(new CGPUShaderLights());
	}

	SuperShader::~SuperShader()
	{

	}

	bool SuperShader::Initialize(const char *path)
	{
		bool lSuccess = true;

		GLSLShader *pNewShader = nullptr;

		try
		{
			//
			// release if something already assigned

			if (nullptr != mShaderBufferId.get())
				mShaderBufferId.reset(nullptr);

			if (nullptr != mShaderShading.get())
				mShaderShading.reset(nullptr);


			//
			// BufferId Shader

			pNewShader = new GLSLShader();

			if (nullptr == pNewShader)
				throw std::exception("failed to allocate memory for a shader");

			FBString vertex_path(path, SHADER_BUFFERID_VERTEX);
			FBString fragment_path(path, SHADER_BUFFERID_FRAGMENT);

			if (false == pNewShader->LoadShaders(vertex_path, fragment_path))
				throw std::exception("failed to load and prepare a shader");
			
			// samplers and locations
			pNewShader->Bind();

			GLint loc = pNewShader->findLocation("sampler0");
			if (loc >= 0)
				glUniform1i(loc, 0);

			mBufferIdLoc.colorId = pNewShader->findLocation("ColorId");
			mBufferIdLoc.useDiffuseSampler = pNewShader->findLocation("UseDiffuseSampler");

			pNewShader->UnBind();

			mShaderBufferId.reset(pNewShader);

			//
			// Phong Shading Shader

			pNewShader = new GLSLShader();

			if (nullptr == pNewShader)
				throw std::exception("failed to allocate memory for a shader");

			vertex_path = FBString(path, SHADER_SHADING_VERTEX);
			fragment_path = FBString(path, SHADER_SHADING_FRAGMENT);

			if (false == pNewShader->LoadShaders(vertex_path, fragment_path))
				throw std::exception("failed to load and prepare a shader");
			
			// samplers and locations
			pNewShader->Bind();

			loc = pNewShader->findLocation("samplerDiffuse");
			if (loc >= 0)
				glUniform1i(loc, SAMPLER_SLOT_DIFFUSE);
			loc = pNewShader->findLocation("samplerDisplacement");
			if (loc >= 0)
				glUniform1i(loc, SAMPLER_SLOT_DISPLACE);
			loc = pNewShader->findLocation("samplerReflect");
			if (loc >= 0)
				glUniform1i(loc, SAMPLER_SLOT_REFLECT);
			loc = pNewShader->findLocation("samplerTransparency");
			if (loc >= 0)
				glUniform1i(loc, SAMPLER_SLOT_TRANSPARENCY);
			loc = pNewShader->findLocation("samplerSpecular");
			if (loc >= 0)
				glUniform1i(loc, SAMPLER_SLOT_SPECULAR);
			loc = pNewShader->findLocation("samplerNormal");
			if (loc >= 0)
				glUniform1i(loc, SAMPLER_SLOT_NORMAL);
			loc = pNewShader->findLocation("samplerMatCap");
			if (loc >= 0)
				glUniform1i(loc, SAMPLER_SLOT_MATCAP);
			loc = pNewShader->findLocation("samplerDetail");
			if (loc >= 0)
				glUniform1i(loc, SAMPLER_SLOT_DETAIL);

			mShadingLoc.displacementOption = pNewShader->findLocation("displacementOption");
			mShadingLoc.displacementMatrix = pNewShader->findLocation("displacementMatrix");

			mShadingLoc.numberOfDirLights = pNewShader->findLocation("numberOfDirLights");
			mShadingLoc.numberOfPointLights = pNewShader->findLocation("numberOfPointLights");

			mShadingLoc.globalAmbientLight = pNewShader->findLocation("globalAmbientLight");

			mShadingLoc.fogColor = pNewShader->findLocation("fogColor");
			mShadingLoc.fogOptions = pNewShader->findLocation("fogOptions");

			mShadingLoc.rimOptions = pNewShader->findLocation("rimOptions");
			mShadingLoc.rimColor = pNewShader->findLocation("rimColor");

			mShadingLoc.switchAlbedoTosRGB = pNewShader->findLocation("switchAlbedoTosRGB");
			mShadingLoc.useMatCap = pNewShader->findLocation("useMatCap");
			mShadingLoc.useLightmap = pNewShader->findLocation("useLightmap");

			pNewShader->UnBind();

			mShaderShading.reset(pNewShader);

		}
		catch (const std::exception &e)
		{
			FBTrace("Failed to initialize a shader %s\n", e.what());

			if (nullptr != pNewShader)
			{
				delete pNewShader;
				pNewShader = nullptr;

				lSuccess = false;
			}
		}

		return lSuccess;
	}

	bool SuperShader::BeginShading(FBRenderOptions* pRenderOptions, FBArrayTemplate<FBLight*>* pAffectingLightList)
	{
		mLastBinded = nullptr;
		mLastLightmapId = 0;

		if (false == pRenderOptions->IsIDBufferRendering())
		{
			// skip rendering during reflection pass

			if (false == CCameraInfoCachePrep(pRenderOptions->GetRenderingCamera(), mCameraCache) )
			{
			//	return false;
			}

			if (nullptr != mShaderShading.get())
			{
				mShaderShading->Bind();
				mLastBinded = mShaderShading.get();

				glEnableVertexAttribArray(0);		// position
				glEnableVertexAttribArray(1);		// uv
				glEnableVertexAttribArray(2);		// normal

				SetCameraTransform(mLastTransform, pRenderOptions);
				
				//
				PrepFBSceneLights();
				
				// TODO: bind a default scene light list
				PrepLightsInViewSpace(mGPUSceneLights.get());

				MapLightsOnGPU();

				BindLights(true);

				FBColor ambientColor = FBGlobalLight::TheOne().AmbientColor;
				UploadGlobalAmbient(ambientColor);

				FBColor fogColor = FBGlobalLight::TheOne().FogColor;
				bool fogEnable = FBGlobalLight::TheOne().FogEnable;
				double fogBegin = FBGlobalLight::TheOne().FogBegin;
				double fogEnd = FBGlobalLight::TheOne().FogEnd;
				double fogDensity = FBGlobalLight::TheOne().FogDensity;
				FBFogMode mode = (FBFogMode) FBGlobalLight::TheOne().FogMode.AsInt();

				UploadFogInformation(fogColor, fogEnable, fogBegin, fogEnd, fogDensity, mode);

				SetDisplacementInfo(false, 100.0, 50.0);
				SetLightmap(0, 0.0);
			}
		}
		else
		{
			if (nullptr != mShaderBufferId.get())
			{
				mShaderBufferId->Bind();
				mLastBinded = mShaderBufferId.get();		
			}
		}

		mAlpha = 1.0;
		mLastTexId = 0;
		mLastMaterial = nullptr;
		mLastModel = nullptr;

		return true;
	}

	void SuperShader::EndShading(FBRenderOptions *pRenderOptions)
	{
		if (nullptr != mLastBinded)
		{
			mLastBinded->UnBind();
			mLastBinded = nullptr;

			glDisableVertexAttribArray(0);		// position
			glDisableVertexAttribArray(1);		// uv
			glDisableVertexAttribArray(2);		// normal
			glDisableVertexAttribArray(3);		// tangent

			for (int i = 5; i >= 0; --i)
			{
				glActiveTexture(GL_TEXTURE0+i);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
		}

		if (0 != mLastTexId)
		{
			glBindTexture(GL_TEXTURE_2D, 0);
			mLastTexId = 0;
		}
	}

	
	GLuint GetTextureId(FBMaterial *pMaterial, const FBMaterialTextureType textureType, bool forceUpdate)
	{
		GLuint texId = 0;
		bool lForceUpdate = forceUpdate;

		if (FBTexture *pTexture = pMaterial->GetTexture(textureType))
		{
			FBVideo* pVideo = pTexture->Video;
			if (FBIS(pVideo, FBVideoClipImage))
			{
				FBVideoClipImage* clipImage = (FBVideoClipImage*)pVideo;
				if (true == clipImage->ImageSequence)
				{
					lForceUpdate = true;
				}
			}

			texId = pTexture->TextureOGLId;
			if (0 == texId || true == lForceUpdate)
			{
				pTexture->OGLInit();
				texId = pTexture->TextureOGLId;
			}
		}

		return texId;
	}

	void SuperShader::SwitchMaterial(FBRenderOptions* pRenderOptions, FBShaderModelInfo* pShaderModelInfo, FBMaterial* pMaterial, double pShaderTransparencyFactor, bool forceUpdate)
	{
		GLuint ambId = 0;
		GLuint difId = 0;
		GLuint transId = 0;
		GLuint specId = 0;
		GLuint reflId = 0;
		GLuint dispId = 0;
		GLuint normId = 0;

		if (pMaterial)
		{
			mAlpha = (1.0 - pMaterial->TransparencyFactor) * pShaderTransparencyFactor;

			if (pShaderModelInfo->GetOriginalTextureFlag())
			{
				//ambId = GetTextureId(pMaterial, kFBMaterialTextureAmbient);
				difId = GetTextureId(pMaterial, kFBMaterialTextureDiffuse, forceUpdate);
				transId = GetTextureId(pMaterial, kFBMaterialTextureTransparent, forceUpdate);
				specId = GetTextureId(pMaterial, kFBMaterialTextureSpecular, forceUpdate);
				reflId = GetTextureId(pMaterial, kFBMaterialTextureReflection, forceUpdate);
				dispId = GetTextureId(pMaterial, kFBMaterialTextureDisplacementColor, forceUpdate);
				normId = GetTextureId(pMaterial, kFBMaterialTextureNormalMap, forceUpdate);
			}
		}


		// DONE: bind a texture matrix !
		if (nullptr != mLastBinded)
		{
			
			if (false == pRenderOptions->IsIDBufferRendering())
			{
				TMaterial lMaterial;
				SetMaterial(lMaterial, pMaterial);
				lMaterial.shaderTransparency = (float)pShaderTransparencyFactor;

				// DONE: prepare SSBO
				mBufferMaterial.UpdateData(sizeof(TMaterial), 1, &lMaterial);
				mBufferMaterial.Bind(1);

				if (dispId > 0 && lMaterial.useDisplacement > 0.0f)
				{
					glActiveTexture(GL_TEXTURE0 + SAMPLER_SLOT_DISPLACE);
					glBindTexture(GL_TEXTURE_2D, dispId);

					FBColor dispColor = pMaterial->DisplacementColor;
					const double *dispMatrix = pMaterial->GetTexture(kFBMaterialTextureDisplacementColor)->GetMatrix();

					UploadDisplacementInfo(true, (double)lMaterial.useDisplacement, dispColor[0], dispMatrix);
				}
				else
				{
					//!
					UploadDisplacementInfo(false);
				}

				if (specId > 0 && lMaterial.useSpecular > 0.0f)
				{
					glActiveTexture(GL_TEXTURE0 + SAMPLER_SLOT_SPECULAR);
					glBindTexture(GL_TEXTURE_2D, specId);
				}
				if (transId > 0 && lMaterial.useTransparency > 0.0f)
				{
					glActiveTexture(GL_TEXTURE0 + SAMPLER_SLOT_TRANSPARENCY);
					glBindTexture(GL_TEXTURE_2D, transId);
				}
				if (normId > 0 && lMaterial.useNormalmap > 0.0f)
				{
					glActiveTexture(GL_TEXTURE0 + SAMPLER_SLOT_NORMAL);
					glBindTexture(GL_TEXTURE_2D, normId);

					glEnableVertexAttribArray(3);
				}
				else
				{
					glDisableVertexAttribArray(3);
				}
				if (reflId > 0 && lMaterial.useReflect > 0.0f)
				{
					glActiveTexture(GL_TEXTURE0 + SAMPLER_SLOT_REFLECT);
					glBindTexture(GL_TEXTURE_2D, reflId);
				}

				if (ambId > 0)
				{
					glActiveTexture(GL_TEXTURE0 + SAMPLER_SLOT_DETAIL);
					glBindTexture(GL_TEXTURE_2D, ambId);
				}

				glActiveTexture(GL_TEXTURE0);

			}
			else
			{
				if (mBufferIdLoc.useDiffuseSampler >= 0)
				{
					glUniform1f(mBufferIdLoc.useDiffuseSampler, (difId > 0));
				}
			}
		}
		

		if (difId != mLastTexId)
		{
			
			glBindTexture(GL_TEXTURE_2D, difId);
			mLastTexId = difId;
		}
		
		//SetTexture(mParamColorTextureValidVS, mParamColorTextureValidPS, mParamColorTexture, mParamColorTextureMatrix, lDiffuseTexture->GetTextureObject(), lDiffuseTexture->GetMatrix());
	}


	void SuperShader::ShaderPassModelDraw(FBRenderOptions* pRenderOptions, FBRenderingPass pPass, FBShaderModelInfo* pInfo, bool forceUpdateTextures)
	{
		CHECK_GL_ERROR();

		FBModel *pModel = pInfo->GetFBModel();
		GLuint lightmapId = 0;
		double transparency = 0.0;

		if (nullptr != mLastBinded)
		{
			if (false == pRenderOptions->IsIDBufferRendering())
			{
				// DONE: prepare SSBO

				SetTransform(mLastTransform, pRenderOptions, pInfo);

				mBufferTransform.UpdateData(sizeof(TTransform), 1, &mLastTransform);
				mBufferTransform.Bind(0);


				for (int i = 0, count = pModel->Textures.GetCount(); i < count; ++i)
				{
					FBTexture *pTexture = pModel->Textures[i];
					FBTextureUseType useType = (FBTextureUseType) pTexture->UseType.AsInt();

					// BUG: kFBTextureUseSphericalReflexionMap means use type lightmap !
					if (kFBTextureUseSphericalReflexionMap == useType)
					{
						lightmapId = pTexture->TextureOGLId;
						if (0 == lightmapId || true == forceUpdateTextures)
						{
							pTexture->OGLInit();
							lightmapId = pTexture->TextureOGLId;
						}
						transparency = pTexture->Alpha;
					}
				}

				SetLightmap(lightmapId, transparency);
			}
			else
			{
				FBColor lColorId = pModel->UniqueColorId;

				if (mBufferIdLoc.colorId >= 0)
				{
					glUniform4f(mBufferIdLoc.colorId, (float)lColorId[0], (float)lColorId[1], (float)lColorId[2], (float)mAlpha);
				}
			}
		}
	}

	void SuperShader::SetMatCap(GLuint texId)
	{
		if (mShadingLoc.useMatCap >= 0)
		{
			if (texId > 0)
			{
				glActiveTexture(GL_TEXTURE0 + SAMPLER_SLOT_MATCAP);
				glBindTexture(GL_TEXTURE_2D, texId);
				glActiveTexture(GL_TEXTURE0);

				glUniform1f(mShadingLoc.useMatCap, 1.0f);
			}
			else
			{
				glUniform1f(mShadingLoc.useMatCap, 0.0f);
			}
		}
	}

	void SuperShader::SetLightmap(GLuint texId, double transparency)
	{
		if (mShadingLoc.useLightmap >= 0)
		{
			if (texId > 0 && transparency > 0.0)
			{
				if (texId != mLastLightmapId)
				{
					glActiveTexture(GL_TEXTURE0 + SAMPLER_SLOT_DETAIL);
					glBindTexture(GL_TEXTURE_2D, texId);
					glActiveTexture(GL_TEXTURE0);

					glUniform1f(mShadingLoc.useLightmap, 1.0f * (float)transparency);

					mLastLightmapId = texId;
				}
			}
			else
			{
				glUniform1f(mShadingLoc.useLightmap, 0.0f);
			}
		}
	}

	void SuperShader::UploadModelViewMatrixArrayForDrawInstanced(const double* pModelViewMatrixArray, int pCount)
	{
		//cgSetBufferSubData(mParamModelViewArrayBuffer, mParamModelViewArrayBufferOffset, 4 * 4 * sizeof (double) * pCount, pModelViewMatrixArray);
	}



	bool SuperShader::CCameraInfoCachePrep(FBCamera *pCamera, CCameraInfoCache &cache)
	{
		using namespace nv;
		if (nullptr == pCamera)
			return false;

		FBMatrix mv, p, mvInv;

		pCamera->GetCameraMatrix(mv, kFBModelView);
		pCamera->GetCameraMatrix(p, kFBProjection);

		
		FBMatrixInverse(mvInv, mv);

		for (int i = 0; i<16; ++i)
		{
			cache.mv4.mat_array[i] = (nv_scalar)mv[i];
			cache.mvInv4.mat_array[i] = (nv_scalar)mvInv[i];
			cache.p4.mat_array[i] = (nv_scalar)p[i];

			cache.mv[i] = mv[i];
		}

		FBVector3d v;
		pCamera->GetVector(v);
		for (int i = 0; i<3; ++i)
			cache.pos[i] = (float)v[i];
		//cache.pos = vec4( &cache.mv4.x );

		cache.fov = pCamera->FieldOfView;
		cache.width = pCamera->CameraViewportWidth;
		cache.height = pCamera->CameraViewportHeight;
		cache.nearPlane = pCamera->NearPlaneDistance;
		cache.farPlane = pCamera->FarPlaneDistance;

		return true;
	}

	void SuperShader::UploadRimInformation(double useRim, double rimPower, double *rimColor)
	{
		if (mShadingLoc.rimOptions >= 0)
		{
			glUniform4f(mShadingLoc.rimOptions, useRim, rimPower, 0.0f, 0.0f);
		}
		if (mShadingLoc.rimColor >= 0)
		{
			glUniform4f(mShadingLoc.rimColor, rimColor[0], rimColor[1], rimColor[2], 1.0f);
		}
	}

	void SuperShader::UploadFogInformation(double *color, bool enable, double begin, double end, double density, FBFogMode mode)
	{
		if (mShadingLoc.fogColor >= 0)
		{
			glUniform4f(mShadingLoc.fogColor, (float)color[0], (float)color[1], (float)color[2], (enable)?1.0 : 0.0);
		}
		if (mShadingLoc.fogOptions >= 0)
		{
			glUniform4f(mShadingLoc.fogOptions, (float)begin, (float)end, (float)density, (float)mode);
		}
	}

	void SuperShader::SetDisplacementInfo(bool useDisp, double dispMult, double dispCenter)
	{
		if (mShadingLoc.displacementOption >= 0)
		{
			if (false == useDisp)
			{
				glUniform4f(mShadingLoc.displacementOption, 0.0f, 0.0f, 0.0f, 0.0f);
			}
			else
			{
				glUniform4f(mShadingLoc.displacementOption, (float)dispMult, (float)dispCenter, 0.0f, 0.0f);
			}
		}

		mLastUseDisplacement = useDisp;
		mLastDispCenter = dispCenter;
		mLastDispMult = dispMult;
	}

	void SuperShader::UploadDisplacementInfo(bool useDisp)
	{
		if (useDisp != mLastUseDisplacement)
		{
			UploadDisplacementInfo(useDisp, mLastDispMult, mLastDispCenter, mLastDispMatrix);
		}
	}

	void SuperShader::UploadDisplacementInfo(bool useDisp, double dispMult, double dispCenter, const double *dispMatrix)
	{
		if (useDisp != mLastUseDisplacement || dispMult != mLastDispMult || dispCenter != mLastDispCenter)
		{
			if (mShadingLoc.displacementOption >= 0)
			{
				if (false == useDisp)
				{
					glUniform4f(mShadingLoc.displacementOption, 0.0f, 0.0f, 0.0f, 0.0f);
				}
				else
				{
					glUniform4f(mShadingLoc.displacementOption, (float)dispMult, (float)dispCenter, 0.0f, 0.0f);

					float m[16];
					for (int i = 0; i < 16; ++i)
						m[i] = (float)dispMatrix[i];

					glUniformMatrix4fv(mShadingLoc.displacementMatrix, 1, GL_FALSE, m);
				}
			}

			mLastUseDisplacement = useDisp;
			mLastDispCenter = dispCenter;
			mLastDispMult = dispMult;
			mLastDispMatrix = dispMatrix;
		}
	}

};