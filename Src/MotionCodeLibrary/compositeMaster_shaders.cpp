
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: compositeMaster_shaders.cpp
//
//	Author Sergei Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/OpenMoBu
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "compositeMaster_shaders.h"
//#include "compositeMaster_common.h"
//--- SDK include
#include <fbsdk/fbsdk.h>
#include <exception>
#include "FileUtils.h"

//////

const char * FBPropertyBaseEnum<ECompositeBlendType>::mStrings[] = {
	"Normal",
	"Lighten",
	"Darken",
	"Multiply",
	"Average",
	"Add",
	"Substract",
	"Difference",
	"Negation",
	"Exclusion",
	"Screen",
	"Overlay",
	"SoftLight",
	"HardLight",
	"ColorDodge",
	"ColorBurn",
	"LinearDodge",
	"LinearBurn",
	"LinearLight",
	"VividLight",
	"PinLight",
	"HardMix",
	"Reflect",
	"Glow",
	"Phoenix",
	0};

////////////////////////////////////////

const char *GetCompositeShaderVertexName( const ECompositeShader shader )
{
	switch (shader)
	{
	case eCompositeShaderBlit:
		return COMPOSITE_FINAL_VERTEX;

	case eCompositeShaderBlendNormal:	// TODO: add 25 blend modes here
	case eCompositeShaderBlendLighten:
	case eCompositeShaderBlendDarken:
	case eCompositeShaderBlendMultiply:
	case eCompositeShaderBlendAverage:
	case eCompositeShaderBlendAdd:
	case eCompositeShaderBlendSubstract:
	case eCompositeShaderBlendDifference:
	case eCompositeShaderBlendNegation:
	case eCompositeShaderBlendExclusion:
	case eCompositeShaderBlendScreen:
	case eCompositeShaderBlendOverlay:
	case eCompositeShaderBlendSoftLight:
	case eCompositeShaderBlendHardLight:
	case eCompositeShaderBlendColorDodge:
	case eCompositeShaderBlendColorBurn:
	case eCompositeShaderBlendLinearDodge:
	case eCompositeShaderBlendLinearBurn:
	case eCompositeShaderBlendLinearLight:
	case eCompositeShaderBlendVividLight:
	case eCompositeShaderBlendPinLight:
	case eCompositeShaderBlendHardMix:
	case eCompositeShaderBlendReflect:
	case eCompositeShaderBlendGlow:
	case eCompositeShaderBlendPhoenix:
		return COMPOSITE_BLEND_VERTEX;

	case eCompositeShaderBlur:
		return COMPOSITE_BLUR_VERTEX;

	case eCompositeShaderCSB:
		return COMPOSITE_CSB_VERTEX;

	case eCompositeShaderHalfTone:
		return COMPOSITE_HALFTONE_VERTEX;

	case eCompositeShaderDOF:
		return COMPOSITE_DOF_VERTEX;

	case eCompositeShaderFogNormal:
	case eCompositeShaderFogLighten:
	case eCompositeShaderFogDarken:
	case eCompositeShaderFogMultiply:
	case eCompositeShaderFogAverage:
	case eCompositeShaderFogAdd:
	case eCompositeShaderFogSubstract:
	case eCompositeShaderFogDifference:
	case eCompositeShaderFogNegation:
	case eCompositeShaderFogExclusion:
	case eCompositeShaderFogScreen:
	case eCompositeShaderFogOverlay:
	case eCompositeShaderFogSoftLight:
	case eCompositeShaderFogHardLight:
	case eCompositeShaderFogColorDodge:
	case eCompositeShaderFogColorBurn:
	case eCompositeShaderFogLinearDodge:
	case eCompositeShaderFogLinearBurn:
	case eCompositeShaderFogLinearLight:
	case eCompositeShaderFogVividLight:
	case eCompositeShaderFogPinLight:
	case eCompositeShaderFogHardMix:
	case eCompositeShaderFogReflect:
	case eCompositeShaderFogGlow:
	case eCompositeShaderFogPhoenix:
		return COMPOSITE_FOG_VERTEX;

	case eCompositeShaderMask:
		return COMPOSITE_MASK_VERTEX;

	case eCompositeShaderToonLines:
		return COMPOSITE_TOONLINES_VERTEX;
	case eCompositeShaderToonLinesMS:
		return COMPOSITE_TOONLINES_MS_VERTEX;

	case eCompositeShaderSSAO:
		return COMPOSITE_SSAO_VERTEX;
	case eCompositeShaderSSAOMS:
		return COMPOSITE_SSAO_MS_VERTEX;
	case eCompositeShaderProcessNormals:
		return COMPOSITE_PROCESS_NORMALS_VERTEX;
	case eCompositeShaderPosterization:
		return COMPOSITE_POSTERIZATION_VERTEX;
	case eCompositeShaderChangeColor:
		return COMPOSITE_CHANGECOLOR_VERTEX;
	case eCompositeShaderLUT:
		return COMPOSITE_LUT_VERTEX;
	case eCompositeShaderFilmGrain:
		return COMPOSITE_FILMGRAIN_VERTEX;

	case eCompositeShaderFXAA_Default:
		return COMPOSITE_FXAA_DEFAULT_VERTEX;
	case eCompositeShaderFXAA_Fastest:
		return COMPOSITE_FXAA_FASTEST_VERTEX;
	case eCompositeShaderFXAA_High_Quality:
		return COMPOSITE_FXAA_HIGH_VERTEX;
	case eCompositeShaderFXAA_Extreme_Quality:
		return COMPOSITE_FXAA_EXTREME_VERTEX;

	case eCompositeShaderCrosshatch:
		return COMPOSITE_CROSSHATCH_VERTEX;
	case eCompositeShaderSolidColor:
		return COMPOSITE_SOLIDCOLOR_VERTEX;
	}

	return 0;
}
const char *GetCompositeShaderFragmentName( const ECompositeShader shader )
{
	switch (shader)
	{
	case eCompositeShaderBlit:
		return COMPOSITE_FINAL_FRAGMENT;

	case eCompositeShaderBlendNormal:	// TODO: add 25 blend modes here
	case eCompositeShaderBlendLighten:
	case eCompositeShaderBlendDarken:
	case eCompositeShaderBlendMultiply:
	case eCompositeShaderBlendAverage:
	case eCompositeShaderBlendAdd:
	case eCompositeShaderBlendSubstract:
	case eCompositeShaderBlendDifference:
	case eCompositeShaderBlendNegation:
	case eCompositeShaderBlendExclusion:
	case eCompositeShaderBlendScreen:
	case eCompositeShaderBlendOverlay:
	case eCompositeShaderBlendSoftLight:
	case eCompositeShaderBlendHardLight:
	case eCompositeShaderBlendColorDodge:
	case eCompositeShaderBlendColorBurn:
	case eCompositeShaderBlendLinearDodge:
	case eCompositeShaderBlendLinearBurn:
	case eCompositeShaderBlendLinearLight:
	case eCompositeShaderBlendVividLight:
	case eCompositeShaderBlendPinLight:
	case eCompositeShaderBlendHardMix:
	case eCompositeShaderBlendReflect:
	case eCompositeShaderBlendGlow:
	case eCompositeShaderBlendPhoenix:
		return COMPOSITE_BLEND_FRAGMENT;

	case eCompositeShaderBlur:
		return COMPOSITE_BLUR_FRAGMENT;

	case eCompositeShaderCSB:
		return COMPOSITE_CSB_FRAGMENT;

	case eCompositeShaderHalfTone:
		return COMPOSITE_HALFTONE_FRAGMENT;

	case eCompositeShaderDOF:
		return COMPOSITE_DOF_FRAGMENT;

	case eCompositeShaderFogNormal:
	case eCompositeShaderFogLighten:
	case eCompositeShaderFogDarken:
	case eCompositeShaderFogMultiply:
	case eCompositeShaderFogAverage:
	case eCompositeShaderFogAdd:
	case eCompositeShaderFogSubstract:
	case eCompositeShaderFogDifference:
	case eCompositeShaderFogNegation:
	case eCompositeShaderFogExclusion:
	case eCompositeShaderFogScreen:
	case eCompositeShaderFogOverlay:
	case eCompositeShaderFogSoftLight:
	case eCompositeShaderFogHardLight:
	case eCompositeShaderFogColorDodge:
	case eCompositeShaderFogColorBurn:
	case eCompositeShaderFogLinearDodge:
	case eCompositeShaderFogLinearBurn:
	case eCompositeShaderFogLinearLight:
	case eCompositeShaderFogVividLight:
	case eCompositeShaderFogPinLight:
	case eCompositeShaderFogHardMix:
	case eCompositeShaderFogReflect:
	case eCompositeShaderFogGlow:
	case eCompositeShaderFogPhoenix:
		return COMPOSITE_FOG_FRAGMENT;

	case eCompositeShaderMask:
		return COMPOSITE_MASK_FRAGMENT;

	case eCompositeShaderToonLines:
		return COMPOSITE_TOONLINES_FRAGMENT;
	case eCompositeShaderToonLinesMS:
		return COMPOSITE_TOONLINES_MS_FRAGMENT;

	case eCompositeShaderSSAO:
		return COMPOSITE_SSAO_FRAGMENT;
	case eCompositeShaderSSAOMS:
		return COMPOSITE_SSAO_MS_FRAGMENT;
	case eCompositeShaderProcessNormals:
		return COMPOSITE_PROCESS_NORMALS_FRAGMENT;
	case eCompositeShaderPosterization:
		return COMPOSITE_POSTERIZATION_FRAGMENT;
	case eCompositeShaderChangeColor:
		return COMPOSITE_CHANGECOLOR_FRAGMENT;
	case eCompositeShaderLUT:
		return COMPOSITE_LUT_FRAGMENT;
	case eCompositeShaderFilmGrain:
		return COMPOSITE_FILMGRAIN_FRAGMENT;

	case eCompositeShaderFXAA_Default:
		return COMPOSITE_FXAA_DEFAULT_FRAGMENT;
	case eCompositeShaderFXAA_Fastest:
		return COMPOSITE_FXAA_FASTEST_FRAGMENT;
	case eCompositeShaderFXAA_High_Quality:
		return COMPOSITE_FXAA_HIGH_FRAGMENT;
	case eCompositeShaderFXAA_Extreme_Quality:
		return COMPOSITE_FXAA_EXTREME_FRAGMENT;

	case eCompositeShaderCrosshatch:
		return COMPOSITE_CROSSHATCH_FRAGMENT;
	case eCompositeShaderSolidColor:
		return COMPOSITE_SOLIDCOLOR_FRAGMENT;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////
//

CompositeShaderManagerImpl::CompositeShaderManagerImpl()
{
	for (int i=0; i<eCompositeShaderCount; ++i)
	{
		mShaderUseMask[i] = false;

		mShadersNoMask[i] = nullptr;
		mShaderLocNoMask[i] = nullptr;
		
		mShadersWithMask[i] = nullptr;
		mShaderLocWithMask[i] = nullptr;
	}
}

//! a destructor
CompositeShaderManagerImpl::~CompositeShaderManagerImpl()
{
	for (int i=0; i<eCompositeShaderCount; ++i)
	{
		FreeShader(mShadersNoMask[i], mShaderLocNoMask[i]);
		FreeShader(mShadersWithMask[i], mShaderLocWithMask[i]);
	}
}

ShaderBaseLocations *CompositeShaderManagerImpl::CreateShaderLocations(const ECompositeShader shader)
{
	switch(shader)
	{
	case eCompositeShaderBlit: 
		return new ShaderBlitLocations();

	case eCompositeShaderBlendNormal: 
	case eCompositeShaderBlendLighten:
	case eCompositeShaderBlendDarken:
	case eCompositeShaderBlendMultiply:
	case eCompositeShaderBlendAverage:
	case eCompositeShaderBlendAdd:
	case eCompositeShaderBlendSubstract:
	case eCompositeShaderBlendDifference:
	case eCompositeShaderBlendNegation:
	case eCompositeShaderBlendExclusion:
	case eCompositeShaderBlendScreen:
	case eCompositeShaderBlendOverlay:
	case eCompositeShaderBlendSoftLight:
	case eCompositeShaderBlendHardLight:
	case eCompositeShaderBlendColorDodge:
	case eCompositeShaderBlendColorBurn:
	case eCompositeShaderBlendLinearDodge:
	case eCompositeShaderBlendLinearBurn:
	case eCompositeShaderBlendLinearLight:
	case eCompositeShaderBlendVividLight:
	case eCompositeShaderBlendPinLight:
	case eCompositeShaderBlendHardMix:
	case eCompositeShaderBlendReflect:
	case eCompositeShaderBlendGlow:
	case eCompositeShaderBlendPhoenix:
		return new ShaderBlendLocations();

	case eCompositeShaderBlur: return new ShaderBlurLocations();
	case eCompositeShaderProcessNormals: return new ShaderProcessNormalsLocations();
	case eCompositeShaderPosterization: return new ShaderPosterizationLocations();
	case eCompositeShaderChangeColor: return new ShaderChangeColorLocations();
	case eCompositeShaderFilmGrain: return new ShaderFilmGrainLocations();

	case eCompositeShaderCSB: return new ShaderCSBLocations();
	case eCompositeShaderHalfTone: return new ShaderHalfToneLocations();
	case eCompositeShaderDOF: return new ShaderDOFLocations();

	case eCompositeShaderFogNormal:
	case eCompositeShaderFogLighten:
	case eCompositeShaderFogDarken:
	case eCompositeShaderFogMultiply:
	case eCompositeShaderFogAverage:
	case eCompositeShaderFogAdd:
	case eCompositeShaderFogSubstract:
	case eCompositeShaderFogDifference:
	case eCompositeShaderFogNegation:
	case eCompositeShaderFogExclusion:
	case eCompositeShaderFogScreen:
	case eCompositeShaderFogOverlay:
	case eCompositeShaderFogSoftLight:
	case eCompositeShaderFogHardLight:
	case eCompositeShaderFogColorDodge:
	case eCompositeShaderFogColorBurn:
	case eCompositeShaderFogLinearDodge:
	case eCompositeShaderFogLinearBurn:
	case eCompositeShaderFogLinearLight:
	case eCompositeShaderFogVividLight:
	case eCompositeShaderFogPinLight:
	case eCompositeShaderFogHardMix:
	case eCompositeShaderFogReflect:
	case eCompositeShaderFogGlow:
	case eCompositeShaderFogPhoenix:
		return new ShaderFogLocations();

	case eCompositeShaderMask: 
		return new ShaderMaskLocations();

	case eCompositeShaderToonLines: 
	case eCompositeShaderToonLinesMS: 
		return new ShaderToonLinesLocations();

	case eCompositeShaderSSAO: 
	case eCompositeShaderSSAOMS: 
		return new ShaderSSAOLocations();

	case eCompositeShaderFXAA_Default:
	case eCompositeShaderFXAA_Fastest:
	case eCompositeShaderFXAA_High_Quality:
	case eCompositeShaderFXAA_Extreme_Quality:
		return new ShaderFXAALocations();

	case eCompositeShaderCrosshatch:
		return new ShaderCrossHatchLocations();
	case eCompositeShaderSolidColor:
		return new ShaderSolidColorLocations();
	}

	return nullptr;
}


FBString CompositeBlendTypeToString(const ECompositeBlendType mode)
{
	FBString result("#define BlendOperation				BlendMultiply\n");

	switch (mode)
	{
	case eCompositeBlendNormal: result = "#define BlendOperation				BlendNormal\n";
		break;
	case eCompositeBlendLighten: result = "#define BlendOperation(base,blend)				BlendLighten(base,blend)\n";
		break;
	case eCompositeBlendDarken: result = "#define BlendOperation(base,blend)				BlendDarken(base,blend)\n";
		break;
	case eCompositeBlendMultiply: result = "#define BlendOperation				BlendMultiply\n";
		break;
	case eCompositeBlendAverage: result = "#define BlendOperation(base,blend)				BlendAverage(base,blend)\n";
		break;
	case eCompositeBlendAdd: result = "#define BlendOperation(base,blend)				BlendAdd(base,blend)\n";
		break;
	case eCompositeBlendSubstract: result = "#define BlendOperation(base,blend)				BlendSubstract(base,blend)\n";
		break;
	case eCompositeBlendDifference: result = "#define BlendOperation(base,blend)				BlendDifference(base,blend)\n";
		break;
	case eCompositeBlendNegation: result = "#define BlendOperation(base,blend)				BlendNegation(base,blend)\n";
		break;
	case eCompositeBlendExclusion: result = "#define BlendOperation(base,blend)				BlendExclusion(base,blend)\n";
		break;
	case eCompositeBlendScreen: result = "#define BlendOperation(base,blend)				BlendScreen(base,blend)\n";
		break;
	case eCompositeBlendOverlay: result = "#define BlendOperation(base,blend)				BlendOverlay(base,blend)\n";
		break;
	case eCompositeBlendSoftLight: result = "#define BlendOperation(base,blend)				BlendSoftLight(base,blend)\n";
		break;
	case eCompositeBlendHardLight: result = "#define BlendOperation(base,blend)				BlendHardLight(base,blend)\n";
		break;
	case eCompositeBlendColorDodge: result = "#define BlendOperation(base,blend)				BlendColorDodge(base,blend)\n";
		break;
	case eCompositeBlendColorBurn: result = "#define BlendOperation(base,blend)				BlendColorBurn(base,blend)\n";
		break;
	case eCompositeBlendLinearDodge: result = "#define BlendOperation(base,blend)				BlendLinearDodge(base,blend)\n";
		break;
	case eCompositeBlendLinearBurn: result = "#define BlendOperation(base,blend)				BlendLinearBurn(base,blend)\n";
		break;
	case eCompositeBlendLinearLight: result = "#define BlendOperation(base,blend)				BlendLinearLight(base,blend)\n";
		break;
	case eCompositeBlendVividLight: result = "#define BlendOperation(base,blend)				BlendVividLight(base,blend)\n";
		break;
	case eCompositeBlendPinLight: result = "#define BlendOperation(base,blend)				BlendPinLight(base,blend)\n";
		break;
	case eCompositeBlendHardMix: result = "#define BlendOperation(base,blend)				BlendHardMix(base,blend)\n";
		break;
	case eCompositeBlendReflect: result = "#define BlendOperation(base,blend)				BlendReflect(base,blend)\n";
		break;
	case eCompositeBlendGlow: result = "#define BlendOperation(base,blend)				BlendGlow(base,blend)\n";
		break;
	case eCompositeBlendPhoenix: result = "#define BlendOperation(base,blend)				BlendPhoenix(base,blend)\n";
		break;
	}

	return result;
}

void CompositeShaderManagerImpl::UnLoadShader( const ECompositeShader shader, const bool useMask )
{
	if (useMask)
	{
		FreeShader(mShadersWithMask[shader], mShaderLocWithMask[shader]);
	}
	else
	{
		FreeShader(mShadersNoMask[shader], mShaderLocNoMask[shader]);
	}
}

bool CompositeShaderManagerImpl::CheckAndLoadShader( const ECompositeShader shader, const bool useMask )
{
	return (nullptr != GetShaderPtr(shader, useMask));
}

const GLSLShaderProgram* CompositeShaderManagerImpl::GetShaderPtr( const ECompositeShader shader, const bool useMask )
{
	const int32_t shaderIndex = static_cast<int32_t>(shader);

	if (shaderIndex < 0 || shaderIndex >= ECompositeShader::eCompositeShaderCount)
		return nullptr;

	GLSLShaderProgram* pShader = nullptr;

	if (useMask)
	{
		pShader = mShadersWithMask[shaderIndex];
	}
	else
	{
		pShader = mShadersNoMask[shaderIndex];
	}

	
	if (pShader == nullptr)
	{
		UnLoadShader(shader, useMask);

		pShader = new GLSLShaderProgram();
		ShaderBaseLocations *pLocations = CreateShaderLocations(shader);
		mShaderUseMask[shaderIndex] = useMask;

		FBString strHeader("#version 120\n");
		if (shader >= eCompositeShaderBlendNormal && shader <= eCompositeShaderBlendPhoenix)
			strHeader = strHeader + CompositeBlendTypeToString( (ECompositeBlendType) (shaderIndex - (int)eCompositeShaderBlendNormal) );
		else if (shader >= eCompositeShaderFogNormal && shader <= eCompositeShaderFogPhoenix)
			strHeader = strHeader + CompositeBlendTypeToString( (ECompositeBlendType) (shaderIndex - (int)eCompositeShaderFogNormal) );
		
		if (useMask)
			strHeader = strHeader + "#define USE_MASK\n";

		pShader->SetHeaderText(strHeader);

		if (true == InitShader( GetCompositeShaderVertexName(shader), GetCompositeShaderFragmentName(shader), pShader, pLocations, useMask ) )
		{
			if (useMask)
			{
				mShadersWithMask[shaderIndex] = pShader;
				mShaderLocWithMask[shaderIndex] = pLocations;
			}
			else
			{
				mShadersNoMask[shaderIndex] = pShader;
				mShaderLocNoMask[shaderIndex] = pLocations;
			}
		}
	}

	return pShader;
}

const ShaderBaseLocations *CompositeShaderManagerImpl::GetShaderLocationsPtr( const ECompositeShader shader, const bool useMask )
{
	if (useMask)
	{
		return mShaderLocWithMask[shader];
	}
	else
	{
		return mShaderLocNoMask[shader];
	}
	
}

GLhandleARB CompositeShaderManagerImpl::GetVertexShader()
{
	const GLSLShaderProgram *pShader = GetShaderPtr( eCompositeShaderBlit, mShaderUseMask[eCompositeShaderBlit] );
	if (pShader)
	{
		return pShader->GetVertexShader();
	}

	return 0;
}

bool CompositeShaderManagerImpl::Bind( const ECompositeShader shader, const bool useMask )
{
	mShader = const_cast<GLSLShaderProgram*>(GetShaderPtr( shader, useMask ));
	mLocations = const_cast<ShaderBaseLocations*>(GetShaderLocationsPtr( shader, useMask ));

	if (mShader)
		mShader->Bind();

	return (mShader != nullptr);
}

void CompositeShaderManagerImpl::UnBind()
{
	if (mShader)
	{
		mShader->UnBind();

		mShader = nullptr;
		mLocations = nullptr;
	}
}

void CompositeShaderManagerImpl::SetMaskUniforms( const bool invertCompositeMask ) const
{
	if (mLocations)
	{
		mLocations->SetMaskUniforms(invertCompositeMask);
	}
}

void CompositeShaderManagerImpl::SetBlendUniforms(const float _opacity, const bool _blendWithAColor, const float *_blendColor) const
{
	if (mLocations)
	{
		( (ShaderBlendLocations*) mLocations)->Set( _opacity, _blendWithAColor, _blendColor );
	}
}

void CompositeShaderManagerImpl::SetBlurUniforms( const float blur_h, const float blur_v ) const
{
	if (mLocations)
	{
		( (ShaderBlurLocations*) mLocations)->Set( blur_h, blur_v );
	}
}

void CompositeShaderManagerImpl::SetProcessNormalsUniforms( const double *x, const double *y, const double *z ) const
{
	if (mLocations)
	{
		( (ShaderProcessNormalsLocations*) mLocations)->Set( x, y, z );
	}
}

void CompositeShaderManagerImpl::SetPosterizationUniforms( const double numberOfColors, const double gamma ) const
{
	if (mLocations)
	{
		( (ShaderPosterizationLocations*) mLocations)->Set( numberOfColors, gamma );
	}
}

void CompositeShaderManagerImpl::SetChangeColorUniforms(const double *_color1, const double *_replace1, const double *_weights1,
		const float _changeColor2, const double *_color2, const double *_replace2, const double *_weights2,
		const float _changeColor3, const double *_color3, const double *_replace3, const double *_weights3) const
{
	if (mLocations)
	{
		( (ShaderChangeColorLocations*) mLocations)->Set( _color1, _replace1, _weights1, _changeColor2, _color2, _replace2, _weights2, _changeColor3, _color3, _replace3, _weights3 );
	}
}

void CompositeShaderManagerImpl::SetFilmGrainUniforms(const float _textureWidth, const float _textureHeight, const float _timer,
	const float _grainamount, const float _colored, const float _coloramount, const float _grainsize, const float _lumamount) const
{
	if (mLocations)
	{
		( (ShaderFilmGrainLocations*) mLocations)->Set(_textureWidth, _textureHeight, _timer, _grainamount, _colored, _coloramount, _grainsize, _lumamount);
	}
}

void CompositeShaderManagerImpl::SetFXAAUniforms(const float _textureWidth, const float _textureHeight) const
{
	if (mLocations)
	{
		( (ShaderFXAALocations*) mLocations)->Set(_textureWidth, _textureHeight);
	}
}

void CompositeShaderManagerImpl::SetLUTUniforms() const
{
	if (mLocations)
	{
		( (ShaderLUTLocations*) mLocations)->Set();
	}
}

void CompositeShaderManagerImpl::SetCSBUniforms(const float _contrast, const float _brightness, const float _saturation, const float _gamma, bool inverse, float hue, float hueSat, float lightness) const
{
	if (mLocations)
	{
		( (ShaderCSBLocations*) mLocations)->Set( _contrast, _brightness, _saturation, _gamma, inverse, hue, hueSat, lightness );
	}
}

void CompositeShaderManagerImpl::SetSolidColorUniforms(float solidR, float solidG, float solidB, float solidA, float bottomR, float bottomG, float bottomB, float bottomA, const bool isGradient) const
{
	if (mLocations)
	{
		( (ShaderSolidColorLocations*) mLocations)->Set( solidR, solidG, solidB, solidA, bottomR, bottomG, bottomB, bottomA, isGradient );
	}
}

void CompositeShaderManagerImpl::SetHalfToneUniforms(const float _width, const float _height, const float _frequency) const
{
	if (mLocations)
	{
		( (ShaderHalfToneLocations*) mLocations)->Set( _width, _height, _frequency );
	}
}

// 3d DOF

void CompositeShaderManagerImpl::SetDOFUniforms(	const float	_focalDistance,
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
	if (mLocations)
	{
		( (ShaderDOFLocations*) mLocations)->Set(  
						_focalDistance,
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
}

void CompositeShaderManagerImpl::SetDOFUniforms( const float _zNear,
						const float _zFar ) const
{
	if (mLocations)
	{
		( (ShaderDOFLocations*) mLocations)->Set( _zNear, _zFar );
	}
}

void CompositeShaderManagerImpl::SetDOFUniforms( const int _autoFocus, const float _focalDistance ) const
{
	if (mLocations)
	{
		( (ShaderDOFLocations*) mLocations)->Set( _autoFocus, _focalDistance );
	}
}

// 3d fog

void CompositeShaderManagerImpl::SetFogUniforms( const int _alphaValid ) const
{
	if (mLocations)
	{
		( (ShaderFogLocations*) mLocations)->Set( _alphaValid );
	}
}

void CompositeShaderManagerImpl::SetFogUniforms( const float _r, const float _g, const float _b,
						const float _density,
						const float _znear,
						const float _zfar,
						const float _fognear,
						const float _fogfar,
						const float _fogborder,
						const float _logarithmic,
						const float _highQualityDepth,
						const float _volumeObject) const
{
	if (mLocations)
	{
		( (ShaderFogLocations*) mLocations)->Set(  _r, _g, _b,
						_density,
						_znear,
						_zfar,
						_fognear,
						_fogfar,
						_fogborder,
						_logarithmic,
						_highQualityDepth,
						_volumeObject );
	}
}

void CompositeShaderManagerImpl::SetFogUniforms( const float *invModelMatrix, const float *volumeMin, const float *volumeMax, const float *color, const float density, const float feather ) const
{
	if (mLocations)
	{
		( (ShaderFogLocations*) mLocations)->Set(  invModelMatrix, volumeMin, volumeMax, color, density, feather );
	}
}

// toon lines

void CompositeShaderManagerImpl::SetToonLinesOffsets( const int count, const float *offsets ) const
{
	if (mLocations)
	{
		( (ShaderToonLinesLocations*) mLocations)->Set( count, offsets );
	}
}

void CompositeShaderManagerImpl::SetToonLinesUniforms(	const float _znear, 
								const float _zfar, 
								const float _falloff, 
								const float _logarithmic,
								const float _highQualityDepth,
								const int _numberOfSamples,
								const int _width,
								const int _height ) const
{
	if (mLocations)
	{
		( (ShaderToonLinesLocations*) mLocations)->Set( _znear, _zfar, 
								_falloff, 
								_logarithmic,
								_highQualityDepth,
								_numberOfSamples,
								_width,
								_height );
	}
}

// ssao

void CompositeShaderManagerImpl::SetSSAOUniforms(	const float _distance,
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
	if (mLocations)
	{
		( (ShaderSSAOLocations*) mLocations)->Set( _distance,
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
}

void CompositeShaderManagerImpl::SetSSAOUniforms(	const int	_numberOfSamples,
							const int	_width,
							const int	_height ) const
{
	if (mLocations)
	{
		( (ShaderSSAOLocations*) mLocations)->Set( _numberOfSamples, _width, _height );
	}
}

bool CompositeShaderManagerImpl::InitShader(const char *vertex_filename, const char *fragment_filename, GLSLShaderProgram *&pShader, ShaderBaseLocations *&pLocations, bool useMask)
{
	if (pShader == nullptr || pLocations == nullptr)
		return false;

	bool result = true;
	FBString effectPath, effectFullName;
	
	try
	{
		char effectPath[MAX_PATH];

		if ( !FindEffectLocation( fragment_filename, effectPath, MAX_PATH ) )
			throw std::exception( "Failed to locate shader files" );

		// most of shaders share the same simple vertex shader
		if (vertex_filename == nullptr)
		{
			if ( !pShader->LoadShaders( GetVertexShader(), FBString(effectPath, fragment_filename) ) )
				throw std::exception( "Failed to load shader" );
		}	
		else
		{
			if ( !pShader->LoadShaders( FBString(effectPath, vertex_filename), FBString(effectPath, fragment_filename) ) )
				throw std::exception( "Failed to load shader" );
		}

		//
		// find locations for all neede shader uniforms
		
		pLocations->Init(pShader, useMask);

	}
	catch ( const std::exception &e )
	{
		FBMessageBox( "Composite Master Tool", e.what(), "Ok" );
		result = false;

		FreeShader(pShader, pLocations);
	}

	return result;
}

void CompositeShaderManagerImpl::FreeShader(GLSLShaderProgram *&pShader, ShaderBaseLocations *&pLocations)
{
	if (pShader)
	{
		delete pShader;
		pShader = nullptr;
	}

	if (pLocations)
	{
		delete pLocations;
		pLocations = nullptr;
	}
}