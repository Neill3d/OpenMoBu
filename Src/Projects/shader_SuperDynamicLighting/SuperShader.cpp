
// SuperShader.cpp
/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "SuperShader.h"
#include "SuperShader_glsl.h"
#include "CheckGLError.h"
#include "mobu_logging.h"
#include <map>
#include <glm/gtc/matrix_transform.hpp>
#include "glm_utils.h"

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
#define SAMPLER_SLOT_SHADOW			8

namespace Graphics {

	GLuint SuperShader::GetSamplerSlotShadow() const
	{
		return SAMPLER_SLOT_SHADOW;
	}


	//////////////////////////////////////////////////////////////////////////////////////////////////
	
	// return false if transformation matrix is reflected (rendering reflection pass)

	void SuperShader::SetCameraTransform(TTransform &transform, FBRenderOptions* pRenderOptions)
	{
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

		transform.eyePos = glm::vec4(static_cast<float>(eyePos[0]), static_cast<float>(eyePos[1]), static_cast<float>(eyePos[2]), 1.0f);

		FBSVector scl;

		FBMatrixToScaling(scl, lCameraMVMatrix);
		if (scl[1] < 0.0)
		{
			transform.eyePos[3] = -1.0f;
		}

		FBMatrixToGLM(transform.m4_World, lWorldMatrix);
		FBMatrixToGLM(transform.m4_View, lCameraMVMatrix);
		FBMatrixToGLM(transform.m4_Proj, lCameraVPMatrix);
	}

	void SuperShader::SetTransform(TTransform &transform, FBRenderOptions* pRenderOptions, FBShaderModelInfo* pInfo)
	{
		FBMatrix lModelMatrix;
		if (pInfo->GetFBModel())
			pInfo->GetFBModel()->GetMatrix(lModelMatrix, kModelTransformation_Geometry);

		FBMatrixToGLM(transform.m4_Model, lModelMatrix);

		// DONE: assign a normal matrix !
		const glm::mat4 tm = transform.m4_View * transform.m4_Model;
		transform.normalMatrix = glm::inverse(tm);
		transform.normalMatrix = glm::transpose(transform.normalMatrix);
	}

	void SuperShader::SetMaterial(TMaterial &mat, FBMaterial *pMaterial)
	{
		if (nullptr == pMaterial)
			return;

		memset(&mat, 0, sizeof(TMaterial));

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
			mat.emissiveColor[i] = static_cast<float>(lEmissiveColor[i]);
			mat.diffuseColor[i] = static_cast<float>(lDiffuseColor[i]);
			mat.ambientColor[i] = static_cast<float>(lAmbientColor[i]);
			mat.specularColor[i] = static_cast<float>(lSpecularColor[i]);
			mat.reflectColor[i] = static_cast<float>(lReflectionColor[i]);
		}

		mat.specExp = shin;

