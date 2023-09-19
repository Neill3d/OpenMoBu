
/**	\file	postprocessing_effect.cxx

Sergei <Neill3d> Solokhin 2018-2019

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "postEffectChain.h"
#include "postEffectUtils.h"
#include "postEffectBuffers.h"
#include "postEffectBase.h"
#include "postEffectFishEye.h"
#include <string>
#include <FileUtils.h>
#include "ReadFileScope.h"
#include "glslShaderManager.h"
#include "shaderFactory.h"
#include "ogl_utils.h"

using namespace SharedGraphicsLibrary;
using namespace PostProcessingEffects;

// shared shaders

constexpr const char* SHADER_DEPTH_LINEARIZE_VERTEX{ "\\GLSL\\simple.vsh" };
constexpr const char* SHADER_DEPTH_LINEARIZE_FRAGMENT{ "\\GLSL\\depthLinearize.fsh" };

constexpr const char* SHADER_BLUR_VERTEX{ "\\GLSL\\simple.vsh" };
constexpr const char* SHADER_BLUR_FRAGMENT{ "\\GLSL\\blur.fsh" };

constexpr const char* SHADER_MIX_VERTEX{ "\\GLSL\\simple.vsh" };
constexpr const char* SHADER_MIX_FRAGMENT{ "\\GLSL\\mix.fsh" };

constexpr const char* SHADER_DOWNSCALE_VERTEX{ "\\GLSL\\downscale.vsh" };
constexpr const char* SHADER_DOWNSCALE_FRAGMENT{ "\\GLSL\\downscale.fsh" };


////////////////////////////////////////////////////////////////////////////////////
// post effect chain

//! a constructor
PostEffectChain::PostEffectChain(CLoggerCallback* logger, SharedGraphicsLibrary::CGLSLShaderManager* shaderManager)
	: m_Logger(logger)
	, m_ShaderManager(shaderManager)
{
	m_IsCompressedDataReady = false;
	m_NeedReloadShaders = true;
	m_LocDepthLinearizeClipInfo = -1;
	m_LocBlurRes = -1;
	m_LocBlurSharpness = -1;
	m_LastCompressTime = 0.0;
}

//! a destructor
PostEffectChain::~PostEffectChain()
{

}

void PostProcessingEffects::PostEffectChain::SetupAppPath(const std::string&& appPath, const std::vector<std::string>&& pluginPaths)
{
	m_AppPath = std::move(appPath);
	m_PluginPaths = std::move(pluginPaths);
}

void PostEffectChain::ChangeContext()
{
	FreeShaders();
	m_NeedReloadShaders = true;
	m_IsCompressedDataReady = false;
	m_LastCompressTime = 0.0;
}

bool PostEffectChain::Prep(UICallback* uiCallback, const SViewInfo& viewInfo)
{
	bool lSuccess = true;

	memcpy(&m_LastCommonParams, &uiCallback->commonParams, sizeof(SUICommonParams));
	memcpy(&m_LastViewInfo, &viewInfo, sizeof(SViewInfo));
	
	if (!uiCallback->Ok())
		return false;

	if (uiCallback->IsNeedToReloadShaders())
	{
		m_NeedReloadShaders = true;
		uiCallback->SetReloadShadersState(false);
	}

	if (m_NeedReloadShaders)
	{
		if (false == LoadShaders(m_AppPath, m_PluginPaths))
			lSuccess = false;

		m_NeedReloadShaders = false;
	}

	// update UI values

	if (m_LastCommonParams.useFishEye && m_FishEye.get())
		m_FishEye->CollectUIValues(uiCallback, viewInfo);
	
	if (m_LastCommonParams.useColorCorrection && m_Color.get())
		m_Color->CollectUIValues(uiCallback, viewInfo);
	
	if (m_LastCommonParams.useVignetting && m_Vignetting.get())
		m_Vignetting->CollectUIValues(uiCallback, viewInfo);
	
	if (m_LastCommonParams.useFilmGrain && m_FilmGrain.get())
		m_FilmGrain->CollectUIValues(uiCallback, viewInfo);

	if (m_LastCommonParams.useLensFlare && m_LensFlare.get())
		m_LensFlare->CollectUIValues(uiCallback, viewInfo);

	if (m_LastCommonParams.useSSAO && m_SSAO.get())
		m_SSAO->CollectUIValues(uiCallback, viewInfo);

	if (m_LastCommonParams.useDepthOfField && m_DOF.get())
		m_DOF->CollectUIValues(uiCallback, viewInfo);

	if (m_LastCommonParams.useDisplacement && m_Displacement.get())
		m_Displacement->CollectUIValues(uiCallback, viewInfo);

	if (m_LastCommonParams.useMotionBlur && m_MotionBlur.get())
		m_MotionBlur->CollectUIValues(uiCallback, viewInfo);

	return lSuccess;
}

bool PostEffectChain::BeginFrame(PostEffectBuffers *buffers)
{
	return true;

	/*
	if (false == buffers->Ok())
		return false;

	if (false == mSettings.Ok())
		return false;

	bool lSuccess = false;

	// if we need a preview
	if (mSettings->OutputPreview && buffers->GetPreviewWidth() > 1 && nullptr != buffers->GetBufferDownscalePtr())
	{
		
		// grab texture data

		lSuccess = buffers->PreviewCompressBegin();
	}

	return lSuccess;
	*/
}

