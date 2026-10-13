
/** \file texture_usereffect.cxx

Sergei <Neill3d> Solokhin 2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "texture_usereffect.h"

#include "posteffect_shader_userobject.h"
#include "posteffect_shader.h"
#include "mobu_logging.h"
#include "hashUtils.h"

#include <ctime>

namespace
{
	// samplers that are provided by the scene render pass, there is no scene behind the texture
	constexpr int UNSUPPORTED_DEPTH = 1;
	constexpr int UNSUPPORTED_LINEAR_DEPTH = 2;
	constexpr int UNSUPPORTED_NORMAL = 4;
	constexpr int UNSUPPORTED_MASK = 8;

	// number of compositions to repeat after a shader (re)load, nested effects build their property schemes on the first render
	constexpr int WARMUP_FRAMES = 3;
}

//--- FiLMBOX Registration & Implementation.
FBClassImplementation(TextureUserEffect);										//Register class
FBStorableCustomTextureImplementation(TextureUserEffect, Texture);				//Register to the store/retrieve system
TextureUserEffectShadingElementImplementation(TextureUserEffect, "Texture User Effect", "browsing/template_layeredtexture.png");	//Register to the asset browser shading element category folder.

const char* FBPropertyBaseEnum<ETextureUserEffectUpdateMode>::mStrings[] = {
	"Every Frame",
	"On Change",
	"Manual",
	0
};

/************************************************
 *  Constructor.
 ************************************************/
TextureUserEffect::TextureUserEffect(const char* pName, HIObject pObject)
	: FBLayeredTexture(pName, pObject)
{
	FBClassInit;
}

void TextureUserEffect::ActionRefresh(HIObject pObject, bool value)
{
	TextureUserEffect* p = FBCast<TextureUserEffect>(pObject);
	if (p && value)
	{
		p->SetLayerConfigDirty();
	}
}

void TextureUserEffect::SetCustomComposition(HIObject pObject, bool value)
{
	TextureUserEffect* p = FBCast<TextureUserEffect>(pObject);
	if (p)
	{
		p->CustomComposition.SetPropertyValue(value);
		p->SetLayerConfigDirty();
	}
}

const char* TextureUserEffect::GetStatusText(HIObject pObject)
{
	TextureUserEffect* p = FBCast<TextureUserEffect>(pObject);
	if (!p)
		return "";

	p->mStatusText = p->ComposeStatusText();
	return p->mStatusText.c_str();
}

/************************************************
 *  FiLMBOX Constructor.
 ************************************************/
bool TextureUserEffect::FBCreate()
{
	FBPropertyPublish(this, UserEffect, "User Effect", nullptr, nullptr);
	FBPropertyPublish(this, CustomComposition, "Custom Composition", nullptr, SetCustomComposition);
	FBPropertyPublish(this, UpdateMode, "Update Mode", nullptr, nullptr);
	FBPropertyPublish(this, Refresh, "Refresh", nullptr, ActionRefresh);
	FBPropertyPublish(this, Status, "Status", GetStatusText, nullptr);

	UserEffect.SetSingleConnect(true);
	UserEffect.SetFilter(FBUserObject::GetInternalClassId());

	CustomComposition = true;
	UpdateMode = eTextureUserEffectUpdateEveryFrame;

	Status.ModifyPropertyFlag(kFBPropertyFlagReadOnly, true);
	Status.ModifyPropertyFlag(kFBPropertyFlagNotSavable, true);

	return true;
}

/************************************************
 *  FiLMBOX Destructor.
 ************************************************/
void TextureUserEffect::FBDestroy()
{
	mEffectMap.clear();
	mBuffers.reset();
	ParentClass::FBDestroy();
}

bool TextureUserEffect::FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	return ParentClass::FbxStore(pFbxObject, pStoreWhat);
}

bool TextureUserEffect::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	const bool result = ParentClass::FbxRetrieve(pFbxObject, pStoreWhat);

	if (pStoreWhat == kFbxObjectStore::kCleanup)
	{
		SetLayerConfigDirty();
	}
	return result;
}

EffectShaderUserObject* TextureUserEffect::GetUserEffect()
{
	if (UserEffect.GetCount() > 0 && FBIS(UserEffect.GetAt(0), EffectShaderUserObject))
	{
		return FBCast<EffectShaderUserObject>(UserEffect.GetAt(0));
	}
	return nullptr;
}

void TextureUserEffect::OnUserEffectChanged()
{
	SetLayerConfigDirty();
}

