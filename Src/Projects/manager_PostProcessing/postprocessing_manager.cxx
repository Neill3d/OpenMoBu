
/** \file   postprocessing_manager.cxx

	Sergei <Neill3d> Solokhin 2018

	GitHub page - https://github.com/Neill3d/OpenMoBu
	Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include <Windows.h>
#include "postprocessing_manager.h"

#include "postprocessing_helper.h"

//--- Registration defines
#define POSTPROCESSING_MANAGER__CLASS POSTPROCESSING_MANAGER__CLASSNAME
#define POSTPROCESSING_MANAGER__NAME  POSTPROCESSING_MANAGER__CLASSSTR

//--- FiLMBOX implementation and registration
FBCustomManagerImplementation(POSTPROCESSING_MANAGER__CLASS);  // Manager class name.
FBRegisterCustomManager(POSTPROCESSING_MANAGER__CLASS);         // Manager class name.


#define IS_INSIDE_MAIN_CYCLE			(mEnterId==1)
#define IS_RENDERING_OFFLINE			(mAttachedFBO[mEnterId-1] > 0)

#define SHADER_SIMPLE_VERTEX			"\\GLSL\\simple.vsh"
#define SHADER_SIMPLE_FRAGMENT			"\\GLSL\\simple.fsh"

#define RENDER_HUD_RECT_TOP				"RectangleTop"
#define RENDER_HUD_RECT_BOTTOM			"RectangleBottom"

// track the state of OpenGL viewport context
HGLRC	gCurrentContext = 0;

Manager_PostProcessing *gManager = nullptr;

bool GRenderAfterRender()
{
	if (nullptr != gManager)
	{
		return gManager->RenderAfterRender(gManager->mLastProcessCompositions, false);
	}

	return false;
}

/*
source: DEBUG_SOURCE_X where X may be API,
SHADER_COMPILER, WINDOW_SYSTEM,
THIRD_PARTY, APPLICATION, OTHER
type: DEBUG_TYPE_X where X may be ERROR,
MARKER, OTHER, DEPRECATED_BEHAVIOR,
UNDEFINED_BEHAVIOR, PERFORMANCE,
PORTABILITY, {PUSH, POP}_GROUP
severity: DEBUG_SEVERITY_{HIGH, MEDIUM},
DEBUG_SEVERITY_{LOW, NOTIFICATION}
*/
void DebugOGL_Callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, const void*userParam)
{
	if (type == GL_DEBUG_TYPE_ERROR)
	{
		FBTrace(">> ERROR!\n");
		FBTrace("debug message - %s\n", message);
	}
}

/************************************************
 *  FiLMBOX Creation
 ************************************************/
bool Manager_PostProcessing::FBCreate()
{
	//mPaneId = 0;
	mEnterId = 0;
	mFrameId = 0;
	mLastPaneCount = 0;

	//mPropPaneCount = nullptr;

	mVideoRendering = false;

	for (int i = 0; i < 4; ++i)
	{
		mViewerViewport[i] = 0;
		//mLocalViewport[i] = 0;
		mSchematicView[i] = false;
	}
	
	gManager = this;

	mLastSendTimeSecs = 0.0;

	mFirstRun = true;
	
	mIsSynced = false;

	mLastSyncTime = FBTime::Zero;
	mSyncDuration = FBTime(0, 0, 2);

#if BROADCAST_PREVIEW == 1
	mSendPreview = false;
	mSocketPort = 8885;
	mSendAddress.Set(192, 168, 0, 133, 8887);
#endif
	//mSettingsMerge = false;

	mSocketSender = nullptr;
	mSocketRecv = nullptr;

	mDoVideoClipTimewrap = false;

    return true;
}


/************************************************
 *  FiLMBOX Destruction.
 ************************************************/
void Manager_PostProcessing::FBDestroy()
{
    // Free any user memory here.
}


/************************************************
 *  Execution callback.
 ************************************************/
bool Manager_PostProcessing::Init()
{
	glewInit();

	
	//
#ifdef _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(DebugOGL_Callback, nullptr);
#endif

	//
	mMainFrameBuffer.InitTextureInternalFormat();

    return true;
}

bool Manager_PostProcessing::Open()
{
	mApplication.OnFileNewCompleted.Add(this, (FBCallback)&Manager_PostProcessing::EventFileNew);
	mApplication.OnFileOpen.Add(this, (FBCallback)&Manager_PostProcessing::EventFileOpen);
	mApplication.OnFileOpenCompleted.Add(this, (FBCallback)&Manager_PostProcessing::EventFileOpenComplete);

	mSystem.Scene->OnChange.Add(this, (FBCallback)&Manager_PostProcessing::EventSceneChange);

	mSystem.OnUIIdle.Add(this, (FBCallback)&Manager_PostProcessing::OnUIIdle);

	mSystem.OnVideoFrameRendering.Add(this, (FBCallback)&Manager_PostProcessing::OnVideoFrameRendering);

	//FBEvaluateManager::TheOne().OnEvaluationPipelineEvent.Add(this, (FBCallback)&Manager_PostProcessing::OnPerFrameEvaluationPipelineCallback);
	//FBEvaluateManager::TheOne().OnSynchronizationEvent.Add(this, (FBCallback)&Manager_PostProcessing::OnPerFrameSynchronizationCallback);
	FBEvaluateManager::TheOne().OnRenderingPipelineEvent.Add(this, (FBCallback)&Manager_PostProcessing::OnPerFrameRenderingPipelineCallback);

    return true;
}