bool PostEffectChain::Process(PostEffectBuffers *buffers, double systime)
{
	m_IsCompressedDataReady = false;

	if (false == buffers->Ok())
		return false;

	if (!m_LastCommonParams.isOk)
		return false;

	// prepare chain count and order

	int count = 0;

	if (m_LastCommonParams.useFishEye)
		count += 1;
	if (m_LastCommonParams.useColorCorrection)
		count += 1;
	if (m_LastCommonParams.useVignetting)
		count += 1;
	if (m_LastCommonParams.useFilmGrain)
		count += 1;
	if (m_LastCommonParams.useLensFlare)
		count += 1;
	if (m_LastCommonParams.useSSAO)
		count += 1;
	if (m_LastCommonParams.useDepthOfField)
		count += 1;
	if (m_LastCommonParams.useDisplacement)
		count += 1;
	if (m_LastCommonParams.useMotionBlur)
		count += 1;

	if (0 == count && !m_LastCommonParams.doOutputPreview)
		return false;

	m_Chain.resize(count);

	count = 0;

	int blurAndMix = -1;
	int blurAndMix2 = -1;

	// ordering HERE

	if (m_LastCommonParams.useSSAO)
	{
		m_Chain[count] = m_SSAO.get();

		if (m_LastCommonParams.doSSAOBlur)
		{
			blurAndMix = count;
		}
		count += 1;
	}
	if (m_LastCommonParams.useMotionBlur)
	{
		m_Chain[count] = m_MotionBlur.get();
		count += 1;
	}
	if (m_LastCommonParams.useDepthOfField)
	{
		m_Chain[count] = m_DOF.get();
		count += 1;
	}
	if (m_LastCommonParams.useColorCorrection)
	{
		m_Chain[count] = m_Color.get();
		if (m_LastCommonParams.doBloom)
		{
			blurAndMix2 = count;
		}
		count += 1;
	}
	if (m_LastCommonParams.useLensFlare)
	{
		m_Chain[count] = m_LensFlare.get();
		count += 1;
	}
	if (m_LastCommonParams.useDisplacement)
	{
		m_Chain[count] = m_Displacement.get();
		count += 1;
	}
	if (m_LastCommonParams.useFishEye)
	{
		m_Chain[count] = m_FishEye.get();
		count += 1;
	}
	if (m_LastCommonParams.useFilmGrain)
	{
		m_Chain[count] = m_FilmGrain.get();
		count += 1;
	}
	if (m_LastCommonParams.useVignetting)
	{
		m_Chain[count] = m_Vignetting.get();
		count += 1;
	}

	if (m_LastCommonParams.useSSAO)
	{
		const GLuint depthId = buffers->GetSrcBufferPtr()->GetDepthObject();

		// prep data

		float znear = static_cast<float>(m_LastViewInfo.nearPlane);
		float zfar = static_cast<float>(m_LastViewInfo.farPlane);
		bool perspective = (m_LastViewInfo.isPerspective);

		float clipInfo[4] = {
			znear * zfar,
			znear - zfar,
			zfar,
			(perspective) ? 1.0f : 0.0f
		};

		FrameBuffer *pBufferDepth = buffers->GetBufferDepthPtr();

		// render

		pBufferDepth->Bind();

		m_ShaderManager->Bind(m_ShaderDepthLinearize);
		
		glBindTexture(GL_TEXTURE_2D, depthId);

		if (m_LocDepthLinearizeClipInfo >= 0)
			m_ShaderManager->SetUniformVector4(m_LocDepthLinearizeClipInfo, clipInfo);

		drawOrthoQuad2d(pBufferDepth->GetWidth(), pBufferDepth->GetHeight());

		m_ShaderManager->UnBind();
		pBufferDepth->UnBind();

		// DONE: bind a depth texture
		const GLuint linearDepthId = pBufferDepth->GetColorObject();

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, linearDepthId);
		glActiveTexture(GL_TEXTURE0);
	}
	
	if (m_LastCommonParams.useDepthOfField)
	{
		const GLuint depthId = buffers->GetSrcBufferPtr()->GetDepthObject();

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthId);
		glActiveTexture(GL_TEXTURE0);
	}

	// compute effect chain with double buffer

	// DONE: when buffer is attached, buffer is used itself !
	bool lSuccess = false;
	const bool generateMips = m_LastCommonParams.doGenerateMipMaps;

	if (count > 0)
	{
		GLuint texid = 0;
		
		const int w = buffers->GetWidth();
		const int h = buffers->GetHeight();

		const float blurSharpness = 0.1f * m_LastCommonParams.blurSharpness;
		const float invRes[2] = { 1.0f / float(w), 1.0f / float(h) };

		// generate mipmaps for the first target
		texid = buffers->GetSrcBufferPtr()->GetColorObject();

		if (true == generateMips)
		{
			if (texid > 0)
			{
				glBindTexture(GL_TEXTURE_2D, texid);
				glGenerateMipmap(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
		}
		
		for (int i = 0; i < count; ++i)
		{
			m_Chain[i]->Bind();

			for (int j = 0; j < m_Chain[i]->GetNumberOfPasses(); ++j)
			{
				m_Chain[i]->PrepPass(j);

				texid = buffers->GetSrcBufferPtr()->GetColorObject();
				glBindTexture(GL_TEXTURE_2D, texid);

				if (generateMips)
				{
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				}
				else
				{
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				}

				buffers->GetDstBufferPtr()->Bind();

				drawOrthoQuad2d(w, h);

				buffers->GetDstBufferPtr()->UnBind(generateMips);

				//
				buffers->SwapBuffers();
			}

			m_Chain[i]->UnBind();

			// if we need more passes, blur and mix for SSAO
			if (i == blurAndMix || i == blurAndMix2)
			{
				if (!m_LastCommonParams.doOnlyAO || (i == blurAndMix2))
				{
					// Bilateral Blur Pass

					texid = buffers->GetSrcBufferPtr()->GetColorObject();
					glBindTexture(GL_TEXTURE_2D, texid);

					buffers->GetBufferBlurPtr()->Bind();
					m_ShaderManager->Bind(m_ShaderBlur);

					if (m_LocBlurSharpness >= 0)
						m_ShaderManager->SetUniformFloat(m_LocBlurSharpness, blurSharpness);
					if (m_LocBlurRes >= 0)
						m_ShaderManager->SetUniformVector2f(m_LocBlurRes, invRes[0], invRes[1]);

					const float color_shift = (m_LastCommonParams.doBloom) ? 0.01f * m_LastCommonParams.bloomMinBright : 0.0f;
					m_ShaderManager->SetUniformFloat(m_ShaderBlur, "g_ColorShift", color_shift);
					
					drawOrthoQuad2d(w, h);

					m_ShaderManager->UnBind();
					buffers->GetBufferBlurPtr()->UnBind();

					//
					buffers->SwapBuffers();

					// Mix AO and Color Pass
					const GLuint blurId = buffers->GetBufferBlurPtr()->GetColorObject();
					texid = buffers->GetSrcBufferPtr()->GetColorObject();

					glActiveTexture(GL_TEXTURE3);
					glBindTexture(GL_TEXTURE_2D, blurId);
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, texid);

					buffers->GetDstBufferPtr()->Bind();
					m_ShaderManager->Bind(m_ShaderMix);

					if (m_LastCommonParams.doBloom)
					{
						m_ShaderManager->SetUniformVector(m_ShaderMix, "gBloom", 0.01f * m_LastCommonParams.bloomTone, 0.01f * m_LastCommonParams.bloomStretch, 0.0f, 1.0f);
					}
					else
					{
						m_ShaderManager->SetUniformVector(m_ShaderMix, "gBloom", 0.0f, 0.0f, 0.0f, 0.0f);
					}

					drawOrthoQuad2d(w, h);

					m_ShaderManager->UnBind();
					buffers->GetDstBufferPtr()->UnBind();

					glActiveTexture(GL_TEXTURE3);
					glBindTexture(GL_TEXTURE_2D, 0);
					glActiveTexture(GL_TEXTURE0);

					//
					buffers->SwapBuffers();
				}
				else
				{
					// Bilateral Blur Pass

					texid = buffers->GetSrcBufferPtr()->GetColorObject();
					glBindTexture(GL_TEXTURE_2D, texid);

					buffers->GetDstBufferPtr()->Bind();
					m_ShaderManager->Bind(m_ShaderBlur);

					if (m_LocBlurSharpness >= 0)
						m_ShaderManager->SetUniformFloat(m_LocBlurSharpness, blurSharpness);
					if (m_LocBlurRes >= 0)
						m_ShaderManager->SetUniformVector2f(m_LocBlurRes, invRes[0], invRes[1]);

					drawOrthoQuad2d(w, h);

					m_ShaderManager->UnBind();
					buffers->GetDstBufferPtr()->UnBind();

					//
					buffers->SwapBuffers();
				}
			}
		}

		lSuccess = true;
	}

	// if we need a preview (rate - compress 25-30 frames per second)
	if (m_LastCommonParams.doOutputPreview)
	{
		int updaterate = m_LastCommonParams.outputPreviewRate;
		if (updaterate <= 0 || updaterate > 30)
			updaterate = 10;

		double rate = 1.0 / updaterate;

		if (0.0 == m_LastCompressTime || (systime - m_LastCompressTime) > rate)
		{
			GLuint texid = buffers->GetSrcBufferPtr()->GetColorObject();
			glBindTexture(GL_TEXTURE_2D, texid);

			buffers->GetBufferDownscalePtr()->Bind();
			m_ShaderManager->Bind(m_ShaderDownscale);

			GLint loc = m_ShaderManager->FindLocation(m_ShaderDownscale, "texelSize");
			if (loc >= 0)
				m_ShaderManager->SetUniformVector2f(loc, 1.0f / (float)buffers->GetWidth(), 1.0f / (float)buffers->GetHeight());

			drawOrthoQuad2d(buffers->GetPreviewWidth(), buffers->GetPreviewHeight());

			m_ShaderManager->UnBind();
			buffers->GetBufferDownscalePtr()->UnBind();

			const unsigned int previewW = buffers->GetPreviewWidth();
			const unsigned int previewH = buffers->GetPreviewHeight();
			const double ratio = 1.0 * (double)buffers->GetWidth() / (double)buffers->GetHeight();

			if (m_LastCommonParams.doOutputUseCompression)
			{
				GLint compressionCode = 0;

				if (buffers->PreviewOpenGLCompress(m_LastCommonParams.outputCompression, compressionCode))
				{
					//mSettings->SetPreviewTextureId(buffers->GetPreviewCompressedColor(), ratio, previewW, previewH,
						//static_cast<int32_t>(buffers->GetUnCompressedSize()), 
						//static_cast<int32_t>(buffers->GetCompressedSize()), 
						//compressionCode, systime);

					m_IsCompressedDataReady = true;
				}

				
			}
			else
			{
				GLint compressionCode = GL_RGB8;

				// TODO:
				//mSettings->SetPreviewTextureId(buffers->GetPreviewColor(), ratio, previewW, previewH,
					//static_cast<int32_t>(buffers->GetUnCompressedSize()),
					//static_cast<int32_t>(buffers->GetCompressedSize()), 
					//compressionCode, systime);
					
			}
			
			m_LastCompressTime = systime;
		}
	}

	//
	if (m_LastCommonParams.useDepthOfField)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0);
	}
	if (m_LastCommonParams.useSSAO)
	{
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0);
	}

	return lSuccess;
}