std::string TextureUserEffect::ComposeStatusText()
{
	EffectShaderUserObject* userEffect = GetUserEffect();
	if (!userEffect)
		return "No effect";

	std::string text = userEffect->ComposeStatusText();

	const int unsupported = mUnsupportedSamplers.load(std::memory_order_acquire);
	if (unsupported != 0)
	{
		text += " | Warning: not available for a texture -";
		if (unsupported & UNSUPPORTED_DEPTH) text += " depth";
		if (unsupported & UNSUPPORTED_LINEAR_DEPTH) text += " linear depth";
		if (unsupported & UNSUPPORTED_NORMAL) text += " world normal";
		if (unsupported & UNSUPPORTED_MASK) text += " mask";
	}
	return text;
}

bool TextureUserEffect::PlugNotify(FBConnectionAction pAction, FBPlug* pThis, int pIndex, FBPlug* pPlug, FBConnectionType pConnectionType, FBPlug* pNewPlug)
{
	if (pThis == &UserEffect)
	{
		if (pAction == kFBConnectedSrc)
		{
			ConnectSrc(pPlug);
			SetLayerConfigDirty();
		}
		else if (pAction == kFBDisconnectedSrc)
		{
			DisconnectSrc(pPlug);
			SetLayerConfigDirty();
		}
	}

	return ParentClass::PlugNotify(pAction, pThis, pIndex, pPlug, pConnectionType, pNewPlug);
}

bool TextureUserEffect::EvaluateAnimationNodes(FBEvaluateInfo* pEvaluateInfo)
{
	const bool result = ParentClass::EvaluateAnimationNodes(pEvaluateInfo);

	// time dependent effects need a new composition for each frame
	if (CustomComposition && UpdateMode == eTextureUserEffectUpdateEveryFrame && GetUserEffect())
	{
		SetLayerConfigDirty();
	}

	return result;
}

void TextureUserEffect::TextureLayerComposition(FBTime pTime, FBTime pTimeInCurrentTimeRef, int pWidth, int pHeight)
{
	// NOTE: render-to-texture target is already set up by MoBu, size of the target is pWidth x pHeight
	EffectShaderUserObject* userEffect = GetUserEffect();

	if (!CustomComposition || !userEffect)
	{
		ParentClass::TextureLayerComposition(pTime, pTimeInCurrentTimeRef, pWidth, pHeight);
		return;
	}

	if (pWidth <= 0 || pHeight <= 0)
		return;

	RenderUserEffect(userEffect, pTime, pTimeInCurrentTimeRef, pWidth, pHeight);
}

void TextureUserEffect::ClearTarget(int width, int height)
{
	const FBColorAndAlpha bgColor = BackgroundColor;

	glPushAttrib(GL_VIEWPORT_BIT | GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, width, height);
	glClearColor(static_cast<float>(bgColor[0]), static_cast<float>(bgColor[1]), static_cast<float>(bgColor[2]), static_cast<float>(bgColor[3]));
	glClear(GL_COLOR_BUFFER_BIT);
	glPopAttrib();
}

