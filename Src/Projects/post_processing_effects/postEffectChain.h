#pragma once

// postprocessing_effectChain
/*
Sergei <Neill3d> Solokhin 2018-2022

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "GL/glew.h"
#include "postEffectUtils.h"
#include <memory>
#include <vector>
#include <string>

// forward
namespace SharedGraphicsLibrary
{
	struct CGLSLShader;
	class CGLSLShaderManager;
}
class CLoggerCallback;

namespace PostProcessingEffects
{
	// forward
	class PostEffectBuffers;
	class CPostEffectBase;
	
	/// <summary>
	/// a chain of post effects, we start from 3d rendered texture and go effect by effect with adjusting it
	/// </summary>
	class PostEffectChain
	{
	public:
		//! a constructor
		PostEffectChain(CLoggerCallback* logger, SharedGraphicsLibrary::CGLSLShaderManager* shaderManager);
		//! a destructor
		~PostEffectChain();

		void SetupAppPath(const std::string&& appPath, const std::vector<std::string>&& pluginPaths);

		void ChangeContext();
		// w,h - local buffer size for processing, pCamera - current pane camera for processing
		bool Prep(UICallback* uiCallback, const SViewInfo& viewInfo);

		bool BeginFrame(PostEffectBuffers* buffers);
		bool Process(PostEffectBuffers* buffers, double time);

		bool IsCompressedDataReady() const { return m_IsCompressedDataReady; }

	protected:

		CLoggerCallback* m_Logger{ nullptr };
		SharedGraphicsLibrary::CGLSLShaderManager* m_ShaderManager{ nullptr };

		SUICommonParams		m_LastCommonParams;
		SViewInfo			m_LastViewInfo;

		// instances of each effect
		std::unique_ptr<CPostEffectBase>		m_FishEye;
		std::unique_ptr<CPostEffectBase>		m_Color;
		std::unique_ptr<CPostEffectBase>		m_Vignetting;
		std::unique_ptr<CPostEffectBase>		m_FilmGrain;
		std::unique_ptr<CPostEffectBase>		m_LensFlare;
		std::unique_ptr<CPostEffectBase>		m_SSAO;
		std::unique_ptr<CPostEffectBase>		m_DOF;
		std::unique_ptr<CPostEffectBase>		m_Displacement;
		std::unique_ptr<CPostEffectBase>		m_MotionBlur;

		// shared shaders ids

		int		m_ShaderDepthLinearize;	//!< linearize depth for other filters (DOF, SSAO, Bilateral Blur, etc.)
		int		m_ShaderBlur;		//!< needed for SSAO
		int		m_ShaderMix;		//!< needed for SSAO
		int		m_ShaderDownscale;

		// order execution chain
		std::vector<CPostEffectBase*>		m_Chain;

		GLint							m_LocDepthLinearizeClipInfo;
		GLint							m_LocBlurSharpness;
		GLint							m_LocBlurRes;


		bool							m_NeedReloadShaders;
		bool							m_IsCompressedDataReady;
		double							m_LastCompressTime;

		CPostEffectBase* NewPostEffect(const EPostProcessEffectType type, const char* shadersLocation);

		/// <summary>
		/// try to locate shader files and load shader objects from them
		/// </summary>
		/// <param name="appPath">system application path</param>
		/// <param name="pluginPaths">registered plugin paths</param>
		/// <returns>true if load operation was successful</returns>
		bool LoadShaders(const std::string& appPath, const std::vector<std::string>& pluginPaths);
		void FreeShaders();

	private:
		
		std::string					m_AppPath;
		std::vector<std::string>	m_PluginPaths;
		
		const bool CheckShadersPath(const char* path) const;

		void LoadDepthLinearizeShader(const std::string& shadersPath);
		void LoadBlurShader(const std::string& shadersPath);
		void LoadMixShader(const std::string& shadersPath);
		void LoadDownScaleShader(const std::string& shadersPath);

	};
}