CPostEffectBase *PostEffectChain::NewPostEffect(const EPostProcessEffectType type, const char *shadersLocation)
{

	CPostEffectBase *newEffect = nullptr;

	switch (type)
	{
	case EPostProcessEffectType::FISHEYE:
		newEffect = new PostEffectFishEye(m_Logger, m_ShaderManager);
		break;
	case EPostProcessEffectType::COLOR:
		//newEffect = new PostEffectColor();
		break;
	case EPostProcessEffectType::VIGNETTE:
		//newEffect = new PostEffectVignetting();
		break;
	case EPostProcessEffectType::FILMGRAIN:
		//newEffect = new PostEffectFilmGrain();
		break;
	case EPostProcessEffectType::LENSFLARE:
		//newEffect = new PostEffectLensFlare();
		break;
	case EPostProcessEffectType::SSAO:
		//newEffect = new PostEffectSSAO();
		break;
	case EPostProcessEffectType::DOF:
		//newEffect = new PostEffectDOF();
		break;
	case EPostProcessEffectType::DISPLACEMENT:
		//newEffect = new PostEffectDisplacement();
		break;
	case EPostProcessEffectType::MOTIONBLUR:
		//newEffect = new PostEffectMotionBlur();
		break;
	}


	try
	{
		if (!newEffect)
			throw std::exception("failed to allocate memory for the shader");
	
		for (int i = 0; i < newEffect->GetNumberOfShaders(); ++i)
		{
			//std::string vertex_path = std::string(shadersLocation) + newEffect->GetVertexFname(i);
			//std::string fragment_path = std::string(shadersLocation) + newEffect->GetFragmentFname(i);
			//std::string ui_path = std::string(shadersLocation) + newEffect->GetUIFName();

			if (false == newEffect->Load(i, shadersLocation))
			{
				throw std::exception("failed to load and prepare effect");
			}

			// samplers and locations
			newEffect->PrepUniforms(i);
		}
	}
	catch (const std::exception &e)
	{
		m_Logger->Trace("Post Effect Chain ERROR: %s\n", e.what());

		delete newEffect;
		newEffect = nullptr;
	}

	return newEffect;
}

