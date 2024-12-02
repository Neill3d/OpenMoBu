
// SuperDynamicLighting_shader.cxx
/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

// Class declaration
#include "SuperDynamicLighting_shader.h"
#include "SuperShaderModelInfo.h"
#include "FileUtils.h"
#include "FBResourcePathResolver.h"
#include <math.h>
#include "FBLightProxy.h"
#include "FBModelProxy.h"

//--- Registration defines
#define DYNAMICLIGHTING__CLASS DYNAMICLIGHTING__CLASSNAME
#define DYNAMICLIGHTING__DESC	"Super Dynamic Lighting" // This is what shows up in the shader window ...


//--- FiLMBOX Registration & Implementation.
FBShaderImplementation(	DYNAMICLIGHTING__CLASS	);
FBRegisterShader	(DYNAMICLIGHTING__DESCSTR,	    // Unique name
                     DYNAMICLIGHTING__CLASS,		// Class
                     DYNAMICLIGHTING__DESCSTR,	    // Short description
                     DYNAMICLIGHTING__DESC,		    // Long description
                     FB_DEFAULT_SDK_ICON	);		// Icon filename (default=Open Reality icon)

Graphics::SceneManager*	SuperDynamicLighting::mSceneManager = nullptr;
int						SuperDynamicLighting::mSceneManagerRefCount = 0;

Graphics::SuperShader* SuperDynamicLighting::mpLightShader = nullptr;
int						SuperDynamicLighting::mpLightShaderRefCount = 0;

/************************************************
*	FiLMBOX Constructor.
************************************************/
bool SuperDynamicLighting::FBCreate()
{
	mSceneManagerRefCount++;
    mpLightShaderRefCount++;

	FBPropertyPublish(this, ReloadShaders, "Reload Shaders", nullptr, SetReloadShaders);

	FBPropertyPublish(this, UseSceneLights, "Use Scene Lights", nullptr, nullptr);
	FBPropertyPublish(this, AffectingLights, "AffectingLights", nullptr, nullptr);
    AffectingLights.SetFilter(FBLight::GetInternalClassId());
    AffectingLights.SetSingleConnect(false);

	FBPropertyPublish(this, Transparency, "Transparency", nullptr, SetTransparencyProperty);
    Transparency.SetInt(kFBAlphaSourceNoAlpha);
    RenderingPass.SetInt(GetRenderingPassNeededForAlpha( (FBAlphaSource)Transparency.AsInt()));

	FBPropertyPublish(this, TransparencyFactor, "TransparencyFactor", nullptr, nullptr);
    TransparencyFactor.SetMinMax(0.0, 1.0);
    TransparencyFactor = 1.0;

	//
	FBPropertyPublish(this, Shadows, "Shadows", nullptr, nullptr);
	Shadows = true;

	FBPropertyPublish(this, ShadowMapSize, "ShadowMapSize", nullptr, nullptr);
	ShadowMapSize.SetMinMax(128, 8192, true, false);
	ShadowMapSize = 2048;

	FBPropertyPublish(this, ShadowPCFKernelSize, "ShadowPCFKernelSize", nullptr, nullptr);
	ShadowPCFKernelSize.SetMinMax(1, 9, true, true);
	ShadowPCFKernelSize = 9;

	FBPropertyPublish(this, ShadowCasters, "ShadowCasters", nullptr, nullptr);
	ShadowCasters.SetFilter(FBModel::GetInternalClassId());
	ShadowCasters.SetSingleConnect(false);

	FBPropertyPublish(this, ShadowStrength, "ShadowStrength", nullptr, nullptr);
	ShadowStrength.SetMinMax(0.0, 1.0f, true, true);
	ShadowStrength = 1.0f;

	FBPropertyPublish(this, OffsetFactor, "Offset Factor", nullptr, nullptr);
	OffsetFactor.SetMinMax(-10.0, 10.0);
	OffsetFactor = 2.0;

	FBPropertyPublish(this, OffsetUnits, "Offset Units", nullptr, nullptr);
	OffsetUnits.SetMinMax(-100000.0, 100000.0);
	OffsetUnits = 4.0;

	//
	FBPropertyPublish(this, SwitchAlbedoTosRGB, "Switch Albedo To sRGB", nullptr, nullptr);
	SwitchAlbedoTosRGB = false;

	FBPropertyPublish(this, ForceUpdateTextures, "Force Update Textures", nullptr, nullptr);
	ForceUpdateTextures = false;

	// rim lighting

	FBPropertyPublish(this, UseRim, "Use Rim Lighting", nullptr, nullptr);
	FBPropertyPublish(this, RimPower, "Rim Power", nullptr, nullptr);
	FBPropertyPublish(this, RimColor, "Rim Color", nullptr, nullptr);
	
	UseRim.SetMinMax(0.0, 100.0);
	UseRim = 0.0;
	RimPower.SetMinMax(0.0, 200.0);
	RimPower = 100.0;
	RimColor = FBColor(1.0, 1.0, 1.0);

	// matcap

	FBPropertyPublish(this, UseMatCap, "Use MatCap", nullptr, nullptr);
	FBPropertyPublish(this, MatCapTexture, "MatCap Texture", nullptr, nullptr);

	UseMatCap = false;
	MatCapTexture.SetSingleConnect(true);
	MatCapTexture.SetFilter(FBTexture::GetInternalClassId());

    //Set up shader capacity.  It seems cg2.0 has problem regarding INSTNCEID currently.
    //SetShaderCapacity(FBShaderCapacity(kFBShaderCapacityMaterialEffect | kFBShaderCapacityDrawInstanced), true);
    SetShaderCapacity(FBShaderCapacity(kFBShaderCapacityMaterialEffect), true);

    //Hook up the callback 
    SetShaderPassActionCallback(FBShaderPassActionCallback(kFBShaderPassTypeBegin | kFBShaderPassTypeEnd | kFBShaderPassInstanceBegin | kFBShaderPassInstanceEnd | kFBShaderPassMaterialBegin | kFBShaderPassMaterialEnd | kFBShaderPassModelDraw));

    SetDrawInstancedMaximumSize(kMaxDrawInstancedSize);

	UseSceneLights = false;
	mNeedUpdateLightsList = true;
	mNeedUpdateTextures = true;
	mSkipRendering = false;

    return true;
}


