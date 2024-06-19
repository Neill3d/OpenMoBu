
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: compositeMaster_shaders.h
//
//	Author Sergei Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/OpenMoBu
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- SDK include
#include <fbsdk/fbsdk.h>

#include <GL\glew.h>
#include "glslShader.h"
#include "Types.h"

//
/// GLSL shaders for composition components
//
// Note: zero vertex shader means that will use simple vertex shader

#define	COMPOSITE_FINAL_VERTEX		"\\GLSL\\simple.vsh"
#define COMPOSITE_FINAL_FRAGMENT	"\\GLSL\\compositeBlit.fsh"

#define	COMPOSITE_BLEND_VERTEX		0
#define COMPOSITE_BLEND_FRAGMENT	"\\GLSL\\compositeBlend.fsh"

#define	COMPOSITE_BLUR_VERTEX		0
#define COMPOSITE_BLUR_FRAGMENT		"\\GLSL\\compositeBlur.fsh"

#define COMPOSITE_CSB_VERTEX		0
#define COMPOSITE_CSB_FRAGMENT		"\\GLSL\\compositeCSB.fsh"

#define	COMPOSITE_HALFTONE_VERTEX	0
#define COMPOSITE_HALFTONE_FRAGMENT	"\\GLSL\\compositeHalfTone.fsh"

#define	COMPOSITE_DOF_VERTEX		0
#define COMPOSITE_DOF_FRAGMENT		"\\GLSL\\composite3dDOF.fsh"

#define	COMPOSITE_FOG_VERTEX		"\\GLSL\\composite3dFog.vsh"
#define COMPOSITE_FOG_FRAGMENT		"\\GLSL\\composite3dFog.fsh"

#define	COMPOSITE_MASK_VERTEX		0
#define COMPOSITE_MASK_FRAGMENT		"\\GLSL\\composite3dMask.fsh"

#define	COMPOSITE_TOONLINES_VERTEX		0
#define COMPOSITE_TOONLINES_FRAGMENT	"\\GLSL\\compositeToonLines.fsh"

#define	COMPOSITE_TOONLINES_MS_VERTEX	0
#define COMPOSITE_TOONLINES_MS_FRAGMENT	"\\GLSL\\compositeToonLinesMS.fsh"

#define	COMPOSITE_SSAO_VERTEX		"\\GLSL\\composite3dFog.vsh"
#define COMPOSITE_SSAO_FRAGMENT		"\\GLSL\\compositeSSAO.fsh"

#define	COMPOSITE_SSAO_MS_VERTEX	0
#define COMPOSITE_SSAO_MS_FRAGMENT	"\\GLSL\\compositeSSAOMS.fsh"

#define	COMPOSITE_PROCESS_NORMALS_VERTEX	0
#define COMPOSITE_PROCESS_NORMALS_FRAGMENT	"\\GLSL\\compositeProcessNormals.fsh"

#define	COMPOSITE_POSTERIZATION_VERTEX		0
#define COMPOSITE_POSTERIZATION_FRAGMENT	"\\GLSL\\compositePosterization.fsh"

#define	COMPOSITE_CHANGECOLOR_VERTEX		0
#define COMPOSITE_CHANGECOLOR_FRAGMENT		"\\GLSL\\compositeChangeColor.fsh"

#define	COMPOSITE_LUT_VERTEX				0
#define COMPOSITE_LUT_FRAGMENT				"\\GLSL\\compositeLUT.fsh"

#define	COMPOSITE_FILMGRAIN_VERTEX			0
#define COMPOSITE_FILMGRAIN_FRAGMENT		"\\GLSL\\compositeFilmGrain.fsh"

#define	COMPOSITE_FXAA_DEFAULT_VERTEX		"\\GLSL\\FXAA.vert"
#define COMPOSITE_FXAA_DEFAULT_FRAGMENT		"\\GLSL\\FXAA_Default.frag"

#define	COMPOSITE_FXAA_FASTEST_VERTEX		"\\GLSL\\FXAA.vert"
#define COMPOSITE_FXAA_FASTEST_FRAGMENT		"\\GLSL\\FXAA_Fastest.frag"

#define	COMPOSITE_FXAA_HIGH_VERTEX			"\\GLSL\\FXAA.vert"
#define COMPOSITE_FXAA_HIGH_FRAGMENT		"\\GLSL\\FXAA_High_Quality.frag"

#define	COMPOSITE_FXAA_EXTREME_VERTEX		"\\GLSL\\FXAA.vert"
#define COMPOSITE_FXAA_EXTREME_FRAGMENT		"\\GLSL\\FXAA_Extreme_Quality.frag"

#define	COMPOSITE_CROSSHATCH_VERTEX			0
#define COMPOSITE_CROSSHATCH_FRAGMENT		"\\GLSL\\compositeCrosshatch.fsh"

#define	COMPOSITE_SOLIDCOLOR_VERTEX			0
#define COMPOSITE_SOLIDCOLOR_FRAGMENT		"\\GLSL\\compositeSolidColor.fsh"

////////////////////////////////////////////////////
// shader locations

enum ECompositeBlendType
{
	eCompositeBlendNormal,
	eCompositeBlendLighten,
	eCompositeBlendDarken,
	eCompositeBlendMultiply,
	eCompositeBlendAverage,
	eCompositeBlendAdd,
	eCompositeBlendSubstract,
	eCompositeBlendDifference,
	eCompositeBlendNegation,
	eCompositeBlendExclusion,
	eCompositeBlendScreen,
	eCompositeBlendOverlay,
	eCompositeBlendSoftLight,
	eCompositeBlendHardLight,
	eCompositeBlendColorDodge,
	eCompositeBlendColorBurn,
	eCompositeBlendLinearDodge,
	eCompositeBlendLinearBurn,
	// Linear Light is another contrast-increasing mode
	// If the blend color is darker than midgray, Linear Light darkens the image by decreasing the brightness. If the blend color is lighter than midgray, the result is a brighter image due to increased brightness.
	eCompositeBlendLinearLight,
	eCompositeBlendVividLight,
	eCompositeBlendPinLight,
	eCompositeBlendHardMix,
	eCompositeBlendReflect,
	eCompositeBlendGlow,
	eCompositeBlendPhoenix
};

