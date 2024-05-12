
// postprocessing_effectMotionBlur.cpp
/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "postprocessing_effectMotionBlur.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include "postprocessing_helper.h"

#define SHADER_MOTIONBLUR_NAME				"MotionBlur"
#define SHADER_MOTIONBLUR_VERTEX			"\\GLSL\\simple.vsh"
#define SHADER_MOTIONBLUR_FRAGMENT			"\\GLSL\\motionblur.fsh"

////////////////////////////////////////////////////////////////////////////////////
// post SSAO

//! a constructor
PostEffectMotionBlur::PostEffectMotionBlur()
	: PostEffectBase()
{
	for (int i = 0; i < LOCATIONS_COUNT; ++i)
		mLoc.arr[i] = -1;
}

//! a destructor
PostEffectMotionBlur::~PostEffectMotionBlur()
{
}

const char *PostEffectMotionBlur::GetName()
{
	return SHADER_MOTIONBLUR_NAME;
}
const char *PostEffectMotionBlur::GetVertexFname(const int)
{
	return SHADER_MOTIONBLUR_VERTEX;
}
const char *PostEffectMotionBlur::GetFragmentFname(const int)
{
	return SHADER_MOTIONBLUR_FRAGMENT;
}

bool PostEffectMotionBlur::PrepUniforms(const int shaderIndex)
{
	bool lSuccess = false;

	GLSLShader* mShader = mShaders[shaderIndex];
	if (nullptr != mShader)
	{
		mShader->Bind();

		GLint loc = mShader->findLocation("colorSampler");
		if (loc >= 0)
			glUniform1i(loc, 0);
		loc = mShader->findLocation("depthSampler");
		if (loc >= 0)
			glUniform1i(loc, 2);
		
		mLoc.zNear = mShader->findLocation("zNear");
		mLoc.zFar = mShader->findLocation("zFar");

		mLoc.dt = mShader->findLocation("dt");

		mLoc.upperClip = mShader->findLocation("upperClip");
		mLoc.lowerClip = mShader->findLocation("lowerClip");

		mLoc.clipInfo = mShader->findLocation("gClipInfo");

		mLoc.projInfo = mShader->findLocation("projInfo");
		mLoc.projOrtho = mShader->findLocation("projOrtho");
		mLoc.InvQuarterResolution= mShader->findLocation("InvQuarterResolution");
		mLoc.InvFullResolution= mShader->findLocation("InvFullResolution");

		mLoc.uInverseModelViewMat = mShader->findLocation("uInverseModelViewMat");
		mLoc.uPrevModelViewProj = mShader->findLocation("uPrevModelViewProj");

		mShader->UnBind();

		//
		
		lSuccess = true;
	}

	return lSuccess;
}

