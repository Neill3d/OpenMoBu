
/** \file   PostProcessContextData.cxx

    Sergei <Neill3d> Solokhin 2018-2022

    GitHub page - https://github.com/Neill3d/OpenMoBu
    Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "postprocesscontextdata.h"
#include "posteffectbuffers.h"
#include "posteffectcontextmobu.h"
#include "postprocessing_helper.h"

#define IS_INSIDE_MAIN_CYCLE			(mEnterId==1)
#define IS_RENDERING_OFFLINE			(mAttachedFBO[mEnterId-1] > 0)

#define SHADER_SIMPLE_VERTEX			"\\GLSL\\simple.vsh"
#define SHADER_SIMPLE_FRAGMENT			"\\GLSL\\simple.fsh"

#define RENDER_HUD_RECT_TOP				"RectangleTop"
#define RENDER_HUD_RECT_BOTTOM			"RectangleBottom"


void PostProcessContextData::Init()
{
    mStartSystemTime = FBSystem::TheOne().SystemTime;
    mLastSystemTime = std::numeric_limits<double>::max();
    mLastLocalTime = std::numeric_limits<double>::max();
    mVideoRendering = false;
    mEvaluatePaneCount = 0;
    mRenderPaneCount = 0;
    isReadyToEvaluate = false;

    for (int i = 0; i < 4; ++i)
    {
        mViewerViewport[i] = 0;
        mSchematicView[i] = false;
    }

    mEffectBuffers0.reset(new PostEffectBuffers());
    mEffectBuffers1.reset(new PostEffectBuffers());
    mEffectBuffers2.reset(new PostEffectBuffers());
    mEffectBuffers3.reset(new PostEffectBuffers());

    //
    mMainFrameBuffer.InitTextureInternalFormat();
}

void PostProcessContextData::Evaluate(FBTime systemTime, FBTime localTime, FBEvaluateInfo* pEvaluteInfoIn)
{
    if (!isReadyToEvaluate)
    {
        return;
    }

    //FBSystem& mSystem = FBSystem::TheOne();
	//FBRenderer* pRenderer = mSystem.Renderer;
    //if (!pRenderer)
	//	return;

    systemTime = systemTime - mStartSystemTime;

    const double sysTimeSecs = systemTime.GetSecondDouble();
    const double localTimeSecs = localTime.GetSecondDouble();

    if (mLastSystemTime == std::numeric_limits<double>::max())
        mLastSystemTime = sysTimeSecs;
    if (mLastLocalTime == std::numeric_limits<double>::max())
        mLastLocalTime = localTimeSecs;

    const double systemTimeDT = sysTimeSecs - mLastSystemTime;
    const double localTimeDT = localTimeSecs - mLastLocalTime;

    IPostEffectContext::Parameters contextParameters;
	contextParameters.localFrame = static_cast<int>(localTime.GetFrame());
	contextParameters.sysTime = sysTimeSecs;
	contextParameters.sysTimeDT = systemTimeDT;
	contextParameters.localTime = localTimeSecs;
	contextParameters.localTimeDT = localTimeDT;

    // for all post processing view panes, evaluate their effect chains

    for (int nPane = 0; nPane < mEvaluatePaneCount; ++nPane)
    {
        const SPaneData& pane = mEvaluatePanes[nPane];
        FBCamera* pCamera = pane.camera; // pRenderer->GetCameraInPane(nPane);

        if (!pane.data || pane.data->IsNeedToReloadShaders() || !pCamera)
            continue;

		int viewportWidth = pCamera->CameraViewportWidth;
		int viewportHeight = pCamera->CameraViewportHeight;

        if (pCamera->SystemCamera)
        {
            viewportWidth = 0;
        }
        else if (!mVideoRendering || nPane > 0)
        {
            if (mSchematicView[nPane])
                viewportWidth = 0;
        }

		contextParameters.w = viewportWidth;
		contextParameters.h = viewportHeight;

        auto iter = mPostFXContextsMap.find(pane.data);
        if (iter == end(mPostFXContextsMap))
        {
            // create new context
            auto effectContext = new PostEffectContextMoBu(pCamera, nullptr, pane.data, pEvaluteInfoIn, contextParameters);
            mPostFXContextsMap.emplace(pane.data, effectContext);
            iter = mPostFXContextsMap.find(pane.data);
		}

		PostEffectContextMoBu* fxContext = iter->second.get();
		fxContext->Evaluate(pEvaluteInfoIn, pCamera, standardEffectsCollection, contextParameters);
    }
}

void PostProcessContextData::Synchronize()
{
    isReadyToEvaluate = false;

    // sync mEvaluatePanes with mRenderPanes
    mEvaluatePaneCount = mRenderPaneCount;

    for (int nPane = 0; nPane < mEvaluatePaneCount; ++nPane)
    {
        SPaneData& evalPane = mEvaluatePanes[nPane];
        evalPane = mRenderPanes[nPane];

        if (!evalPane.data || !evalPane.camera)
            continue;

        if (evalPane.data->IsNeedToReloadShaders())
        {
            isReadyToEvaluate = false;
            break;
		}

        auto iter = mPostFXContextsMap.find(evalPane.data);
        if (iter != end(mPostFXContextsMap))
        {
            PostEffectContextMoBu* fxContext = iter->second.get();
            fxContext->Synchronize();
		}
        isReadyToEvaluate = true;
    }
}

////////////////////////////////////////////////////////////////////////////////////
// RenderBeforeRender
void PostProcessContextData::RenderBeforeRender(const bool processCompositions, const bool renderToBuffer)
{
    mEnterId++;
    
    // attachment point
    if (processCompositions)
    {
        FBSystem& mSystem = FBSystem::TheOne();
        FBRenderer* renderer = mSystem.Renderer;

        for (int nPane = 0; nPane < mRenderPaneCount; ++nPane)
        {
            FBCamera *pCamera = renderer->GetCameraInPane(nPane);
            if (nullptr == pCamera || true == pCamera->SystemCamera
                || true == mVideoRendering || true == mSchematicView[nPane])
            {
                mRenderPanes[nPane].camera = nullptr;
                continue;
            }
            mRenderPanes[nPane].camera = pCamera;

            PostEffectBuffers *currBuffers = nullptr;
            switch (nPane)
            {
            case 0:
                currBuffers = mEffectBuffers0.get();
                break;
            case 1:
                currBuffers = mEffectBuffers1.get();
                break;
            case 2:
                currBuffers = mEffectBuffers2.get();
                break;
            case 3:
                currBuffers = mEffectBuffers3.get();
                break;
            }
        }

        // it will use attached dimentions, if any external buffer is exist
        
        mViewerViewport[2] = mMainFrameBuffer.GetBufferWidth();
        mViewerViewport[3] = mMainFrameBuffer.GetBufferHeight();

        mMainFrameBuffer.BeginRender();

        glViewport(0, 0, mViewerViewport[2], mViewerViewport[3]);

        glEnable(GL_DEPTH_TEST);
    }
}

/////////////////////////////////////////////////////////////////////////////////////
// RenderAfterRender - post processing work after main scene rendering is finished


bool PostProcessContextData::RenderAfterRender(bool processCompositions, bool renderToBuffer, 
    FBTime systemTime, FBTime localTime, FBEvaluateInfo* pEvaluateInfoIn)
{
    bool lStatus = false;

    if (mEnterId <= 0)
        return lStatus;

    //FBSystem& mSystem = FBSystem::TheOne();
	//FBRenderer* pRenderer = mSystem.Renderer;
	//if (!pRenderer)
	//	return lStatus;

    /////////////
    // !!!
    if (processCompositions && 1 == mEnterId)
    {
        
        glDisable(GL_MULTISAMPLE);
        glDisable(GL_DEPTH_TEST);

        mMainFrameBuffer.EndRender();
        mMainFrameBuffer.PrepForPostProcessing(false);	// ?!

        CHECK_GL_ERROR();

        // this is a hack for Reflection shader (to avoid error overhead on glReadBuffers(GL_BACK) )
#ifndef OGL_DEBUG
        EmptyGLErrorStack();
#endif
        
        systemTime = systemTime - mStartSystemTime;
        const double sysTimeSecs = systemTime.GetSecondDouble();
        const double localTimeSecs = localTime.GetSecondDouble();

        if (mLastSystemTime == std::numeric_limits<double>::max())
            mLastSystemTime = sysTimeSecs;
        if (mLastLocalTime == std::numeric_limits<double>::max())
            mLastLocalTime = localTimeSecs;

        const double systemTimeDT = sysTimeSecs - mLastSystemTime;
        const double localTimeDT = localTimeSecs - mLastLocalTime;

        mLastSystemTime = sysTimeSecs;
        mLastLocalTime = localTimeSecs;

        IPostEffectContext::Parameters contextParameters;
        contextParameters.localFrame = static_cast<int>(localTime.GetFrame());
        contextParameters.sysTime = sysTimeSecs;
        contextParameters.sysTimeDT = systemTimeDT;
        contextParameters.localTime = localTimeSecs;
        contextParameters.localTimeDT = localTimeDT;

        for (int nPane = 0; nPane < mRenderPaneCount; ++nPane)
        {
            SPaneData& pane = mRenderPanes[nPane];
            FBCamera* pCamera = pane.camera;
            if (!pCamera)
                continue;

            int localViewport[4] = {
                pCamera->CameraViewportX,
                pCamera->CameraViewportY,
                pCamera->CameraViewportWidth,
                pCamera->CameraViewportHeight
            };
            
            if (true == pCamera->SystemCamera)
            {
                localViewport[2] = 0;
            }
            else if (false == mVideoRendering || nPane > 0)
            {
                if (true == mSchematicView[nPane])
                    localViewport[2] = 0;
            }
            
            PostEffectBuffers *currBuffers = nullptr;
            switch (nPane)
            {
            case 0:
                currBuffers = mEffectBuffers0.get();
                break;
            case 1:
                currBuffers = mEffectBuffers1.get();
                break;
            case 2:
                currBuffers = mEffectBuffers2.get();
                break;
            case 3:
                currBuffers = mEffectBuffers3.get();
                break;
            }

            // not in schematic view
            if (localViewport[2] > 0 && nullptr != currBuffers
                && localViewport[2] == currBuffers->GetWidth()
                && pane.data)
            {
                auto iter = mPostFXContextsMap.find(pane.data);
                if (iter == end(mPostFXContextsMap))
                {
                    continue;
                }
                PostEffectContextMoBu* fxContext = iter->second.get();

                // 1. blit part of a main screen

                DoubleFramebufferRequestScope doubleFramebufferRequest(fxContext->GetFXChain(), currBuffers);

                if (!mMainFrameBuffer.isFboAttached())
                {
                    BlitFBOToFBOOffset(mMainFrameBuffer.GetFinalFBO(), localViewport[0], localViewport[1], localViewport[2], localViewport[3], 0,
                        doubleFramebufferRequest->GetPtr()->GetFrameBuffer(), 0, 0, localViewport[2], localViewport[3], doubleFramebufferRequest->GetWriteAttachment(),
                        true, false, false, false); // copy depth and no any other attachments
                }
                else
                {
                    BlitFBOToFBOOffset(mMainFrameBuffer.GetAttachedFBO(), localViewport[0], localViewport[1], localViewport[2], localViewport[3], 0,
                        doubleFramebufferRequest->GetPtr()->GetFrameBuffer(), 0, 0, localViewport[2], localViewport[3], doubleFramebufferRequest->GetWriteAttachment(),
                        true, false, false, false); // copy depth and no any other attachments
                }


                // 2. process it

                contextParameters.w = localViewport[2];
                contextParameters.h = localViewport[3];

				fxContext->UpdateEvaluateInfo(pEvaluateInfoIn);
				fxContext->UpdateContextParameters(pCamera, contextParameters);
                
                if (!isReadyToEvaluate && pane.data->IsNeedToReloadShaders())
                {
                    standardEffectsCollection.ChangeContext();
                    fxContext->ChangeContext();
                    pane.data->SetReloadShadersState(false);
                }

                bool isReadyToRender = true;
                isReadyToRender &= standardEffectsCollection.Prep(pane.data);
                isReadyToRender &= fxContext->Prep();

                if (isReadyToRender && fxContext->Process(currBuffers, standardEffectsCollection))
                {
                    CHECK_GL_ERROR();

                    // TODO: get from effects chain
                    //const GLuint finalFBO = currBuffers->GetFinalFBO();

                    // special test for an android device, send preview image by udp
#if BROADCAST_PREVIEW == 1
                    if (true == mSendPreview)
                    {
                        SendPreview(currBuffers);
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
                        //glBindFramebuffer(GL_FRAMEBUFFER, finalFBO);

                        DrawHUD(0, 0, localViewport[2], localViewport[3], mMainFrameBuffer.GetWidth(), mMainFrameBuffer.GetHeight());

                        doubleFramebufferRequest->UnBind();
                        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
                    }

                    // 3. blit back a part of a screen

                    if (!mMainFrameBuffer.isFboAttached())
                    {
                        BlitFBOToFBOOffset(doubleFramebufferRequest->GetPtr()->GetFrameBuffer(), 0, 0, localViewport[2], localViewport[3], doubleFramebufferRequest->GetWriteAttachment(),
                            mMainFrameBuffer.GetFinalFBO(), localViewport[0], localViewport[1], localViewport[2], localViewport[3], 0,
                            false, false, false, false); // don't copy depth or any other color attachment
                    }
                    else
                    {
                        BlitFBOToFBOOffset(doubleFramebufferRequest->GetPtr()->GetFrameBuffer(), 0, 0, localViewport[2], localViewport[3], doubleFramebufferRequest->GetWriteAttachment(),
                            mMainFrameBuffer.GetAttachedFBO(), localViewport[0], localViewport[1], localViewport[2], localViewport[3], 0,
                            false, false, false, false); // don't copy depth or any other color attachment
                    }
                }
            }

            if (currBuffers)
            {
                currBuffers->OnFrameRendered();
            }
        }



        if (mAttachedFBO[mEnterId - 1] > 0)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, mAttachedFBO[mEnterId - 1]);
            glDrawBuffer(GL_COLOR_ATTACHMENT0);
        }
        else
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        //}

        // DONE: draw a resulted rect
        // render background
        if (!mMainFrameBuffer.isFboAttached() && mShaderSimple.get())
        {
            mShaderSimple->Bind();

            glBindTexture(GL_TEXTURE_2D, mMainFrameBuffer.GetFinalColorObject());
            drawOrthoQuad2d(mViewerViewport[2], mViewerViewport[3]);

            mShaderSimple->UnBind();

            lStatus = true;
        }


        CHECK_GL_ERROR();
    }
    
    mEnterId--;
    
    if (mEnterId < 0)
    {
        FBTrace("ERROR: wrong entering id!", "Ok");
        mEnterId = 0;
    }
    else
    {
        // offline render
        if (mAttachedFBO[mEnterId] > 0)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, mAttachedFBO[mEnterId]);
            glReadBuffer(GL_COLOR_ATTACHMENT0);
        }

    }

    return lStatus;
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

void PostProcessContextData::PreRenderFirstEntry()
{
    FBSystem& mSystem = FBSystem::TheOne();
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &mAttachedFBO[mEnterId]);

    //mPaneId = 0;
    mFrameId++;

    // grab the whole viewer

    mViewerViewport[0] = mViewerViewport[1] = 0;
    mViewerViewport[2] = mViewerViewport[3] = 0;

    mSchematicView[0] = mSchematicView[1] = mSchematicView[2] = mSchematicView[3] = false;

    FBRenderer *pRenderer = mSystem.Renderer;
    const int schematic = pRenderer->GetSchematicViewPaneIndex();

    if (schematic >= 0)
        mSchematicView[schematic] = true;

    mRenderPaneCount = pRenderer->GetPaneCount();

    // DONE: this is strict post effect pane index, should we choose another one ?!

    for (int i = 0; i < mRenderPaneCount; ++i)
    {
        FBCamera *pCamera = pRenderer->GetCameraInPane(i);
        mRenderPanes[i].camera = pCamera;
        if (!pCamera)
            continue;

        bool paneSharesCamera = false;
        for (int j = 0; j < mRenderPaneCount; ++j)
        {
            if (i != j)
            {
                FBCamera *pOtherCamera = pRenderer->GetCameraInPane(j);
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
        pCamera->FrameSizeMode.GetData(&cameraFrameSizeMode, sizeof(FBCameraFrameSizeMode));
        if (kFBFrameSizeWindow == cameraFrameSizeMode)
        {
            w += x;
            h += y;
        }
        else
        {
            w += 2 * x;
            h += 2 * y;
        }

        if (true == paneSharesCamera)
        {
            w *= 2;
            h *= 2;
        }

        if (w > mViewerViewport[2])
            mViewerViewport[2] = w;
        if (h > mViewerViewport[3])
            mViewerViewport[3] = h;
    }

    //
    // resize, alloc shaders, etc.
    LoadShaders();

    PrepPaneSettings();

    //
    for (int i = 0; i < mRenderPaneCount; ++i)
    {
        if (!mRenderPanes[i].data)
            continue;

        FBCamera *pCamera = mRenderPanes[i].camera;
        if (!pCamera)
            continue;

        bool paneSharesCamera = false;
        for (int j = 0; j < mRenderPaneCount; ++j)
        {
            if (i != j)
            {
                FBCamera *pOtherCamera = pRenderer->GetCameraInPane(j);
                if (pCamera == pOtherCamera)
                {
                    paneSharesCamera = true;
                    break;
                }
            }
        }

        int w = pCamera->CameraViewportWidth;
        int h = pCamera->CameraViewportHeight;

        if (w <= 0 || h <= 0)
            continue;

        // next line could change current fbo

        bool usePreview = mRenderPanes[i].data->OutputPreview;
        double scaleF = mRenderPanes[i].data->OutputScaleFactor;

        switch (i)
        {
        case 0:
            mEffectBuffers0->ReSize(w, h, usePreview, scaleF);
            break;
        case 1:
            mEffectBuffers1->ReSize(w, h, usePreview, scaleF);
            break;
        case 2:
            mEffectBuffers2->ReSize(w, h, usePreview, scaleF);
            break;
        case 3:
            mEffectBuffers3->ReSize(w, h, usePreview, scaleF);
            break;
        }    
    }

    //

    if (mAttachedFBO[mEnterId] > 0)
        mMainFrameBuffer.AttachFBO(mAttachedFBO[mEnterId]);
    else
        mMainFrameBuffer.DetachFBO();
    
    if (mAttachedFBO[mEnterId] == 0 && mViewerViewport[2] > 1 && mViewerViewport[3] > 1)
    {
        mMainFrameBuffer.ReSize(mViewerViewport[2], mViewerViewport[3], 1.0, 0, 0);

        mMainFrameBuffer.BeginRender();
        glViewport(0, 0, mViewerViewport[2], mViewerViewport[3]);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        mMainFrameBuffer.EndRender();
    }
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

bool PostProcessContextData::LoadShaders()
{
    if (nullptr != mShaderSimple.get())
    {
        return true;
    }
    FBSystem& mSystem = FBSystem::TheOne();
    FBString shaders_path(mSystem.ApplicationPath);
    shaders_path = shaders_path + "\\plugins";

    bool status = true;

    if (!CheckShadersPath(shaders_path))
    {
        status = false;

        const FBStringList& plugin_paths = mSystem.GetPluginPath();

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
        FBTrace("[PostProcessing] Failed to find simple shaders!\n");
        return false;
    }

    GLSLShaderProgram *pNewShader = nullptr;

    try
    {
        pNewShader = new GLSLShaderProgram();

        if (nullptr == pNewShader)
        {
            throw std::exception("failed to allocate memory for the simple shader");
        }

        FBString vertex_path(shaders_path, SHADER_SIMPLE_VERTEX);
        FBString fragment_path(shaders_path, SHADER_SIMPLE_FRAGMENT);


        if (false == pNewShader->LoadShaders(vertex_path, fragment_path))
        {
            throw std::exception("failed to load and prepare simple shader");
        }

        // samplers and locations
        pNewShader->Bind();

        GLint loc = pNewShader->findLocation("sampler0");
        if (loc >= 0)
            glUniform1i(loc, 0);

        pNewShader->UnBind();

    }
    catch (const std::exception &e)
    {
        FBTrace("Post Processing Simple Shader: %s\n", e.what());

        delete pNewShader;
        pNewShader = nullptr;

        status = false;
    }

    mShaderSimple.reset(pNewShader);

    return status;
}

void PostProcessContextData::FreeShaders()
{
    mShaderSimple.reset(nullptr);
}

void PostProcessContextData::FreeBuffers()
{
    mMainFrameBuffer.ChangeContext();

    mEffectBuffers0->ChangeContext();
    mEffectBuffers1->ChangeContext();
    mEffectBuffers2->ChangeContext();
    mEffectBuffers3->ChangeContext();
}

void PostProcessContextData::ResetPaneSettings()
{
    mRenderPaneCount = 0;
    isReadyToEvaluate = false;
    for (int i = 0; i < MAX_PANE_COUNT; ++i)
    {
        mRenderPanes[i].data = nullptr;
        mRenderPanes[i].camera = nullptr;
    }
}

bool PostProcessContextData::PrepPaneSettings()
{
    FBSystem& mSystem = FBSystem::TheOne();
    FBScene *pScene = mSystem.Scene;
    FBRenderer *pRenderer = mSystem.Renderer;

    for (int i = 0; i < MAX_PANE_COUNT; ++i)
    {
        mRenderPanes[i].data = nullptr;
        mRenderPanes[i].camera = nullptr;
    }
    
    // find a global settings (without camera attachments)

    PostPersistentData *pGlobalData = nullptr;

    for (int i = 0, count = pScene->UserObjects.GetCount(); i < count; ++i)
    {
        if (FBIS(pScene->UserObjects[i], PostPersistentData))
        {
            PostPersistentData *pData = (PostPersistentData*)pScene->UserObjects[i];

            if (true == pData->Active && (false == pData->UseCameraObject || 0 == pData->Camera.GetCount()))
            {
                pGlobalData = pData;
            }
        }
    }

    // looking for exclusing values

    for (int i = 0; i < MAX_PANE_COUNT; ++i)
    {
        FBCamera *pPaneCamera = pRenderer->GetCameraInPane(i);
        if (pPaneCamera)
        {
            int dstCount = pPaneCamera->GetDstCount();

            for (int j = 0; j < dstCount; ++j)
            {
                FBPlug *pdst = pPaneCamera->GetDst(j);

                if (FBIS(pdst, PostPersistentData))
                {
                    PostPersistentData *pData = (PostPersistentData*)pdst;

                    if (pData->Active && pData->UseCameraObject)
                    {
                        mRenderPanes[i].data = pData;
                        mRenderPanes[i].camera = pPaneCamera;
                        break;
                    }
                }
            }
        }

        // if exclusive pane settings is not assign, then try to assign global one
        if (!mRenderPanes[i].data)
        {
            mRenderPanes[i].data = pGlobalData;
            mRenderPanes[i].camera = pPaneCamera;
        }
    }

    return true;
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