#pragma once

#include "postEffectBase.h"

// forward
class CLoggerCallback;

namespace PostProcessingEffects
{
	class CPostEffectFile : public CPostEffectBase
	{
	public:
		//! a constructor
		CPostEffectFile(CLoggerCallback* logger, SharedGraphicsLibrary::CShaderManagerBase* shaderManager);
		//! a destructor
		virtual ~CPostEffectFile();

		int GetNumberOfShaders() const override { return m_numberOfShaders; }

		const char* GetName() override { return m_shaderName.data(); }
		const char* GetVertexFname(const int shaderIndex) override { return m_vertexProgramPath.data(); }
		const char* GetFragmentFname(const int shaderIndex) override { return m_fragmentProgramPath.data(); }
		const char* GetUIFName() override { return m_effectPath.data(); }

	private:

		int m_numberOfShaders{ 1 };
		std::string_view m_shaderName;
		std::string_view m_vertexProgramPath;
		std::string_view m_fragmentProgramPath;
		std::string_view m_effectPath;
	};
}


