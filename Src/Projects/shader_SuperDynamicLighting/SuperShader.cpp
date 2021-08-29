
// SuperShader.cpp
/*
Sergei <Neill3d> Solokhin 2018-2021

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "SuperShader.h"
#include "SuperShader_glsl.h"
#include "CheckGLError.h"

#include "SuperDynamicLighting_shader.h"

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
		FBCamera *camera = pRenderOptions->GetRenderingCamera();

		if (FBIS(camera, FBCameraSwitcher))
			camera = ((FBCameraSwitcher*)camera)->CurrentCamera;

		FBVector3d	eyePos(0.0, 0.0, 0.0);
		FBMatrix lCameraMVMatrix, lCameraVPMatrix, lWorldMatrix;

		lCameraMVMatrix.Identity();
		lCameraVPMatrix.Identity();

		if (camera != nullptr)
		{
			camera->GetVector(eyePos);
			camera->GetCameraMatrix(lCameraMVMatrix, kFBModelView);
			camera->GetCameraMatrix(lCameraVPMatrix, kFBProjection);
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

	void CGPUShaderLights::UpdateTransformedLights(const mat4 &modelview, const mat4 &rotation, const mat4 &scaling)
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
		: m_ShadowFrameBuffer(1, 1, 0, 0)
	{
		mAlpha = 1.0;
		mLastBinded = nullptr;
		mLastMaterial = nullptr;
		mLastModel = nullptr;
		mLastLightsBinded = nullptr;
		m_GPUSceneLights.reset(new CGPUShaderLights());
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
		m_Initialized = true;
		return lSuccess;
	}

	void SuperShader::RegisterShaderObject(FBShader* shader)
	{
		m_ShaderInstances.push_back(shader);
	}

	void SuperShader::UnRegisterShaderObject(FBShader* shader)
	{
		auto iter = std::find(begin(m_ShaderInstances), end(m_ShaderInstances), shader);
		if (iter != end(m_ShaderInstances))
		{
			m_ShaderInstances.erase(iter);
		}
	}

	bool SuperShader::BeginShading(FBRenderOptions* pRenderOptions, FBArrayTemplate<FBLight*>* pAffectingLightList)
	{
		mLastBinded = nullptr;
		mLastLightmapId = 0;

		if (false == pRenderOptions->IsIDBufferRendering())
		{
			// skip rendering during reflection pass

			CCameraInfoCachePrep(pRenderOptions->GetRenderingCamera(), mCameraCache);

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
				PrepLightsInViewSpace(m_GPUSceneLights.get());

				MapLightsOnGPU();

				BindLights(true);

				FBColor ambientColor = FBGlobalLight::TheOne().AmbientColor;
				UploadGlobalAmbient(ambientColor);

				FBColor fogColor = FBGlobalLight::TheOne().FogColor;
				bool fogEnable = FBGlobalLight::TheOne().FogEnable;
				double fogBegin = FBGlobalLight::TheOne().FogBegin;
				double fogEnd = FBGlobalLight::TheOne().FogEnd;
				double fogDensity = FBGlobalLight::TheOne().FogDensity;
				FBFogMode mode = FBGlobalLight::TheOne().FogMode;

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
					FBTextureUseType useType = pTexture->UseType;

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



	bool SuperShader::CCameraInfoCachePrep(FBCamera *cameraModel, CCameraInfoCache &cache)
	{
		FBCamera* camera = FBIS(cameraModel, FBCameraSwitcher) ? ((FBCameraSwitcher*)cameraModel)->CurrentCamera : cameraModel;

		if (camera == nullptr)
			return false;

		FBMatrix mv, p, mvInv;

		camera->GetCameraMatrix(mv, kFBModelView);
		camera->GetCameraMatrix(p, kFBProjection);

		
		FBMatrixInverse(mvInv, mv);

		for (int i = 0; i<16; ++i)
		{
			cache.mv4.mat_array[i] = (nv_scalar)mv[i];
			cache.mvInv4.mat_array[i] = (nv_scalar)mvInv[i];
			cache.p4.mat_array[i] = (nv_scalar)p[i];

			cache.mv[i] = mv[i];
		}

		FBVector3d v;
		camera->GetVector(v);
		for (int i = 0; i<3; ++i)
			cache.pos[i] = (float)v[i];
		
		cache.fov = camera->FieldOfView;
		cache.width = camera->CameraViewportWidth;
		cache.height = camera->CameraViewportHeight;
		cache.nearPlane = camera->NearPlaneDistance;
		cache.farPlane = camera->FarPlaneDistance;

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

	void SuperShader::SaveFrameBuffer()
	{
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &mLastFramebuffer);
		if (mLastFramebuffer > 0)
		{
			glGetIntegerv(GL_MAX_DRAW_BUFFERS, &mMaxDrawBuffers);
			GLint lActualUse = 0;
			for (GLint i = 0; i < mMaxDrawBuffers && i < MAX_DRAW_BUFFERS; i++)
			{
				glGetIntegerv(GL_DRAW_BUFFER0 + i, (GLint *)&(mDrawBuffers[i]));
				if (mDrawBuffers[i] == GL_NONE)
				{
					break;
				}
				lActualUse++;
			}
			mMaxDrawBuffers = lActualUse;
		}
		else
		{
			mLastFramebuffer = 0;
		}
	}

	void SuperShader::RestoreFrameBuffer()
	{
		if (mLastFramebuffer > 0)
		{
			glBindFramebuffer(GL_FRAMEBUFFER_EXT, mLastFramebuffer);
			if (mMaxDrawBuffers > 0)
			{
				glDrawBuffers(mMaxDrawBuffers, &(mDrawBuffers[0]));
			}
		}
	}

	GLuint CreateDepthTexture(const int size, const bool use_hardware_pcf)
	{
		GLuint id = 0;

		// Create offscreen textures and FBOs for offscreen shadow map rendering.
		glGenTextures(1, &id);

		// Specify texture parameters and attach each texture to an FBO.
		glBindTexture(GL_TEXTURE_2D, id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, size, size, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

		if (use_hardware_pcf)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}

		return id;
	}

	void SuperShader::PrepareShadows()
	{
		// compute all variations with
		//	light, casters, texture size
		//	allocate memory
		//  render

		// all casted lights - m_CastShadowLights, m_CastShadowLightsData
		// all shader instances in the scene - m_ShaderInstances (could have unique lights/custers/size list)

		// compute final amount of combination with unique lights/custers/size

		if (m_ShaderInstances.empty())
		{
			return;
		}

		constexpr int default_map_size = 2048;

		// update list of shadow casters
		
		m_CastShadowLights.clear();
		/*
		for (auto shader_object : m_ShaderInstances)
		{
			SuperDynamicLighting* super_shader = static_cast<SuperDynamicLighting*>(shader_object);
			auto casters = super_shader->GetCasters();

			for (auto light : casters)
			{
				m_CastShadowLights.emplace(light);
			}
		}
		*/
		if (m_CastShadowLights.empty())
			return;

		// reset counters

		for (auto shadow_info : m_ShadowCastCombinations)
		{
			shadow_info->m_UseCounter = 0;
			shadow_info->m_UniqueShaders.clear();
		}

		// prepare combinations
		int number_of_shadows = 0;

		for (auto shader_instance : m_ShaderInstances)
		{
			SuperDynamicLighting* shader = static_cast<SuperDynamicLighting*>(shader_instance);

			if (shader->Shadows)
			{
				// TODO: prepare id

				if (shader->UseSceneLights.AsInt() > 0 || shader->AffectingLights.GetCount() == 0)
				{
					// use all casted lights
					for (auto shadow_info : m_ShadowCastCombinations)
					{
						shadow_info->m_UseCounter += 1;
						number_of_shadows += 1;

						if (shader->ShadowMapSize != default_map_size
							|| shader->ShadowCasters.GetCount() > 0)
						{
							shadow_info->m_UniqueShaders.push_back(shader);
						}
					}
				}
				else
				if (shader->UseSceneLights.AsInt() == 0 && shader->AffectingLights.GetCount() > 0)
				{
					// combination of lights

					for (auto shadow_info : m_ShadowCastCombinations)
					{
						if (shader->AffectingLights.Find(shadow_info->m_Light) >= 0)
						{
							shadow_info->m_UseCounter += 1;
							number_of_shadows += 1;
						}
					}

				}
			}
		}

		if (number_of_shadows == 0)
		{
			// TODO: free gpu memory ?!
			return;
		}

		SaveFrameBuffer();

		// bind framebuffer
		m_ShadowFrameBuffer.Bind();

		// TODO:
		//glDrawBuffer(GL_NONE); //! render only into depth texture
		//glReadBuffer(GL_FALSE);
		
		// render shadow maps
		for (auto shadow_info : m_ShadowCastCombinations)
		{
			// prep light, bind texture for the combination
			// check if we have unique casters

			int processed_instances = 0;

			for (FBShader* the_shader : shadow_info->m_UniqueShaders)
			{
				SuperDynamicLighting* shader = static_cast<SuperDynamicLighting*>(the_shader);
				const int shadow_map_size = shader->ShadowMapSize.AsInt();

				GLuint id = 0;

				// allocate unique texture if not exist
				if (processed_instances >= static_cast<int>(shadow_info->m_Textures.size()))
				{
					// allocate a new texture
					id = CreateDepthTexture(shadow_map_size, true);
				}
				else if (shadow_info->m_Textures[processed_instances].m_Size != shadow_map_size)
				{
					// free and allocate a new texture
					glDeleteTextures(1, &shadow_info->m_Textures[processed_instances].m_Id);
					// TODO: ...
					id = CreateDepthTexture(shadow_map_size, true);
				}
				else
				{
					id = shadow_info->m_Textures[processed_instances].m_Id;
				}

				shadow_info->m_Textures[processed_instances].m_Id = id;

				m_ShadowFrameBuffer.AttachTexture(GL_TEXTURE_2D, id, FrameBuffer::eAttachmentTypeDepth);

				// render shader casters into a current texture
				//
				const std::vector<FBModel*> casters = shader->GetCasters();
				if (!casters.empty())
				{
					RenderShadowCasters(&casters);
				}
				else
				{
					RenderShadowCasters(nullptr);
				}
				
				processed_instances += 1;
			}

			if (processed_instances < shadow_info->m_UseCounter)
			{
				// render with default size and casters
				RenderShadowCasters(nullptr);
			}
		}

		m_ShadowFrameBuffer.UnBind();

		RestoreFrameBuffer();
	}

	void SuperShader::RenderShadowCasters(const std::vector<FBModel*>* shadow_casters)
	{
		/*
		if (shadow_casters)
		{
			for (auto model : *shadow_casters)
			{
				if (model)
				{
					// Set the model matrix
					FBMatrix model_matrix;
					model->GetMatrix(model_matrix);
					cgSetMatrixParameterdc(mParamShadowProjModel, model_matrix);

					// Draw the model
					RenderShadowCaster(model);
				}
			}
		}
		else
		{
			if (FBRenderer* renderer = FBSystem().Renderer)
			{
				for (int i = 0, count = renderer->DisplayableGeometryCount; i<count; i++)
				{
					FBModel* model = renderer->GetDisplayableGeometry(i);
					if (model)
					{
						// Set the model matrix
						FBMatrix model_matrix;
						model->GetMatrix(model_matrix);
						cgSetMatrixParameterdc(mParamShadowProjModel, model_matrix);

						// Draw the model
						RenderShadowCaster(model);
					}
				}
			}
		}
		*/
	}

	void SuperShader::RenderShadowCaster(FBModel* pModel)
	{
		FBModelVertexData* vertex_data = pModel->ModelVertexData;
		if (vertex_data && vertex_data->IsDrawable() /*ACME-2464*/)
		{
			const int region_count = vertex_data->GetSubRegionCount();
			if (region_count)
			{
				vertex_data->EnableOGLVertexData();

				for (int index = 0; index < region_count; index++)
				{
					vertex_data->DrawSubRegion(index);
				}
				
				vertex_data->DisableOGLVertexData();
			}
		}
	}
};