void Manager_PostProcessing::LoadConfig()
{
	// 
	{
		FBConfigFile	lConfig("@PATCHES.txt");

		char buffer[64] = { 0 };
		sprintf_s(buffer, sizeof(char)* 64, "True");
		const char *cbuffer = buffer;
		bool lStatusAuto = lConfig.GetOrSet("Video Clip Timewrap", "Enable", cbuffer, "Put True/False to enable a video clip timewrap feature");

		if (true == lStatusAuto)
		{
			mDoVideoClipTimewrap = (0 == strcmp(cbuffer, "True"));
		}
	}
#if BROADCAST_PREVIEW == 1
	// read a preview udp config
	{
	FBConfigFile	lConfig("@PostProcessing.txt");
	FBString groupName("Send Preview");

	char buffer[64] = { 0 };
	sprintf_s(buffer, sizeof(char)* 64, "False");
	const char *cbuffer = buffer;
	bool lStatusAuto = lConfig.GetOrSet(groupName, "Enable", cbuffer, "Put True/False to send a preview by UDP");

	if (true == lStatusAuto)
	{
		mSendPreview = (0 == strcmp(cbuffer, "True"));
	}

	//
	sprintf_s(buffer, sizeof(char)* 64, "8885");
	cbuffer = buffer;
	lStatusAuto = lConfig.GetOrSet(groupName, "Port", cbuffer, "Defines a UDP socket port");

	if (true == lStatusAuto)
	{
		mSocketPort = atoi(cbuffer);
	}

	//
	sprintf_s(buffer, sizeof(char)* 64, "192.168.0.133");
	cbuffer = buffer;
	lStatusAuto = lConfig.GetOrSet(groupName, "Destination", cbuffer, "Defines an address where to send");

	if (true == lStatusAuto)
	{
		struct sockaddr_in sock;
		int addrSize = sizeof(struct sockaddr_in);
		memset(&sock, 0, addrSize);
		sock.sin_family = AF_INET;

		int rc = WSAStringToAddress((LPSTR)cbuffer, AF_INET, nullptr, (LPSOCKADDR)&sock, &addrSize);
		if (0 == rc)
		{
			mSendAddress.SetAddress(sock.sin_addr.S_un.S_un_b.s_b1, sock.sin_addr.S_un.S_un_b.s_b2, sock.sin_addr.S_un.S_un_b.s_b3,
				sock.sin_addr.S_un.S_un_b.s_b4);
		}
	}

	//
	sprintf_s(buffer, sizeof(char)* 64, "8886");
	cbuffer = buffer;
	lStatusAuto = lConfig.GetOrSet(groupName, "DestPort", cbuffer, "Defines a destination port");

	if (true == lStatusAuto)
	{
		int destport = atoi(cbuffer);
		mSendAddress.SetPortOnly(destport);
	}

}
#endif
}

void Manager_PostProcessing::OnUIIdle(HISender pSender, HKEvent pEvent)
{

	if (true == mFirstRun)
	{
		mFirstRun = false;

		//
		mSystem.OnConnectionNotify.Add(this, (FBCallback)&Manager_PostProcessing::EventConnNotify);
		mSystem.OnConnectionDataNotify.Add(this, (FBCallback)&Manager_PostProcessing::EventConnDataNotify);

		//
		LoadConfig();

#if BROADCAST_PREVIEW == 1
		// open a socket
		if (nullptr == mSocketSender)
		{
			bool ret = OpenSocket(mSocketPort, mSocketPort, false);
			if (false == ret)
			{
				mSendPreview = false;
				mIsSynced = false;
				return;
			}
		}
#endif
	}

	//
#if BROADCAST_PREVIEW == 1
	if (mSendPreview && nullptr != mSocketRecv)
	{
		FBTime sysTime(mSystem.SystemTime);

		if (mIsSynced)
		{
			if (sysTime - mLastSyncTime > mSyncDuration)
			{
				mIsSynced = false;
			}
		}
		
		// look for a registration packet to start sending images
		Network::Address address;
		int bytes_read = ((Network::Socket*)mSocketRecv)->Receive(address, mSendBuffer, MAX_UDP_BUFFER_SIZE);
		if (bytes_read > 3)
		{
			if (Network::CheckMagicNumber(mSendBuffer))
			{
				Network::CPacketHeader *pHeader = (Network::CPacketHeader*) mSendBuffer;

				if (PACKET_REASON_REGISTER == pHeader->reason
					|| PACKET_REASON_FEEDBACK == pHeader->reason)
				{
					mIsSynced = true;
					mSendAddress = address;
					mSendAddress.SetPortOnly(8886);
					mLastSyncTime = sysTime;
				}
			}
		}
	}
#endif
}

bool Manager_PostProcessing::Clear()
{
    return true;
}

bool Manager_PostProcessing::Close()
{
	mSystem.OnUIIdle.Remove(this, (FBCallback)&Manager_PostProcessing::OnUIIdle);

	//FBEvaluateManager::TheOne().OnEvaluationPipelineEvent.Remove(this, (FBCallback)&Manager_PostProcessing::OnPerFrameEvaluationPipelineCallback);
	//FBEvaluateManager::TheOne().OnSynchronizationEvent.Remove(this, (FBCallback)&Manager_PostProcessing::OnPerFrameSynchronizationCallback);
	FBEvaluateManager::TheOne().OnRenderingPipelineEvent.Remove(this, (FBCallback)&Manager_PostProcessing::OnPerFrameRenderingPipelineCallback);

	mApplication.OnFileNewCompleted.Remove(this, (FBCallback)&Manager_PostProcessing::EventFileNew);
	mApplication.OnFileOpen.Remove(this, (FBCallback)&Manager_PostProcessing::EventFileOpen);
	mApplication.OnFileMerge.Remove(this, (FBCallback)&Manager_PostProcessing::EventFileMerge);
	mApplication.OnFileOpenCompleted.Remove(this, (FBCallback)&Manager_PostProcessing::EventFileOpenComplete);

	mSystem.Scene->OnChange.Remove(this, (FBCallback)&Manager_PostProcessing::EventSceneChange);
	mSystem.OnConnectionNotify.Remove(this, (FBCallback)&Manager_PostProcessing::EventConnNotify);
	mSystem.OnConnectionDataNotify.Remove(this, (FBCallback)&Manager_PostProcessing::EventConnDataNotify);
	mSystem.OnVideoFrameRendering.Remove(this, (FBCallback)&Manager_PostProcessing::OnVideoFrameRendering);

	//CloseSocket();

    return true;
}


