
/** \file   postprocessing_manager.cxx

	Sergei <Neill3d> Solokhin 2018

	GitHub page - https://github.com/Neill3d/OpenMoBu
	Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include <Windows.h>
#include <filesystem>
#include "postprocessingmanager.h"

#include "postprocessing_helper.h"

//--- Registration defines
#define POSTPROCESSING_MANAGER__CLASS POSTPROCESSING_MANAGER__CLASSNAME
#define POSTPROCESSING_MANAGER__NAME  POSTPROCESSING_MANAGER__CLASSSTR

//--- FiLMBOX implementation and registration
FBCustomManagerImplementation(POSTPROCESSING_MANAGER__CLASS);  // Manager class name.
FBRegisterCustomManager(POSTPROCESSING_MANAGER__CLASS);         // Manager class name.

#define RENDER_HUD_RECT_TOP				"RectangleTop"
#define RENDER_HUD_RECT_BOTTOM			"RectangleBottom"

// track the state of OpenGL viewport context
HGLRC	PostProcessingManager::gCurrentContext = 0;
std::map<HGLRC, PostProcessContextData*>	PostProcessingManager::gContextMap;

PostProcessingManager *gManager = nullptr;

// define new task cycle index
FBProfiler_CreateTaskCycle(PostProcessRenderer, 0.5f, 0.9f, 0.9f);


bool GRenderAfterRender()
{
	if (nullptr != gManager && !gManager->skipRender)
	{
		return gManager->ExternalRenderAfterRender();
	}

	return false;
}

PostProcessingManager* GetPostProcessingManager()
{
	return gManager;
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
bool PostProcessingManager::FBCreate()
{
	//
	// Register task cycle index in profiler.
	//
	FBProfiling_SetupTaskCycle(PostProcessRenderer);

	mEnterId = 0;
	mFrameId = 0;
	
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
	
	mSocketSender = nullptr;
	mSocketRecv = nullptr;

	mDoVideoClipTimewrap = false;
	skipRender = false;

    return true;
}

/************************************************
 *  FiLMBOX Destruction.
 ************************************************/
void PostProcessingManager::FBDestroy()
{
    // Free any user memory here.
}


/************************************************
 *  Execution callback.
 ************************************************/
bool PostProcessingManager::Init()
{
	glewInit();

	//
#ifdef OGL_DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(DebugOGL_Callback, nullptr);
#endif

	LoadShaderTextInsertions();
    return true;
}

bool PostProcessingManager::Open()
{
	mApplication.OnFileNewCompleted.Add(this, (FBCallback)&PostProcessingManager::EventFileNew);
	mApplication.OnFileOpen.Add(this, (FBCallback)&PostProcessingManager::EventFileOpen);
	mApplication.OnFileOpenCompleted.Add(this, (FBCallback)&PostProcessingManager::EventFileOpenComplete);
	mApplication.OnOverrideFileOpen.Add(this, (FBCallback)&PostProcessingManager::EventFileOpenOverride);

	mSystem.Scene->OnChange.Add(this, (FBCallback)&PostProcessingManager::EventSceneChange);

	mSystem.OnUIIdle.Add(this, (FBCallback)&PostProcessingManager::OnUIIdle);

	mSystem.OnVideoFrameRendering.Add(this, (FBCallback)&PostProcessingManager::OnVideoFrameRendering);

	FBEvaluateManager::TheOne().OnRenderingPipelineEvent.Add(this, (FBCallback)&PostProcessingManager::OnPerFrameRenderingPipelineCallback);

    return true;
}

void PostProcessingManager::LoadShaderTextInsertions()
{
	constexpr const char* KEYWORD_HEADER{ "INSERT: HEADER" };
	constexpr const char* KEYWORD_IMAGE_CROP{ "INSERT: APPLY_IMAGE_CROP" };
	constexpr const char* KEYWORD_MASKING{ "INSERT: APPLY_MASKING" };

	constexpr const char* INSERT_HEADER{ "/GLSL/insert_header.glslf" };
	constexpr const char* INSERT_IMAGE_CROP{ "/GLSL/insert_image_crop.glslf" };
	constexpr const char* INSERT_MASKING{ "/GLSL/insert_masking.glslf" };

	char shadersPath[MAX_PATH];
	if (FindEffectLocation(INSERT_HEADER, shadersPath, MAX_PATH))
	{
		std::string filePath = std::string(shadersPath) + INSERT_HEADER;
		GLSLShaderProgram::AddTextInsertionFromFile(KEYWORD_HEADER, filePath.c_str());
	}

	if (FindEffectLocation(INSERT_IMAGE_CROP, shadersPath, MAX_PATH))
	{
		std::string filePath = std::string(shadersPath) + INSERT_IMAGE_CROP;
		GLSLShaderProgram::AddTextInsertionFromFile(KEYWORD_IMAGE_CROP, filePath.c_str());
	}

	if (FindEffectLocation(INSERT_MASKING, shadersPath, MAX_PATH))
	{
		std::string filePath = std::string(shadersPath) + INSERT_MASKING;
		GLSLShaderProgram::AddTextInsertionFromFile(KEYWORD_MASKING, filePath.c_str());
	}
}

