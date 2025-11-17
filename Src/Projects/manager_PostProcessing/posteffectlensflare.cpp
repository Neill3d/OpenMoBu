
// posteffectlensflare.cpp
/*
Sergei <Neill3d> Solokhin 2018-2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffectlensflare.h"
#include "postpersistentdata.h"

#include "postprocessing_helper.h"
#include "math3d.h"


EffectShaderLensFlare::EffectShaderLensFlare(FBComponent* ownerIn)
	: PostEffectBufferShader(ownerIn)
{
	MakeCommonProperties();

	AddProperty(ShaderProperty("color", "sampler0"))
		.SetType(EPropertyType::TEXTURE)
		.SetValue(CommonEffect::ColorSamplerSlot)
		.SetFlag(IEffectShaderConnections::PropertyFlag::ShouldSkip, true);

	mFlareSeed = &AddProperty(ShaderProperty(PostPersistentData::FLARE_SEED, "flareSeed", nullptr))
		.SetRequired(false);

	mAmount = &AddProperty(ShaderProperty(PostPersistentData::FLARE_AMOUNT, "amount", nullptr))
		.SetScale(0.01f);
	
	mTime = &AddProperty(ShaderProperty("timer", "iTime", nullptr))
		.SetFlag(PropertyFlag::ShouldSkip, true); // NOTE: skip of automatic reading value and let it be done manually

	mLightPos = &AddProperty(ShaderProperty("light_pos", "light_pos", nullptr))
		.SetFlag(PropertyFlag::ShouldSkip, true) // NOTE: skip of automatic reading value and let it be done manually
		.SetType(EPropertyType::VEC4);

	mTint = &AddProperty(ShaderProperty(PostPersistentData::FLARE_TINT, "tint", nullptr))
		.SetType(EPropertyType::VEC4)
		.SetFlag(PropertyFlag::ShouldSkip, true);

	mInner = &AddProperty(ShaderProperty(PostPersistentData::FLARE_INNER, "inner", nullptr))
		.SetScale(0.01f);
	mOuter = &AddProperty(ShaderProperty(PostPersistentData::FLARE_OUTER, "outer", nullptr))
		.SetScale(0.01f);

	mFadeToBorders = &AddProperty(ShaderProperty(PostPersistentData::FLARE_FADE_TO_BORDERS, "fadeToBorders", nullptr))
		.SetType(EPropertyType::BOOL);
	mBorderWidth = &AddProperty(ShaderProperty(PostPersistentData::FLARE_BORDER_WIDTH, "borderWidth", nullptr));
	mFeather = &AddProperty(ShaderProperty(PostPersistentData::FLARE_BORDER_FEATHER, "feather", nullptr))
		.SetScale(0.01f);

	for (int nShader = 0; nShader < NUMBER_OF_SHADERS; ++nShader)
	{
		subShaders[nShader].Init();
	}
}

const char* EffectShaderLensFlare::GetUseMaskingPropertyName() const noexcept
{
	return PostPersistentData::FLARE_USE_MASKING;
}
const char* EffectShaderLensFlare::GetMaskingChannelPropertyName() const noexcept
{
	return PostPersistentData::FLARE_MASKING_CHANNEL;
}

bool EffectShaderLensFlare::OnCollectUI(const IPostEffectContext* effectContext, int maskIndex)
{
	PostPersistentData* data = effectContext->GetPostProcessData();
	if (!data)
		return false;

	const int LastShaderIndex = GetCurrentShader();
	SetCurrentShader(data->FlareType.AsInt());

	if (GetCurrentShader() < 0 || GetCurrentShader() >= NUMBER_OF_SHADERS)
	{
		SetCurrentShader(0);
		const EFlareType newFlareType{ EFlareType::flare1 };
		data->FlareType.SetData((void*)&newFlareType);
	}

	if (LastShaderIndex != GetCurrentShader())
	{
		bHasShaderChanged = true;
	}
	
	// flare seed property is used in bubble and anamorphic flare shaders
	mFlareSeed->SetRequired(GetCurrentShader() != EFlareType::flare1);
	
	const double systemTime = (data->FlareUsePlayTime) ? effectContext->GetLocalTime() : effectContext->GetSystemTime();
	double timerMult = data->FlareTimeSpeed;
	double flareTimer = 0.01 * timerMult * systemTime;
	mTime->SetValue(flareTimer);

	return subShaders[mCurrentShader].CollectUIValues(mCurrentShader, effectContext, maskIndex);
}

int EffectShaderLensFlare::GetNumberOfPasses() const
{
	return subShaders[mCurrentShader].m_NumberOfPasses;
}

bool EffectShaderLensFlare::OnRenderPassBegin(int pass, int width, int height)
{
	const int currentShader = GetCurrentShader();
	assert(currentShader >= 0 && currentShader < GetNumberOfVariations());
	const SubShader& subShader = subShaders[currentShader];

	if (pass >= 0 && pass < static_cast<int>(subShader.m_LightPositions.size()))
	{
		const FBVector3d pos(subShader.m_LightPositions[pass]);
		mLightPos->SetValue(static_cast<float>(pos[0]), static_cast<float>(pos[1]), static_cast<float>(pos[2]), subShader.m_DepthAttenuation);
		
		const FBColor tint(subShader.m_LightColors[pass]);
		mTint->SetValue(static_cast<float>(tint[0]), static_cast<float>(tint[1]), static_cast<float>(tint[2]), 1.0f);
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// EffectShaderLensFlare::SubShader

void EffectShaderLensFlare::SubShader::Init()
{
	m_NumberOfPasses = 1;
}

bool EffectShaderLensFlare::SubShader::CollectUIValues(int shaderIndex, const IPostEffectContext* effectContext, int maskIndex)
{
	PostPersistentData* pData = effectContext->GetPostProcessData();

	m_NumberOfPasses = 1;
	/*
	double seedValue = 30.0;
	if (shaderIndex > 0 && pData->IsLazyLoadReady())
	{
		pData->FlareSeed.GetData(&seedValue, sizeof(double));
	}
	*/
	

	double flarePos[3] = { 0.01 * pData->FlarePosX, 0.01 * pData->FlarePosY, 1.0 };

	m_DepthAttenuation = (pData->FlareDepthAttenuation) ? 1.0f : 0.0f;

	if (pData->UseFlareLightObject && pData->FlareLight.GetCount() > 0)
	{
		ProcessLightObjects(pData, effectContext->GetCamera(), effectContext->GetViewWidth(), effectContext->GetViewHeight(), 
			effectContext->GetSystemTimeDT(), effectContext->GetSystemTime(), flarePos);
	}
	else
	{
		m_LightPositions.clear();
		m_LightColors.clear();
		m_LightAlpha.clear();
	}

	return true;
}