const bool PostEffectChain::CheckShadersPath(const char* path) const
{
	const char* test_shaders[] = {
		SHADER_DEPTH_LINEARIZE_VERTEX,
		SHADER_DEPTH_LINEARIZE_FRAGMENT,

		SHADER_BLUR_VERTEX,
		SHADER_BLUR_FRAGMENT,

		SHADER_MIX_VERTEX,
		SHADER_MIX_FRAGMENT,

		SHADER_DOWNSCALE_VERTEX,
		SHADER_DOWNSCALE_FRAGMENT	
	};

	const std::string pathStr(path);

	for (const char* shader_path : test_shaders)
	{
		const std::string full_path = pathStr + shader_path;

		if (!IsFileExists(full_path.c_str()))
		{
			return false;
		}
	}

	return true;
}

// mSystem.ApplicationPath
bool PostEffectChain::LoadShaders(const std::string& appPath, const std::vector<std::string>& pluginPaths)
{
	FreeShaders();

	std::string shaders_path = appPath + "\\plugins";
	bool status = true;

	if (!CheckShadersPath(shaders_path.c_str()))
	{
		status = false;
		
		for (auto& pluginPath : pluginPaths)
		{
			if (CheckShadersPath(pluginPath.c_str() ))
			{
				shaders_path = pluginPath;
				status = true;
				break;
			}
		}
	}

	if (!status)
	{
		m_Logger->Trace("[PostProcessing] Failed to find shaders location!\n");
		return false;
	}
	
	m_Logger->Trace("[PostProcessing] Shaders Location - %s\n", shaders_path);

	m_FishEye.reset(NewPostEffect(EPostProcessEffectType::FISHEYE, shaders_path.c_str()));
	m_Color.reset(NewPostEffect(EPostProcessEffectType::COLOR, shaders_path.c_str()));
	m_Vignetting.reset(NewPostEffect(EPostProcessEffectType::VIGNETTE, shaders_path.c_str()));
	m_FilmGrain.reset(NewPostEffect(EPostProcessEffectType::FILMGRAIN, shaders_path.c_str()));
	m_LensFlare.reset(NewPostEffect(EPostProcessEffectType::LENSFLARE, shaders_path.c_str()));
	m_SSAO.reset(NewPostEffect(EPostProcessEffectType::SSAO, shaders_path.c_str()));
	m_DOF.reset(NewPostEffect(EPostProcessEffectType::DOF, shaders_path.c_str()));
	m_Displacement.reset(NewPostEffect(EPostProcessEffectType::DISPLACEMENT, shaders_path.c_str()));
	m_MotionBlur.reset(NewPostEffect(EPostProcessEffectType::MOTIONBLUR, shaders_path.c_str()));

	//
	// load shared shaders (blur, mix)

	try
	{
		LoadDepthLinearizeShader(shaders_path);

		LoadBlurShader(shaders_path);

		LoadMixShader(shaders_path);

		LoadDownScaleShader(shaders_path);
	}
	catch (const std::exception &e)
	{
		m_Logger->Trace("Post Effect Chain ERROR: %s\n", e.what());
		return false;
	}

	return true;
}