void PostProcessingManager::LoadConfig()
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

void PostProcessingManager::OnUIIdle(HISender pSender, HKEvent pEvent)
{

	if (true == mFirstRun)
	{
		mFirstRun = false;

		//
		mSystem.OnConnectionNotify.Add(this, (FBCallback)&PostProcessingManager::EventConnNotify);
		mSystem.OnConnectionDataNotify.Add(this, (FBCallback)&PostProcessingManager::EventConnDataNotify);

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

bool PostProcessingManager::Clear()
{
    return true;
}

bool PostProcessingManager::Close()
{
	mSystem.OnUIIdle.Remove(this, (FBCallback)&PostProcessingManager::OnUIIdle);

	FBEvaluateManager::TheOne().OnRenderingPipelineEvent.Remove(this, (FBCallback)&PostProcessingManager::OnPerFrameRenderingPipelineCallback);

	mApplication.OnFileNewCompleted.Remove(this, (FBCallback)&PostProcessingManager::EventFileNew);
	mApplication.OnFileOpen.Remove(this, (FBCallback)&PostProcessingManager::EventFileOpen);
	mApplication.OnFileMerge.Remove(this, (FBCallback)&PostProcessingManager::EventFileMerge);
	mApplication.OnFileOpenCompleted.Remove(this, (FBCallback)&PostProcessingManager::EventFileOpenComplete);
	mApplication.OnOverrideFileOpen.Remove(this, (FBCallback)&PostProcessingManager::EventFileOpenOverride);

	mSystem.Scene->OnChange.Remove(this, (FBCallback)&PostProcessingManager::EventSceneChange);
	mSystem.OnConnectionNotify.Remove(this, (FBCallback)&PostProcessingManager::EventConnNotify);
	mSystem.OnConnectionDataNotify.Remove(this, (FBCallback)&PostProcessingManager::EventConnDataNotify);
	mSystem.OnVideoFrameRendering.Remove(this, (FBCallback)&PostProcessingManager::OnVideoFrameRendering);

	//CloseSocket();

    return true;
}


void PostProcessingManager::EventFileNew(HISender pSender, HKEvent pEvent)
{
	// clear all pointers (start point)

	//ClearOutputCompositePtr();
	//mSettings = nullptr;
}

void PostProcessingManager::EventFileOpen(HISender pSender, HKEvent pEvent)
{
	//mSettings = nullptr;
	skipRender = true;
}

void PostProcessingManager::EventFileOpenOverride(HISender pSender, HKEvent pEvent)
{
	FBEventOverrideFileOpen lEvent(pEvent);
	SetCurrentFileOpenPath(lEvent.FilePath.AsString());
}

void PostProcessingManager::EventFileMerge(HISender pSender, HKEvent pEvent)
{
	//mSettings = nullptr;
	//mSettingsMerge = true;
	
}

void PostProcessingManager::EventFileOpenComplete(HISender pSender, HKEvent pEvent)
{
	//mSettings = nullptr;
	skipRender = false;
}


void PostProcessingManager::EventSceneChange(HISender pSender, HKEvent pEvent)
{
}



void PostProcessingManager::EventConnDataNotify(HISender pSender, HKEvent pEvent)
{
	
}

void PostProcessingManager::EventConnNotify(HISender pSender, HKEvent pEvent)
{
	
}


void PostProcessingManager::OnPerFrameSynchronizationCallback(HISender pSender, HKEvent pEvent)
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

void PostProcessingManager::OnPerFrameEvaluationPipelineCallback(HISender pSender, HKEvent pEvent)
{
	
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnPerRenderingPipelineCallback


void PostProcessingManager::CheckForAContextChange()
{
	HGLRC hContext = wglGetCurrentContext();

	if (0 == gCurrentContext)
	{
		// initialize for the first time
		gCurrentContext = hContext;
	}

	auto iter = gContextMap.find(hContext);

	if (iter == end(gContextMap))
	{
		PostProcessContextData *newData = new PostProcessContextData();
		newData->Init();
		gContextMap.insert(std::make_pair(hContext, newData));
	}

	if (hContext != gCurrentContext)
	{
		gCurrentContext = hContext;
		FBTrace("> !! CHANGE CONTEXT !!\n");
	}
}


void PostProcessingManager::PreRenderFirstEntry()
{
	CheckForAContextChange();

	auto iter = gContextMap.find(gCurrentContext);

	if (iter != end(gContextMap))
	{
		iter->second->PreRenderFirstEntry();
	}
}


void PostProcessingManager::OnPerFrameRenderingPipelineCallback(HISender pSender, HKEvent pEvent)
{

	if (skipRender)
		return;

	FBEventEvalGlobalCallback lFBEvent(pEvent);

	// check for a context change here
	if (mEnterId < 1 && lFBEvent.GetTiming() == kFBGlobalEvalCallbackBeforeRender)
	{
		PreRenderFirstEntry();
	}

	auto iter = gContextMap.find(gCurrentContext);

	if (iter == end(gContextMap))
	{
		return;
	}

	bool usePostProcessing = false;

	for (int i = 0; i<iter->second->mLastPaneCount; ++i)
	{
		if (nullptr != iter->second->mPaneSettings[i])
		{
			usePostProcessing = true;
			break;
		}
	}

	
	switch (lFBEvent.GetTiming())
	{
	case kFBGlobalEvalCallbackBeforeRender:
		{
		if (iter->second->mViewerViewport[2] <= 1 || iter->second->mViewerViewport[3] <= 1)
		{
			usePostProcessing = false;
		}

			mLastProcessCompositions = usePostProcessing;
			iter->second->RenderBeforeRender(usePostProcessing, false);
			
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
			
			//
			// Start PostProcessRenderer task cycle profiling, 
			//
			FBProfilerHelper lProfiling(FBProfiling_TaskCycleIndex(PostProcessRenderer), FBGetDisplayInfo(), FBGetRenderingTaskCycle());

			iter->second->RenderAfterRender(usePostProcessing, false);

		} break;

	default:
		break;
	}

	CHECK_GL_ERROR();
}


bool PostProcessingManager::ExternalRenderAfterRender()
{
	auto iter = gContextMap.find(gCurrentContext);

	if (iter != end(gContextMap))
	{
		return iter->second->RenderAfterRender(mLastProcessCompositions, false);
	}
	return false;
}

void PostProcessingManager::OnVideoFrameRendering(HISender pSender, HKEvent pEvent)
{
	FBEventVideoFrameRendering levent(pEvent);

	if (levent.GetState() == FBEventVideoFrameRendering::eBeginRendering)
	{
		PreRenderFirstEntry();

		auto iter = gContextMap.find(gCurrentContext);
		if (iter == end(gContextMap))
			return;

		// turn off preview mode and switch quality settings if needed
		iter->second->mVideoRendering = true;
		
		PushUpperLowerClipForEffects();
	}
	else if (levent.GetState() == FBEventVideoFrameRendering::eEndRendering)
	{
		auto iter = gContextMap.find(gCurrentContext);
		if (iter == end(gContextMap))
			return;

		// turn on back preview mode and display quality settings
		iter->second->mVideoRendering = false;
		PopUpperLowerClipForEffects();
	}
}

void PostProcessingManager::PrepVideoClipsTimeWrap()
{

	FBScene *pScene = mSystem.Scene;

	for (int i = 0, count = pScene->VideoClips.GetCount(); i < count; ++i)
	{
		FBVideoClip *pVideoClip = pScene->VideoClips[i];

		if (!pVideoClip || FBIS(pVideoClip, FBVideoMemory))
			continue;

		const int firstFrame = pVideoClip->StartFrame;
		const int stopFrame = pVideoClip->StopFrame;

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
				
				if (lastUpdate == -1 || abs((int)dvalue - lastUpdate) >= step)
				{
					int width = pVideoClip->Width;
					int height = pVideoClip->Height;
					FBVideoFormat format;
					pVideoClip->Format.GetData(&format, sizeof(FBVideoFormat));

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
					default:
						glFormat = GL_RGB;
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
void PostProcessingManager::PushUpperLowerClipForEffects()
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

void PostProcessingManager::PopUpperLowerClipForEffects()
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