void EffectShaderLensFlare::SubShader::ProcessLightObjects(PostPersistentData* pData, FBCamera* pCamera, int w, int h, double dt, FBTime systemTime, double* flarePos)
{
	m_NumberOfPasses = pData->FlareLight.GetCount();
	m_LightPositions.resize(m_NumberOfPasses);
	m_LightColors.resize(m_NumberOfPasses);
	m_LightAlpha.resize(m_NumberOfPasses, 0.0f);

	FBMatrix mvp;
	pCamera->GetCameraMatrix(mvp, kFBModelViewProj);

	for (int i = 0; i < m_NumberOfPasses; ++i)
	{
		ProcessSingleLight(pData, pCamera, mvp, i, w, h, dt, flarePos);
	}

	// relative coords to a screen size
	pData->FlarePosX = 100.0 * flarePos[0];
	pData->FlarePosY = 100.0 * flarePos[1];
}

void EffectShaderLensFlare::SubShader::ProcessSingleLight(PostPersistentData* pData, FBCamera* pCamera, FBMatrix& mvp, int index, int w, int h, double dt, double* flarePos)
{
	FBLight* pLight = static_cast<FBLight*>(pData->FlareLight.GetAt(index));

	FBVector3d lightPos;
	pLight->GetVector(lightPos);

	FBVector4d v4;
	FBVectorMatrixMult(v4, mvp, FBVector4d(lightPos[0], lightPos[1], lightPos[2], 1.0));

	v4[0] = static_cast<double>(w) * 0.5 * (v4[0] + 1.0);
	v4[1] = static_cast<double>(h) * 0.5 * (v4[1] + 1.0);

	flarePos[0] = v4[0] / static_cast<double>(w);
	flarePos[1] = v4[1] / static_cast<double>(h);
	flarePos[2] = v4[2];

	m_LightPositions[index].Set(flarePos);
	FBColor color(pLight->DiffuseColor);

	bool isFading = false;

	if (pData->LensFlare_UseOcclusion && pData->FlareOcclusionObjects.GetCount() > 0)
	{
		const int offsetX = pCamera->CameraViewportX;
		const int offsetY = pCamera->CameraViewportY;

		const int x = offsetX + static_cast<int>(v4[0]);
		const int y = offsetY + (h - static_cast<int>(v4[1]));

		FBVector3d camPosition;
		pCamera->GetVector(camPosition);
		
		const double distToLight = VectorLength(VectorSubtract(lightPos, camPosition));

		for (int i = 0, count = pData->FlareOcclusionObjects.GetCount(); i < count; ++i)
		{
			if (FBModel* model = FBCast<FBModel>(pData->FlareOcclusionObjects.GetAt(i)))
			{
				FBVector3d hitPosition, hitNormal;
				if (model->RayCast(pCamera, x, y, hitPosition, hitNormal))
				{
					const double distToHit = VectorLength(VectorSubtract(hitPosition, camPosition));

					if (distToHit < distToLight)
					{
						isFading = true;
						break;
					}
				}
			}

		}
	}

	float alpha = m_LightAlpha[index];
	double occSpeed = 100.0;
	pData->FlareOcclusionSpeed.GetData(&occSpeed, sizeof(double));
	alpha += static_cast<float>(occSpeed) * ((isFading) ? -dt : dt);
	alpha = clamp01(alpha);

	const double f = smoothstep(0.0, 1.0, static_cast<double>(alpha));

	color[0] *= f;
	color[1] *= f;
	color[2] *= f;

	m_LightColors[index].Set(color);
	m_LightAlpha[index] = alpha;
}