void Manager_PostProcessing::EventFileNew(HISender pSender, HKEvent pEvent)
{
	// clear all pointers (start point)

	//ClearOutputCompositePtr();
	//mSettings = nullptr;
}

void Manager_PostProcessing::EventFileOpen(HISender pSender, HKEvent pEvent)
{
	//mSettings = nullptr;
}

void Manager_PostProcessing::EventFileMerge(HISender pSender, HKEvent pEvent)
{
	//mSettings = nullptr;
	//mSettingsMerge = true;
}

void Manager_PostProcessing::EventFileOpenComplete(HISender pSender, HKEvent pEvent)
{
	//mSettings = nullptr;
}


void Manager_PostProcessing::EventSceneChange(HISender pSender, HKEvent pEvent)
{
}



void Manager_PostProcessing::EventConnDataNotify(HISender pSender, HKEvent pEvent)
{
	
}

void Manager_PostProcessing::EventConnNotify(HISender pSender, HKEvent pEvent)
{
	
}


void Manager_PostProcessing::OnPerFrameSynchronizationCallback(HISender pSender, HKEvent pEvent)
{
	FBEventEvalGlobalCallback lFBEvent(pEvent);
	if (lFBEvent.GetTiming() == kFBGlobalEvalCallbackSyn)
	{
		///
		// This callback occurs when both rendering and evaluation pipeline are stopped,
		// plugin developer could add some lightweight scene modification tasks here
		// and no need to worry complicated thread issues. 
		//

	}
}

