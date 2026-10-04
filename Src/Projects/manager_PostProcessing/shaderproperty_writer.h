#pragma once

/**	\file	shaderproperty_writer.h

Sergei <Neill3d> Solokhin 2025-2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

#include "shaderproperty.h"
#include "shaderproperty_scheme.h"
#include "shaderproperty_storage.h"
#include "posteffect_shader.h"
#include "posteffect_context.h"

/**
* Helper class to write shader property values conveniently
*/
class ShaderPropertyWriter
{
public:
    ShaderPropertyWriter(const PostEffectBufferShader* shader, PostEffectContextProxy* context)
        : mEffectHash(shader->GetNameHash())
        , mVariation(shader->GetCurrentVariation())
    {
        scheme = shader->GetPropertySchemePtr();
        mWriteMap = context->GetEffectPropertyValueMap(mEffectHash);
    }

    // Overload for different property types
    template<typename... Args>
    ShaderPropertyWriter& operator()(ShaderPropertyProxy propProxy, Args&&... args)
    {
        const ShaderProperty* prop = (scheme) ? scheme->GetProperty(propProxy) : nullptr;
        if (prop && mWriteMap)
        {
            ShaderPropertyValue newValue(prop->GetDefaultValue());
            newValue.SetValue(std::forward<Args>(args)...);
            mWriteMap->emplace_back(std::move(newValue));
        }
        return *this;  // Allow chaining
    }

private:
    const ShaderPropertyScheme* scheme{ nullptr };
    ShaderPropertyStorage::PropertyValueMap* mWriteMap{ nullptr };
    uint32_t mEffectHash;
    int32_t mVariation;
};