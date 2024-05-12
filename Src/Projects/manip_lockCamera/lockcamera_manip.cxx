
/**	\file	lockcamera_manip.cxx

Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE


*/

//--- Class declarations
#include "lockcamera_manip.h"
//#include <fbsdk/fbsdk-opengl.h>
#include <Windows.h>
#include <GL\GL.h>

//--- Registration defines
#define MANIP_LOCKCAMERA__CLASS	MANIP_LOCKCAMERA__CLASSNAME
#define MANIP_LOCKCAMERA__LABEL	"Manip Lock Camera"
#define MANIP_LOCKCAMERA__DESC	"Manipulator To Lock a Pane Camera"

//--- FiLMBOX implementation and registration
FBManipulatorImplementation(MANIP_LOCKCAMERA__CLASS);
FBRegisterManipulator(			MANIP_LOCKCAMERA__CLASS,
								MANIP_LOCKCAMERA__LABEL,
								MANIP_LOCKCAMERA__DESC,
								FB_DEFAULT_SDK_ICON			);	// Icon filename (default=Open Reality icon)

#define DEFAULT_LOCK_FILENAME	"\\system\\CharacterizationTool\\icons\\ToolBar_Lock.png"
#define DEFAULT_UNLOCK_FILENAME	"\\system\\CharacterizationTool\\icons\\ToolBar_UnLock.png"

//
HGLRC		gLastContext = 0;

#define GL_BGR									GL_BGR_EXT
#define GL_BGRA									GL_BGRA_EXT
#define GL_CLAMP_TO_EDGE 0x812F

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool Manip_LockCamera::FBCreate()
{
	if( FBManipulator::FBCreate() )
	{
		Active = true;
		AlwaysActive = true;
		Visible = false;

		// Properties
		DefaultBehavior		= true;
		ViewerText			= "Lock Current Camera Manipulator";

		// Members
		mLockPath = DEFAULT_LOCK_FILENAME;
		mUnLockPath = DEFAULT_UNLOCK_FILENAME;

		mAlpha = 0.8;
		mStates[0] = mStates[1] = mStates[2] = mStates[3] = STATE_UNLOCK;
		mPosition = FBVector2d(10.0, 10.0);
		mSize = FBVector2d(20.0, 20.0);

		LoadConfig();

		FBEvaluateManager::TheOne().OnRenderingPipelineEvent.Add(this, (FBCallback)&Manip_LockCamera::OnPerFrameRenderingPipelineCallback);
		mSystem.Scene->OnChange.Add(this, (FBCallback)&Manip_LockCamera::EventSceneChange);
		mSystem.OnUIIdle.Add(this, (FBCallback)&Manip_LockCamera::OnUIIdle);

		//mApp.OnFileOpenCompleted.Add(this, (FBCallback)&ORManip_Template::OnFileNew);
		//mApp.OnFileNew.Add(this, (FBCallback)&ORManip_Template::OnFileNew);

		mLockId = 0;
		mUnLockId = 0;

		return true;
	}
	return false;
}


bool getOrSetBool(FBConfigFile &lConfigFile, char *buffer, const char *section, const char *name, const char *comment, bool defValue)
{
	sprintf_s(buffer, sizeof(char)* 256, (defValue) ? "1" : "0");
	const char *lbuffer = buffer;
	lConfigFile.GetOrSet(section, name, lbuffer, comment);

	return (strcmp(buffer, "1") == 0);
};

int getOrSetInt(FBConfigFile &lConfigFile, char *buffer, const char *section, const char *name, const char *comment, int defValue)
{
	sprintf_s(buffer, sizeof(char)* 256, "%d", defValue);
	const char *lbuffer = buffer;
	lConfigFile.GetOrSet(section, name, lbuffer, comment);

	return atoi(buffer);
};

double getOrSetDouble(FBConfigFile &lConfigFile, char *buffer, const char *section, const char *name, const char *comment, double defValue)
{
	sprintf_s(buffer, sizeof(char)* 256, "%.2lf", defValue);
	const char *lbuffer = buffer;
	lConfigFile.GetOrSet(section, name, lbuffer, comment);

	return atof(lbuffer);
};

void getOrSetString(FBString &value, FBConfigFile &lConfigFile, const char *section, const char *name, const char *comment)
{
	//sprintf_s( buffer, sizeof(char)*256, "%s", defValue );
	const char *lbuffer = value;
	lConfigFile.GetOrSet(section, name, lbuffer, comment);
	value = lbuffer;
}