/************************************************
*	FiLMBOX Destructor.
************************************************/
void SuperDynamicLighting::FBDestroy()
{
	// DeRef and Delete scene manager if no longer needed
	mSceneManagerRefCount--;

	if (mSceneManagerRefCount <= 0)
	{
		if (mSceneManager)
		{
			delete mSceneManager;
			mSceneManager = nullptr;
		}
	}

    // Delete lighting shader
    mpLightShaderRefCount--;

	if (mpLightShaderRefCount <= 0)
    {
		if (mpLightShader)
		{
			delete mpLightShader;
			mpLightShader = nullptr;
		}
    }

    ParentClass::FBDestroy();
}

/************************************************
*	Shader functions.
************************************************/

void SuperDynamicLighting::BeginFrameForSharedManagers()
{
#define SHADER_SHADING_VERTEX		"scene_shading.vsh"
#define SHADER_SHADING_FRAGMENT		"scene_shading.fsh"

	if (!mSceneManager)
	{
		mSceneManager = new Graphics::SceneManager();
	}

	if (!mpLightShader)
	{
		const std::vector<std::string> test_shaders = {
			SHADER_SHADING_VERTEX,
			SHADER_SHADING_FRAGMENT
		};

		FBShaderPathResolver	pathResolver;

		const std::filesystem::path shadersPath = pathResolver.FindShaderPath(test_shaders);

		if (shadersPath.empty())
		{
			LOGE("[SyperDynamicLighting] Failed to find shaders!\n");
			return;
		}

		// Create the lighting shader
		mpLightShader = new Graphics::SuperShader();
		if (!mpLightShader->Initialize(shadersPath.generic_string().c_str()))
		{
			FBTrace("Failed to initialize a super lighting effect!\n");
			delete mpLightShader;
			mpLightShader = nullptr;
			mSkipRendering = true;
			Enable = false;
			return;
		}
	}
}


void SuperDynamicLighting::ShaderPassTypeBegin(FBRenderOptions* pRenderOptions, FBRenderingPass pPass)
{
	BeginFrameForSharedManagers();

	StoreCullMode(mCullFaceInfo);
	mLastCullingMode = kFBCullingOff;

	if (Transparency != kFBAlphaSourceNoAlpha)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	}

	// global camera cache and scene lights
	mSceneManager->BeginShading(pRenderOptions);

	// bind a shader here, prepare a global scene light set
	if (!mpLightShader->BeginShading(pRenderOptions, nullptr))
	{
		mSkipRendering = true;
	}
	else
	if (pRenderOptions->IsIDBufferRendering() || nullptr == mpLightShader)
		mSkipRendering = true;
	else
	{
		mSkipRendering = false;
	}

	if (!mSkipRendering)
	{
		mpLightShader->BindLights(true, mSceneManager->GetGPUSceneLightsPtr());
	}

	mSkipRendering = false;

	if (ForceUpdateTextures)
		mNeedUpdateTextures = true;
}