void Manager_PostProcessing::OnPerFrameEvaluationPipelineCallback(HISender pSender, HKEvent pEvent)
{
	
	//
	// Add custom evaluation tasks here will improve the performance if with 
	// parallel pipeline (default) because they occur in the background thread.
	//

	FBEventEvalGlobalCallback lFBEvent(pEvent);
	switch (lFBEvent.GetTiming())
	{
	case kFBGlobalEvalCallbackBeforeDAG:
		//
		// We could add custom tasks here before MoBu perform standard evaluation tasks. 
		//

		break;
	case kFBGlobalEvalCallbackAfterDAG:
	{
										  //
										  // We could add custom tasks here after MoBu finish standard animation & deformation evaluation tasks. 
										  //


	} break;
	case kFBGlobalEvalCallbackAfterDeform:
	{
											 //
											 // We could add custom tasks here after MoBu finish standard deformation evaluation tasks 
											 // (if not use GPU deformation). E.g, update the accelerated spatial scene structure for rendering. 
											 //

											 //mGPUScene->UpdateAfterDeform();

	} break;
	default:
		break;
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnPerRenderingPipelineCallback


////////////////////////////////////////////////////////////////////////////////////
// RenderBeforeRender
void Manager_PostProcessing::RenderBeforeRender(const bool processCompositions, const bool renderToBuffer)
{
	//FBScene *pScene = mSystem.Scene;

	//glGetIntegerv(GL_FRAMEBUFFER_BINDING, &mAttachedFBO[mEnterId]);
	/*
	if (mEnterId < 1)
	{
		if (mAttachedFBO[mEnterId] > 0)
			mMainFrameBuffer.AttachFBO(mAttachedFBO[mEnterId]);
		else
			mMainFrameBuffer.DetachFBO();
	}
	*/
	//if (0 == mPaneId)
	//{
		mEnterId++;
	//}
	
	// attachment point
	if (processCompositions)
	{
		
		// let's run compression threads

		for (int nPane = 0; nPane < mLastPaneCount; ++nPane)
		{
			FBCamera *pCamera = mSystem.Renderer->GetCameraInPane(nPane);
			if (nullptr == pCamera || true == pCamera->SystemCamera
				|| true == mVideoRendering || true == mSchematicView[nPane])
			{
				continue;
			}
			

			PostEffectBuffers *currBuffers = nullptr;
			switch (nPane)
			{
			case 0:
				currBuffers = &mEffectBuffers0;
				break;
			case 1:
				currBuffers = &mEffectBuffers1;
				break;
			case 2:
				currBuffers = &mEffectBuffers2;
				break;
			case 3:
				currBuffers = &mEffectBuffers3;
				break;
			}

			mEffectChain.BeginFrame(currBuffers);
		}

		// it will use attached dimentions, if any external buffer is exist
		//mMainFrameBuffer.ReSize(mViewerViewport[2], mViewerViewport[3], 1.0, 0, 0);

		mViewerViewport[2] = mMainFrameBuffer.GetBufferWidth();
		mViewerViewport[3] = mMainFrameBuffer.GetBufferHeight();

		mMainFrameBuffer.BeginRender();

		glViewport(0, 0, mViewerViewport[2], mViewerViewport[3]);

		glEnable(GL_DEPTH_TEST);
		//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	}
	/*
	else if (mMainFrameBuffer.GetAttachedFBO() > 0)
	{
		mMainFrameBuffer.BeginRender();

		glViewport(0, 0, mViewerViewport[2], mViewerViewport[3]);
		glEnable(GL_DEPTH_TEST);
	}*/
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/////////////////////////////////////////////////////////////////////////////////////
// RenderAfterRender - post processing work after main scene rendering is finished


bool Manager_PostProcessing::RenderAfterRender(const bool processCompositions, const bool renderToBuffer)
{
	bool lStatus = false;

	if (mEnterId <= 0)
		return lStatus;

	//	FBScene *pScene = mSystem.Scene;
	/*
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	int lviewport[4];
	glGetIntegerv( GL_VIEWPORT, lviewport );
	*/

	//mMainFrameBuffer.EndRender();
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);


	/////////////
	// !!!
	if (processCompositions && 1 == mEnterId)
	{
		/*
		if ()
		{
			FBTrace("entering wrong pane index\n");
		}
		*/

		glDisable(GL_MULTISAMPLE);
		glDisable(GL_DEPTH_TEST);

		mMainFrameBuffer.EndRender();
		mMainFrameBuffer.PrepForPostProcessing(false);	// ?!

		CHECK_GL_ERROR();

		// this is a hack for Reflection shader (to avoid error overhead on glReadBuffers(GL_BACK) )
#ifndef _DEBUG
		EmptyGLErrorStack();
#endif
		//if (mLastPostPane == mPaneId)
		//{
		
		FBTime sysTime = mSystem.SystemTime;
		const double sysTimeSecs = sysTime.GetSecondDouble();

			for (int nPane = 0; nPane < mLastPaneCount; ++nPane)
			{
				int localViewport[4];
				FBCamera *pCamera = mSystem.Renderer->GetCameraInPane(nPane);

				localViewport[0] = pCamera->CameraViewportX;
				localViewport[1] = pCamera->CameraViewportY;
				localViewport[2] = pCamera->CameraViewportWidth;
				localViewport[3] = pCamera->CameraViewportHeight;

				if (true == pCamera->SystemCamera)
				{
					localViewport[2] = 0;
				}
				else
				if (false == mVideoRendering || nPane > 0)
				{
					if (true == mSchematicView[nPane])
						localViewport[2] = 0;
				}
				//glBindFramebuffer(GL_FRAMEBUFFER, 0);

				PostEffectBuffers *currBuffers = nullptr;
				switch (nPane)
				{
				case 0:
					currBuffers = &mEffectBuffers0;
					break;
				case 1:
					currBuffers = &mEffectBuffers1;
					break;
				case 2:
					currBuffers = &mEffectBuffers2;
					break;
				case 3:
					currBuffers = &mEffectBuffers3;
					break;
				}

				// not in schematic view
				if (localViewport[2] > 0 && nullptr != currBuffers
					&& localViewport[2] == currBuffers->GetWidth()
					&& nullptr != mPaneSettings[nPane])
				{
					// 1. blit part of a main screen
					const GLuint postBufferObj = currBuffers->PrepAndGetBufferObject();

					if (false == mMainFrameBuffer.isFboAttached())
					{
						BlitFBOToFBOOffset(mMainFrameBuffer.GetFinalFBO(), localViewport[0], localViewport[1], localViewport[2], localViewport[3],
							postBufferObj, 0, 0, localViewport[2], localViewport[3], true, false, false, false);
					}
					else
					{
						BlitFBOToFBOOffset(mMainFrameBuffer.GetAttachedFBO(), localViewport[0], localViewport[1], localViewport[2], localViewport[3],
							postBufferObj, 0, 0, localViewport[2], localViewport[3], true, false, false, false);
					}


					// 2. process it

					mEffectChain.Prep(mPaneSettings[nPane], localViewport[2], localViewport[3], pCamera);

					if (true == mEffectChain.Process(currBuffers, sysTimeSecs)
						&& nullptr != mShaderSimple.get())
					{
						CHECK_GL_ERROR();

						const GLuint finalFBO = currBuffers->GetFinalFBO();

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

						if (true == mPaneSettings[nPane]->DrawHUDLayer)
						{
							glBindFramebuffer(GL_FRAMEBUFFER, finalFBO);

							DrawHUD(0, 0, localViewport[2], localViewport[3], mMainFrameBuffer.GetWidth(), mMainFrameBuffer.GetHeight());

							glBindFramebuffer(GL_FRAMEBUFFER, 0);
						}

						// 3. blit back a part of a screen

						if (false == mMainFrameBuffer.isFboAttached())
						{
							BlitFBOToFBOOffset(finalFBO, 0, 0, localViewport[2], localViewport[3],
								mMainFrameBuffer.GetFinalFBO(), localViewport[0], localViewport[1], localViewport[2], localViewport[3],
								false, false, false, false);
						}
						else
						{
							BlitFBOToFBOOffset(finalFBO, 0, 0, localViewport[2], localViewport[3],
								mMainFrameBuffer.GetAttachedFBO(), localViewport[0], localViewport[1], localViewport[2], localViewport[3],
								false, false, false, false);
						}

					}
		
				}
			}



			if (mAttachedFBO[mEnterId - 1] > 0)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, mAttachedFBO[mEnterId - 1]);
				//glReadBuffer(GL_COLOR_ATTACHMENT0);
				glDrawBuffer(GL_COLOR_ATTACHMENT0);
			}
			else
			{
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}

			//}

			// DONE: draw a resulted rect
			// render background
			if (false == mMainFrameBuffer.isFboAttached()
				&& nullptr != mShaderSimple.get())
			{
				mShaderSimple->Bind();

				glBindTexture(GL_TEXTURE_2D, mMainFrameBuffer.GetFinalColorObject());
				drawOrthoQuad2d(mViewerViewport[2], mViewerViewport[3]);

				mShaderSimple->UnBind();

				lStatus = true;
			}
		

		CHECK_GL_ERROR();
	}
	/*
	mPaneId = mPaneId + 1;
	if (mPaneId >= mLastPaneCount)
		mPaneId = 0;
*/
	//if (0 == mPaneId)
	//{
		mEnterId--;
	//}

	if (mEnterId < 0)
	{
		FBTrace("ERROR: wrong entering id!", "Ok");
		mEnterId = 0;
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	else
	{
		/*
		// offline render
		if (mAttachedFBO[mEnterId] > 0)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, mAttachedFBO[mEnterId]);
			glReadBuffer(GL_COLOR_ATTACHMENT0);
		}
		*/
	}

	return lStatus;
}

void Manager_PostProcessing::CheckForAContextChange()
{
	HGLRC hContext = wglGetCurrentContext();

	if (0 == gCurrentContext)
	{
		// initialize for the first time
		gCurrentContext = hContext;
	}

	if (hContext != gCurrentContext)
	{
		gCurrentContext = hContext;

		mEffectChain.ChangeContext();
		FreeShaders();
		FreeBuffers();
		FreeFonts();

		FBTrace("> !! CHANGE CONTEXT !!\n");
	}
}

void Manager_PostProcessing::OnPerFrameRenderingPipelineCallback(HISender pSender, HKEvent pEvent)
{

	FBEventEvalGlobalCallback lFBEvent(pEvent);

	// check for a context change here
	if (mEnterId < 1 && lFBEvent.GetTiming() == kFBGlobalEvalCallbackBeforeRender)
	{
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &mAttachedFBO[mEnterId]);

		//mPaneId = 0;
		mFrameId++;

		CheckForAContextChange();

		// grab the whole viewer

		mViewerViewport[0] = mViewerViewport[1] = 0;
		mViewerViewport[2] = mViewerViewport[3] = 0;

		mSchematicView[0] = mSchematicView[1] = mSchematicView[2] = mSchematicView[3] = false;

		FBRenderer *pRenderer = mSystem.Renderer;
		const int schematic = pRenderer->GetSchematicViewPaneIndex();

		if (schematic >= 0)
			mSchematicView[schematic] = true;

		mLastPaneCount = pRenderer->GetPaneCount();

		// DONE: this is strict post effect pane index, should we choose another one ?!

		for (int i = 0; i < mLastPaneCount; ++i)
		{
			FBCamera *pCamera = pRenderer->GetCameraInPane(i);
			if (nullptr == pCamera)
				continue;
			
			bool paneSharesCamera = false;
			for (int j = 0; j < mLastPaneCount; ++j)
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
			if (kFBFrameSizeWindow == pCamera->FrameSizeMode)
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
		for (int i = 0; i < mLastPaneCount; ++i)
		{
			if (nullptr == mPaneSettings[i])
				continue;

			FBCamera *pCamera = pRenderer->GetCameraInPane(i);
			if (nullptr == pCamera)
				continue;

			bool paneSharesCamera = false;
			for (int j = 0; j < mLastPaneCount; ++j)
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

			bool usePreview = mPaneSettings[i]->OutputPreview;
			double scaleF = mPaneSettings[i]->OutputScaleFactor;

			switch (i)
			{
			case 0:
				mEffectBuffers0.ReSize(w, h, usePreview, scaleF);
				break;
			case 1:
				mEffectBuffers1.ReSize(w, h, usePreview, scaleF);
				break;
			case 2:
				mEffectBuffers2.ReSize(w, h, usePreview, scaleF);
				break;
			case 3:
				mEffectBuffers3.ReSize(w, h, usePreview, scaleF);
				break;
			}
		}

		//

		if (mAttachedFBO[mEnterId] > 0)
			mMainFrameBuffer.AttachFBO(mAttachedFBO[mEnterId]);
		else
			mMainFrameBuffer.DetachFBO();

		CHECK_GL_ERROR();

		if (mViewerViewport[2] > 1 && mViewerViewport[3] > 1)
		{
			mMainFrameBuffer.ReSize(mViewerViewport[2], mViewerViewport[3], 1.0, 0, 0);

			mMainFrameBuffer.BeginRender();
			glViewport(0, 0, mViewerViewport[2], mViewerViewport[3]);
			glEnable(GL_DEPTH_TEST);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			mMainFrameBuffer.EndRender();

			CHECK_GL_ERROR();
		}
	}

	// nullptr != mPaneSettings[mPaneId] && false == mSchematicView[mPaneId]
	bool usePostProcessing = false;

	for (int i = 0; i<mLastPaneCount; ++i)
	{
		if (nullptr != mPaneSettings[i])
		{
			usePostProcessing = true;
			break;
		}
	}

	if (mViewerViewport[2] <= 1 || mViewerViewport[3] <= 1)
	{
		usePostProcessing = false;
	}

	
	switch (lFBEvent.GetTiming())
	{
	case kFBGlobalEvalCallbackBeforeRender:
		{
			mLastProcessCompositions = usePostProcessing;
			RenderBeforeRender(usePostProcessing, false);
			
			if (true == mDoVideoClipTimewrap)
			{
				PrepVideoClipsTimeWrap();
			}
			

		} break;
	case kFBGlobalEvalCallbackAfterRender:
		{
			//
			// This callback occurs just before swapping GL back/front buffers. 
			// User could do some special effect, HUD or buffer download (via PBO) here. 
			//
			//if (mVideoRendering)
				RenderAfterRender(usePostProcessing, false);

		} break;

	default:
		break;
	}

	//FBTrace("enter id - %d, pane id - %d, frame id - %d\n", mEnterId, mPaneId, mFrameId);

	CHECK_GL_ERROR();
}

void Manager_PostProcessing::OnVideoFrameRendering(HISender pSender, HKEvent pEvent)
{
	FBEventVideoFrameRendering levent(pEvent);

	switch (levent.GetState())
	{
	case FBEventVideoFrameRendering::eBeginRendering:
		// turn off preview mode and switch quality settings if needed
		mVideoRendering = true;
		FreeBuffers();

		// TODO: tweak post processing upper / lower clip
		PushUpperLowerClipForEffects();
		break;
	case FBEventVideoFrameRendering::eEndRendering:
		// turn on back preview mode and display quality settings
		mVideoRendering = false;
		PopUpperLowerClipForEffects();
		break;
	}
}

const bool Manager_PostProcessing::CheckShadersPath(const char* path) const
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

bool Manager_PostProcessing::LoadShaders()
{
	if (nullptr != mShaderSimple.get())
	{
		return true;
	}
	
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

	GLSLShader *pNewShader = nullptr;

	try
	{
		pNewShader = new GLSLShader();

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

void Manager_PostProcessing::FreeShaders()
{
	mShaderSimple.reset(nullptr);
}

void Manager_PostProcessing::FreeBuffers()
{
	mMainFrameBuffer.ChangeContext();

	mEffectBuffers0.ChangeContext();
	mEffectBuffers1.ChangeContext();
	mEffectBuffers2.ChangeContext();
	mEffectBuffers3.ChangeContext();
}

bool Manager_PostProcessing::PrepPaneSettings()
{
	mPaneSettings.resize(4);

	FBScene *pScene = mSystem.Scene;
	FBRenderer *pRenderer = mSystem.Renderer;

	for (int i = 0; i < 4; ++i)
		mPaneSettings[i] = nullptr;

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

	for (int i = 0; i < 4; ++i)
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
						mPaneSettings[i] = pData;
						break;
					}
				}
			}
		}

		// if exclusive pane settings is not assign, then try to assign global one
		if (nullptr == mPaneSettings[i])
		{
			mPaneSettings[i] = pGlobalData;
		}
	}

	return true;
}