void Manip_LockCamera::LoadConfig()
{
	FBConfigFile	lConfig("@LockPaneCamera.txt");

	char buffer[256];
	FBString sessionName("Common");

	mPosition[0] = getOrSetDouble(lConfig, buffer, sessionName, "OffsetX", "Offset from the pane border", mPosition[0]);
	mPosition[1] = getOrSetDouble(lConfig, buffer, sessionName, "OffsetY", "Offset from the pane border", mPosition[1]);

	mSize[0] = getOrSetDouble(lConfig, buffer, sessionName, "SizeX", "Offset from the pane border", mSize[0]);
	mSize[1] = getOrSetDouble(lConfig, buffer, sessionName, "SizeY", "Offset from the pane border", mSize[1]);

	mAlpha = getOrSetDouble(lConfig, buffer, sessionName, "Alpha", "Rect transparency", mAlpha);
	
	getOrSetString(mLockPath, lConfig, sessionName, "Lock Image Path", "image for the lock state, relative to mobu bin");
	getOrSetString(mUnLockPath, lConfig, sessionName, "UnLock Image Path", "image for the unlock state, relative to mobu bin");
}

/************************************************
 *	FiLMBOX Destructor.
 ************************************************/
void Manip_LockCamera::FBDestroy()
{
	FBEvaluateManager::TheOne().OnRenderingPipelineEvent.Remove(this, (FBCallback)&Manip_LockCamera::OnPerFrameRenderingPipelineCallback);
	mSystem.Scene->OnChange.Remove(this, (FBCallback)&Manip_LockCamera::EventSceneChange);
	mSystem.OnUIIdle.Remove(this, (FBCallback)&Manip_LockCamera::OnUIIdle);

	//mApp.OnFileOpenCompleted.Remove(this, (FBCallback)&ORManip_Template::OnFileNew);
	//mApp.OnFileNew.Remove(this, (FBCallback)&ORManip_Template::OnFileNew);

	FBManipulator::FBDestroy();
}


/************************************************
 *	Draw function for manipulator
 ************************************************/
void Manip_LockCamera::ViewExpose()
{
	int paneIndex = mRenderPaneIndex;
	mRenderPaneIndex += 1;

	// TODO: grab current pane dimentions

	int lViewport[4];

	glGetIntegerv(GL_VIEWPORT, lViewport);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0.0, lViewport[2], lViewport[3], 0.0, -1.0, 1.0);

	
	//--- Transformation for camera viewpoint
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	{
		glLoadIdentity();

		if (mAlpha < 1.0)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		

		glEnable(GL_TEXTURE_2D);
		//glDisable(GL_TEXTURE_2D);

		glColor4d(1.0, 1.0, 1.0, mAlpha);
		if (mStates[paneIndex] > STATE_UNLOCK)
		{
			if (STATE_LOCK_SWITCHER == mStates[paneIndex])
				glColor4d(1.0, 0.0, 0.0, mAlpha);

			glBindTexture(GL_TEXTURE_2D, mLockId);
		}	
		else
		{
			glBindTexture(GL_TEXTURE_2D, mUnLockId);
		}

		glBegin(GL_POLYGON);
			glTexCoord2d(0.0, 0.0);
			glVertex2d( mPosition[0], mPosition[1] );
			glTexCoord2d(1.0, 0.0);
			glVertex2d( mPosition[0]+mSize[0], mPosition[1] );
			glTexCoord2d(1.0, 1.0);
			glVertex2d( mPosition[0]+mSize[0], mPosition[1]+mSize[1] );
			glTexCoord2d(0.0, 1.0);
			glVertex2d( mPosition[0], mPosition[1]+mSize[1] );
		glEnd();

		if (mAlpha < 1.0)
		{
			glDisable(GL_BLEND);
		}
		glDisable(GL_TEXTURE_2D);
	}
	glPopMatrix();

}


/************************************************
 *	Deal with maniplator input.
 ************************************************/
