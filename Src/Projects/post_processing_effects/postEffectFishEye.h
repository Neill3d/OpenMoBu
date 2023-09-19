#pragma once

#include "postEffectBase.h"
#include <vector>
#include <string_view>


namespace PostProcessingEffects
{
    
    /// <summary>
    /// Effect of image distortion "fish eye"
    /// </summary>
    class POST_PROCESSING_EFFECTS_API PostEffectFishEye : public CPostEffectBase
    {
    public:
        //! a constructor
        PostEffectFishEye(CLoggerCallback* logger, SharedGraphicsLibrary::CGLSLShaderManager* shaderManager);

        //! a destructor
        virtual ~PostEffectFishEye() {}

        int GetNumberOfShaders() const override { return 1; }

        const char* GetName() override { return NAME; }
        const char* GetVertexFname(const int shaderIndex) override { return VERTEX; }
        const char* GetFragmentFname(const int shaderIndex) override { return FRAGMENT; }
        const char* GetUIFName() { return UI; }

        bool PrepUniforms(const int shaderIndex) override;
        
        bool GenerateUI(bool skipCaching = true);
        bool CollectUIValues(UICallback* uiCallback, const SViewInfo& viewInfo) override;

    protected:

        static constexpr const char* NAME{ "Fish Eye" };
        static constexpr const char* VERTEX{ "\\GLSL\\fishEye.vert.glsl" };
        static constexpr const char* FRAGMENT{ "\\GLSL\\fishEye.frag.glsl" };
        static constexpr const char* UI{ "\\GLSL\\fishEye.ui.json" };

        // UI properties
        // TODO: what are properties type ?
        // property list with name, type
        // and correspodent uniform names

        

        enum class PropertyNames : int
        {
            // this is a general pre-defined
            UPPER_CLIP_LEVEL,
            LOWER_CLIP_LEVEL,

            // this is the shader specific
            FISH_EYE_AMOUNT,
            FISH_EYE_LENS_RADIUS,
            FISH_EYE_SIGN_CURVATURE
        };

        std::vector<int>     m_PropertyIDs;

        // shader uniforms

        enum class ShaderUniforms : int
        {
            upperClip,
            lowerClip,
            amount,
            lensradius,
            signcurvature
        };

        std::vector<int>   m_Locations;
    };
}