void PostProcessingEffects::PostEffectChain::LoadDepthLinearizeShader(const std::string& shadersPath)
{
	const std::string vertex_path = shadersPath + SHADER_DEPTH_LINEARIZE_VERTEX;
	const std::string fragment_path = shadersPath + SHADER_DEPTH_LINEARIZE_FRAGMENT;

	const SharedMotionLibrary::CReadFileScope vertexFile(vertex_path.c_str());
	const SharedMotionLibrary::CReadFileScope fragmentFile(fragment_path.c_str());

	const int newShaderId = m_ShaderManager->NewShader(vertexFile, fragmentFile);
	if (newShaderId < 0)
		throw std::exception("failed to load and prepare depth linearize shader");
	
	// samplers and locations
	m_ShaderManager->Bind(newShaderId);

	GLint loc = m_ShaderManager->FindLocation(newShaderId, "depthSampler");
	if (loc >= 0)
		m_ShaderManager->SetUniformUINT(loc, 0);
		
	m_LocDepthLinearizeClipInfo = m_ShaderManager->FindLocation(newShaderId, "gClipInfo");

	m_ShaderManager->UnBind();
	m_ShaderDepthLinearize = newShaderId;
}

void PostProcessingEffects::PostEffectChain::LoadBlurShader(const std::string& shadersPath)
{
	const std::string vertex_path = shadersPath + SHADER_BLUR_VERTEX;
	const std::string fragment_path = shadersPath + SHADER_BLUR_FRAGMENT;

	const SharedMotionLibrary::CReadFileScope vertexFile(vertex_path.c_str());
	const SharedMotionLibrary::CReadFileScope fragmentFile(fragment_path.c_str());

	const int newShaderId = m_ShaderManager->NewShader(vertexFile, fragmentFile);
	if (newShaderId < 0)
		throw std::exception("failed to load and prepare blur shader");
	
	// samplers and locations
	m_ShaderManager->Bind(newShaderId);

	auto loc = m_ShaderManager->FindLocation(newShaderId, "sampler0");
	if (loc >= 0)
		m_ShaderManager->SetUniformUINT(loc, 0);
	loc = m_ShaderManager->FindLocation(newShaderId, "linearDepthSampler");
	if (loc >= 0)
		m_ShaderManager->SetUniformUINT(loc, 2);

	m_LocBlurSharpness = m_ShaderManager->FindLocation(newShaderId, "g_Sharpness");
	m_LocBlurRes = m_ShaderManager->FindLocation(newShaderId, "g_InvResolutionDirection");

	m_ShaderManager->UnBind();

	m_ShaderBlur = newShaderId;
}

