
#pragma once

// posteffectshader_lineardepth
/*
Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "GL/glew.h"

#include "graphics_framebuffer.h"

#include "glslShaderProgram.h"
#include "Framebuffer.h"

#include "posteffectbase.h"

#include <memory>
#include <bitset>


//////////////////////////////


/// <summary>
/// one single fragment shader that we do one number of passes to process the input
/// </summary>
class PostEffectShaderLinearDepth : PostEffectBufferShader
{
public:

	PostEffectShaderLinearDepth();
	virtual ~PostEffectShaderLinearDepth();

	/// number of variations of the same effect, but with a different algorithm (for instance, 3 ways of making a lens flare effect)
	virtual int GetNumberOfVariations() const override { return 1; }

	//! an effect public name
	virtual const char* GetName() const override;
	//! get a filename of vertex shader, for this effect. returns a relative filename
	virtual const char* GetVertexFname(const int variationIndex) const override;
	//! get a filename of a fragment shader, for this effect, returns a relative filename
	virtual const char* GetFragmentFname(const int variationIndex) const override;

	/// new feature to have several passes for a specified effect
	virtual const int GetNumberOfPasses() const override;
	//! initialize a specific path for drawing
	virtual bool PrepPass(const int pass, int w, int h) override;

protected:
	//! prepare uniforms for a given variation of the effect
	virtual bool OnPrepareUniforms(const int variationIndex) override;
	//! grab from UI all needed parameters to update effect state (uniforms) during evaluation
	virtual bool OnCollectUI(PostPersistentData* pData, const PostEffectContext& effectContext, int maskIndex) override;		//!< grab main UI values for the effect


private:

	GLint	mLocDepthLinearizeClipInfo{ -1 };

	float clipInfo[4];
};
