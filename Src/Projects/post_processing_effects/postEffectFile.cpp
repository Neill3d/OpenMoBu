#include "postEffectFile.h"

using namespace PostProcessingEffects;

CPostEffectFile::CPostEffectFile(CLoggerCallback* logger, SharedGraphicsLibrary::CShaderManagerBase* shaderManager)
	: CPostEffectBase(logger, shaderManager)
{
}

PostProcessingEffects::CPostEffectFile::~CPostEffectFile()
{
}