		mat.emissiveColor[3] = static_cast<float>(lEmissiveFactor);
		mat.diffuseColor[3] = static_cast<float>(lDiffuseFactor);
		mat.ambientColor[3] = static_cast<float>(lAmbientFactor);
		mat.specularColor[3] = static_cast<float>(lSpecularFactor);
		mat.reflectColor[3] = static_cast<float>(lReflectionFactor);
		mat.transparencyColor[3] = static_cast<float>(lTransparencyFactor);

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
			FBMatrixToGLM(mat.diffuseTransform, tm);
		}
		if (nullptr != pMaterial->GetTexture(kFBMaterialTextureTransparent) && lTransparencyFactor > 0.0)
		{
			mat.useTransparency = 1.0f;

			const double *tm = pMaterial->GetTexture(kFBMaterialTextureTransparent)->GetMatrix();
			FBMatrixToGLM(mat.transparencyTransform, tm);
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
			FBMatrixToGLM(mat.specularTransform, tm);
		}
		if (nullptr != pMaterial->GetTexture(kFBMaterialTextureReflection) && lReflectionFactor > 0.0)
		{
			mat.useReflect = 1.0f;

			const double *tm = pMaterial->GetTexture(kFBMaterialTextureReflection)->GetMatrix();
			FBMatrixToGLM(mat.reflectTransform, tm);
		}
		if (nullptr != pMaterial->GetTexture(kFBMaterialTextureNormalMap) && lBumpFactor > 0.0)
		{
			mat.useNormalmap = 1.0f * (float) lBumpFactor;

			const double *tm = pMaterial->GetTexture(kFBMaterialTextureNormalMap)->GetMatrix();
			FBMatrixToGLM(mat.normalTransform, tm);
		}
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
		mGPUSceneLights.reset(new ShaderLightManager());
	}

	SuperShader::~SuperShader()
	{

	}

	bool SuperShader::Initialize(const char *path)
	{
		bool lSuccess = true;

		//
		// release if something already assigned

		if (mShaderBufferId.get())
			mShaderBufferId.reset(nullptr);

		if (mShaderShading.get())
			mShaderShading.reset(nullptr);

		{
			//
			// BufferId Shader

			std::unique_ptr<GLSLShader> shader = std::make_unique<GLSLShader>();

			const FBString vertex_path(path, SHADER_BUFFERID_VERTEX);
			const FBString fragment_path(path, SHADER_BUFFERID_FRAGMENT);

			if (!shader->LoadShaders(vertex_path, fragment_path))
			{
				LOGE("[SuperShader] failed to load and prepare a bufferid shader");
				return false;
			}
		
			// samplers and locations
			shader->Bind();

			if (GLint loc = shader->findLocation("sampler0") >= 0)
				glUniform1i(loc, 0);

			BufferIdShaderUniformLocations.colorId = shader->findLocation("ColorId");
			BufferIdShaderUniformLocations.useDiffuseSampler = shader->findLocation("UseDiffuseSampler");

			shader->UnBind();

			mShaderBufferId = std::move(shader);
		}
		
		{
			//
			// Phong Shading Shader

			std::unique_ptr<GLSLShader> shader = std::make_unique<GLSLShader>();

			const FBString vertex_path = FBString(path, SHADER_SHADING_VERTEX);
			const FBString fragment_path = FBString(path, SHADER_SHADING_FRAGMENT);

			if (!shader->LoadShaders(vertex_path, fragment_path))
			{
				LOGE("[SuperShader] failed to load and prepare a phone shader");
				return false;
			}
				
			
			// samplers and locations
			shader->Bind();

			std::vector<std::pair<const char*, int>> uniforms = {
				{"samplerDiffuse", SAMPLER_SLOT_DIFFUSE},
				{"samplerDisplacement", SAMPLER_SLOT_DISPLACE},
				{"samplerReflect", SAMPLER_SLOT_REFLECT},
				{"samplerTransparency", SAMPLER_SLOT_TRANSPARENCY},
				{"samplerSpecular", SAMPLER_SLOT_SPECULAR},
				{"samplerNormal", SAMPLER_SLOT_NORMAL},
				{"samplerMatCap", SAMPLER_SLOT_MATCAP},
				{"samplerDetail", SAMPLER_SLOT_DETAIL},
				{"samplerShadowMaps", SAMPLER_SLOT_SHADOW}
			};

			for (const auto& [name, slot] : uniforms) {
				if (GLint loc = shader->findLocation(name); loc >= 0) {
					glUniform1i(loc, slot);
				}
				else
				{
					LOGE("[SuperShader] phone shader failed to set uniform for slot %s", name);
				}
			}

			std::map<const char*, GLint*> uniformMap = {
				{"displacementOption", &PhongShaderUniformLocations.displacementOption},
				{"displacementMatrix", &PhongShaderUniformLocations.displacementMatrix},
				
				{"numberOfDirLights", &PhongShaderUniformLocations.numberOfDirLights},
				{"numberOfPointLights", &PhongShaderUniformLocations.numberOfPointLights},
				{"numberOfShadows", &PhongShaderUniformLocations.numberOfShadows},

				{"globalAmbientLight", &PhongShaderUniformLocations.globalAmbientLight},

				{"fogColor", &PhongShaderUniformLocations.fogColor},
				{"fogOptions", &PhongShaderUniformLocations.fogOptions},

				{"rimOptions", &PhongShaderUniformLocations.rimOptions},
				{"rimColor", &PhongShaderUniformLocations.rimColor},

				{"switchAlbedoTosRGB", &PhongShaderUniformLocations.switchAlbedoTosRGB},
				{"useMatCap", &PhongShaderUniformLocations.useMatCap},
				{"useLightmap", &PhongShaderUniformLocations.useLightmap},
			};

			for (const auto& [name, locationPtr] : uniformMap) {
				*locationPtr = shader->findLocation(name);
				if (*locationPtr == -1) {
					LOGE("[SuperShader] Warning: Uniform %s not found in phong shader.", name);
				}
			}

			shader->UnBind();

			mShaderShading = std::move(shader);
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

	
	GLuint SuperShader::GetTextureId(FBMaterial *pMaterial, const FBMaterialTextureType textureType, bool forceUpdate)
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
		if (mLastBinded)
		{
			if (!pRenderOptions->IsIDBufferRendering())
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
				if (BufferIdShaderUniformLocations.useDiffuseSampler >= 0)
				{
					glUniform1f(BufferIdShaderUniformLocations.useDiffuseSampler, (difId > 0));
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

				if (BufferIdShaderUniformLocations.colorId >= 0)
				{
					glUniform4f(BufferIdShaderUniformLocations.colorId, (float)lColorId[0], (float)lColorId[1], (float)lColorId[2], (float)mAlpha);
				}
			}
		}
	}

	void SuperShader::SetMatCap(GLuint texId)
	{
		if (PhongShaderUniformLocations.useMatCap >= 0)
		{
			if (texId > 0)
			{
				glActiveTexture(GL_TEXTURE0 + SAMPLER_SLOT_MATCAP);
				glBindTexture(GL_TEXTURE_2D, texId);
				glActiveTexture(GL_TEXTURE0);

				glUniform1f(PhongShaderUniformLocations.useMatCap, 1.0f);
			}
			else
			{
				glUniform1f(PhongShaderUniformLocations.useMatCap, 0.0f);
			}
		}
	}

	void SuperShader::SetLightmap(GLuint texId, double transparency)
	{
		if (PhongShaderUniformLocations.useLightmap >= 0)
		{
			if (texId > 0 && transparency > 0.0)
			{
				if (texId != mLastLightmapId)
				{
					glActiveTexture(GL_TEXTURE0 + SAMPLER_SLOT_DETAIL);
					glBindTexture(GL_TEXTURE_2D, texId);
					glActiveTexture(GL_TEXTURE0);

					glUniform1f(PhongShaderUniformLocations.useLightmap, 1.0f * (float)transparency);

					mLastLightmapId = texId;
				}
			}
			else
			{
				glUniform1f(PhongShaderUniformLocations.useLightmap, 0.0f);
			}
		}
	}

	void SuperShader::UploadModelViewMatrixArrayForDrawInstanced(const double* pModelViewMatrixArray, int pCount)
	{
		//cgSetBufferSubData(mParamModelViewArrayBuffer, mParamModelViewArrayBufferOffset, 4 * 4 * sizeof (double) * pCount, pModelViewMatrixArray);
	}



	bool SuperShader::CCameraInfoCachePrep(FBCamera *pCamera, CCameraInfoCache &cache)
	{
		if (!pCamera)
			return false;

		FBMatrix mv, p, mvInv;

		pCamera->GetCameraMatrix(mv, kFBModelView);
		pCamera->GetCameraMatrix(p, kFBProjection);

		FBMatrixInverse(mvInv, mv);

		FBMatrixToGLM(cache.mv4, mv);
		FBMatrixToGLM(cache.mvInv4, mvInv);
		FBMatrixToGLM(cache.p4, p);

		memcpy(cache.mv, mv, sizeof(double) * 16);

		FBVector3d v;
		pCamera->GetVector(v);
		for (int i = 0; i<3; ++i)
			cache.pos[i] = static_cast<float>(v[i]);
		
		cache.fov = pCamera->FieldOfView;
		cache.width = pCamera->CameraViewportWidth;
		cache.height = pCamera->CameraViewportHeight;
		cache.nearPlane = pCamera->NearPlaneDistance;
		cache.farPlane = pCamera->FarPlaneDistance;

		return true;
	}

	void SuperShader::UploadRimInformation(double useRim, double rimPower, double *rimColor)
	{
		if (PhongShaderUniformLocations.rimOptions >= 0)
		{
			glUniform4f(PhongShaderUniformLocations.rimOptions, useRim, rimPower, 0.0f, 0.0f);
		}
		if (PhongShaderUniformLocations.rimColor >= 0)
		{
			glUniform4f(PhongShaderUniformLocations.rimColor, rimColor[0], rimColor[1], rimColor[2], 1.0f);
		}
	}

	void SuperShader::UploadFogInformation(double *color, bool enable, double begin, double end, double density, FBFogMode mode)
	{
		if (PhongShaderUniformLocations.fogColor >= 0)
		{
			glUniform4f(PhongShaderUniformLocations.fogColor, (float)color[0], (float)color[1], (float)color[2], (enable)?1.0 : 0.0);
		}
		if (PhongShaderUniformLocations.fogOptions >= 0)
		{
			glUniform4f(PhongShaderUniformLocations.fogOptions, (float)begin, (float)end, (float)density, (float)mode);
		}
	}

	void SuperShader::SetDisplacementInfo(bool useDisp, double dispMult, double dispCenter)
	{
		if (PhongShaderUniformLocations.displacementOption >= 0)
		{
			if (false == useDisp)
			{
				glUniform4f(PhongShaderUniformLocations.displacementOption, 0.0f, 0.0f, 0.0f, 0.0f);
			}
			else
			{
				glUniform4f(PhongShaderUniformLocations.displacementOption, (float)dispMult, (float)dispCenter, 0.0f, 0.0f);
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
			if (PhongShaderUniformLocations.displacementOption >= 0)
			{
				if (false == useDisp)
				{
					glUniform4f(PhongShaderUniformLocations.displacementOption, 0.0f, 0.0f, 0.0f, 0.0f);
				}
				else
				{
					glUniform4f(PhongShaderUniformLocations.displacementOption, (float)dispMult, (float)dispCenter, 0.0f, 0.0f);

					float m[16];
					for (int i = 0; i < 16; ++i)
						m[i] = (float)dispMatrix[i];

					glUniformMatrix4fv(PhongShaderUniformLocations.displacementMatrix, 1, GL_FALSE, m);
				}
			}

			mLastUseDisplacement = useDisp;
			mLastDispCenter = dispCenter;
			mLastDispMult = dispMult;
			mLastDispMatrix = dispMatrix;
		}
	}

};