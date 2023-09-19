#pragma once

#include "libraryMain.h"

#include "postEffectUtils.h"
#include "logger.h"

#include <memory>
#include <vector>
#include <string>

#include "StringPoolProvider.h"
#include "EffectPropertyList.h"

#include <string_view>

// forward
namespace SharedGraphicsLibrary
{
	struct CGLSLShader;
	class CShaderManagerBase;
	class CGLSLShaderManager;
}

namespace PostProcessingEffects
{

	/// <summary>
	/// A bridge between collecting UI values and running glsl shader
	/// </summary>
	class POST_PROCESSING_EFFECTS_API CPostEffectBase
	{
	public:

		//! a constructor
		CPostEffectBase(CLoggerCallback* logger, SharedGraphicsLibrary::CShaderManagerBase* shaderManager);
		//! a destructor
		virtual ~CPostEffectBase();

		virtual int GetNumberOfShaders() const = 0;

		virtual const char* GetName() = 0;
		virtual const char* GetVertexFname(const int shaderIndex) = 0;
		virtual const char* GetFragmentFname(const int shaderIndex) = 0;
		virtual const char* GetUIFName() = 0;

		// load and initialize shader from a specified location
		bool Load(const int shaderIndex, const char* resourcePath);

		virtual bool PrepUniforms(const int shaderIndex) { return false; }
		// grab main UI values for the effect
		virtual bool CollectUIValues(UICallback* uiCallback, const SViewInfo& viewInfo) { return false; }

		void PrepareUniforms();

		// new feature to have several passes for a specified effect
		virtual const int GetNumberOfPasses() const { return 1; }
		virtual bool PrepPass(const int pass) { return true; }

		virtual void Bind();
		virtual void UnBind();

		int GetShaderIndex(const int selectionIndex) const { return m_ShadersSelection[selectionIndex]; }
		int GetShaderIndex() const { return m_ShadersSelection[m_CurrentShader]; }

		template <typename T> T* GetShaderTypedPtr() { return (T*)&m_ShadersSelection[m_CurrentShader]; }

	protected:

		CLoggerCallback* m_Logger{ nullptr };
		SharedGraphicsLibrary::CShaderManagerBase* m_ShaderManager{ nullptr };

		CStringPoolProvider		m_StringPoolProvider;
		CEffectPropertyList		m_Properties;

		int						m_CurrentShader{ 0 };
		std::vector<int>		m_ShadersSelection; // shaders used by the effect and allocated in ShadersManager

		void SetCurrentShader(const int index) { m_CurrentShader = index; }
		void FreeShaders();

		void ParseUIFile(const char* filePath);
	};
}