bool Manager_PostProcessing::EmptyGLErrorStack()
{
	bool wasError = false;

	for (GLenum glErr = glGetError(); glErr != GL_NO_ERROR; glErr = glGetError())
	{
		wasError = true;
	}

	return wasError;
}


void Manager_PostProcessing::DrawHUD(int panex, int paney, int panew, int paneh, int vieww, int viewh)
{
	/*
	const int panex = GetPanePosX();
	const int paney = GetPanePosY();
	const int panew = GetPaneWidth();
	const int paneh = GetPaneHeight();
	*/
	

	FBScene *pScene = mSystem.Scene;
	//FBRenderer *pRenderer = mSystem.Renderer;

	//glPushAttrib(GL_COLOR_BUFFER_BIT | GL_POLYGON_BIT | GL_TRANSFORM_BIT | GL_DEPTH_BUFFER_BIT);
	{

		glViewport(panex, paney, panew, paneh);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		//const double lViewPortDim = 1.0;
		//gluOrtho2D(-lViewPortDim, lViewPortDim, -lViewPortDim, lViewPortDim);

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
					mRectElements[numberOfRectElems] = (FBHUDRectElement*) pElem;
					numberOfRectElems += 1;
				}
				else if (FBIS(pElem, FBHUDTextElement))
				{
					mTextElements[numberOfTextElems] = (FBHUDTextElement*) pElem;
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

		//static bool firstFontUse = true;

		// prep text fonts
		if (mTextElements.size() > 0)
		{
			/*
			if (firstFontUse)
			{
				mFontManager.Init();
				firstFontUse = false;
			}
			*/
			//mFontManager.Resize(panew, paneh);

			
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
			
			
			//mFontManager.TextClear();

			auto textiter = begin(mTextElements);
			auto fontiter = begin(mElemFonts);

			for (; textiter != end(mTextElements); ++textiter, ++fontiter)
			{
				if ((*textiter)->Show)
				{
					DrawHUDText(*textiter,  *fontiter, panex, paney, panew, paneh, vieww, viewh);
				}
			}

			//mFontManager.Display();
		}
	}
	//glPopAttrib();
}