enum ECompositeShader
{
	eCompositeShaderBlit,
	eCompositeShaderBlendNormal,	// TODO: add 25 blend modes here
	eCompositeShaderBlendLighten,
	eCompositeShaderBlendDarken,
	eCompositeShaderBlendMultiply,
	eCompositeShaderBlendAverage,
	eCompositeShaderBlendAdd,
	eCompositeShaderBlendSubstract,
	eCompositeShaderBlendDifference,
	eCompositeShaderBlendNegation,
	eCompositeShaderBlendExclusion,
	eCompositeShaderBlendScreen,
	eCompositeShaderBlendOverlay,
	eCompositeShaderBlendSoftLight,
	eCompositeShaderBlendHardLight,
	eCompositeShaderBlendColorDodge,
	eCompositeShaderBlendColorBurn,
	eCompositeShaderBlendLinearDodge,
	eCompositeShaderBlendLinearBurn,
	eCompositeShaderBlendLinearLight,
	eCompositeShaderBlendVividLight,
	eCompositeShaderBlendPinLight,
	eCompositeShaderBlendHardMix,
	eCompositeShaderBlendReflect,
	eCompositeShaderBlendGlow,
	eCompositeShaderBlendPhoenix,
	eCompositeShaderBlur,
	eCompositeShaderCSB,
	eCompositeShaderHalfTone,
	eCompositeShaderDOF,
	eCompositeShaderFogNormal,
	eCompositeShaderFogLighten,
	eCompositeShaderFogDarken,
	eCompositeShaderFogMultiply,
	eCompositeShaderFogAverage,
	eCompositeShaderFogAdd,
	eCompositeShaderFogSubstract,
	eCompositeShaderFogDifference,
	eCompositeShaderFogNegation,
	eCompositeShaderFogExclusion,
	eCompositeShaderFogScreen,
	eCompositeShaderFogOverlay,
	eCompositeShaderFogSoftLight,
	eCompositeShaderFogHardLight,
	eCompositeShaderFogColorDodge,
	eCompositeShaderFogColorBurn,
	eCompositeShaderFogLinearDodge,
	eCompositeShaderFogLinearBurn,
	eCompositeShaderFogLinearLight,
	eCompositeShaderFogVividLight,
	eCompositeShaderFogPinLight,
	eCompositeShaderFogHardMix,
	eCompositeShaderFogReflect,
	eCompositeShaderFogGlow,
	eCompositeShaderFogPhoenix,
	eCompositeShaderMask,
	eCompositeShaderToonLines,
	eCompositeShaderToonLinesMS,
	eCompositeShaderSSAO,
	eCompositeShaderSSAOMS,
	eCompositeShaderProcessNormals,
	eCompositeShaderPosterization,
	eCompositeShaderChangeColor,
	eCompositeShaderLUT,
	eCompositeShaderFilmGrain,
	eCompositeShaderFXAA_Default,
	eCompositeShaderFXAA_Fastest,
	eCompositeShaderFXAA_High_Quality,
	eCompositeShaderFXAA_Extreme_Quality,
	eCompositeShaderCrosshatch,
	eCompositeShaderSolidColor,
	eCompositeShaderCount
};

FBString CompositeBlendTypeToString(const ECompositeBlendType mode);

const char *GetCompositeShaderVertexName( const ECompositeShader shader );
const char *GetCompositeShaderFragmentName( const ECompositeShader shader );


struct ShaderBaseLocations
{
	bool status{ false };

	GLint		maskSampler{ -1 };
	GLint		maskInverse{ -1 };

	ShaderBaseLocations()
	{
		status = false;
		maskSampler = 0;
		maskInverse = 0;
	}

	virtual ~ShaderBaseLocations() = default;

	void Init(GLSLShader *pShader, bool useMaskState)
	{
		if (useMaskState)
		{
			maskSampler = pShader->findLocation( "maskSampler" );
			maskInverse = pShader->findLocation( "maskInverse" );

			pShader->Bind();
			pShader->setUniformUINT(maskSampler, 7);
			pShader->setUniformFloat(maskInverse, 0.0f);
			pShader->UnBind();
		}
		else
		{
			maskSampler = 0;
			maskInverse = 0;
		}

		OnInit(pShader, useMaskState);
	}

	void SetMaskUniforms( const bool invertCompositeMask )
	{
		GLSLShader::setUniformFloat( maskInverse, (invertCompositeMask) ? 1.0f : 0.0f );
	}

	virtual void OnInit(GLSLShader *pShader, bool useMaskState)
	{
	}
};

struct	ShaderBlitLocations : public ShaderBaseLocations
{
	GLint		sampler{ 0 };

	virtual void OnInit(GLSLShader *pShader, bool useMaskState) override
	{
		if (pShader == nullptr)
		{
			status = false;
			return;
		}
		
		sampler = pShader->findLocation( "sampler" );
		
		pShader->Bind();
		
		if (sampler >= 0)
			pShader->setUniformUINT( sampler, 0 );

		pShader->UnBind();

		status = true;
	}
};

struct ShaderBlendLocations : public ShaderBaseLocations
{
	GLint		aSampler{ 0 };
	GLint		bSampler{ 0 };
	
	GLint		opacity{ 0 };

	GLint		doBlendWithAColor{ 0 };
	GLint		blendColor{ 0 };

	// composite mask
	GLint		maskSampler{ 0 };
	GLint		maskInverse{ 0 };

	virtual void OnInit(GLSLShader *pShader, bool useMaskState) override
	{
		if (pShader == nullptr)
		{
			status = false;
			return;
		}

		aSampler = pShader->findLocation( "aTex" );
		bSampler = pShader->findLocation( "bTex" );
		
		opacity = pShader->findLocation( "opacity" );
		
		doBlendWithAColor = pShader->findLocation( "doBlendWithAColor" );
		blendColor = pShader->findLocation( "blendColor" );

		maskSampler = pShader->findLocation( "maskSampler" );
		maskInverse = pShader->findLocation( "maskInverse" );

		pShader->Bind();
		
		if (aSampler >= 0)
			pShader->setUniformUINT( aSampler, 0 );
		if (bSampler >= 0)
			pShader->setUniformUINT( bSampler, 1 );
		if (maskSampler >= 0)
			pShader->setUniformUINT( maskSampler, 7 );
		pShader->UnBind();

		status = true;
	}

	void Set(const float _opacity, const bool _blendWithAColor, const float *_blendColor) const
	{
		GLSLShader::setUniformFloat( opacity, _opacity );
		GLSLShader::setUniformFloat( doBlendWithAColor, (_blendWithAColor) ? 1.0f : 0.0f );
		GLSLShader::setUniformVector( blendColor, _blendColor[0], _blendColor[1], _blendColor[2], _blendColor[3] );
	}
};

struct ShaderBlurLocations : public ShaderBaseLocations
{
	GLint		sampler{ 0 };
	GLint		scale{ 0 };

	virtual void OnInit(GLSLShader *pShader, bool useMaskState) override
	{
		if (pShader == nullptr)
		{
			status = false;
			return;
		}

		sampler = pShader->findLocation( "mainSampler" );
		scale = pShader->findLocation( "scale" );

		pShader->Bind();
		if (sampler >= 0)
			pShader->setUniformUINT( sampler, 0 );
		pShader->UnBind();

		status = true;
	}

	void Set(const float _x, const float _y)
	{
		GLSLShader::setUniformVector(scale, _x, _y, 0.0f, 1.0f);
	}
};

struct ShaderProcessNormalsLocations : public ShaderBaseLocations
{
	GLint		sampler{ 0 };
	GLint		colorX{ 0 };
	GLint		colorY{ 0 };
	GLint		colorZ{ 0 };

	virtual void OnInit(GLSLShader *pShader, bool useMaskState) override
	{
		if (pShader == nullptr)
		{
			status = false;
			return;
		}

		sampler = pShader->findLocation( "mainTex" );
		colorX = pShader->findLocation( "colorX" );
		colorY = pShader->findLocation( "colorY" );
		colorZ = pShader->findLocation( "colorZ" );

		pShader->Bind();
		if (sampler >= 0)
			pShader->setUniformUINT( sampler, 0 );
		pShader->UnBind();

		status = true;
	}

