#pragma once

/** \file texture_usereffect.h

Sergei <Neill3d> Solokhin 2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- SDK include
#include <fbsdk/fbsdk.h>

#include <atomic>
#include <memory>
#include <string>

#include "posteffect_context.h"
#include "posteffect_buffers.h"

// forward
class EffectShaderUserObject;

//--- Registration define
#define TEXTUREUSEREFFECT__CLASSNAME	TextureUserEffect
#define TEXTUREUSEREFFECT__CLASSSTR		"TextureUserEffect"
#define TEXTUREUSEREFFECT__DESCSTR		"User Effect Texture"

/** Shading Element Class implementation (asset system).
*	This should be placed in the source code file for a class.
*/
#define TextureUserEffectShadingElementImplementation(ClassName,ElemName,IconFileName)\
	HIObject RegisterShadingElement##ClassName##Create(HIObject /*pOwner*/, const char* pName, void* /*pData*/){\
		ClassName* Class = new ClassName(pName);\
		Class->mAllocated = true;\
		if( Class->FBCreate() ){\
			return Class->GetHIObject();\
		} else {\
			delete Class;\
			return NULL;}}\
	FBLibraryModule(ClassName##ShadingElement){\
		FBRegisterObject(ClassName##R2, "Browsing/Templates/Shading Elements", ElemName, "", RegisterShadingElement##ClassName##Create, true, IconFileName);}

/// when the texture has to be re-rendered
enum ETextureUserEffectUpdateMode
{
	eTextureUserEffectUpdateEveryFrame,	//!< re-render on every evaluation (for time dependent shaders)
	eTextureUserEffectUpdateOnChange,	//!< re-render when inputs or effect properties are changed
	eTextureUserEffectUpdateManual		//!< re-render only on the Refresh action
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// A layered texture that renders one user effect (EffectShaderUserObject) into its render-to-texture target.
///  Unlike PostPersistentData it doesn't run a full post processing chain and it is not a screen space effect,
///  there is no scene camera, so camera related values are defaults.
/// The output size is the size of a target buffer that is prepared by MoBu (see TextureLayerComposition)
/// </summary>
FB_FORWARD(TextureUserEffect);
class TextureUserEffect : public FBLayeredTexture
{
	//--- FiLMBOX declaration.
	FBStorableClassDeclare(TextureUserEffect, FBLayeredTexture);

public:
	TextureUserEffect(const char* pName = nullptr, HIObject pObject = nullptr);

	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate() override;
	virtual void FBDestroy() override;

public: // PROPERTIES

	FBPropertyListObject		UserEffect;			//!< connected user effect to render (single connection)
	FBPropertyBool				CustomComposition;	//!< switch between the user effect rendering and the default layers composition
	FBPropertyBaseEnum<ETextureUserEffectUpdateMode>	UpdateMode;
	FBPropertyAction			Refresh;			//!< request a new rendering, for the manual update mode
	FBPropertyString			Status;				//!< read only, a short text about the state: No effect, Ok, etc.

	// inherited:
	//  Layers[0] is an input texture for the effect (optional, without it the input sampler is empty)
	//  BackgroundColor is used to clear the target when there is nothing to render

public:

	//! get a connected user effect, or nullptr
	EffectShaderUserObject* GetUserEffect();

	//! called by a connected user effect when its state is changed (a reload is requested, etc.)
	void OnUserEffectChanged();

	std::string ComposeStatusText();

	/** Override of the layers composition, MoBu has already prepared a render-to-texture target.
	*   \param  pWidth, pHeight - size of the target, the effect is rendered with this size
	*/
	virtual void TextureLayerComposition(FBTime pTime, FBTime pTimeInCurrentTimeRef, int pWidth, int pHeight) override;

	virtual bool FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat) override;
	virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat) override;

	virtual bool EvaluateAnimationNodes(FBEvaluateInfo* pEvaluateInfo) override;

	virtual bool PlugNotify(FBConnectionAction pAction, FBPlug* pThis, int pIndex, FBPlug* pPlug, FBConnectionType pConnectionType, FBPlug* pNewPlug) override;

protected:

	std::string		mStatusText; //!< storage for a string that is returned from the Status property getter

	// render thread resources and state
	// NOTE: the effect is rendered outside of the post processing chain, so it has its own buffer pool,
	//  property storage and a default (no camera) context cache

	std::unique_ptr<PostEffectBuffers>				mBuffers;
	ShaderPropertyStorage::EffectMap				mEffectMap;	//!< collected property values (written and read in the render thread)
	PostEffectContextProxy::Cache					mCache;
	void*											mGLContext{ nullptr };	//!< to detect a graphics context change
	const EffectShaderUserObject*					mLastEffect{ nullptr };
	int												mWarmupFrames{ 0 };		//!< property scheme is built during the first renders, repeat the composition to get all the values
	double											mLastLocalTime{ 0.0 };
	double											mLastSysTime{ 0.0 };
	std::atomic<int>								mUnsupportedSamplers{ 0 };	//!< bit mask of the used samplers that are not available for the texture

	//! clear the target (it is already bound by MoBu) with the background color
	void ClearTarget(int width, int height);
	void RenderUserEffect(EffectShaderUserObject* userEffect, FBTime pTime, FBTime pTimeInCurrentTimeRef, int width, int height);

	static void ActionRefresh(HIObject pObject, bool value);
	static void SetCustomComposition(HIObject pObject, bool value);
	static const char* GetStatusText(HIObject pObject);
};
