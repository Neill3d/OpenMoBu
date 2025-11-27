#pragma once

/**	\file	shaderpropertywriter.h

Sergei <Neill3d> Solokhin 2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

#include "shaderpropertystorage.h"
#include "posteffectbuffershader.h"
#include "posteffectcontext.h"
#include "effectshaderconnections.h"

/**
* Helper class to write shader property values conveniently
*/
class ShaderPropertyWriter
{
public:
    ShaderPropertyWriter(PostEffectBufferShader* shader, IPostEffectContext* context)
        : mStorage(context->GetShaderPropertyStorage())
        , mEffectHash(shader->GetNameHash())
        , mVariation(shader->GetCurrentShader())
    {
        mWriteMap = &mStorage->GetWritePropertyMap(mEffectHash);
    }

    // Overload for different property types
    template<typename... Args>
    ShaderPropertyWriter& operator()(IEffectShaderConnections::ShaderProperty* prop, Args&&... args)
    {
        if (prop && mStorage && mWriteMap)
        {
            IEffectShaderConnections::ShaderPropertyValue newValue(prop->GetDefaultValue());
            newValue.SetValue(std::forward<Args>(args)...);
            mWriteMap->emplace_back(std::move(newValue));
        }
        return *this;  // Allow chaining
    }

private:
    ShaderPropertyStorage* mStorage;
    ShaderPropertyStorage::PropertyValueMap* mWriteMap;
    uint32_t mEffectHash;
    int32_t mVariation;
};