	void Set(const double *_x, const double *_y, const double *_z)
	{
		GLSLShader::setUniformVector(colorX, _x[0], _x[1], _x[2], 1.0f);
		GLSLShader::setUniformVector(colorY, _y[0], _y[1], _y[2], 1.0f);
		GLSLShader::setUniformVector(colorZ, _z[0], _z[1], _z[2], 1.0f);
	}
};

struct ShaderPosterizationLocations : public ShaderBaseLocations
{
	GLint		sampler{ 0 };
	GLint		numberOfColors{ 0 };
	GLint		gamma{ 0 };

	virtual void OnInit(GLSLShader *pShader, bool useMaskState) override
	{
		if (pShader == nullptr)
		{
			status = false;
			return;
		}

		sampler = pShader->findLocation( "mainSampler" );
		numberOfColors = pShader->findLocation( "numColors" );
		gamma = pShader->findLocation( "gamma" );

		pShader->Bind();
		if (sampler >= 0)
			pShader->setUniformUINT( sampler, 0 );
		pShader->UnBind();

		status = true;
	}

	void Set(const double _numberOfColors, const double _gamma)
	{
		GLSLShader::setUniformFloat(numberOfColors, _numberOfColors);
		GLSLShader::setUniformFloat(gamma, _gamma);
	}
};

struct ShaderChangeColorLocations : public ShaderBaseLocations
{
	GLint		sampler{ -1 };

	GLint		color1{ -1 };
	GLint		replace1{ -1 };
	GLint		weights1{ -1 };

	GLint		changeColor{ -1 };

	GLint		color2{ -1 };
	GLint		replace2{ -1 };
	GLint		weights2{ -1 };

	GLint		color3{ -1 };
	GLint		replace3{ -1 };
	GLint		weights3{ -1 };

	virtual void OnInit(GLSLShader *pShader, bool useMaskState) override
	{
		if (pShader == nullptr)
		{
			status = false;
			return;
		}

		sampler = pShader->findLocation( "mainSampler" );
		color1 = pShader->findLocation( "color1" );
		replace1 = pShader->findLocation( "replace1" );
		weights1 = pShader->findLocation( "weights1" );

		changeColor = pShader->findLocation( "flag" );

		color2 = pShader->findLocation( "color2" );
		replace2 = pShader->findLocation( "replace2" );
		weights2 = pShader->findLocation( "weights2" );

		color3 = pShader->findLocation( "color3" );
		replace3 = pShader->findLocation( "replace3" );
		weights3 = pShader->findLocation( "weights3" );

		pShader->Bind();
		if (sampler >= 0)
			pShader->setUniformUINT( sampler, 0 );
		pShader->UnBind();

		status = true;
	}

	void Set(const double *_color1, const double *_replace1, const double *_weights1, 
		const float _changeColor2, const double *_color2, const double *_replace2, const double *_weights2,
		const float _changeColor3, const double *_color3, const double *_replace3, const double *_weights3)
	{
		GLSLShader::setUniformVector(color1, (float)_color1[0], (float)_color1[1], (float)_color1[2], 1.0);
		GLSLShader::setUniformVector(replace1, (float)_replace1[0], (float)_replace1[1], (float)_replace1[2], 1.0);
		GLSLShader::setUniformVector(weights1, (float)_weights1[0], (float)_weights1[1], (float)_weights1[2], (float)_weights1[3]);

		// TODO: setUp changecolor 2,3
		if (changeColor >= 0)
		{
			GLSLShader::setUniformVector(changeColor, _changeColor2, _changeColor3, 0.0f, 0.0f );
		}

		if (_changeColor2 > 0)
		{
			GLSLShader::setUniformVector(color2, (float)_color2[0], (float)_color2[1], (float)_color2[2], 1.0);
			GLSLShader::setUniformVector(replace2, (float)_replace2[0], (float)_replace2[1], (float)_replace2[2], 1.0);
			GLSLShader::setUniformVector(weights2, (float)_weights2[0], (float)_weights2[1], (float)_weights2[2], (float)_weights2[3]);
		}
		if (_changeColor3 > 0)
		{
			GLSLShader::setUniformVector(color3, (float)_color3[0], (float)_color3[1], (float)_color3[2], 1.0);
			GLSLShader::setUniformVector(replace3, (float)_replace3[0], (float)_replace3[1], (float)_replace3[2], 1.0);
			GLSLShader::setUniformVector(weights3, (float)_weights3[0], (float)_weights3[1], (float)_weights3[2], (float)_weights3[3]);
		}
	}
};


struct ShaderLUTLocations : public ShaderBaseLocations
{
	GLint		sampler{ -1 };

	virtual void OnInit(GLSLShader *pShader, bool useMaskState) override
	{
		if (pShader == nullptr)
		{
			status = false;
			return;
		}

		sampler = pShader->findLocation( "mainSampler" );
		
		pShader->Bind();
		if (sampler >= 0)
			pShader->setUniformUINT( sampler, 0 );
		pShader->UnBind();

		status = true;
	}

	void Set()
	{
	}
};

struct ShaderFilmGrainLocations : public ShaderBaseLocations
{
	GLint		sampler{ -1 };

	GLint		textureWidth{ -1 };
	GLint		textureHeight{ -1 };
	GLint		timer{ -1 };

	GLint		grainAmount{ -1 };
	GLint		colored{ -1 };
	GLint		colorAmount{ -1 };
	GLint		grainSize{ -1 };
	GLint		lumAmount{ -1 };

	virtual void OnInit(GLSLShader *pShader, bool useMaskState) override
	{
		if (pShader == nullptr)
		{
			status = false;
			return;
		}

		sampler = pShader->findLocation( "mainSampler" );

		textureWidth = pShader->findLocation( "textureWidth" );
		textureHeight = pShader->findLocation( "textureHeight" );
		
		timer = pShader->findLocation( "timer" );

		grainAmount = pShader->findLocation( "grainamount" );
		colored = pShader->findLocation( "colored" );

		colorAmount = pShader->findLocation( "coloramount" );
		grainSize = pShader->findLocation( "grainsize" );
		lumAmount = pShader->findLocation( "lumamount" );

		pShader->Bind();
		if (sampler >= 0)
			pShader->setUniformUINT( sampler, 0 );
		pShader->UnBind();

		status = true;
	}

	void Set(const float _textureWidth, const float _textureHeight, const float _timer, 
		const float _grainamount, const float _colored, const float _coloramount, const float _grainsize, const float _lumamount)
	{
		GLSLShader::setUniformFloat(textureHeight, _textureWidth);
		GLSLShader::setUniformFloat(textureWidth, _textureHeight);
		GLSLShader::setUniformFloat(timer, _timer);

		GLSLShader::setUniformFloat(grainAmount, _grainamount);
		GLSLShader::setUniformFloat(colored, _colored);
		GLSLShader::setUniformFloat(colorAmount, _coloramount);
		GLSLShader::setUniformFloat(grainSize, _grainsize);
		GLSLShader::setUniformFloat(lumAmount, _lumamount);
	}
};


struct ShaderFXAALocations : public ShaderBaseLocations
{
	GLint		sampler{ -1 };