void SuperDynamicLighting::ShaderPassTypeEnd(FBRenderOptions* pRenderOptions, FBRenderingPass pPass)
{
	if (true == mSkipRendering)
		return;

	if (Transparency != kFBAlphaSourceNoAlpha)
	{
		glDisable(GL_BLEND);
	}

	// global unbind
	mpLightShader->EndShading();

	FetchCullMode(mCullFaceInfo);

	if (Shadows)
	{
		glActiveTexture(GL_TEXTURE0 + mpLightShader->GetSamplerSlotShadow());
		shadowManager.UnBind();
		glActiveTexture(GL_TEXTURE0);
	}
	
	mNeedUpdateTextures = false;
}

void SuperDynamicLighting::ShaderPassInstanceBegin(FBRenderOptions* pRenderOptions, FBRenderingPass pPass)
{
	if (true == mSkipRendering)
		return;

	mHasExclusiveLights = false;

	if (!pRenderOptions->IsIDBufferRendering() && mpLightShader)
	{
		if (UseSceneLights == false && AffectingLights.GetCount() > 0)
		{
			mpLightShader->BindLights(false, mSceneManager->GetGPUSceneLightsPtr(), GetShaderLightsPtr());
			mHasExclusiveLights = true;
		}

		double useRim = 0.01 * UseRim;
		double rimPower = 0.01 * RimPower;
		FBColor rimColor = RimColor;
		mpLightShader->UploadRimInformation(useRim, rimPower, rimColor);
		mpLightShader->UploadSwitchAlbedoTosRGB(SwitchAlbedoTosRGB);

		//
		if (true == UseMatCap && MatCapTexture.GetCount() > 0)
		{
			FBTexture *pTexture = (FBTexture*)MatCapTexture.GetAt(0);
			GLuint texId = pTexture->TextureOGLId;
			if (0 == texId || true == mNeedUpdateTextures)
			{
				pTexture->OGLInit();
				texId = pTexture->TextureOGLId;
			}

			mpLightShader->SetMatCap(texId);
		}
		else
		{
			mpLightShader->SetMatCap(0);
		}

		EventBeforeRenderNotify();


	}	
}

void SuperDynamicLighting::ShaderPassInstanceEnd(FBRenderOptions* pRenderOptions, FBRenderingPass pPass)
{
	if (true == mSkipRendering)
		return;

	if (mHasExclusiveLights)
		mpLightShader->BindLights(false, mSceneManager->GetGPUSceneLightsPtr());

	glActiveTexture(GL_TEXTURE0 + mpLightShader->GetSamplerSlotShadow());
	shadowManager.UnBind();
	glActiveTexture(GL_TEXTURE0);
}

void SuperDynamicLighting::ShaderPassMaterialBegin(FBRenderOptions* pRenderOptions, FBRenderingPass pPass, FBShaderModelInfo* pInfo)
{
	if (true == mSkipRendering || !mpLightShader)
	{
		return;
	}
	
    mpLightShader->SwitchMaterial(pRenderOptions, pInfo, pInfo->GetFBMaterial(), TransparencyFactor, mNeedUpdateTextures);
}

void SuperDynamicLighting::ShaderPassMaterialEnd(FBRenderOptions* pRenderOptions, FBRenderingPass pPass, FBShaderModelInfo* pInfo)
{
}

void SuperDynamicLighting::ShaderPassModelDraw(FBRenderOptions* pRenderOptions, FBRenderingPass pPass, FBShaderModelInfo* pInfo)
{
	if (true == mSkipRendering || !mpLightShader)
	{
		return;
	}
	
	FBModel *pModel = pInfo->GetFBModel();

	if (nullptr != pModel)
	{
		FBModelCullingMode cullMode = pModel->GetCullingMode();

		if (cullMode != mLastCullingMode)
		{
			switch (cullMode)
			{
			case kFBCullingOff:
				glDisable(GL_CULL_FACE);
				break;
			case kFBCullingOnCW:
				glEnable(GL_CULL_FACE);
				glCullFace(GL_FRONT);
				break;
			case kFBCullingOnCCW:
				glEnable(GL_CULL_FACE);
				glCullFace(GL_BACK);
				break;
			}

			mLastCullingMode = cullMode;
			//((CRenderOptions&)options).SetLastCullingMode(cullMode);
		}
	}

	mpLightShader->ShaderPassModelDraw(pRenderOptions, pPass, pInfo, mNeedUpdateTextures);

	// bind vertex buffers

	SuperShaderModelInfo *lInfo = (SuperShaderModelInfo*)pInfo;
	if (nullptr != lInfo && false == pRenderOptions->IsIDBufferRendering())
	{
		lInfo->Bind();
	}

}