void PostProcessingEffects::PostEffectChain::LoadMixShader(const std::string& shadersPath)
{
	const std::string vertex_path = shadersPath + SHADER_MIX_VERTEX;
	const std::string fragment_path = shadersPath + SHADER_MIX_FRAGMENT;
	const SharedMotionLibrary::CReadFileScope vertexFile(vertex_path.c_str());
	const SharedMotionLibrary::CReadFileScope fragmentFile(fragment_path.c_str());

	const int newShaderId = m_ShaderManager->NewShader(vertexFile, fragmentFile);
	if (newShaderId < 0)
		throw std::exception("failed to load and prepare mix shader");

	m_ShaderManager->Bind(newShaderId);

	auto loc = m_ShaderManager->FindLocation(newShaderId, "sampler0");
	if (loc >= 0) m_ShaderManager->SetUniformUINT(loc, 0);
	
	loc = m_ShaderManager->FindLocation(newShaderId, "sampler1");
	if (loc >= 0) m_ShaderManager->SetUniformUINT(loc, 3);

	m_ShaderManager->UnBind();
	m_ShaderMix = newShaderId;
}

void PostEffectChain::LoadDownScaleShader(const std::string& shadersPath)
{
	const std::string vertex_path = shadersPath + SHADER_DOWNSCALE_VERTEX;
	const std::string fragment_path = shadersPath + SHADER_DOWNSCALE_FRAGMENT;
	const SharedMotionLibrary::CReadFileScope vertexFile(vertex_path.c_str());
	const SharedMotionLibrary::CReadFileScope fragmentFile(fragment_path.c_str());

	const int newShaderId = m_ShaderManager->NewShader(vertexFile, fragmentFile);
	if (newShaderId < 0)
		throw std::exception("failed to load and prepare downscale shader");
 
	m_ShaderManager->Bind(newShaderId);

	auto loc = m_ShaderManager->FindLocation(newShaderId, "sampler");
	if (loc >= 0)
		m_ShaderManager->SetUniformUINT(loc, 0);

	m_ShaderManager->UnBind();
	m_ShaderDownscale = newShaderId;
}

void PostEffectChain::FreeShaders()
{
	m_FishEye.reset(nullptr);
	m_Color.reset(nullptr);
	m_Vignetting.reset(nullptr);
	m_FilmGrain.reset(nullptr);
	m_LensFlare.reset(nullptr);
	m_SSAO.reset(nullptr);
	m_DOF.reset(nullptr);
	m_Displacement.reset(nullptr);
	m_MotionBlur.reset(nullptr);

	m_ShaderManager->Free(m_ShaderDepthLinearize);
	m_ShaderManager->Free(m_ShaderBlur);
	m_ShaderManager->Free(m_ShaderMix);
	m_ShaderManager->Free(m_ShaderDownscale);
	/*
	m_ShaderDepthLinearize.reset(nullptr);
	m_ShaderBlur.reset(nullptr);
	m_ShaderMix.reset(nullptr);
	m_ShaderDownscale.reset(nullptr);
	*/
}