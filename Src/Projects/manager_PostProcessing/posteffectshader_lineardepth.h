
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

	//! prepare uniforms for a given variation of the effect
	virtual bool PrepUniforms(const int variationIndex);
	//! grab from UI all needed parameters to update effect state (uniforms) during evaluation
	virtual bool CollectUIValues(PostPersistentData* pData, PostEffectContext& effectContext);		//!< grab main UI values for the effect

	/// new feature to have several passes for a specified effect
	virtual const int GetNumberOfPasses() const;
	//! initialize a specific path for drawing
	virtual bool PrepPass(const int pass);

private:

	GLint	mLocDepthLinearizeClipInfo{ -1 };

	float clipInfo[4];
};