void SuperDynamicLighting::ShaderPassDrawShadowBegin(FBRenderOptions* pRenderOptions)
{
    //Here we should setup the shader's draw shadow states
}

void SuperDynamicLighting::ShaderPassDrawShadowEnd(FBRenderOptions* pRenderOptions)
{
    //Here we should clean the shader's draw shadow states
}

void SuperDynamicLighting::UploadModelViewMatrixArrayForDrawInstanced(const double* pModelViewMatrixArray, int pCount)
{
	if (true == mSkipRendering)
		return;

    mpLightShader->UploadModelViewMatrixArrayForDrawInstanced(pModelViewMatrixArray, pCount);
}

FBShaderModelInfo* SuperDynamicLighting::NewShaderModelInfo(HKModelRenderInfo pModelRenderInfo, int pSubRegionIndex)
{
    FBShaderModelInfo* lShaderModelInfo = new SuperShaderModelInfo(this, pModelRenderInfo, pSubRegionIndex);
    return lShaderModelInfo;
}

void SuperDynamicLighting::UpdateModelShaderInfo(FBRenderOptions* pOptions, FBShaderModelInfo* pModelRenderInfo)
{
    //pModelRenderInfo->UpdateModelShaderInfo(GetShaderVersion());

    unsigned int lVBOFormat = kFBGeometryArrayID_Point | kFBGeometryArrayID_Normal;

    if (pModelRenderInfo->GetOriginalTextureFlag())
    {
        FBMaterial* lMaterial = pModelRenderInfo->GetFBMaterial();
        if (lMaterial)
        {
            //FBTexture* lDiffuseTexture = pMaterial->GetTexture(kFBMaterialTextureDiffuse);

            FBTexture* lNormalMapTexture = lMaterial->GetTexture(kFBMaterialTextureBump);
            if (lNormalMapTexture == NULL) 
                lNormalMapTexture = lMaterial->GetTexture(kFBMaterialTextureNormalMap);

            if (lNormalMapTexture)
                lVBOFormat = lVBOFormat | kFBGeometryArrayID_Tangent | kFBGeometryArrayID_Binormal;
        }
    }

	// check if second uv set is needed in some texture
	if (pModelRenderInfo->GetFBModel())
	{
		bool needSecondUV = false;

		FBModel *pModel = pModelRenderInfo->GetFBModel();

		for (int i = 0; i<pModel->Textures.GetCount(); ++i)
		{
			FBTexture *pTexture = pModel->Textures[i];

			FBString uvset("");
			FBProperty *lProp = pTexture->PropertyList.Find("UVSet");
			if (lProp)
			{
				uvset = lProp->AsString();
			}

			if (uvset != "")
			{
				needSecondUV = true;
				break;
			}
		}

		if (needSecondUV)
		{
			lVBOFormat = lVBOFormat | kFBGeometryArrayID_SecondUVSet;
		}
	}

	pModelRenderInfo->SetGeometryArrayIds(lVBOFormat);
	pModelRenderInfo->UpdateModelShaderInfo(GetShaderVersion());

    //pModelRenderInfo->SetGeometryArrayIds(lVBOFormat);
}

void SuperDynamicLighting::SetTransparencyType(FBAlphaSource pTransparency)
{
    if (Transparency.AsInt() != pTransparency)
    {
        Transparency.SetInt(pTransparency);
        //To trigger render to update the model-shader information.
        InvalidateShaderVersion();
    }
}

FBAlphaSource SuperDynamicLighting::GetTransparencyType()
{
    return (FBAlphaSource) Transparency.AsInt();
}

void SuperDynamicLighting::SetTransparencyProperty(HIObject pObject, FBAlphaSource pState)
{     
	SuperDynamicLighting* lShader = FBCast<SuperDynamicLighting>(pObject);
    if (lShader->Transparency.AsInt() != pState)
    {
        lShader->Transparency.SetPropertyValue(pState);
        lShader->RenderingPass.SetPropertyValue(GetRenderingPassNeededForAlpha(pState));
         
        // if shader use alpha and thus generate custom shape than the original geometry shape, 
        // we need to let it handle DrawShadow functiionality as well. 
        lShader->SetShaderCapacity(kFBShaderCapacityDrawShadow, pState != kFBAlphaSourceNoAlpha); 
    }
}

void SuperDynamicLighting::SetReloadShaders(HIObject pObject, bool val)
{
	SuperDynamicLighting* lShader = FBCast<SuperDynamicLighting>(pObject);
	if (lShader && val)
	{
		lShader->DoReloadShaders();
	}
}