void TextureUserEffect::RenderUserEffect(EffectShaderUserObject* userEffect, FBTime pTime, FBTime pTimeInCurrentTimeRef, int width, int height)
{
	// 0. a graphics context change, all hardware resources are not valid any more

	void* const currentContext = static_cast<void*>(wglGetCurrentContext());
	if (currentContext != mGLContext)
	{
		if (mBuffers)
		{
			mBuffers->OnContextChanged();
		}
		mEffectMap.clear();
		userEffect->SetReloadShadersState(true);
		mGLContext = currentContext;
		mWarmupFrames = WARMUP_FRAMES;
	}

	if (!mBuffers)
	{
		mBuffers = std::make_unique<PostEffectBuffers>();
	}

	if (mLastEffect != userEffect)
	{
		mLastEffect = userEffect;
		mWarmupFrames = WARMUP_FRAMES;
	}

	// 1. make sure the effect shader is loaded

	if (userEffect->IsNeedToReloadShaders())
	{
		userEffect->DoReloadShaders();
		mWarmupFrames = WARMUP_FRAMES;
	}

	PostEffectBufferShader* shader = userEffect->GetUserShaderPtr();
	if (!shader || !shader->IsActive() || !userEffect->Active
		|| !shader->GetShaderPtr() || !shader->GetShaderPtr()->IsValid())
	{
		mUnsupportedSamplers.store(0, std::memory_order_release);
		ClearTarget(width, height);
		return;
	}

	// 2. preserve MoBu render-to-texture state, the effect rendering is going to rebind a framebuffer and change matrices

	GLint savedFBO = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &savedFBO);
	GLint savedDrawBuffer = 0;
	glGetIntegerv(GL_DRAW_BUFFER, &savedDrawBuffer);

	glPushAttrib(GL_VIEWPORT_BIT | GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_CURRENT_BIT);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_FOG);
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);

	// 3. context cache, there is no scene camera, so all the camera related values are defaults

	const double localTime = pTimeInCurrentTimeRef.GetSecondDouble();
	FBTime systemTime = FBSystem::TheOne().SystemTime;
	const double sysTime = systemTime.GetSecondDouble();

	{
		PostEffectContextProxy::Parameters& params = mCache.parameters;
		params.x = 0;
		params.y = 0;
		params.w = width;
		params.h = height;
		params.localTime = localTime;
		params.localTimeDT = localTime - mLastLocalTime;
		params.sysTime = sysTime;
		params.sysTimeDT = sysTime - mLastSysTime;
		params.localFrame = static_cast<int>(pTime.GetFrame());
		params.isSkipFrame = false;
		mLastLocalTime = localTime;
		mLastSysTime = sysTime;

		std::time_t now = std::time(nullptr);
		std::tm localTm;
		localtime_s(&localTm, &now);
		mCache.iDate[0] = static_cast<float>(localTm.tm_year + 1900);
		mCache.iDate[1] = static_cast<float>(localTm.tm_mon + 1);
		mCache.iDate[2] = static_cast<float>(localTm.tm_mday);
		mCache.iDate[3] = static_cast<float>(localTm.tm_hour * 3600 + localTm.tm_min * 60 + localTm.tm_sec);
	}

	// 4. collect property values, the same map is used to write and to read (single thread)

	PostEffectContextProxy proxy(
		nullptr, // camera
		nullptr, // evaluate info
		nullptr, // standard effects collection
		nullptr, // post process data
		nullptr, // effects chain
		&mEffectMap,
		mCache);

	const ShaderPropertyScheme* schemeBefore = shader->GetPropertySchemePtr();

	EffectShaderPropertyProcessor propertyProcessor;
	propertyProcessor.CollectUIValues(userEffect, &proxy, shader, -1);

	// 5. the input and the output buffer

	GLuint srcTextureId = 0;
	if (Layers.GetCount() > 0 && Layers[0])
	{
		FBTexture* inputTexture = Layers[0];
		if (inputTexture->TextureOGLId == 0)
		{
			inputTexture->OGLInit();
		}
		srcTextureId = inputTexture->TextureOGLId;
	}

	const std::string bufferName = std::string("TextureUserEffect_") + GetFullName();
	const uint32_t bufferNameKey = xxhash32(bufferName);

	constexpr bool autoResize = false;
	FrameBuffer* buffer = mBuffers->RequestFramebuffer(bufferNameKey, width, height,
		PostEffectBuffers::GetFlagsForSingleColorBuffer(), 1, autoResize);

	bool isRendered = false;
	if (buffer && buffer->GetFrameBuffer() != 0)
	{
		PostEffectRenderContext renderContext;
		renderContext.buffers = mBuffers.get();
		renderContext.srcTextureId = srcTextureId;
		renderContext.depthTextureId = 0;
		renderContext.width = width;
		renderContext.height = height;
		renderContext.targetFramebuffer = buffer;
		renderContext.colorAttachment = 0;
		renderContext.generateMips = false;
		renderContext.userTextureSlot = CommonEffect::UserSamplerSlot;

		glActiveTexture(GL_TEXTURE0);
		shader->Render(renderContext, &proxy);
		isRendered = true;

		// release texture slots that were used by the effect
		for (GLint slot = renderContext.userTextureSlot - 1; slot >= 0; --slot)
		{
			glActiveTexture(GL_TEXTURE0 + slot);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		glActiveTexture(GL_TEXTURE0);
	}

	// 6. samplers of the scene render pass are not available here

	int unsupported = 0;
	const ShaderPropertyScheme* schemeAfter = shader->GetPropertySchemePtr();
	if (schemeAfter && !schemeAfter->IsEmpty())
	{
		if (shader->IsDepthSamplerUsed()) unsupported |= UNSUPPORTED_DEPTH;
		if (shader->IsLinearDepthSamplerUsed()) unsupported |= UNSUPPORTED_LINEAR_DEPTH;
		if (shader->IsWorldNormalSamplerUsed()) unsupported |= UNSUPPORTED_NORMAL;
		if (shader->IsMaskSamplerUsed()) unsupported |= UNSUPPORTED_MASK;
	}
	mUnsupportedSamplers.store(unsupported, std::memory_order_release);

	// 7. restore MoBu target and copy the result into it

	glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(savedFBO));
	glDrawBuffer(static_cast<GLenum>(savedDrawBuffer));

	if (isRendered)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, buffer->GetFrameBuffer());
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(savedFBO));

		glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(savedFBO));
	}
	else
	{
		ClearTarget(width, height);
	}

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glPopAttrib();

	mBuffers->OnFrameRendered();

	// 8. property schemes are built during the first renders (user properties are created from the shader uniforms)
	//  and the values we have collected are incomplete, repeat the composition

	if (schemeAfter != schemeBefore)
	{
		mWarmupFrames = WARMUP_FRAMES;
	}
	if (mWarmupFrames > 0)
	{
		--mWarmupFrames;
		SetLayerConfigDirty();
	}
}