	// vec2
	// {x_} = 1.0/screenWidthInPixels
    // {_y} = 1.0/screenHeightInPixels
	GLint		RCPFrame{ -1 };

	virtual void OnInit(GLSLShader *pShader, bool useMaskState) override
	{
		if (pShader == nullptr)
		{
			status = false;
			return;
		}

		sampler = pShader->findLocation( "uSourceTex" );
		RCPFrame = pShader->findLocation( "RCPFrame" );

		pShader->Bind();
		if (sampler >= 0)
			pShader->setUniformUINT( sampler, 0 );
		pShader->UnBind();

		status = true;
	}

	void Set(const float _textureWidth, const float _textureHeight)
	{
		GLSLShader::setUniformVector2f(RCPFrame, _textureWidth, _textureHeight);
	}
};

struct ShaderCrossHatchLocations : public ShaderBaseLocations
{
	GLint		sampler{ -1 };

	virtual void OnInit(GLSLShader *pShader, bool useMaskState) override
	{
		if (pShader == nullptr)
		{
			status = false;
			return;
		}

		sampler = pShader->findLocation( "mainSampler" );
		
		pShader->Bind();
		if (sampler >= 0)
			pShader->setUniformUINT( sampler, 0 );
		pShader->UnBind();

		status = true;
	}

	void Set()
	{
	}
};

struct ShaderSolidColorLocations : public ShaderBaseLocations
{
	GLint		solidColor{ -1 };
	GLint		bottomColor{ -1 };
	GLint		gradient{ -1 };

	virtual void OnInit(GLSLShader *pShader, bool useMaskState) override
	{
		if (pShader == nullptr)
		{
			status = false;
			return;
		}

		solidColor = pShader->findLocation( "solidColor" );
		bottomColor = pShader->findLocation( "bottomColor" );
		gradient = pShader->findLocation( "gradient" );

		status = true;
	}

	void Set(float solidR, float solidG, float solidB, float solidA, float bottomR, float bottomG, float bottomB, float bottomA, const bool isGradient)
	{
		GLSLShader::setUniformVector( solidColor, solidR, solidG, solidB, solidA );
		GLSLShader::setUniformVector( bottomColor, bottomR, bottomG, bottomB, bottomA );
		GLSLShader::setUniformFloat( gradient, (isGradient) ? 1.0f : 0.0f );
	}
};

struct ShaderCSBLocations : public ShaderBaseLocations
{
	GLint		sampler{ -1 };
	GLint		csb{ -1 };
	GLint		hue{ -1 };
	
	virtual void OnInit(GLSLShader *pShader, bool maskState) override
	{
		if (pShader == nullptr)
		{
			status = false;
			return;
		}

		sampler = pShader->findLocation( "mainTex" );
		csb = pShader->findLocation( "gCSB" );
		hue = pShader->findLocation( "gHue" );
		
		status = true;
	}

	void Set(const float _contrast, const float _brightness, const float _saturation, const float _gamma, bool inverseState, float _hue, float _hueSat, float _lightness)
	{
		GLSLShader::setUniformVector( csb, _contrast, _saturation, _brightness, _gamma );
		GLSLShader::setUniformVector( hue, _hue, _hueSat, _lightness, (inverseState) ? 1.0f : 0.0f);
	}
};

struct ShaderHalfToneLocations : public ShaderBaseLocations
{
	GLint		sampler{ -1 };
	GLint		width{ -1 };
	GLint		height{ -1 };
	GLint		frequency{ -1 };

	virtual void OnInit(GLSLShader *pShader, bool useMaskState) override
	{
		if (pShader == nullptr)
		{
			status = false;
			return;
		}

		sampler = pShader->findLocation( "mainTex" );
		width = pShader->findLocation( "width" );
		height = pShader->findLocation( "height" );
		frequency = pShader->findLocation( "frequency" );

		status = true;
	}

	void Set( const float _width, const float _height, const float _frequency)
	{
		GLSLShader::setUniformFloat( width, _width );
		GLSLShader::setUniformFloat( height, _height );
		GLSLShader::setUniformFloat( frequency, _frequency );
	}
};

struct ShaderDOFLocations : public ShaderBaseLocations
{
	GLint				depthSampler{ -1 };
	GLint				colorSampler{ -1 };

	GLint				focalDistance{ -1 };
	GLint				focalRange{ -1 };
	GLint				FStop{ -1 };
	GLint				zNear{ -1 };
	GLint				zFar{ -1 };
	GLint				width{ -1 };
	GLint				height{ -1 };
	GLint				autoFocus{ -1 };
	GLint				blurForeground{ -1 };
	GLint				vignetting{ -1 };
	GLint				samples{ -1 };
	GLint				rings{ -1 };
	GLint				CoC{ -1 };
	GLint				threshold{ -1 };
	GLint				gain{ -1 };
	GLint				bias{ -1 };
	GLint				fringe{ -1 };
	GLint				noise{ -1 };

	GLint				pentagon{ -1 };
	GLint				feather{ -1 };
	GLint				logarithmic{ -1 };
	GLint				highQualityDepth{ -1 };

	virtual void OnInit(GLSLShader *pShader, bool useMaskState) override
	{
		if (pShader == nullptr)
		{
			status = false;
			return;
		}

		depthSampler = pShader->findLocation( "depthTex" );
		colorSampler = pShader->findLocation( "colorTex" );

		focalDistance = pShader->findLocation( "focalDistance" );
		focalRange = pShader->findLocation( "focalRange" );
		FStop = pShader->findLocation( "fstop" );
		zNear = pShader->findLocation( "zNear" );
		zFar = pShader->findLocation( "zFar" );
		width = pShader->findLocation( "textureWidth" );
		height = pShader->findLocation( "textureHeight" );
		autoFocus = pShader->findLocation( "autofocus" );
		blurForeground = pShader->findLocation( "blurForeground" );

		vignetting = pShader->findLocation( "vignetting" );
		samples = pShader->findLocation( "samples" );
		rings = pShader->findLocation( "rings" );
		CoC = pShader->findLocation( "CoC" );
		threshold = pShader->findLocation( "threshold" );
		gain = pShader->findLocation( "gain" );
		bias = pShader->findLocation( "bias" );
		fringe = pShader->findLocation( "fringe" );
		noise = pShader->findLocation( "noise" );
		pentagon = pShader->findLocation( "pentagon" );
		feather = pShader->findLocation( "feather" );

		logarithmic = pShader->findLocation( "logarithmicDepth" );
		highQualityDepth = pShader->findLocation( "highQualityDepth" );

		pShader->Bind();
		if (depthSampler >= 0)
			pShader->setUniformUINT( depthSampler, 1 );
		if (colorSampler >= 0)
			pShader->setUniformUINT( colorSampler, 0 );
		pShader->UnBind();

		status = true;
	}

