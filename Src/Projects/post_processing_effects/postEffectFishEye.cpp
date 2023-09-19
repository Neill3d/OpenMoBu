
#include "postEffectFishEye.h"
#include <magic_enum.hpp>
#include "json.hpp"
#include "glslShaderManager.h"

using namespace SharedGraphicsLibrary;
using namespace PostProcessingEffects;

//! a constructor
PostEffectFishEye::PostEffectFishEye(CLoggerCallback* logger, SharedGraphicsLibrary::CGLSLShaderManager* shaderManager)
    : CPostEffectBase(logger, shaderManager)
{
    constexpr size_t count{ magic_enum::enum_count<ShaderUniforms>() };
    static_assert(magic_enum::enum_count<ShaderUniforms>() == magic_enum::enum_count<PropertyNames>());

    m_Locations.resize(count, 0);
    m_PropertyIDs.resize(count, 0);
}

bool PostEffectFishEye::PrepUniforms(const int shaderIndex)
{
    const int shaderId = GetShaderIndex(shaderIndex);
    // TODO: maybe one look up to get the actual shader struct and to not have lookup for every individual call ?!
    {
        m_ShaderManager->Bind(shaderId);

        int loc = m_ShaderManager->FindLocation(shaderId, "sampler0");
        if (loc >= 0)
            m_ShaderManager->SetUniformUINT(loc, 0);
        
        constexpr size_t count = magic_enum::enum_count<ShaderUniforms>();
        for (int i = 0; i < count; ++i)
        {
            m_Locations[i] = m_ShaderManager->FindLocation(shaderId, magic_enum::enum_name(static_cast<ShaderUniforms>(i)).data());
        }

        m_ShaderManager->UnBind();
        return true;
    }

    return false;
}


bool PostEffectFishEye::CollectUIValues(UICallback* uiCallback, const SViewInfo& viewInfo)
{
    const double values[magic_enum::enum_count<ShaderUniforms>()] =
    {   
        0.01 * uiCallback->GetUIElementValue(m_PropertyIDs[0]), 
        1.0 - 0.01 * uiCallback->GetUIElementValue(m_PropertyIDs[1]), // lower clip level
        0.01 * uiCallback->GetUIElementValue(m_PropertyIDs[2]), 
        uiCallback->GetUIElementValue(m_PropertyIDs[3]), 
        uiCallback->GetUIElementValue(m_PropertyIDs[4]) 
    };
    
    //if (CGLSLShader* shader = GetShaderPtr())
    const int shaderId = GetShaderIndex();
    {
        m_ShaderManager->Bind(shaderId);

        constexpr size_t count = magic_enum::enum_count<ShaderUniforms>();
        for (int i = 0; i < count; ++i)
        {
            if (m_Locations[i] >= 0)
            {
                m_ShaderManager->SetUniformFloat(m_Locations[i], static_cast<float>(values[i]));
            }
        }

        m_ShaderManager->UnBind();
        return true;
    }

    return false;
}