bool PostEffectMotionBlur::CollectUIValues(PostPersistentData *pData, int w, int h, FBCamera *pCamera)
{
	bool lSuccess = false;

	FBTime localTime = FBSystem::TheOne().LocalTime;
	//FBTime systemTime = FBSystem::TheOne().SystemTime;

	const double upperClip = pData->UpperClip;
	const double lowerClip = pData->LowerClip;

	const double _amount = pData->MotionBlurAmount;

	float znear = (float) pCamera->NearPlaneDistance;
	float zfar = (float) pCamera->FarPlaneDistance;
	FBCameraType cameraType;
	pCamera->Type.GetData(&cameraType, sizeof(FBCameraType));
	bool perspective = (cameraType == FBCameraType::kFBCameraTypePerspective);
	//float fov = (float) pCamera->FieldOfView;

	// calculate a diagonal fov

	// convert to mm
	double filmWidth = 25.4 * pCamera->FilmSizeWidth;
	double filmHeight = 25.4 * pCamera->FilmSizeHeight;

	double diag = sqrt(filmWidth*filmWidth + filmHeight*filmHeight);
	double focallen = pCamera->FocalLength;

	float fov = 2.0 * atan(diag / (focallen * 2.0));

	float clipInfo[4];

	clipInfo[0] = znear * zfar;
	clipInfo[1] = znear - zfar;
	clipInfo[2] = zfar;
	clipInfo[3] = (perspective) ? 1.0f : 0.0f;


	FBMatrix dproj, dinvProj, dinvModelview;
	pCamera->GetCameraMatrix(dproj, kFBProjection);
	pCamera->GetCameraMatrix(dinvProj, kFBProjInverse);

	pCamera->GetCameraMatrix(dinvModelview, kFBModelViewProj);
	FBMatrixInverse(dinvModelview, dinvModelview);

	//
	float fInvModelView[16];
	float fprevModelViewProj[16];

	for (int i = 0; i < 16; ++i)
	{
		fInvModelView[i] = (float)dinvModelview[i];
		fprevModelViewProj[i] = (float)mLastModelViewProj[i];
	}

	//

	float P[16];
	for (int i = 0; i < 16; ++i)
	{
		P[i] = (float)dproj[i];
	}

	float projInfoPerspective[] = {
		2.0f / (P[4 * 0 + 0]),       // (x) * (R - L)/N
		2.0f / (P[4 * 1 + 1]),       // (y) * (T - B)/N
		-(1.0f - P[4 * 2 + 0]) / P[4 * 0 + 0], // L/N
		-(1.0f + P[4 * 2 + 1]) / P[4 * 1 + 1], // B/N
	};

	float projInfoOrtho[] = {
		2.0f / (P[4 * 0 + 0]),      // ((x) * R - L)
		2.0f / (P[4 * 1 + 1]),      // ((y) * T - B)
		-(1.0f + P[4 * 3 + 0]) / P[4 * 0 + 0], // L
		-(1.0f - P[4 * 3 + 1]) / P[4 * 1 + 1], // B
	};

	int useOrtho = (false == perspective) ? 1 : 0;
	int projOrtho = useOrtho;
	float *projInfo = useOrtho ? projInfoOrtho : projInfoPerspective;

	float projScale;
	if (useOrtho){
		projScale = float(h) / (projInfoOrtho[1]);
	}
	else {
		projScale = float(h) / (tanf(fov * 0.5f) * 2.0f);
	}

	// radius

//	float meters2viewspace = 1.0f;
//	float R = (float)pData->SSAO_Radius * meters2viewspace;
//	float R2 = R * R;
//	float negInvR2 = -1.0f / R2;
//	float RadiusToScreen = R * 0.5f * projScale;

	// resolution
	int quarterWidth = ((w + 3) / 4);
	int quarterHeight = ((h + 3) / 4);

	GLSLShader* mShader = GetShaderPtr();
	if (nullptr != mShader)
	{
		mShader->Bind();

		if (mLoc.zNear >= 0)
			glUniform1f(mLoc.zNear, znear);
		if (mLoc.zFar >= 0)
			glUniform1f(mLoc.zFar, zfar);

		if (mLoc.upperClip >= 0)
			glUniform1f(mLoc.upperClip, 0.01f * (float)upperClip);

		if (mLoc.lowerClip >= 0)
			glUniform1f(mLoc.lowerClip, 1.0f - 0.01f * (float)lowerClip);

		if (mLoc.clipInfo >= 0)
			glUniform4fv(mLoc.clipInfo, 1, clipInfo);

		// proj
		if (mLoc.projInfo >= 0)
			glUniform4fv(mLoc.projInfo, 1, projInfo);
		if (mLoc.projOrtho >= 0)
			glUniform1i(mLoc.projOrtho, projOrtho);

		// matrices
		if (mLoc.uInverseModelViewMat >= 0)
			glUniformMatrix4fv(mLoc.uInverseModelViewMat, 1, GL_FALSE, fInvModelView);
		if (mLoc.uPrevModelViewProj >= 0)
			glUniformMatrix4fv(mLoc.uPrevModelViewProj, 1, GL_FALSE, fprevModelViewProj);

		// resolution
		if (mLoc.InvQuarterResolution >= 0)
			glUniform2f(mLoc.InvQuarterResolution, 1.0f / float(quarterWidth), 1.0f / float(quarterHeight));
		if (mLoc.InvFullResolution >= 0)
			glUniform2f(mLoc.InvFullResolution, 1.0f / float(w), 1.0f / float(h));

		int localFrame = localTime.GetFrame();
		int lastFrame = mLastTime.GetFrame();

		if (0 == localFrame || (localFrame != lastFrame))
		{
			// store modelview proj for the next frame
			pCamera->GetCameraMatrix(mLastModelViewProj, kFBModelViewProj);

			if (mLoc.dt >= 0)
			{
				float dt = (float)(localTime.GetSecondDouble() - mLastTime.GetSecondDouble());
				dt *= 0.01f * (float)_amount;
				glUniform1f(mLoc.dt, dt);
			}

			mLastTime = localTime;
		}

		mShader->UnBind();

		
		lSuccess = true;
	}

	return lSuccess;
}

void PostEffectMotionBlur::Bind()
{
	// bind a random texture
	
	PostEffectBase::Bind();
}

void PostEffectMotionBlur::UnBind()
{
	// bind a random texture
	
	PostEffectBase::UnBind();
}
