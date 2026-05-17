
/** \file   PostProcessContextData.cxx

    Sergei <Neill3d> Solokhin 2018-2026

    GitHub page - https://github.com/Neill3d/OpenMoBu
    Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "postprocesscontextdata.h"
#include "posteffect_buffers.h"
#include "posteffect_contextmobu.h"
#include "postprocessing_helper.h"

#define IS_INSIDE_MAIN_CYCLE			(mEnterId==1)
#define IS_RENDERING_OFFLINE			(mAttachedFBO[mEnterId-1] > 0)

#define SHADER_SIMPLE_VERTEX			"\\GLSL\\simple.vsh"
#define SHADER_SIMPLE_FRAGMENT			"\\GLSL\\simple.fsh"

#define RENDER_HUD_RECT_TOP				"RectangleTop"
#define RENDER_HUD_RECT_BOTTOM			"RectangleBottom"


//////////////////////////////////////////////////////////////////////////////////
// RenderFrameGate
void RenderFrameGate::Enter()
{
    mEnterId++;

    if (mEnterId == 1)
    {
        mFrameId++;
    }
}

void RenderFrameGate::Leave()
{
    mEnterId--;
}

void RenderFrameGate::Reset()
{
    mEnterId = 0;
}

bool RenderFrameGate::IsFirstEnter() const
{
    return mEnterId == 1;
}

//////////////////////////////////////////////////////////////////////////////////
// PostProcessContextData

void PostProcessContextData::Init()
{
    mStartSystemTime = FBSystem::TheOne().SystemTime;
    mLastSystemTime = std::numeric_limits<double>::max();
    mLastLocalTime = std::numeric_limits<double>::max();
	mIsTimeInitialized = false;
    mVideoRendering = false;
    mEvaluatePaneCount = 0;
    mRenderPaneCount = 0;
	SetReadyToEvaluate(false);

    mSchematicViewIndex = -1;
    for (int i = 0; i < 4; ++i)
    {
        mViewerViewport[i] = 0;
    }

    for (int n = 0; n < MAX_PANE_COUNT; ++n)
    {
        mPaneEffectBuffers[n].reset(new PostEffectBuffers());
	}

    //
    mMainFrameBuffer.InitTextureInternalFormat();
}

void PostProcessContextData::Evaluate(FBTime systemTime, FBTime localTime, FBEvaluateInfo* pEvaluateInfoIn)
{
    const bool ready = IsReadyToEvaluate();
    if (!ready || FBMergeTransactionIsOn())
    {
        // in a process of shaders reloading, skip evaluation for now
        return;
    }
    
    PostEffectContextProxy::Parameters contextParameters;
    PrepareContextParameters(contextParameters, systemTime, localTime);

    // for all post processing view panes, evaluate their effect chains

    for (int nPane = 0; nPane < mEvaluatePaneCount; ++nPane)
    {
        const SPaneData& pane = mEvaluatePanes[nPane];

        if (!pane.data || !pane.camera || !pane.fxContext)
        {
            continue;
        }
        
		PrepareContextParametersForCamera(contextParameters, pane.camera, nPane);
        pane.fxContext->Evaluate(pEvaluateInfoIn, pane.camera, contextParameters);
    }
}


bool PostProcessContextData::IsReadyToEvaluate() const
{
	return isReadyToEvaluate.load(std::memory_order_acquire);
}
void PostProcessContextData::SetReadyToEvaluate(bool ready)
{
	isReadyToEvaluate.store(ready, std::memory_order_release);
}

bool PostProcessContextData::IsNeedToResetPaneSettings() const
{
	return isNeedToResetPaneSettings.load(std::memory_order_acquire);
}

void PostProcessContextData::SetNeedToResetPaneSettings(bool reset)
{
	isNeedToResetPaneSettings.store(reset, std::memory_order_release);
}

void PostProcessContextData::ReloadShaders(PostPersistentData* data, PostEffectContextMoBu* fxContext, 
    FBEvaluateInfo* pEvaluateInfoIn, FBCamera* pCamera, const PostEffectContextProxy::Parameters& contextParameters)
{
    if (!fxContext->ReloadShaders())
    {
        LOGE("[PostProcessContextData::ReloadShaders] failed to reload shaders!\n");
        data->Active = false;
        return;
    }

    fxContext->Evaluate(pEvaluateInfoIn, pCamera, contextParameters);
    fxContext->Synchronize();
}

void PostProcessContextData::VideoRenderingBegin()
{
	ENSURE(!mVideoRendering);
	mVideoRendering = true;
}
void PostProcessContextData::VideoRenderingEnd()
{
	ENSURE(mVideoRendering);
	mVideoRendering = false;
}

void PostProcessContextData::UpdatePostProcessingFlag()
{
    mHasPostProcessing = false;

    for (int i = 0; i < mRenderPaneCount; ++i)
    {
        if (mRenderPanes[i].hasPostProcess)
        {
            mHasPostProcessing = true;
            break;
        }
    }
}

void PostProcessContextData::PrepareEachPaneCamera()
{
    // grab the whole viewer

    mViewerViewport[0] = mViewerViewport[1] = 0;
    mViewerViewport[2] = mViewerViewport[3] = 0;

    FBSystem& system = FBSystem::TheOne();
    FBRenderer* pRenderer = system.Renderer;

    mSchematicViewIndex = pRenderer->GetSchematicViewPaneIndex();
    mRenderPaneCount = pRenderer->GetPaneCount();

    for (int i = 0; i < mRenderPaneCount; ++i)
    {
        SPaneData& pane = mRenderPanes[i];

        FBCamera* pCamera = pRenderer->GetCameraInPane(i);
        const bool validCamera = (i != mSchematicViewIndex && pCamera && !pCamera->SystemCamera);

        FBCamera* prevCamera = pane.camera;
        pane.camera = validCamera ? pCamera : nullptr;
        pane.paneIndex = i;
		pane.fxContext = nullptr;
		pane.isCameraChanged = (prevCamera != pane.camera);
        pane.hasValidCamera = validCamera;
    }

    for (int i = 0; i < mRenderPaneCount; ++i)
    {
        FBCamera* pCamera = mRenderPanes[i].camera;
        if (!pCamera)
            continue;

        bool paneSharesCamera = false;
        for (int j = 0; j < mRenderPaneCount; ++j)
        {
            if (i != j)
            {
                FBCamera* pOtherCamera = mRenderPanes[j].camera;
                if (pCamera == pOtherCamera)
                {
                    paneSharesCamera = true;
                    break;
                }
            }
        }

        int x = pCamera->CameraViewportX;
        int y = pCamera->CameraViewportY;
        int w = pCamera->CameraViewportWidth;
        int h = pCamera->CameraViewportHeight;

        if (w <= 0 || h <= 0)
            continue;

        //
        FBCameraFrameSizeMode cameraFrameSizeMode;
        pCamera->FrameSizeMode.GetData(&cameraFrameSizeMode, sizeof(FBCameraFrameSizeMode), FBGetDisplayInfo());
        const bool bIsFrameSizeWindow = kFBFrameSizeWindow == cameraFrameSizeMode;

        w += (bIsFrameSizeWindow) ? x : 2 * x;
        h += (bIsFrameSizeWindow) ? y : 2 * y;

        if (paneSharesCamera)
        {
            w *= 2;
            h *= 2;
        }

        mViewerViewport[2] = (w > mViewerViewport[2]) ? w : mViewerViewport[2];
        mViewerViewport[3] = (h > mViewerViewport[3]) ? h : mViewerViewport[3];
    }
}

bool PostProcessContextData::PrepareEachPanePersistanceData()
{
    FBSystem& mSystem = FBSystem::TheOne();
    FBScene* pScene = mSystem.Scene;

    for (int i = 0; i < MAX_PANE_COUNT; ++i)
    {
        mRenderPanes[i].data = nullptr;
        mRenderPanes[i].hasPostProcess = false;
    }

    // find a global settings (without camera attachments)
    PostPersistentData* pGlobalData = nullptr;

    for (int i = 0, count = pScene->UserObjects.GetCount(); i < count; ++i)
    {
        if (FBIS(pScene->UserObjects[i], PostPersistentData))
        {
            FBUserObject* pUserObject = pScene->UserObjects[i];
            PostPersistentData* pData = static_cast<PostPersistentData*>(pUserObject);

            if (pData->Active && (!pData->UseCameraObject || !pData->Camera.GetCount()))
            {
                pGlobalData = pData;
            }
        }
    }

    // looking for exclusing values

    for (int i = 0; i < MAX_PANE_COUNT; ++i)
    {
        if (FBCamera* pPaneCamera = mRenderPanes[i].camera)
        {
            const int dstCount = pPaneCamera->GetDstCount();
            for (int j = 0; j < dstCount; ++j)
            {
                FBPlug* pdst = pPaneCamera->GetDst(j);
                if (FBIS(pdst, PostPersistentData))
                {
                    PostPersistentData* pData = static_cast<PostPersistentData*>(pdst);

                    if (pData->Active && pData->UseCameraObject)
                    {
                        mRenderPanes[i].data = pData;
                        mRenderPanes[i].hasPostProcess = true;
                        break;
                    }
                }
            }
        }

        // if exclusive pane settings is not assign, then try to assign global one
        if (!mRenderPanes[i].data && mRenderPanes[i].hasValidCamera && pGlobalData)
        {
            mRenderPanes[i].data = pGlobalData;
            mRenderPanes[i].hasPostProcess = true;
        }
    }

    return true;
}

void PostProcessContextData::PrepareEachPaneContext()
{
    FBSystem& system = FBSystem::TheOne();
    FBRenderer* pRenderer = system.Renderer;

    mSchematicViewIndex = pRenderer->GetSchematicViewPaneIndex();
    mRenderPaneCount = pRenderer->GetPaneCount();

    static const PostEffectContextProxy::Parameters emptyParameters{};

    for (int i = 0; i < mRenderPaneCount; ++i)
    {
        SPaneData& pane = mRenderPanes[i];

        if (!pane.camera || !pane.data)
            continue;

        if (!mFXContexts[i] || pane.isCameraChanged)
        {
            mFXContexts[i] = std::make_unique<PostEffectContextMoBu>(
                pane.camera,
                nullptr,
                pane.data,
                nullptr,
                &standardEffectsCollection,
                emptyParameters);
        }
        else
        {
            mFXContexts[i]->SetPostProcessData(pane.data);
        }

        pane.fxContext = mFXContexts[i].get();
    }
}

void PostProcessContextData::PreparePaneBuffers()
{
    // enterid is 1 and attach index is 0
    const int enterId = mFrameGate.GetEnterId() - 1;
    ENSURE(enterId >= 0 && enterId < MAX_ATTACH_STACK);
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &mAttachedFBO[enterId]);

    //
    // resize, alloc shaders, etc.
    LoadSimpleBlitShader();

    // resize each pane framebuffer
    for (int i = 0; i < mRenderPaneCount; ++i)
    {
		SPaneData& pane = mRenderPanes[i];
        if (!pane.data)
            continue;

        FBCamera* pCamera = pane.camera;
        if (!pCamera)
            continue;

        const int w = pCamera->CameraViewportWidth;
        const int h = pCamera->CameraViewportHeight;

        if (w <= 0 || h <= 0)
            continue;

        // next line could change current fbo

        bool usePreview = pane.data->OutputPreview;
        double scaleF = pane.data->OutputScaleFactor;

        mPaneEffectBuffers[i]->ReSize(w, h, usePreview, scaleF);
    }

    if (mAttachedFBO[enterId] == 0 && mViewerViewport[2] > 1 && mViewerViewport[3] > 1)
    {
        ENSURE(mViewerViewport[2] > 0 && mViewerViewport[3] > 0);
        mMainFrameBuffer.ReSize(mViewerViewport[2], mViewerViewport[3], 1.0, 0, 0);

        mMainFrameBuffer.BeginRender();

        glViewport(0, 0, mViewerViewport[2], mViewerViewport[3]);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mMainFrameBuffer.EndRender();
    }
}

void PostProcessContextData::Synchronize()
{
    const int enterId = mFrameGate.GetEnterId();
    ENSURE(enterId == 0);
    mFrameGate.Reset();

    if (IsNeedToResetPaneSettings())
    {
        // reset all pane settings
        ResetPaneSettings();
        SetNeedToResetPaneSettings(false);
        SetReadyToEvaluate(false);
        return;
    }

    // sync mEvaluatePanes with mRenderPanes
    mEvaluatePaneCount = mRenderPaneCount;
    bool isReady = mEvaluatePaneCount > 0;

    for (int nPane = 0; nPane < mEvaluatePaneCount; ++nPane)
    {
        SPaneData& evalPane = mEvaluatePanes[nPane];
        SPaneData& renderPane = mRenderPanes[nPane];
        evalPane.CopyFrom(renderPane);

        if (!renderPane.hasPostProcess)
        {
            continue;
        }

        if (!evalPane.data || !evalPane.camera || !evalPane.fxContext)
        {
            isReady = false;
            continue;
        }

        if (evalPane.fxContext->IsAnyReloadShadersRequested())
        {
            isReady = false;
            break;
        }

        evalPane.fxContext->Synchronize();
    }

    SetReadyToEvaluate(isReady);
}

////////////////////////////////////////////////////////////////////////////////////
// RenderBeforeRender
void PostProcessContextData::RenderBeforeRender()
{
    mFrameGate.Enter();

    if (mFrameGate.IsFirstEnter())
    {
        PrepareEachPaneCamera();
        PrepareEachPanePersistanceData();
        PrepareEachPaneContext();
        UpdatePostProcessingFlag();

        if (HasPostProcessing())
        {
            PreparePaneBuffers();
        }
    }

    // attachment point
    if (HasPostProcessing())
    {
        const int enterId = mFrameGate.GetEnterId() - 1; // enter id 1 has attached index 0
        ENSURE(enterId >= 0 && enterId < MAX_ATTACH_STACK);
        if (mAttachedFBO[enterId] > 0)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, mAttachedFBO[enterId]);
        }
        else
        {
            mMainFrameBuffer.BeginRender();
        }

        glEnable(GL_DEPTH_TEST);
    }
}

/////////////////////////////////////////////////////////////////////////////////////
// RenderAfterRender - post processing work after main scene rendering is finished


bool PostProcessContextData::RenderAfterRender(FBTime systemTime, FBTime localTime, FBEvaluateInfo* pEvaluateInfoIn)
{
    systemTime -= mStartSystemTime;

    const bool isFirstEnter = mFrameGate.IsFirstEnter();
    mFrameGate.Leave();

    // we dicrement enterid, so enterid is equal to attached index
    const int enterId = mFrameGate.GetEnterId();
    if (enterId < 0)
    {
        // we leave the frame from the manipulator, don't need to process the frame once again
        // NOTE: manipulator calls external renderAfterRender for each pane!
        mFrameGate.Reset();
        return false;
    }
    
    if (HasPostProcessing() && isFirstEnter)
    {
        glDisable(GL_MULTISAMPLE);
        glDisable(GL_DEPTH_TEST);

        mMainFrameBuffer.EndRender(); // unbind any framebuffer
        
        const GLuint fboInOut = (mAttachedFBO[enterId] > 0) ? mAttachedFBO[enterId] : mMainFrameBuffer.GetFinalFBO();

        // this is a hack for Reflection shader (to avoid error overhead on glReadBuffers(GL_BACK) )
#ifndef OGL_DEBUG
        EmptyGLErrorStack();
#endif
        PostEffectContextProxy::Parameters params;
		PrepareContextParameters(params, systemTime, localTime);
        
        // NOTE: at the moment only first pane could do effect processing, no need to render more panes than that
        constexpr const int RENDER_PANE_LIMIT{ 1 };
        for (int nPane = 0; nPane < mRenderPaneCount && nPane < RENDER_PANE_LIMIT; ++nPane)
        {
            SPaneData& pane = mRenderPanes[nPane];
			RenderPane(pEvaluateInfoIn, pane, mPaneEffectBuffers[nPane].get(), params, fboInOut);
        }

		BuffersPoolCollection();

        if (mAttachedFBO[enterId] > 0)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, mAttachedFBO[enterId]);
            glDrawBuffer(GL_COLOR_ATTACHMENT0);
        }
        else
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // DONE: draw a resulted rect
            // render background
            if (!mMainFrameBuffer.isFboAttached() && mShaderSimple.get())
            {
                mShaderSimple->Bind();

                glBindTexture(GL_TEXTURE_2D, mMainFrameBuffer.GetFinalColorObject());
                drawOrthoQuad2d(mViewerViewport[2], mViewerViewport[3]);

                mShaderSimple->UnBind();
            }
        }

        mLastSystemTime = systemTime.GetSecondDouble();
        mLastLocalTime = localTime.GetSecondDouble();
		mIsTimeInitialized = true;
    }

    return true;
}


void PostProcessContextData::RenderPane(FBEvaluateInfo* pEvaluateInfoIn, 
    SPaneData& pane, 
    PostEffectBuffers* paneBuffers, 
    PostEffectContextProxy::Parameters& params,
    GLuint fboInOut)
{
    if (!pane.IsValid() || !paneBuffers)
        return;

    FBCamera* pCamera = pane.camera;
    PostEffectContextMoBu* fxContext = pane.fxContext;

    PrepareContextParametersForCamera(params, pCamera, pane.paneIndex);

    // not in schematic view
    if (params.w <= 0 || params.w > paneBuffers->GetWidth())
        return;

    if (!IsReadyToEvaluate() && fxContext->IsAnyReloadShadersRequested())
    {
        ReloadShaders(pane.data, fxContext, pEvaluateInfoIn, pCamera, params);
    }

    const bool isReadyToRender = standardEffectsCollection.IsOk()
        && fxContext->IsReadyToRender()
        && !fxContext->IsAnyReloadShadersRequested();

    if (!isReadyToRender)
        return;

    // 1. blit a pane area of a main buffer into a pane buffer

    DoubleFramebufferRequestScope doubleFramebufferRequest(fxContext->GetFXChain(), paneBuffers);
    
    BlitFBOToFBOOffset(fboInOut, params.x, params.y, params.w, params.h, 0,
        doubleFramebufferRequest->GetPtr()->GetFrameBuffer(), 0, 0, params.w, params.h, doubleFramebufferRequest->GetWriteAttachment(),
        true, false, false, false); // copy depth and no any other attachments

    // 2. process it

    if (!fxContext->Render(pEvaluateInfoIn, paneBuffers))
        return;

    // special test for an android device, send preview image by udp
#if BROADCAST_PREVIEW == 1
    if (true == mSendPreview)
    {
        SendPreview(paneBuffers);
        mPaneSettings[nPane]->IsSynced = mIsSynced;
        if (mIsSynced)
        {
            mPaneSettings[nPane]->DeviceAddress = FBVector4d((double)mSendAddress.GetA(), (double)mSendAddress.GetB(),
                (double)mSendAddress.GetC(), (double)mSendAddress.GetD());
            mPaneSettings[nPane]->DevicePort = mSendAddress.GetPort();
        }

    }
#endif
    // 2.5 HUDs

    if (true == pane.data->DrawHUDLayer)
    {
        // effect should be ended up with writing into target
        doubleFramebufferRequest->Bind();

        DrawHUD(0, 0, params.w, params.h, mMainFrameBuffer.GetWidth(), mMainFrameBuffer.GetHeight());

        doubleFramebufferRequest->UnBind();
    }

    // 3. blit back a pane area into a full main buffer
    BlitFBOToFBOOffset(doubleFramebufferRequest->GetPtr()->GetFrameBuffer(), 0, 0, params.w, params.h, doubleFramebufferRequest->GetWriteAttachment(),
        fboInOut, params.x, params.y, params.w, params.h, 0,
        false, false, false, false); // don't copy depth or any other color attachment
}

void PostProcessContextData::PrepareContextParameters(PostEffectContextProxy::Parameters& contextParametersOut, FBTime systemTime, FBTime localTime) const
{
    const double sysTimeSecs = systemTime.GetSecondDouble();
    const double localTimeSecs = localTime.GetSecondDouble();

    const double systemTimeDT = (mIsTimeInitialized) ? sysTimeSecs - mLastSystemTime : 0.0;
    const double localTimeDT = (mIsTimeInitialized) ? localTimeSecs - mLastLocalTime : 0.0;

    contextParametersOut.localFrame = static_cast<int>(localTime.GetFrame());
    contextParametersOut.sysTime = sysTimeSecs;
    contextParametersOut.sysTimeDT = systemTimeDT;
    contextParametersOut.localTime = localTimeSecs;
    contextParametersOut.localTimeDT = localTimeDT;
}

void PostProcessContextData::PrepareContextParametersForCamera(PostEffectContextProxy::Parameters& contextParametersOut, FBCamera* pCamera, int nPane) const
{
    if (!pCamera)
        return;

    int viewportX = pCamera->CameraViewportX;
    int viewportY = pCamera->CameraViewportY;
    int viewportWidth = pCamera->CameraViewportWidth;
    int viewportHeight = pCamera->CameraViewportHeight;

    bool isSkipFrame = false;
    if (pCamera->SystemCamera)
    {
        viewportWidth = 0;
        isSkipFrame = true;
    }
    else if (!mVideoRendering || nPane > 0)
    {
        if (nPane == mSchematicViewIndex)
        {
            viewportWidth = 0;
            isSkipFrame = true;
        }
    }

    contextParametersOut.x = viewportX;
    contextParametersOut.y = viewportY;
    contextParametersOut.w = viewportWidth;
    contextParametersOut.h = viewportHeight;
    contextParametersOut.isSkipFrame = isSkipFrame;
}


void PostProcessContextData::BuffersPoolCollection()
{
    for (size_t nPane = 0; nPane < mPaneEffectBuffers.size(); ++nPane)
    {
        if (mPaneEffectBuffers[nPane].get())
        {
            mPaneEffectBuffers[nPane]->OnFrameRendered();
        }
    }
}

bool PostProcessContextData::EmptyGLErrorStack()
{
    bool wasError = false;
    for (GLenum glErr = glGetError(); glErr != GL_NO_ERROR; glErr = glGetError())
    {
        wasError = true;
    }
    return wasError;
}


const bool PostProcessContextData::CheckShadersPath(const char* path) const
{
    const char* test_shaders[] = {
        SHADER_SIMPLE_VERTEX,
        SHADER_SIMPLE_FRAGMENT
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

bool PostProcessContextData::LoadSimpleBlitShader()
{
    if (mShaderSimple.get())
    {
        // already loaded
        return true;
    }
    FBSystem& system = FBSystem::TheOne();
    FBString shadersPath(system.ApplicationPath);
    shadersPath = shadersPath + "\\plugins";

    if (!CheckShadersPath(shadersPath))
    {
        bool found = false;
        const FBStringList& plugin_paths = system.GetPluginPath();

        for (int i = 0; i < plugin_paths.GetCount(); ++i)
        {
            if (CheckShadersPath(plugin_paths[i]))
            {
                shadersPath = plugin_paths[i];
                found = true;
                break;
            }
        }

        if (!found)
        {
            FBTrace("[PostProcessing] Failed to find simple shaders!\n");
            return false;
        }
    }

    auto pNewShader = std::make_unique<GLSLShaderProgram>();

    FBString vertexPath(shadersPath, SHADER_SIMPLE_VERTEX);
    FBString fragmentPath(shadersPath, SHADER_SIMPLE_FRAGMENT);

    if (!pNewShader->LoadShaders(vertexPath, fragmentPath))
    {
        LOGE("Post Processing Simple Shader: %s\n", fragmentPath);
        return false;
    }

    // samplers and locations
    if (pNewShader->Bind())
    {
        if (GLint loc = pNewShader->findLocation("sampler0"); loc >= 0)
        {
            glUniform1i(loc, 0);
        }

        pNewShader->UnBind();
    }

    mShaderSimple = std::move(pNewShader);
    return true;
}

void PostProcessContextData::FreeShaders()
{
    mShaderSimple.reset(nullptr);
}

void PostProcessContextData::FreeBuffers()
{
    mMainFrameBuffer.ChangeContext();

    for (int i = 0; i < MAX_PANE_COUNT; ++i)
    {
		mPaneEffectBuffers[i]->ChangeContext();
    }
}

void PostProcessContextData::ResetPaneSettings()
{
    mEvaluatePaneCount = 0;
    mRenderPaneCount = 0;
	SetReadyToEvaluate(false);
    for (int i = 0; i < MAX_PANE_COUNT; ++i)
    {
        mEvaluatePanes[i].Clear();
        mRenderPanes[i].Clear();
		mFXContexts[i].reset(nullptr);
    }
}

void PostProcessContextData::DrawHUD(int panex, int paney, int panew, int paneh, int vieww, int viewh)
{
    FBSystem& mSystem = FBSystem::TheOne();
    FBScene *pScene = mSystem.Scene;

    {
        glViewport(panex, paney, panew, paneh);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        glOrtho(0.0, panew, 0.0, paneh, -1.0, 1.0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // collect huds

        int numberOfRectElems = 0;
        int numberOfTextElems = 0;

        for (int i = 0, count = pScene->HUDs.GetCount(); i < count; ++i)
        {
            FBHUD *pHud = pScene->HUDs[i];

            for (int j = 0, elemCount = pHud->Elements.GetCount(); j < elemCount; ++j)
            {
                FBHUDElement *pElem = pHud->Elements[j];

                if (FBIS(pElem, FBHUDRectElement))
                {
                    numberOfRectElems += 1;
                }
                else if (FBIS(pElem, FBHUDTextElement))
                {
                    numberOfTextElems += 1;
                }
            }
        }

        if ((numberOfRectElems + numberOfTextElems) <= 0)
            return;

        mRectElements.resize(numberOfRectElems);
        mTextElements.resize(numberOfTextElems);

        // assign elems
        numberOfRectElems = 0;
        numberOfTextElems = 0;

        for (int i = 0, count = pScene->HUDs.GetCount(); i < count; ++i)
        {
            FBHUD *pHud = pScene->HUDs[i];

            for (int j = 0, elemCount = pHud->Elements.GetCount(); j < elemCount; ++j)
            {
                FBHUDElement *pElem = pHud->Elements[j];

                if (FBIS(pElem, FBHUDRectElement))
                {
                    mRectElements[numberOfRectElems] = (FBHUDRectElement*)pElem;
                    numberOfRectElems += 1;
                }
                else if (FBIS(pElem, FBHUDTextElement))
                {
                    mTextElements[numberOfTextElems] = (FBHUDTextElement*)pElem;
                    numberOfTextElems += 1;
                }
            }
        }

        // draw rects

        for (auto iter = begin(mRectElements); iter != end(mRectElements); ++iter)
        {
            if ((*iter)->Show)
            {
                DrawHUDRect(*iter, panex, paney, panew, paneh, vieww, viewh);
            }
        }

        // prep text fonts
        if (mTextElements.size() > 0)
        {

#if defined(HUD_FONT)
            if (mElemFonts.size() != mTextElements.size())
            {
                FreeFonts();
                mElemFonts.resize(mTextElements.size());

                for (int ii = 0; ii < mTextElements.size(); ++ii)
                {
                    CFont *pFont = new CFont();
                    pFont->Init();

                    mElemFonts[ii] = pFont;
                }
            }

            auto textiter = begin(mTextElements);
            auto fontiter = begin(mElemFonts);

            for (; textiter != end(mTextElements); ++textiter, ++fontiter)
            {
                if ((*textiter)->Show)
                {
                    DrawHUDText(*textiter, *fontiter, panex, paney, panew, paneh, vieww, viewh);
                }
            }
#endif
        }
    }
}

void PostProcessContextData::DrawHUDRect(FBHUDRectElement *pRect, int panex, int paney, int panew, int paneh, int vieww, int viewh)
{
    bool posPer = pRect->PositionByPercent;
    bool sclPer = pRect->ScaleByPercent;
    //bool sclAsp = pRect->ScaleUniformly;

    double posx = pRect->X;
    double posy = pRect->Y;

    if (posPer)
    {
        posx = 0.01 * posx * panew;
        posy = 0.01 * posy * paneh;
    }

    double wid = pRect->Width;
    double hei = pRect->Height;

    if (sclPer)
    {
        wid = 0.01 * wid * panew;
        hei = 0.01 * hei * paneh;
    }

    FBHUDElementHAlignment hAlign;
    pRect->Justification.GetData(&hAlign, sizeof(FBHUDElementHAlignment));
    FBHUDElementHAlignment hDock;
    pRect->HorizontalDock.GetData(&hDock, sizeof(FBHUDElementHAlignment));
    FBHUDElementVAlignment vDock;
    pRect->VerticalDock.GetData(&vDock, sizeof(FBHUDElementVAlignment));

    switch (hAlign)
    {
    case kFBHUDLeft:
        break;
    case kFBHUDRight:
        posx -= wid;
        break;
    case kFBHUDCenter:
        posx -= 0.5 * wid;
        break;
    }

    switch (hDock)
    {
    case kFBHUDLeft:
        break;
    case kFBHUDRight:
        posx += panew;
        break;
    case kFBHUDCenter:
        posx += 0.5 * panew;
        break;
    }

    switch (vDock)
    {
    case FBHUDElementVAlignment::kFBHUDBottom:
        break;
    case FBHUDElementVAlignment::kFBHUDTop:
        posy += (paneh - hei);
        break;
    case FBHUDElementVAlignment::kFBHUDVCenter:
        posy += 0.5 * (paneh - hei);
        break;
    }


    FBColorAndAlpha lBackground = pRect->Color;

    // HACK:
    lBackground[3] = 1.0;

    glColor4dv(lBackground);

    const bool lUseBlendForBackground = lBackground[3] <= (254.0 / 255);
    if (lUseBlendForBackground)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    glBegin(GL_QUADS);

    glVertex2d(posx, posy);
    glVertex2d(posx + wid, posy);
    glVertex2d(posx + wid, posy + hei);
    glVertex2d(posx, posy + hei);

    glEnd();

    glDisable(GL_BLEND);
}
#if defined(HUD_FONT)
void PostProcessContextData::DrawHUDText(FBHUDTextElement *pRect, CFont *pFont, int panex, int paney, int panew, int paneh, int vieww, int viewh)
{
    //	if (nullptr == pFont)
    //		return;

    bool posPer = pRect->PositionByPercent;
    bool sclPer = pRect->ScaleByPercent;
    //bool sclAsp = pRect->ScaleUniformly;

    double posx = pRect->X;
    double posy = pRect->Y;

    if (posPer)
    {
        posx = 0.01 * posx * panew;
        posy = 0.01 * posy * paneh;
    }

    double wid = pRect->Width;
    double hei = pRect->Height;

    if (sclPer)
    {
        wid = 0.01 * wid * panew;
        hei = 0.01 * hei * paneh;
    }

    FBHUDElementHAlignment hAlign;
    pRect->Justification.GetData(&hAlign, sizeof(FBHUDElementHAlignment));
    FBHUDElementHAlignment hDock;
    pRect->HorizontalDock.GetData(&hDock, sizeof(FBHUDElementHAlignment));
    FBHUDElementVAlignment vDock;
    pRect->VerticalDock.GetData(&vDock, sizeof(FBHUDElementVAlignment));

    switch (hAlign)
    {
    case kFBHUDLeft:
        break;
    case kFBHUDRight:
        posx -= wid;
        break;
    case kFBHUDCenter:
        posx -= 0.5 * wid;
        break;
    }

    switch (hDock)
    {
    case kFBHUDLeft:
        break;
    case kFBHUDRight:
        posx += panew;
        break;
    case kFBHUDCenter:
        posx += 0.5 * panew;
        break;
    }

    switch (vDock)
    {
    case kFBHUDBottom:
        break;
    case kFBHUDTop:
        posy += (paneh - hei);
        break;
    case kFBHUDCenter:
        posy += 0.5 * (paneh - hei);
        break;
    }


    // get number of characters

    FBString content(pRect->Content);
    FBString refString("");

    char buffer[64] = { 0 };
    FBProperty *pRefProperty = nullptr;

    for (int nprop = 0; nprop < pRect->PropertyList.GetCount(); ++nprop)
    {
        FBProperty *prop = pRect->PropertyList[nprop];
        if (true == prop->IsReferenceProperty())
        {

            for (int nSrc = 0, nSrcCount = prop->GetSrcCount(); nSrc < nSrcCount; ++nSrc)
            {
                FBPlug *plug = prop->GetSrc(nSrc);
                if (FBIS(plug, FBProperty))
                {
                    pRefProperty = (FBProperty*)plug;
                    break;
                }
            }

            if (nullptr != pRefProperty)
                break;
        }
    }

    if (nullptr == pRefProperty)
    {
        sprintf_s(buffer, sizeof(char) * 64, content);
    }
    else
    {

        if (kFBPT_Time == pRefProperty->GetPropertyType() && true == pRect->ForceTimeCodeDisplay)
        {
            FBTime time;
            pRefProperty->GetData(&time, sizeof(FBTime));
            refString = time.GetTimeString(kFBTimeModeDefault, FBTime::eSMPTE);

            sprintf_s(buffer, sizeof(char) * 64, content, refString);
        }
        else if (kFBPT_double == pRefProperty->GetPropertyType())
        {
            double value = 0.0;
            pRefProperty->GetData(&value, sizeof(double));

            sprintf_s(buffer, sizeof(char) * 64, content, value);
        }
        else if (kFBPT_int == pRefProperty->GetPropertyType())
        {
            int value = 0.0;
            pRefProperty->GetData(&value, sizeof(int));

            sprintf_s(buffer, sizeof(char) * 64, content, value);
        }
        else
        {
            refString = pRefProperty->AsString();
            sprintf_s(buffer, sizeof(char) * 64, content, refString);
        }

    }

    const int numchars = static_cast<int>(strlen(buffer));

    // draw a background

    FBColorAndAlpha lColor = pRect->Color;
    FBColorAndAlpha lBackground = pRect->BackgroundColor;

    // HACK:
    lColor[3] = 1.0;
    lBackground[3] = 1.0;

    double fontHei = floor(0.53 * hei);
    // logical average width for each character
    double sclw = 0.65; // *(double)paneh / (double)viewh;
    double fontWid = (wid * sclw) / (double)numchars;
    fontWid = floor(fontWid);


    //
    if (fontHei < 5.0)
        fontHei = 5.0;
    else if (fontHei > 300.0)
        fontHei = 300.0;

    glColor4dv(lBackground);

    const bool lUseBlendForBackground = lBackground[3] <= (254.0 / 255);
    if (lUseBlendForBackground)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    glBegin(GL_QUADS);

    glVertex2d(posx, posy);
    glVertex2d(posx + wid, posy);
    glVertex2d(posx + wid, posy + hei);
    glVertex2d(posx, posy + hei);

    glEnd();

    glDisable(GL_BLEND);


    // draw text
    glColor4dv(lColor);

    pFont->Resize(panew, paneh);

    pFont->TextClear();
    pFont->TextAdd(posx, posy, 0.75f * (float)hei, wid, hei, buffer, static_cast<uint32_t>(strlen(buffer)));

    pFont->Display();
}
#endif

void PostProcessContextData::FreeFonts()
{
#if defined(HUD_FONT)	
    for (auto iter = begin(mElemFonts); iter != end(mElemFonts); ++iter)
    {
        CFont *pFont = *iter;
        if (pFont)
        {
            delete pFont;
            pFont = nullptr;
        }
    }

    mElemFonts.clear();
#endif
}