void Manager_PostProcessing::DrawHUDRect(FBHUDRectElement *pRect, int panex, int paney, int panew, int paneh, int vieww, int viewh)
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

	FBHUDElementHAlignment hAlign = pRect->Justification;
	FBHUDElementHAlignment hDock = pRect->HorizontalDock;
	FBHUDElementVAlignment vDock = pRect->VerticalDock;

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

void Manager_PostProcessing::DrawHUDText(FBHUDTextElement *pRect, CFont *pFont, int panex, int paney, int panew, int paneh, int vieww, int viewh)
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

	FBHUDElementHAlignment hAlign = pRect->Justification;
	FBHUDElementHAlignment hDock = pRect->HorizontalDock;
	FBHUDElementVAlignment vDock = pRect->VerticalDock;

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
		sprintf_s(buffer, sizeof(char)* 64, content);
	}
	else
	{
		
		if (kFBPT_Time == pRefProperty->GetPropertyType() && true == pRect->ForceTimeCodeDisplay)
		{
			FBTime time;
			pRefProperty->GetData(&time, sizeof(FBTime));
			refString = time.GetTimeString(kFBTimeModeDefault, FBTime::eSMPTE);

			sprintf_s(buffer, sizeof(char)* 64, content, refString);
		}
		else if (kFBPT_double == pRefProperty->GetPropertyType())
		{
			double value = 0.0;
			pRefProperty->GetData(&value, sizeof(double));

			sprintf_s(buffer, sizeof(char)* 64, content, value);
		}
		else if (kFBPT_int == pRefProperty->GetPropertyType())
		{
			int value = 0.0;
			pRefProperty->GetData(&value, sizeof(int));

			sprintf_s(buffer, sizeof(char)* 64, content, value);
		}
		else
		{
			refString = pRefProperty->AsString();
			sprintf_s(buffer, sizeof(char)* 64, content, refString);
		}
	
	}

	const int numchars = strlen(buffer);

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

	//int lfHeight = -MulDiv((int)fontHei, GetDeviceCaps(wglGetCurrentDC(), LOGPIXELSY), 72);
	//int lfWidth = -MulDiv((int)fontWid, GetDeviceCaps(wglGetCurrentDC(), LOGPIXELSX), 72);

	// TODO: i'm not tracking font name changes !!
	/*
	if ( (abs((float)lfHeight - pFont->GetHeight()) > 0.1f) 
		|| (abs((float)lfWidth - pFont->GetWidth()) > 0.1f))
	{
		FBString font(pRect->Font);

		pFont->FreeFont();
		pFont->InitFont(wglGetCurrentDC(), font, lfHeight, lfWidth);
	}
	*/
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
	
	//float rasterWidth = 0.0f;
	//float rasterHeight = 0.0f;

	//mFont->ComputeDimensions(buffer, strlen(buffer), rasterWidth, rasterHeight);

	//float scalef = 1.0f * (float)wid / rasterWidth;

	//pFont->PositionText(posx + 0.5 * fontWid, posy + 0.25 * hei);
	//pFont->glDrawText(buffer, strlen(buffer));

	pFont->Resize(panew, paneh);

	pFont->TextClear();
	pFont->TextAdd(posx, posy, 0.75f * (float)hei, wid, hei, buffer, static_cast<uint32_t>(strlen(buffer)));

	pFont->Display();

	//mFontManager.TextAdd(posx, posy + 0.95f * hei, 0.75f * (float)hei, buffer, strlen(buffer));

}