	void Set( const float	_focalDistance,
						const float _focalRange,
						const float	_fstop,
						const float _width,
						const float _height,
						const int	_autoFocus,
						const float	_blurForeground,
						const int	_vignetting,
						const int	_samples,
						const int	_rings,
						const float _CoC,
						const float _threshold,
						const float _gain,
						const float _bias,
						const float _fringe,
						const int	_noise,
						const int	_pentagon,
						const float	_pentagonFeather,
						const float	_logarithmic,
						const float _highQualityDepth )
	{
		GLSLShader::setUniformFloat( focalDistance, _focalDistance );
		GLSLShader::setUniformFloat( focalRange, _focalRange );
		GLSLShader::setUniformFloat( FStop, _fstop );
		GLSLShader::setUniformFloat( width, _width );
		GLSLShader::setUniformFloat( height, _height );
		GLSLShader::setUniformUINT( autoFocus, _autoFocus );
		GLSLShader::setUniformFloat( blurForeground, _blurForeground );
		GLSLShader::setUniformUINT( vignetting, _vignetting );
		GLSLShader::setUniformUINT( samples, _samples );
		GLSLShader::setUniformUINT( rings, _rings );
		GLSLShader::setUniformFloat( CoC, _CoC );
		GLSLShader::setUniformFloat( threshold, _threshold );
		GLSLShader::setUniformFloat( gain, _gain );
		GLSLShader::setUniformFloat( bias, _bias );
		GLSLShader::setUniformFloat( fringe, _fringe );
		GLSLShader::setUniformUINT( noise, _noise );
		GLSLShader::setUniformUINT( pentagon, _pentagon );
		GLSLShader::setUniformFloat( feather, _pentagonFeather );
		GLSLShader::setUniformFloat( logarithmic, _logarithmic );
		GLSLShader::setUniformFloat( highQualityDepth, _highQualityDepth );
	}

	void Set( const float _zNear, const float _zFar )
	{
		GLSLShader::setUniformFloat( zNear, _zNear );
		GLSLShader::setUniformFloat( zFar, _zFar );
	}

	void Set( const int	_autoFocus, const float _focalDistance )
	{
		GLSLShader::setUniformUINT( autoFocus, _autoFocus );
		GLSLShader::setUniformFloat( focalDistance, _focalDistance );
	}
};

struct ShaderFogLocations : public ShaderBaseLocations
{
	GLint				alphaSampler{ -1 };
	GLint				alphaValid{ -1 };
	GLint				depthSampler{ -1 };
	GLint				colorSampler{ -1 };

	GLint				volumeObject{ -1 };

	GLint				color{ -1 };
	GLint				density{ -1 };
	GLint				zNear{ -1 };
	GLint				zFar{ -1 };
	GLint				fogNear{ -1 };
	GLint				fogFar{ -1 };
	GLint				fogBorder{ -1 };
	GLint				logarithmic{ -1 };
	GLint				highQualityDepth{ -1 };

	GLint				invModel{ -1 };
	GLint				volumeMin{ -1 };
	GLint				volumeMax{ -1 };

	virtual void OnInit(GLSLShader *pShader, bool useMaskState) override
	{
		if (pShader == nullptr)
		{
			status = false;
			return;
		}

		depthSampler = pShader->findLocation( "depthTex" );
		colorSampler = pShader->findLocation( "colorTex" );
		alphaSampler = pShader->findLocation( "alphaTex" );
		alphaValid = pShader->findLocation( "AlphaTextureValid" );
		
		volumeObject = pShader->findLocation( "volumeObject" );

		color = pShader->findLocation( "color" );
		density = pShader->findLocation( "density" );
		zNear = pShader->findLocation( "zNear" );
		zFar = pShader->findLocation( "zFar" );
		fogNear = pShader->findLocation( "fogNear" );
		fogFar = pShader->findLocation( "fogFar" );
		fogBorder = pShader->findLocation( "fogBorder" );
		logarithmic = pShader->findLocation( "logarithmicDepth" );
		highQualityDepth = pShader->findLocation( "highQualityDepth" );

		invModel = pShader->findLocation( "invVolumeMatrix" );

		volumeMin = pShader->findLocation( "volumeMin" );
		volumeMax = pShader->findLocation( "volumeMax" );

		pShader->Bind();

		GLint attrLoc;
		
		attrLoc = glGetAttribLocation( pShader->GetProgramObj(), "position" );
		if (attrLoc >= 0)
			glBindAttribLocation( pShader->GetProgramObj(), 0, "position" );
		attrLoc = glGetAttribLocation( pShader->GetProgramObj(), "texcoord" );
		if (attrLoc >= 0)
			glBindAttribLocation( pShader->GetProgramObj(), 1, "texcoord" );
		attrLoc = glGetAttribLocation( pShader->GetProgramObj(), "attrNear" );
		if (attrLoc >= 0)
			glBindAttribLocation( pShader->GetProgramObj(), 2, "attrNear" );
		attrLoc = glGetAttribLocation( pShader->GetProgramObj(), "attrFar" );
		if (attrLoc >= 0)
			glBindAttribLocation( pShader->GetProgramObj(), 3, "attrFar" );


		if (alphaSampler >= 0)
			pShader->setUniformUINT( alphaSampler, 2 );
		if (depthSampler >= 0)
			pShader->setUniformUINT( depthSampler, 1 );
		if (colorSampler >= 0)
			pShader->setUniformUINT( colorSampler, 0 );

		pShader->UnBind();

		status = true;
	}

	void Set( const int	_alphaValid )
	{
		GLSLShader::setUniformUINT( alphaValid, _alphaValid );
	}

	void Set( const float _r, const float _g, const float _b,
						const float _density,
						const float _znear,
						const float _zfar,
						const float _fognear,
						const float _fogfar,
						const float _fogborder,
						const float _logarithmic,
						const float _highQualityDepth,
						const float	_volumeObject )
	{
		GLSLShader::setUniformVector( color, _r, _g, _b, 1.0f );
		GLSLShader::setUniformFloat( density, _density );
		GLSLShader::setUniformFloat( zNear, _znear );
		GLSLShader::setUniformFloat( zFar, _zfar );
		GLSLShader::setUniformFloat( fogNear, _fognear );
		GLSLShader::setUniformFloat( fogFar, _fogfar );
		GLSLShader::setUniformFloat( fogBorder, _fogborder );
		GLSLShader::setUniformFloat( logarithmic, _logarithmic );
		GLSLShader::setUniformFloat( highQualityDepth, _highQualityDepth );
		GLSLShader::setUniformFloat( volumeObject, _volumeObject );
	}

	void Set( const float *invModelMatrix, const float *_volumeMin, const float *_volumeMax, const float *_color, const float _density, const float _feather )
	{
		if (invModel >= 0)
			GLSLShader::setUniformMatrix( invModel, invModelMatrix );

		if (volumeMin >= 0)
			GLSLShader::setUniformVector( volumeMin, _volumeMin[0], _volumeMin[1], _volumeMin[2], _volumeMin[3] );
		if (volumeMax >= 0)
			GLSLShader::setUniformVector( volumeMax, _volumeMax[0], _volumeMax[1], _volumeMax[2], _volumeMax[3] );

		if (color >= 0)
			GLSLShader::setUniformVector( color, _color[0], _color[1], _color[2], _color[3] );
		if (density >= 0)
			GLSLShader::setUniformFloat( density, _density );
		if (fogBorder >= 0)
			GLSLShader::setUniformFloat( fogBorder, _feather );
	}
};

