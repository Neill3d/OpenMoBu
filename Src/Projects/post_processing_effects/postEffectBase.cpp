
#include "postEffectBase.h"
#include "ShaderFactory.h"
#include "glslShaderManager.h"
#include "shaderManager.h"
#include "ReadFileScope.h"
#include "json.hpp"

#include "EffectProperty.h"
#include "EffectPropertyList.h"

using namespace PostProcessingEffects;
using namespace SharedGraphicsLibrary;
using namespace SharedMotionLibrary;

/////////////////////////////////////////////////////////////////////////
// EffectBase

CPostEffectBase::CPostEffectBase(CLoggerCallback* logger, SharedGraphicsLibrary::CShaderManagerBase* shaderManager)
    : m_Logger(logger)
    , m_ShaderManager(shaderManager)
{ }

CPostEffectBase::~CPostEffectBase()
{
    //FreeShaders();
}

// load and initialize shader from a specified location

void CPostEffectBase::FreeShaders()
{
    for (auto shaderptr : m_ShadersSelection)
    {
        m_ShaderManager->Free(shaderptr);
    }
    m_ShadersSelection.clear();
}


EEffectType ConvertStringToEffectType(const std::string_view& str)
{
    if (str.compare("Number"))
        return EEffectType::NUMBER;
    else if (str.compare("Integer"))
        return EEffectType::INT;
    else if (str.compare("Bool"))
        return EEffectType::BOOL;
    else if (str.compare("Vector"))
        return EEffectType::VECTOR;
    else if (str.compare("Vector2"))
        return EEffectType::VECTOR2;
    else if (str.compare("Vector4"))
        return EEffectType::VECTOR4;
    return EEffectType::COUNT;
}


void PostProcessingEffects::CPostEffectBase::ParseUIFile(const char* filePath)
{
    auto jsonData = tao::json::from_file(filePath);

    auto propertiesArray = jsonData.get_array();

    for (int i = 0; i < propertiesArray.size(); ++i)
    {
        auto name = m_StringPoolProvider.AddString(propertiesArray[i].get_string_view());
        
        auto tooltip = propertiesArray[i].at("name").get_string_view();
        auto uniform = propertiesArray[i].at("uniform").get_string_view();
        auto typeStr = propertiesArray[i].at("type").get_string_view();

        SEffectProperty&& prop = m_Properties.CreateProperty();

        prop.name = m_StringPoolProvider.AddString(name);
        prop.tooltip = !tooltip.empty() ? m_StringPoolProvider.AddString(tooltip) : std::string_view();
        prop.uniform = !uniform.empty() ? m_StringPoolProvider.AddString(uniform) : std::string_view();
        prop.valueType = ConvertStringToEffectType(typeStr);
        prop.minValue = static_cast<float>(propertiesArray[i].at("minValue").get_double());
        prop.maxValue = static_cast<float>(propertiesArray[i].at("maxValue").get_double());

        constexpr const char* defaultValueName{ "defaultValue" };

        switch (prop.valueType)
        {
        case EEffectType::NUMBER: prop.floatValue = static_cast<float>(propertiesArray[i].at(defaultValueName).get_double());
            break;
        case EEffectType::INT: prop.intValue = static_cast<int>(propertiesArray[i].at(defaultValueName).get_signed());
            break;
        case EEffectType::BOOL: prop.boolValue = propertiesArray[i].at(defaultValueName).get_boolean();
            break;
        case EEffectType::STRING: strcpy_s(prop.str, sizeof(char) * SEffectProperty::STR_SIZE, propertiesArray[i].at(defaultValueName).get_string_view().data());
            break;
        // TODO: add enum, vector
        default:
            m_Logger->Trace("Error while parsing property type, not supported");
        }

        m_Properties.AddProperty(name, std::move(prop));
    }
}



bool IsNullOrEmpty(const char* str)
{
    if (str == nullptr)
        return true;
    if (strnlen_s(str, 128) == 0)
        return true;

    return false;
}


bool CPostEffectBase::Load(const int shaderIndex, const char* resourcePath)
{
    if (m_ShadersSelection.size() > shaderIndex)
    {
        m_ShaderManager->Free(m_ShadersSelection[shaderIndex]);
    }

    bool lSuccess = true;
    //CGLSLShader* shader = CShaderFactory::NewShaderTyped<GLSLShader>(); // NewShader(ShaderType::GLSL);

    try
    {
        const std::string loadVertexPath = std::string(resourcePath) + GetVertexFname(shaderIndex);
        const std::string loadFragmentPath = std::string(resourcePath) + GetFragmentFname(shaderIndex);
        const std::string loadUIPath = std::string(resourcePath) + GetUIFName();

        const CReadFileScope vertexFile(loadVertexPath.c_str());
        const CReadFileScope fragmentFile(loadFragmentPath.c_str());

        const int newShader = m_ShaderManager->NewShader(vertexFile, fragmentFile);
        if (newShader < 0)
        {
            throw std::exception("failed to locate and load shader files");
        }

        if (m_ShadersSelection.size() > shaderIndex)
        {
            m_ShadersSelection[shaderIndex] = newShader;
        }
        else
        {
            m_ShadersSelection.push_back(newShader);
        }

        // TODO: load ui
        ParseUIFile(loadUIPath.c_str());
    }
    catch (const std::exception &e)
    {
        if (m_Logger)
            m_Logger->Trace("Post Effect %s ERROR: %s\n", GetName(), e.what());

        return false;
    }

    return lSuccess;
}

void PostProcessingEffects::CPostEffectBase::PrepareUniforms()
{
    Bind();

    // NOTE: at the moment only a direct assignment from property into uniform is supported

    for (int i = 0, count=m_Properties.GetCount(); i < count; ++i)
    {
        const SEffectProperty& prop = m_Properties[i];

        if (prop.uniform.empty())
            continue;

        switch (prop.valueType)
        {
        case EEffectType::NUMBER: m_ShaderManager->SetUniformFloat(GetShaderIndex(), prop.uniform.data(), prop.floatValue);
            break;
        case EEffectType::INT: m_ShaderManager->SetUniformUINT(GetShaderIndex(), prop.uniform.data(), prop.intValue);
            break;
        case EEffectType::VECTOR: m_ShaderManager->SetUniformVector(GetShaderIndex(), prop.uniform.data(), prop.vec3.x, prop.vec3.y, prop.vec3.z, 1.0f);
            break;
        }
    }

    UnBind();
}

void CPostEffectBase::Bind()
{
    m_ShaderManager->Bind(GetShaderIndex());
}
void CPostEffectBase::UnBind()
{
    m_ShaderManager->UnBind();
}