void Manager_PostProcessing::FreeFonts()
{

	//mFontManager.Free();

	
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
	
}

void Manager_PostProcessing::PrepVideoClipsTimeWrap()
{

	FBScene *pScene = mSystem.Scene;

	for (int i = 0, count = pScene->VideoClips.GetCount(); i < count; ++i)
	{

		FBVideoClip *pVideoClip = pScene->VideoClips[i];

		if (FBIS(pVideoClip, FBVideoMemory))
			continue;

		int firstFrame = pVideoClip->StartFrame;
		int stopFrame = pVideoClip->StopFrame;

		if (firstFrame != stopFrame && stopFrame > 0)
		{
			FBPropertyDouble *prop = (FBPropertyDouble*)pVideoClip->PropertyList.Find("VideoTimeWrap");
			FBPropertyInt	*lastUpdateProp = (FBPropertyInt*)pVideoClip->PropertyList.Find("vtwLast");
			FBPropertyInt	*updateStepProp = (FBPropertyInt*)pVideoClip->PropertyList.Find("vtwStep");
			FBPropertyBool	*enableProp = (FBPropertyBool*)pVideoClip->PropertyList.Find("vtwEnable");

			if (nullptr != enableProp && 0 == enableProp->AsInt())
			{
				continue;
			}

			int lastUpdate = -1;
			int step = 1;

			if (nullptr != lastUpdateProp)
				lastUpdate = lastUpdateProp->AsInt();
			if (nullptr != updateStepProp)
				step = updateStepProp->AsInt();

			if (nullptr != prop && true == prop->IsAnimated())
			{
				FBPropertyAnimatableDouble *animprop = (FBPropertyAnimatableDouble*)prop;

				FBTime currTime = mSystem.LocalTime;
				double dvalue;

				animprop->GetAnimationNode()->Evaluate(&dvalue, currTime);

				FBTime offsetTime(0, 0, 0, (int)dvalue);
				//pVideoClip->TimeOffset = offsetTime;
				//pVideoClip->CurrentFrame = (int)dvalue;
				//pVideoClip->ImageIncUpdateID();

				if (lastUpdate == -1 || abs((int)dvalue - lastUpdate) >= step)
				{
					int width = pVideoClip->Width;
					int height = pVideoClip->Height;
					FBVideoFormat format = pVideoClip->Format;

					GLenum glFormat = GL_RGB;

					switch (format)
					{
					case kFBVideoFormat_ARGB_32:
						glFormat = GL_RGBA;
						break;
					case kFBVideoFormat_BGRA_32:
						glFormat = GL_BGRA;
						break;
					case kFBVideoFormat_ABGR_32:
						glFormat = GL_ABGR_EXT;
						break;
					case kFBVideoFormat_RGBA_32:
						glFormat = GL_RGBA;
						break;
					}

					unsigned char *buffer = pVideoClip->GetImage((int)dvalue);

					GLuint texId = pVideoClip->GetTextureID();

					if (texId > 0)
					{
						glBindTexture(GL_TEXTURE_2D, texId);

						int red, green, blue, alpha;
						int internalFormat;

						glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_RED_SIZE, &red);
						glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_GREEN_SIZE, &green);
						glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_BLUE_SIZE, &blue);
						glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_ALPHA_SIZE, &alpha);
						glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);

						glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, glFormat, GL_UNSIGNED_BYTE, buffer);

						glBindTexture(GL_TEXTURE_2D, 0);

					}

					if (nullptr != lastUpdateProp)
						lastUpdateProp->SetInt((int)dvalue);
				}
			}
		}
	}


}
/*
bool Manager_PostProcessing::OpenSocket(const int portSend, const int portRecv, bool blocking)
{
	bool lResult = false;

	mSocketSender = new Network::Socket();
	if (mSocketSender)
	{
		lResult = ((Network::Socket*)mSocketSender)->Open(8881, true);
	}

	//
	mSocketRecv = new Network::Socket();
	if (mSocketRecv)
	{
		lResult = ((Network::Socket*)mSocketRecv)->Open(8885, false);
	}

	return lResult;
}

void Manager_PostProcessing::CloseSocket()
{
	if (mSocketSender)
	{
		((Network::Socket*)mSocketSender)->Close();
		delete mSocketSender;
		mSocketSender = nullptr;
	}

	//
	if (mSocketRecv)
	{
		((Network::Socket*)mSocketRecv)->Close();
		delete mSocketRecv;
		mSocketRecv = nullptr;
	}
}

void Manager_PostProcessing::SendPreview(PostEffectBuffers *buffers)
{
	FBTime sysTime = mSystem.SystemTime;
	const double sysTimeSecs = sysTime.GetSecondDouble();

	// for debug
	//mIsSynced = true;
	//mSendAddress.Set(192, 168, 1, 76, 8886);
#if BROADCAST_PREVIEW == 1
	if (true == mIsSynced && true == mEffectChain.IsCompressedDataReady() && nullptr != mSocketSender)
	{
		// DONE: we should send header as well with (w, h, aspect, etc.)

		if (mLastSendTimeSecs == 0.0 || (sysTimeSecs - mLastSendTimeSecs) > 0.05)	// 0.05
		{

			// header
			unsigned char *ptr = mSendBuffer;
			Network::FillBufferWithHeader(ptr, PACKET_REASON_IMAGE, (float)sysTimeSecs);
			ptr += sizeof(Network::CPacketHeader);

			Network::CPacketImageHeader *header = (Network::CPacketImageHeader*) ptr;
			buffers->MapCompressedData((float)sysTimeSecs, *header);

			unsigned char *imageData = buffers->GetCompressedData();
			unsigned int imageSize = buffers->GetCompressedSize();

			// DONE: send in tiles

			const int tileSize = DEFAULT_TILE_SIZE;
			const int numberOfTiles = 1 + imageSize / tileSize;	// 1k for each tile

			header->numTiles = numberOfTiles;
			header->tileSize = tileSize;

			unsigned int packetsize = sizeof(Network::CPacketHeader) + sizeof(Network::CPacketImageHeader);

			int bytes_send = ((Network::Socket*)mSocketSender)->Send(mSendAddress, mSendBuffer, packetsize);
			if (bytes_send != packetsize)
			{
				FBTrace("[Post processing send image] Failed to send a udp header\n");
			}

			// send tiles
			((Network::CPacketHeader*)mSendBuffer)->reason = PACKET_REASON_IMAGE_TILE;

			
			for (int i = 0; i < numberOfTiles; ++i)
			{
				ptr = mSendBuffer + sizeof(Network::CPacketHeader);

				Network::CPacketImageTile *tileHeader = (Network::CPacketImageTile*) ptr;
				tileHeader->tileCount = numberOfTiles;
				tileHeader->tileIndex = i;

				const unsigned char *tileData = imageData + tileSize * i;
				unsigned int iTileSize = imageSize - tileSize * i;
				if (iTileSize > tileSize)
					iTileSize = tileSize;

				tileHeader->tileSize = iTileSize;
				tileHeader->tileOffset = tileSize * i;

				ptr = ptr + sizeof(Network::CPacketImageTile);

				memcpy(ptr, tileData, iTileSize);

				packetsize = sizeof(Network::CPacketHeader) + sizeof(Network::CPacketImageTile) + iTileSize;

				bytes_send = ((Network::Socket*)mSocketSender)->Send(mSendAddress, mSendBuffer, packetsize);
				if (bytes_send != packetsize)
				{
					FBTrace("[Post processing send image] Failed to send a udp packet\n");
				}
			}

			mLastSendTimeSecs = sysTimeSecs;
		}

	}
#endif
}
*/
void Manager_PostProcessing::PushUpperLowerClipForEffects()
{
	FBScene *pScene = mSystem.Scene;

	// let's find a clip values

	double lower = -1.0;
	double upper = -1.0;

	for (int i = 0, count = pScene->HUDs.GetCount(); i < count; ++i)
	{
		FBHUD *pHud = pScene->HUDs[i];

		for (int j = 0, elemCount = pHud->Elements.GetCount(); j < elemCount; ++j)
		{
			if (FBIS(pHud->Elements[j], FBHUDRectElement))
			{
				FBHUDRectElement *pelem = (FBHUDRectElement*) pHud->Elements[j];

				if (nullptr != strstr(pelem->Name, RENDER_HUD_RECT_BOTTOM))
				{
					lower = pelem->Height;
				}
				else if (nullptr != strstr(pelem->Name, RENDER_HUD_RECT_TOP))
				{
					upper = pelem->Height;
				}
			}
		}
	}

	//
	
	for (int i = 0, count = pScene->UserObjects.GetCount(); i < count; ++i)
	{
		if (FBIS(pScene->UserObjects[i], PostPersistentData))
		{
			PostPersistentData *pData = (PostPersistentData*)pScene->UserObjects[i];

			if (true == pData->Active && true == pData->AutoClipFromHUD)
			{
				pData->PushClipSettings(upper, lower);
			}
		}
	}
}

void Manager_PostProcessing::PopUpperLowerClipForEffects()
{
	FBScene *pScene = mSystem.Scene;
	//

	for (int i = 0, count = pScene->UserObjects.GetCount(); i < count; ++i)
	{
		if (FBIS(pScene->UserObjects[i], PostPersistentData))
		{
			PostPersistentData *pData = (PostPersistentData*)pScene->UserObjects[i];

			if (true == pData->Active && true == pData->AutoClipFromHUD)
			{
				pData->PopClipSettings();
			}
		}
	}
}