void SuperDynamicLighting::DoReloadShaders()
{
	if (mpLightShader)
	{
		delete mpLightShader;
		mpLightShader = nullptr;
	}
}

void SuperDynamicLighting::DetachDisplayContext(FBRenderOptions* pOptions, FBShaderModelInfo* pInfo)
{
	DoReloadShaders();

	if (mShaderLights.get())
		mShaderLights.reset(nullptr);
	mNeedUpdateLightsList = true;
	mNeedUpdateTextures = true;

	shadowManager.ChangeContext();
}

bool SuperDynamicLighting::PlugDataNotify(FBConnectionAction pAction, FBPlug* pThis, void* pData, void* pDataOld, int pDataSize)
{
	if (pAction == kFBCandidated)
	{
		if (pThis == &UseSceneLights)
		{
			mNeedUpdateLightsList = true;
		}
	}

	return ParentClass::PlugDataNotify(pAction, pThis, pData, pDataOld, pDataSize);
}

bool SuperDynamicLighting::PlugNotify(FBConnectionAction pAction, FBPlug* pThis, int pIndex, FBPlug* pPlug, FBConnectionType pConnectionType, FBPlug* pNewPlug)
{
	if (pThis == &AffectingLights || pThis == &ShadowCasters)
	{
		if (pAction == kFBConnectedSrc)
		{
			ConnectSrc(pPlug);
			AskToUpdateLightList();
		}
		else if (pAction == kFBDisconnectedSrc)
		{
			DisconnectSrc(pPlug);
			AskToUpdateLightList();
		}
	}
	else if (pThis == &MatCapTexture)
	{
		if (pAction == kFBConnectedSrc)
		{
			ConnectSrc(pPlug);
		}
		else if (pAction == kFBDisconnectedSrc)
		{
			DisconnectSrc(pPlug);
		}
	}
	
	return ParentClass::PlugNotify(pAction, pThis, pIndex, pPlug, pConnectionType, pNewPlug);
}

void SuperDynamicLighting::AskToUpdateLightList()
{
	mNeedUpdateLightsList = true;
}


void SuperDynamicLighting::EventBeforeRenderNotify()
{
	if (!mSceneManager)
		return;

	// if we have any lights attached
	//	we should update lights every frame

	if (Enable == true
		&& UseSceneLights == false
		&& AffectingLights.GetCount() > 0)
	{
		mNeedUpdateLightsList = true;
	}

	if (!mNeedUpdateLightsList || UseSceneLights == true)
		return;

	{
		std::vector<std::shared_ptr<Graphics::LightProxy>> lights;

		for (int i = 0; i < AffectingLights.GetCount(); ++i)
		{
			lights.emplace_back(std::make_shared<Graphics::FBLightProxy>(FBCast<FBLight>(AffectingLights[i])));
		}
		shadowManager.SetLights(std::move(lights));

		//

		std::vector<std::shared_ptr<Graphics::ModelProxy>> models;

		for (int i = 0; i < ShadowCasters.GetCount(); ++i)
		{
			models.emplace_back(std::make_shared<Graphics::FBModelProxy>(FBCast<FBModel>(ShadowCasters[i])));
		}
		shadowManager.SetShadowCasters(std::move(models));
	}

	mSceneManager->ResetShadowInformation();

	if (Shadows)
	{
		double offsetFactor, offsetUnits;
		OffsetFactor.GetData(&offsetFactor, sizeof(double));
		OffsetUnits.GetData(&offsetUnits, sizeof(double));

		shadowManager.SetProperties({
				ShadowMapSize,
				true, // use PCF
				ShadowPCFKernelSize,
				static_cast<float>(offsetFactor),
				static_cast<float>(offsetUnits)
			});

		shadowManager.Render();

		// copy shadow layer and matrix information from shadow manager into scene manager lights
		mSceneManager->UpdateShadowInformation(&shadowManager);

		// bind back a lighting shader!
		mpLightShader->BindShader();
			
		glActiveTexture(GL_TEXTURE0 + mpLightShader->GetSamplerSlotShadow());
		shadowManager.Bind();
		glActiveTexture(GL_TEXTURE0);

		shadowManager.BindShadowsBuffer(4);
	}

	if (!mShaderLights.get())
		mShaderLights.reset(new Graphics::LightGPUBuffersManager());

	mSceneManager->PrepShaderLights(&AffectingLights, mLightsPtr, mShaderLights.get());
	mSceneManager->PrepLightsInViewSpace(mShaderLights.get());
		
	mShaderLights->MapOnGPU();
	mShaderLights->PrepGPUPtr();

	mNeedUpdateLightsList = false;
}