struct ShaderMaskLocations : public ShaderBaseLocations
{
	GLint				maskSampler{ -1 };
	GLint				colorSampler{ -1 };

	virtual void OnInit(GLSLShader *pShader, bool useMaskState) override
	{
		if (pShader == nullptr)
		{
			status = false;
			return;
		}

		maskSampler = pShader->findLocation( "maskTex" );
		colorSampler = pShader->findLocation( "colorTex" );
		
		status = true;
	}
};

struct ShaderToonLinesLocations : public ShaderBaseLocations
{
	GLint				zNear{ -1 };
	GLint				zFar{ -1 };

	GLint				colorSampler{ -1 };		// non-AA texture
	GLint				depthSampler{ -1 };		// AA texture
	GLint				normalSampler{ -1 };	// AA texture

	GLint				numberOfSamples{ -1 };	// number of samples in AA

	GLint				screenWidth{ -1 };
	GLint				screenHeight{ -1 };

	GLint				falloff{ -1 };

	GLint				texCoordOffsets{ -1 };

	GLint				logarithmic{ -1 };
	GLint				highQualityDepth{ -1 };

	virtual void OnInit(GLSLShader *pShader, bool useMaskState) override
	{
		if (pShader == nullptr)
		{
			status = false;
			return;
		}

		zNear = pShader->findLocation( "zNear" );
		zFar = pShader->findLocation( "zFar" );

		colorSampler = pShader->findLocation( "colorSampler" );
		depthSampler = pShader->findLocation( "depthSampler" );
		normalSampler = pShader->findLocation( "normalSampler" );

		numberOfSamples = pShader->findLocation( "numberOfSamples" );

		screenWidth = pShader->findLocation( "SCREEN_WIDTH" );
		screenHeight = pShader->findLocation( "SCREEN_HEIGHT" );

		//loc.edgeForce = shader.findLocation( "edgeForce" );
		//loc.baseThresh = shader.findLocation( "baseThresh" );
		falloff = pShader->findLocation( "falloff" );

		texCoordOffsets = pShader->findLocation( "textureCoordOffset" );
		
		logarithmic = pShader->findLocation( "logarithmicDepth" );
		highQualityDepth = pShader->findLocation( "highQualityDepth" );

		pShader->Bind();

		if (colorSampler >= 0)
			pShader->setUniformUINT( colorSampler, 0 );
		if (depthSampler >= 0)
			pShader->setUniformUINT( depthSampler, 1 );
		if (normalSampler >= 0)
			pShader->setUniformUINT( normalSampler, 2 );

		pShader->UnBind();

		status = true;
	}

	void Set( const int count, const float *offsets )
	{
		glUniform2fv( texCoordOffsets, count, offsets );
	}

	void Set(	const float _znear, 
				const float _zfar,
				const float _falloff, 
				const float _logarithmic,
				const float _highQualityDepth,
				const int _numberOfSamples,
				const int _width,
				const int _height )
	{
		GLSLShader::setUniformFloat( zFar, _zfar );
		GLSLShader::setUniformFloat( zNear, _znear );
		GLSLShader::setUniformFloat( falloff, _falloff );
		
		GLSLShader::setUniformFloat( logarithmic, _logarithmic );
		GLSLShader::setUniformFloat( highQualityDepth, _highQualityDepth );

		GLSLShader::setUniformUINT( numberOfSamples, _numberOfSamples );
		GLSLShader::setUniformUINT( screenWidth, _width );
		GLSLShader::setUniformUINT( screenHeight, _height );
	}
};

struct ShaderSSAOLocations : public ShaderBaseLocations
{
	GLint				zNear{ -1 };
	GLint				zFar{ -1 };

	GLint				colorSampler{ -1 };
	GLint				depthSampler{ -1 };
	GLint				normalSampler{ -1 };

	GLint				distance{ -1 };
	GLint				filterRadius{ -1 };
	GLint				gamma{ -1 };
	GLint				contrast{ -1 };
	GLint				onlyAO{ -1 };
	
	GLint				invProj{ -1 };
	GLint				viewMatrix{ -1 };

	GLint				numberOfSamples{ -1 };	// number of samples in AA

	GLint				screenWidth{ -1 };
	GLint				screenHeight{ -1 };

	GLint				logarithmic{ -1 };
	GLint				highQualityDepth{ -1 };

	virtual void OnInit(GLSLShader *pShader, bool useMaskState) override
	{
		if (pShader == nullptr)
		{
			status = false;
			return;
		}

		zNear= pShader->findLocation( "zNear" );
		zFar = pShader->findLocation( "zFar" );

		colorSampler = pShader->findLocation( "colorSampler" );
		depthSampler = pShader->findLocation( "depthSampler" );
		normalSampler = pShader->findLocation( "normalSampler" );

		distance = pShader->findLocation( "distanceThreshold" );
		filterRadius = pShader->findLocation( "filterRadius" );
		gamma = pShader->findLocation( "gamma" );
		contrast = pShader->findLocation( "contrast" );
		onlyAO = pShader->findLocation( "only_ao" );
		invProj = pShader->findLocation( "g_matInvProjection" );
		viewMatrix = pShader->findLocation( "g_matViewMatrix" );

		numberOfSamples = pShader->findLocation( "numberOfSamples" );

		screenWidth = pShader->findLocation( "SCREEN_WIDTH" );
		screenHeight = pShader->findLocation( "SCREEN_HEIGHT" );

		logarithmic = pShader->findLocation( "logarithmicDepth" );
		highQualityDepth = pShader->findLocation( "highQualityDepth" );

		pShader->Bind();

		GLint attrLoc;
		
		attrLoc = glGetAttribLocation( pShader->GetProgramObj(), "position" );
		if (attrLoc >= 0)
			glBindAttribLocation( pShader->GetProgramObj(), 0, "position" );
		attrLoc = glGetAttribLocation( pShader->GetProgramObj(), "texcoord" );
		if (attrLoc >= 0)
			glBindAttribLocation( pShader->GetProgramObj(), 1, "texcoord" );
		attrLoc = glGetAttribLocation( pShader->GetProgramObj(), "attrNear" );
		if (attrLoc >= 0)
			glBindAttribLocation( pShader->GetProgramObj(), 2, "attrNear" );
		attrLoc = glGetAttribLocation( pShader->GetProgramObj(), "attrFar" );
		if (attrLoc >= 0)
			glBindAttribLocation( pShader->GetProgramObj(), 3, "attrFar" );

		if (colorSampler >= 0)
			pShader->setUniformUINT( colorSampler, 0 );
		if (depthSampler >= 0)
			pShader->setUniformUINT( depthSampler, 1 );
		if (normalSampler >= 0)
			pShader->setUniformUINT( normalSampler, 2 );

		pShader->UnBind();

		status = true;
	}

