
#pragma once

// posteffectdisplacement.h
/*
Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffectbase.h"
#include <random>

// forward
class PostPersistentData;

class EffectShaderDisplacement : public PostEffectBufferShader, public CommonEffectUniforms
{
private:
	static constexpr const char* SHADER_NAME = "Displacement";
	static constexpr const char* SHADER_VERTEX = "/GLSL/displacement.vsh";
	static constexpr const char* SHADER_FRAGMENT = "/GLSL/displacement.fsh";
	static constexpr const char* ENABLE_MASKING_PROPERTY_NAME = "Disp Use Masking";

public:

	EffectShaderDisplacement();
	virtual ~EffectShaderDisplacement();

	int GetNumberOfVariations() const override { return 1; }

	const char* GetName() const override;
	const char* GetVertexFname(const int shaderIndex) const override;
	const char* GetFragmentFname(const int shaderIndex) const override;

	virtual bool PrepUniforms(const int shaderIndex) override;
	virtual bool CollectUIValues(PostPersistentData* pData, const PostEffectContext& effectContext, int maskIndex) override;

	virtual void UploadUniforms(PostEffectBuffers* buffers, FrameBuffer* dstBuffer, int colorAttachment, const GLuint inputTextureId, int w, int h, bool generateMips) override;

protected:

	// shader locations
	enum { LOCATIONS_COUNT = 7 };
	union
	{
		struct
		{
			// locations

			GLint		iTime;
			GLint		iSpeed;

			GLint		useQuakeEffect;

			GLint		xDistMag;
			GLint		yDistMag;

			GLint		xSineCycles;
			GLint		ySineCycles;
		};

		GLint		arr[LOCATIONS_COUNT];
	} mLoc;

	struct ShaderData
	{
		float iTime;
		float iSpeed;
		float useQuakeEffect;
		float xDistMag;
		float yDistMag;
		float xSineCycles;
		float ySineCycles;
	} mData;
};

/// <summary>
/// effect with once shader - displacement, output directly to effects chain dst buffer
/// </summary>
typedef PostEffectSingleShader<EffectShaderDisplacement> PostEffectDisplacement;

/*
struct PostEffectDisplacement : public PostEffectSingleShader
{
public:

	//! a constructor
	PostEffectDisplacement();

	//! a destructor
	virtual ~PostEffectDisplacement();

	const char* GetEnableMaskPropertyName() const override { return "Disp Use Masking"; }
};*/