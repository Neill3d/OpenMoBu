
#pragma once

// posteffectlensflare
/*
Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffectbase.h"
#include "postprocessing_helper.h"
#include <limits>

// forward
class PostPersistentData;

/// <summary>
/// lens flare post processing effect
/// </summary>
struct PostEffectLensFlare : public PostEffectBase
{
public:
	static const int NUMBER_OF_SHADERS{ 3 };

	//! a constructor
	PostEffectLensFlare();

	//! a destructor
	virtual ~PostEffectLensFlare();

	int GetNumberOfShaders() const override { return NUMBER_OF_SHADERS; }

	const char *GetName() const override;
	const char *GetVertexFname(const int shaderIndex) const override;
	const char *GetFragmentFname(const int shaderIndex) const override;

	virtual bool PrepUniforms(const int shaderIndex) override;
	virtual bool CollectUIValues(PostPersistentData *pData, PostEffectContext& effectContext) override;

	virtual const int GetNumberOfPasses() const override;
	virtual bool PrepPass(const int pass) override;

protected:

	//Louis
	EFlareType		FlareType{ EFlareType::flare1 };
	
	// shader locations

	struct SubShader : public CommonEffectUniforms
	{
	public:

		SubShader() = default;
		virtual ~SubShader()
		{}

		enum { LOCATIONS_COUNT = 12 };
		union
		{
			struct
			{
				//Louis
				GLint		seed;

				GLint		amount;

				GLint		textureWidth;
				GLint		textureHeight;

				GLint		timer;

				GLint		light_pos;

				GLint		tint;
				GLint		inner;
				GLint		outer;

				GLint		fadeToBorders;
				GLint		borderWidth;
				GLint		feather;
			};

			GLint		mLocations[LOCATIONS_COUNT];
		};

		int				m_NumberOfPasses{ 1 };
		float			m_DepthAttenuation{ 1.0f };

		std::vector<FBVector3d>	m_LightPositions; // window xy and depth (for attenuation)
		std::vector<FBColor>	m_LightColors;
		std::vector<float>		m_LightAlpha;

		const char* GetEnableMaskPropertyName() const override { return "Flare Use Masking"; }

		void Init();
		bool PrepUniforms(GLSLShader* mShader);
		bool CollectUIValues(const int shaderIndex, GLSLShader* mShader, PostPersistentData *pData, PostEffectContext& effectContext);
		bool PrepPass(GLSLShader* mShader, const int pass);

	private:
		void ProcessLightObjects(PostPersistentData* pData, FBCamera* pCamera, int w, int h, double dt, FBTime systemTime, double* flarePos);

		void ProcessSingleLight(PostPersistentData* pData, FBCamera* pCamera, FBMatrix& mvp, int index, int w, int h, double dt, double* flarePos);

		void UpdateShaderUniforms(GLSLShader* mShader, PostPersistentData* pData, int w, int h, 
			double seedValue, double flareAmount, double flareTimer, double* flarePos, 
			FBColor& flareTint, double flareInner, double flareOuter, float fadeToBordersValue, 
			double borderWidthValue, double featherValue);
	};

	SubShader	subShaders[NUMBER_OF_SHADERS];
};