	void Set(	const float _distance,
				const float _filterX, 
				const float _filterY,
				const float _gamma,
				const float _contrast,
				const int	_onlyAO,
				const float	_znear,
				const float _zfar,
				const float *_invProjMatrix,
				const float *_viewMatrix,
				const float _logarithmic,
				const float _highQualityDepth )
	{
		GLSLShader::setUniformFloat( distance, _distance );
		GLSLShader::setUniformVector2f( filterRadius, _filterX, _filterY );
		GLSLShader::setUniformFloat( gamma, _gamma );
		GLSLShader::setUniformFloat( contrast, _contrast );
		GLSLShader::setUniformUINT( onlyAO, _onlyAO );
		GLSLShader::setUniformFloat( zNear, _znear );
		GLSLShader::setUniformFloat( zFar, _zfar );
		GLSLShader::setUniformMatrix( invProj, _invProjMatrix );
		GLSLShader::setUniformMatrix( viewMatrix, _viewMatrix );

		GLSLShader::setUniformFloat( logarithmic, _logarithmic );
		GLSLShader::setUniformFloat( highQualityDepth, _highQualityDepth );
	}

	void Set( const int	_numberOfSamples,
							const int	_width,
							const int	_height )
	{
		GLSLShader::setUniformUINT( numberOfSamples, _numberOfSamples );
		GLSLShader::setUniformUINT( screenWidth, _width );
		GLSLShader::setUniformUINT( screenHeight, _height );
	}
};



//////////////////////////////////////////////////////////////////////////////////////
//

class CompositeShaderManagerImpl : public Singleton<CompositeShaderManagerImpl>
{
public:

	//! a constructor
	CompositeShaderManagerImpl();

	//! a destructor
	~CompositeShaderManagerImpl();

	// get specified composite shader for drawing
	bool		CheckAndLoadShader( const ECompositeShader shader, const bool useMask );
	void		UnLoadShader( const ECompositeShader shader, const bool useMask );

	const GLSLShader	*GetShaderPtr( const ECompositeShader shader, const bool useMask );
	const ShaderBaseLocations *GetShaderLocationsPtr( const ECompositeShader shader, const bool useMask );

	// function to bind some shader
	bool	Bind( const ECompositeShader shader, const bool useMask );
	void	UnBind();

	// set some specified shader parameters
	void	SetMaskUniforms( const bool invertCompositeMask ) const;
	void	SetBlendUniforms(const float _opacity, const bool _blendWithAColor, const float *_blendColor) const;
	void	SetBlurUniforms( const float blur_h, const float blur_v ) const;
	void	SetProcessNormalsUniforms( const double *x, const double *y, const double *z ) const;
	void	SetPosterizationUniforms( const double numberOfColors, const double gamma ) const;

	void SetChangeColorUniforms(const double *_color1, const double *_replace1, const double *weights1,
		const float _changeColor2, const double *_color2, const double *_replace2, const double *weights2,
		const float _changeColor3, const double *_color3, const double *_replace3, const double *weights3) const;

	void SetFilmGrainUniforms(const float _textureWidth, const float _textureHeight, const float _timer,
		const float _grainamount, const float _colored, const float _coloramount, const float _grainsize, const float _lumamount) const;

	void SetFXAAUniforms(const float _textureWidth, const float _textureHeight) const;

	void SetLUTUniforms() const;
	void SetCrossHatchUniforms() const;

	void	SetCSBUniforms(const float _contrast, const float _brightness, const float _saturation, const float _gamma, bool inverse, float hue, float hueSat, float lightness) const;
	void	SetHalfToneUniforms(const float _width, const float _height, const float _frequency) const;

	void SetSolidColorUniforms(float solidR, float solidG, float solidB, float solidA, float bottomR, float bottomG, float bottomB, float bottomA, const bool isGradient) const;

	// 3d DOF

	void	SetDOFUniforms(	const float	_focalDistance,
							const float _focalRange,
							const float	_fstop,
							const float _width,
							const float _height,
							const int	_autoFocus,
							const float	_blurForeground,
							const int	_vignetting,
							const int	_samples,
							const int	_rings,
							const float _CoC,
							const float _threshold,
							const float _gain,
							const float _bias,
							const float _fringe,
							const int	_noise,
							const int	_pentagon,
							const float	_pentagonFeather,
							const float	_logarithmic,
							const float _highQualityDepth ) const;

	void	SetDOFUniforms( const float _zNear,
							const float _zFar ) const;

	void	SetDOFUniforms( const int _autoFocus, const float _focalDistance ) const;

	// 3d fog

	void	SetFogUniforms( const int _alphaValid ) const;

	void	SetFogUniforms( const float _r, const float _g, const float _b,
							const float _density,
							const float _znear,
							const float _zfar,
							const float _fognear,
							const float _fogfar,
							const float _fogborder,
							const float _logarithmic,
							const float _highDepthRange,
							const float _volumeObject ) const;

	void SetFogUniforms( const float *invModelMatrix, const float *volumeMin, const float *volumeMax, const float *color, const float density, const float feather ) const;

	// toon lines

	void	SetToonLinesOffsets( const int count, const float *offsets ) const;

	void	SetToonLinesUniforms(	const float _znear, const float _zfar, 
									const float _falloff, 
									const float _logarithmic,
									const float _highQualityDepth,
									const int _numberOfSamples,
									const int _width,
									const int _height ) const;

	// ssao

	void	SetSSAOUniforms(	const float _distance,
								const float _filterX, const float _filterY,
								const float _gamma,
								const float _contrast,
								const int	_onlyAO,
								const float	_znear,
								const float _zfar,
								const float *_invProjMatrix,
								const float *_viewMatrix,
								const float _logarithmic,
								const float _highQualityDepth ) const;

	void	SetSSAOUniforms(	const int	_numberOfSamples,
								const int	_width,
								const int	_height ) const;

private:

	//
	bool					mShaderUseMask[eCompositeShaderCount];

	// version with an undefined mask
	GLSLShader				*mShadersNoMask[eCompositeShaderCount];
	ShaderBaseLocations		*mShaderLocNoMask[eCompositeShaderCount];

	// version with a defined mask
	GLSLShader				*mShadersWithMask[eCompositeShaderCount];
	ShaderBaseLocations		*mShaderLocWithMask[eCompositeShaderCount];

	bool	InitShader(const char *vertex_filename, const char *fragment_filename, GLSLShader *&pShader, ShaderBaseLocations *&pLocations, bool useMask);
	void	FreeShader(GLSLShader *&pShader, ShaderBaseLocations *&pLocations);

	GLhandleARB	GetVertexShader();

	void	InitShaderLocations();

	ShaderBaseLocations *CreateShaderLocations(const ECompositeShader shader );

private:

	GLSLShader* mShader{ nullptr };
	ShaderBaseLocations* mLocations{ nullptr };
};

/////////////////////////////////////////////////////////////////////////////////////////
// CompositeShaderManager


class CompositeShaderManager
{
public:
	//! a constructor
	CompositeShaderManager()
	{
		impl = &CompositeShaderManagerImpl::instance();
	}
	//! a destructor
	~CompositeShaderManager()
	{
		//impl = &CompositeShaderManagerImpl::instance();
	}

	// get specified composite shader for drawing
	bool			CheckAndLoadShader( const ECompositeShader shader, bool useMask )
	{
		return impl->CheckAndLoadShader(shader, useMask);
	}

	void		UnLoadShader( const ECompositeShader shader, const bool useMask )
	{
		impl->UnLoadShader(shader, useMask);
	}