bool Manip_LockCamera::ViewInput(int pMouseX, int pMouseY, FBInputType pAction, int pButtonKey, int pModifier)
{
	switch( pAction )
	{
		case kFBKeyPress:
		{
			// When a keyboard key is pressed.
		}
		break;
		case kFBKeyRelease:
		{
			// When a keyboard key is released.
		}
		break;
		case kFBButtonDoubleClick:
		{
			// Mouse button double-clicked.
		}
		break;
		case kFBButtonPress:
		{
			FBRenderer *pRenderer = mSystem.Renderer;
			unsigned int currPane = pRenderer->GetSelectedPaneIndex();
			pRenderer->GetPaneCount();


			// Mouse button clicked.
			
			FBCamera* lCamera = pRenderer->GetCameraInPane(currPane);
			bool camSwitcher = pRenderer->IsCameraSwitcherInPane(currPane);

			double v1[2] = {mPosition[0], mPosition[1]};
			double v2[2] = { mPosition[0] + mSize[0], mPosition[1] + mSize[1] };

			int startPos = 0; // - lCamera->CameraViewportX

			if (currPane == 1 || currPane == 3)
				startPos = lCamera->WindowWidth / 2;

			
			/*
			int x = pMouseX - startPos;
			int y = pMouseY;

			int height = lCamera->WindowHeight / 2; // lCamera->CameraViewportHeight;

			if (3 == numberOfPanes)
			{
				if (currPane == 2)
					y = y - height;
			}
			else if (4 == numberOfPanes)
			{
				if (currPane >= 2)
					y = y - height;
			}
			*/

			int x = pMouseX - GetPanePosX();
			int y = pMouseY - GetPanePosY();

			if (x > v1[0] && x < v2[0] && y > v1[1] && y < v2[1])
			{
				mStates[currPane] = (mStates[currPane] > STATE_UNLOCK) ? STATE_UNLOCK : STATE_LOCK_CAMERA;
				
				FBCamera *pNewCamera = nullptr;

				if (true == camSwitcher && STATE_LOCK_CAMERA == mStates[currPane])
					mStates[currPane] = STATE_LOCK_SWITCHER;
				else if (STATE_LOCK_CAMERA == mStates[currPane])
					pNewCamera = lCamera;

				switch (currPane)
				{
				case 0:
					mCameraPane0 = pNewCamera;
					break;
				case 1:
					mCameraPane1 = pNewCamera;
					break;
				case 2:
					mCameraPane2 = pNewCamera;
					break;
				case 3:
					mCameraPane3 = pNewCamera;
					break;
				}
				
			}
#ifdef _DEBUG
			FBTrace("curr pane - %d, mousex - %d, mousey - %d\n", currPane, x, y);
#endif
		}
		break;
		case kFBButtonRelease:
		{
			// Mouse button released.
		}
		break;
		case kFBMotionNotify:
		{
			// When there is mouse movement in the viewer window
		}
		break;
		case kFBDragging:
		{
			// Items are being dragged.
		}
		break;
		case kFBDropping:
		{
			// Items are being dropped.
		}
		break;
	}
	return true;
}


void Manip_LockCamera::OnPerFrameRenderingPipelineCallback(HISender pSender, HKEvent pEvent)
{
	FBEventEvalGlobalCallback lFBEvent(pEvent);


	switch (lFBEvent.GetTiming())
	{
	case kFBGlobalEvalCallbackBeforeRender:
	{

											  mRenderPaneIndex = 0;

											  HGLRC currRc = wglGetCurrentContext();
											  if (0 == gLastContext || currRc != gLastContext)
											  {
												  FreeTextures();
												  gLastContext = currRc;
											  }

											  static bool firstLoad = true;
											  if (true == firstLoad || 0 == mLockId)
											  {
												  LoadTextures();
												  firstLoad = false;
											  }
											  

	} break;
	case kFBGlobalEvalCallbackAfterRender:
	{
											 //
											 // This callback occurs just before swapping GL back/front buffers. 
											 // User could do some special effect, HUD or buffer download (via PBO) here. 
											 //



	} break;

	default:
		break;
	}
}

