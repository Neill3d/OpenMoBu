
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
#include <math.h>

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

Graphics::SuperShader*	SuperDynamicLighting::mpLightShader = nullptr;
int						SuperDynamicLighting::mpLightShaderRefCount = 0;

/************************************************
*	FiLMBOX Constructor.
************************************************/
bool SuperDynamicLighting::FBCreate()
{
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
    // Delete lighting shader
    mpLightShaderRefCount--;

    if (mpLightShaderRefCount == 0)
    {
		if (nullptr != mpLightShader)
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
bool CheckShadersPath(const char* path)
{
#define SHADER_SHADING_VERTEX		"/GLSL/scene_shading.vsh"
#define SHADER_SHADING_FRAGMENT		"/GLSL/scene_shading.fsh"

	constexpr const char* test_shaders[] = {
		SHADER_SHADING_VERTEX,
		SHADER_SHADING_FRAGMENT
	};

	for (const char* shader_path : test_shaders)
	{
		FBString full_path(path, shader_path);

		if (!IsFileExists(full_path))
		{
			return false;
		}
	}

	return true;
}

void SuperDynamicLighting::ShaderPassTypeBegin(FBRenderOptions* pRenderOptions, FBRenderingPass pPass)
{
    if ( nullptr == mpLightShader )
    {
        // Setup the path to the shader ...
        FBSystem system;
        
		FBString shaders_path(system.ApplicationPath);
		shaders_path = shaders_path + "\\plugins";

		bool status = true;

		if (!CheckShadersPath(shaders_path))
		{
			status = false;
			
			const FBStringList& plugin_paths = system.GetPluginPath();

			for (int i = 0; i < plugin_paths.GetCount(); ++i)
			{
				if (CheckShadersPath(plugin_paths[i]))
				{
					shaders_path = plugin_paths[i];
					status = true;
					break;
				}
			}
		}

		if (status == false)
		{
			FBTrace("[SyperDynamicLighting] Failed to find shaders!\n");
			return;
		}

		shaders_path += "/GLSL/";

        // Create the lighting shader
        mpLightShader = new Graphics::SuperShader();
        if( !mpLightShader->Initialize(shaders_path) )
        {
			FBTrace("Failed to initialize a super lighting effect!\n");
			delete mpLightShader;
			mpLightShader = nullptr;
			mSkipRendering = true;
			Enable = false;
            return;
        }
    }


	StoreCullMode(mCullFaceInfo);
	mLastCullingMode = kFBCullingOff;

	// bind a shader here, prepare a global scene light set
	if (false == mpLightShader->BeginShading(pRenderOptions, nullptr))
	{
		mSkipRendering = true;
	}
	else
	if (true == pRenderOptions->IsIDBufferRendering() || nullptr == mpLightShader)
		mSkipRendering = true;
	else
	{
		mSkipRendering = false;
	}

	mSkipRendering = false;

	if (ForceUpdateTextures)
		mNeedUpdateTextures = true;
}

void SuperDynamicLighting::ShaderPassTypeEnd(FBRenderOptions* pRenderOptions, FBRenderingPass pPass)
{
	if (true == mSkipRendering)
		return;

	// global unbind
	mpLightShader->EndShading();

	FetchCullMode(mCullFaceInfo);

	mNeedUpdateTextures = false;
}

void SuperDynamicLighting::ShaderPassInstanceBegin(FBRenderOptions* pRenderOptions, FBRenderingPass pPass)
{
	if (true == mSkipRendering)
		return;

	mHasExclusiveLights = false;

	if (false == pRenderOptions->IsIDBufferRendering() && nullptr != mpLightShader)
	{
		if (false == UseSceneLights && AffectingLights.GetCount() > 0)
		{
			mpLightShader->BindLights(false, GetShaderLightsPtr());
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
		mpLightShader->BindLights(false);
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
	if (nullptr != mpLightShader)
	{
		delete mpLightShader;
		mpLightShader = nullptr;
	}
}

void SuperDynamicLighting::DetachDisplayContext(FBRenderOptions* pOptions, FBShaderModelInfo* pInfo)
{
	// TODO:
	DoReloadShaders();

	if (mShaderLights.get())
		mShaderLights.reset(nullptr);
	mNeedUpdateLightsList = true;
	mNeedUpdateTextures = true;
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
	if (pThis == &AffectingLights)
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
	// if we have any lights attached
	//	we should update lights every frame

	if (true == Enable
		&& false == UseSceneLights
		&& AffectingLights.GetCount() > 0)
	{
		mNeedUpdateLightsList = true;
	}

	if (mNeedUpdateLightsList && false == UseSceneLights)
	{
		if (nullptr == mShaderLights.get())
			mShaderLights.reset(new Graphics::ShaderLightManager());

		if (nullptr != mpLightShader)
		{
			mpLightShader->PrepShaderLights(UseSceneLights,
				&AffectingLights, mLightsPtr, mShaderLights.get());

			mpLightShader->PrepLightsInViewSpace(mShaderLights.get());
		}
		
		mShaderLights->MapOnGPU();
		mShaderLights->PrepGPUPtr();

		mNeedUpdateLightsList = false;
	}
}