	const GLSLShader	*GetShaderPtr( const ECompositeShader shader, bool useMask )
	{
		return impl->GetShaderPtr(shader, useMask);
	}
	const ShaderBaseLocations *GetShaderLocationsPtr( const ECompositeShader shader, const bool useMask )
	{
		return impl->GetShaderLocationsPtr(shader, useMask);
	}

	// function to bind some shader
	bool	Bind( const ECompositeShader shader, const bool useMask ) const
	{
		return impl->Bind(shader, useMask);
	}
	void	UnBind() const
	{
		impl->UnBind();
	}

	// set some specified shader parameters
	void SetMaskUniforms( const bool invertCompositeMask ) const
	{
		impl->SetMaskUniforms(invertCompositeMask);
	}

	void	SetBlendUniforms(const float _opacity, const bool _blendWithAColor, const float *_blendColor) const
	{
		impl->SetBlendUniforms(_opacity, _blendWithAColor, _blendColor);
	}
	void	SetBlurUniforms( const float blur_h, const float blur_v ) const
	{
		impl->SetBlurUniforms(blur_h, blur_v);
	}
	
	void	SetProcessNormalsUniforms( const double *x, const double *y, const double *z ) const
	{
		impl->SetProcessNormalsUniforms(x, y, z);
	}
	
	void SetPosterizationUniforms(const double numberOfColors, const double gamma) const
	{
		impl->SetPosterizationUniforms(numberOfColors, gamma);
	}

	void SetChangeColorUniforms(const double *_color1, const double *_replace1, const double *_weights1,
		const float _changeColor2, const double *_color2, const double *_replace2, const double *_weights2,
		const float _changeColor3, const double *_color3, const double *_replace3, const double *_weights3) const
	{
		impl->SetChangeColorUniforms(_color1, _replace1, _weights1, _changeColor2, _color2, _replace2, _weights2, _changeColor3, _color3, _replace3, _weights3);
	}

	void SetFilmGrainUniforms(const float _textureWidth, const float _textureHeight, const float _timer,
		const float _grainamount, const float _colored, const float _coloramount, const float _grainsize, const float _lumamount) const
	{
		impl->SetFilmGrainUniforms(_textureWidth, _textureHeight, _timer, _grainamount, _colored, _coloramount, _grainsize, _lumamount);
	}

	void SetFXAAUniforms(const float _textureWidth, const float _textureHeight) const
	{
		impl->SetFXAAUniforms(_textureWidth, _textureHeight);
	}

	void SetLUTUniforms() const
	{
		impl->SetLUTUniforms();
	}

	void SetCrossHatchUniforms() const
	{
		impl->SetCrossHatchUniforms();
	}

	void SetSolidColorUniforms(float solidR, float solidG, float solidB, float solidA, float bottomR, float bottomG, float bottomB, float bottomA, const bool isGradient) const
	{
		impl->SetSolidColorUniforms(solidR, solidG, solidB, solidA, bottomR, bottomG, bottomB, bottomA, isGradient);
	}

	void	SetCSBUniforms(const float _contrast, const float _brightness, const float _saturation, const float _gamma, bool inverse, float hue, float hueSat, float lightness) const
	{
		impl->SetCSBUniforms(_contrast, _brightness, _saturation, _gamma, inverse, hue, hueSat, lightness );
	}
	void	SetHalfToneUniforms(const float _width, const float _height, const float _frequency) const
	{
		impl->SetHalfToneUniforms( _width, _height, _frequency );
	}

	// 3d DOF

	void	SetDOFUniforms(	const float	_focalDistance,
							const float _focalRange,
							const float	_fstop,
							const float _width,
							const float _height,
							const int	_autoFocus,
							const float	_blurForeground,
							const int	_vignetting,
							const int	_samples,
							const int	_rings,
							const float _CoC,
							const float _threshold,
							const float _gain,
							const float _bias,
							const float _fringe,
							const int	_noise,
							const int	_pentagon,
							const float	_pentagonFeather,
							const float	_logarithmic,
							const float _highQualityDepth ) const
	{
		impl->SetDOFUniforms( _focalDistance,
			_focalRange,
			_fstop,
			_width,
			_height,
			_autoFocus,
			_blurForeground,
			_vignetting,
			_samples,
			_rings,
			_CoC,
			_threshold,
			_gain,
			_bias,
			_fringe,
			_noise,
			_pentagon,
			_pentagonFeather,
			_logarithmic,
			_highQualityDepth );
	}

	void	SetDOFUniforms( const float _zNear,
							const float _zFar ) const
	{
		impl->SetDOFUniforms( _zNear, _zFar );
	}

	void	SetDOFUniforms( const int _autoFocus, const float _focalDistance ) const
	{
		impl->SetDOFUniforms( _autoFocus, _focalDistance );
	}

	// 3d fog

	void	SetFogUniforms( const int _alphaValid )
	{
		impl->SetFogUniforms( _alphaValid );
	}

	void	SetFogUniforms( const float _r, const float _g, const float _b,
							const float _density,
							const float _znear,
							const float _zfar,
							const float _fognear,
							const float _fogfar,
							const float _fogborder,
							const float _logarithmic,
							const float _highDepthRange,
							const float _volumeObject )
	{
		impl->SetFogUniforms( _r, _g, _b,
			_density,
			_znear,
			_zfar,
			_fognear,
			_fogfar,
			_fogborder,
			_logarithmic,
			_highDepthRange,
			_volumeObject );
	}

	void	SetFogUniforms( const float *invModelMatrix, const float *volumeMin, const float *volumeMax, const float *color, const float density, const float feather )
	{
		impl->SetFogUniforms(invModelMatrix, volumeMin, volumeMax, color, density, feather);
	}

	// toon lines

	void	SetToonLinesOffsets( const int count, const float *offsets )
	{
		impl->SetToonLinesOffsets( count, offsets);
	}

	void	SetToonLinesUniforms(	const float _znear, const float _zfar, 
									const float _falloff, 
									const float _logarithmic,
									const float _highQualityDepth,
									const int _numberOfSamples,
									const int _width,
									const int _height ) const
	{
		impl->SetToonLinesUniforms( _znear, _zfar,
			_falloff,
			_logarithmic,
			_highQualityDepth,
			_numberOfSamples,
			_width,
			_height );
	}

	// ssao

	void	SetSSAOUniforms(	const float _distance,
								const float _filterX, 
								const float _filterY,
								const float _gamma,
								const float _contrast,
								const int	_onlyAO,
								const float	_znear,
								const float _zfar,
								const float *_invProjMatrix,
								const float *_viewMatrix,
								const float _logarithmic,
								const float _highQualityDepth ) const
	{
		impl->SetSSAOUniforms( _distance,
			_filterX,
			_filterY,
			_gamma,
			_contrast,
			_onlyAO,
			_znear,
			_zfar,
			_invProjMatrix,
			_viewMatrix,
			_logarithmic,
			_highQualityDepth );
	}

	void	SetSSAOUniforms(	const int	_numberOfSamples,
								const int	_width,
								const int	_height ) const
	{
		impl->SetSSAOUniforms( _numberOfSamples, _width, _height );
	}

private:

	CompositeShaderManagerImpl* impl{ nullptr };
};