void Manip_LockCamera::OnUIIdle(HISender pSender, HKEvent pEvent)
{
	FBRenderer *pRenderer = mSystem.Renderer;
	unsigned int currPane = pRenderer->GetSelectedPaneIndex();

	FBCamera* lCamera = pRenderer->GetCameraInPane(currPane);

	if (nullptr != lCamera)
	{
		if (STATE_LOCK_CAMERA == mStates[currPane])
		{
			FBCamera *pPaneCamera = nullptr;

			switch (currPane)
			{
			case 0:
				pPaneCamera = (mCameraPane0.Ok()) ? mCameraPane0 : nullptr;
				break;
			case 1:
				pPaneCamera = (mCameraPane1.Ok()) ? mCameraPane1 : nullptr;
				break;
			case 2:
				pPaneCamera = (mCameraPane2.Ok()) ? mCameraPane2 : nullptr;
				break;
			case 3:
				pPaneCamera = (mCameraPane3.Ok()) ? mCameraPane3 : nullptr;
				break;
			}

			if (nullptr != pPaneCamera) 
			{
				if (lCamera != pPaneCamera)
					pRenderer->SetCameraInPane(pPaneCamera, currPane);
				if (true == pRenderer->IsCameraSwitcherInPane(currPane))
					pRenderer->SetCameraSwitcherInPane(currPane, false);
			}
		}
		else if (STATE_LOCK_SWITCHER == mStates[currPane])
		{
			pRenderer->SetCameraSwitcherInPane(currPane, true);
		}
	}
}

void Manip_LockCamera::EventSceneChange(HISender pSender, HKEvent pEvent)
{
	FBEventSceneChange	e(pEvent);
	FBSceneChangeType type;
	e.Type.GetData(&type, sizeof(FBSceneChangeType));

	if (kFBSceneChangeDetach == type && FBIS(e.ChildComponent, FBCamera))
	{
		if (FBIS(e.Component, FBScene))
		{
			FBCamera *lCamera = (FBCamera*)(FBComponent*)e.ChildComponent;

			if (lCamera == mCameraPane0)
			{
				mStates[0] = STATE_UNLOCK;
				mCameraPane0 = nullptr;
			}
			else if (lCamera == mCameraPane1)
			{
				mStates[1] = STATE_UNLOCK;
				mCameraPane1 = nullptr;
			}
			else if (lCamera == mCameraPane2)
			{
				mStates[2] = STATE_UNLOCK;
				mCameraPane2 = nullptr;
			}
			else if (lCamera == mCameraPane3)
			{
				mStates[3] = STATE_UNLOCK;
				mCameraPane3 = nullptr;
			}
		}
	}
}

void Manip_LockCamera::OnFileNew(HISender pSender, HKEvent pEvent)
{
	mStates[0] = mStates[1] = mStates[2] = mStates[3] = STATE_UNLOCK;
	mCameraPane0 = nullptr;
	mCameraPane1 = nullptr;
	mCameraPane2 = nullptr;
	mCameraPane3 = nullptr;
}

void Manip_LockCamera::LoadTextures()
{
	if (mLockId > 0)
		return;

	FBString path(mSystem.ApplicationPath);

	path = path.Left(path.GetLen() - 4);
	FBString lockPath(path, mLockPath);
	FBString unlockPath(path, mUnLockPath);

	mLockId = UploadImageIntoTexture(lockPath, 0);
	mUnLockId = UploadImageIntoTexture(unlockPath, 0);
}

void Manip_LockCamera::FreeTextures()
{
	if (mLockId > 0)
	{
		glDeleteTextures(1, &mLockId);
		mLockId = 0;
	}
	if (mUnLockId > 0)
	{
		glDeleteTextures(1, &mUnLockId);
		mUnLockId = 0;
	}
}

unsigned int Manip_LockCamera::UploadImageIntoTexture(const char *filename, const unsigned int useId)
{
	GLuint id = 0;
	FBImage *pImage = new FBImage(filename);

	if (pImage)
	{
		if (pImage->Width > 0)
		{
			pImage->VerticalFlip();
			unsigned char *buffer = pImage->GetBufferAddress();

			GLuint internalFormat = GL_RGB8;
			GLuint format = GL_RGB;

			int width = pImage->Width;
			int height = pImage->Height;
			FBImageFormat imageFormat;
			pImage->Format.GetData(&imageFormat, sizeof(FBImageFormat));

			if (kFBImageFormatBGRA32 == imageFormat)
			{
				internalFormat = GL_RGBA8;
				format = GL_BGRA;
			}
			else if (kFBImageFormatRGBA32 == imageFormat)
			{
				internalFormat = GL_RGBA8;
				format = GL_RGBA;
			}
			else if (kFBImageFormatRGB24 == imageFormat)
			{
				internalFormat = GL_RGB8;
				format = GL_RGB;
			}

			id = useId;
			if (0 == id)
				glGenTextures(1, &id);

			glBindTexture(GL_TEXTURE_2D, id);

			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);                         // set 1-byte alignment
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, buffer);

			glBindTexture(GL_TEXTURE_2D, 0);

		}


		delete pImage;
	}
	return id;
}