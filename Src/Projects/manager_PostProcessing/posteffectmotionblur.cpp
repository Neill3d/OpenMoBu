
// posteffectmotionblur.cpp
/*
Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffectmotionblur.h"
#include "postpersistentdata.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include "postprocessing_helper.h"

#define SHADER_MOTIONBLUR_NAME				"MotionBlur"
#define SHADER_MOTIONBLUR_VERTEX			"\\GLSL\\simple.vsh"
#define SHADER_MOTIONBLUR_FRAGMENT			"\\GLSL\\motionblur.fsh"


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

const char *PostEffectMotionBlur::GetName() const
{
	return SHADER_MOTIONBLUR_NAME;
}
const char *PostEffectMotionBlur::GetVertexFname(const int) const
{
	return SHADER_MOTIONBLUR_VERTEX;
}
const char *PostEffectMotionBlur::GetFragmentFname(const int) const
{
	return SHADER_MOTIONBLUR_FRAGMENT;
}

bool PostEffectMotionBlur::PrepUniforms(const int shaderIndex)
{
	bool lSuccess = false;

	GLSLShaderProgram* mShader = mShaders[shaderIndex];
	if (nullptr != mShader)
	{
		mShader->Bind();

		GLint loc = mShader->findLocation("colorSampler");
		if (loc >= 0)
			glUniform1i(loc, 0);
		loc = mShader->findLocation("depthSampler");
		if (loc >= 0)
			glUniform1i(loc, 2);
		
		PrepareUniformLocations(mShader);

		mLoc.zNear = mShader->findLocation("zNear");
		mLoc.zFar = mShader->findLocation("zFar");

		mLoc.dt = mShader->findLocation("dt");
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

bool PostEffectMotionBlur::CollectUIValues(PostPersistentData *pData, PostEffectContext& effectContext)
{
	bool lSuccess = false;

	const double _amount = pData->MotionBlurAmount;

	float znear = (float) effectContext.camera->NearPlaneDistance;
	float zfar = (float) effectContext.camera->FarPlaneDistance;
	FBCameraType cameraType;
	effectContext.camera->Type.GetData(&cameraType, sizeof(FBCameraType));
	bool perspective = (cameraType == FBCameraType::kFBCameraTypePerspective);
	
	// calculate a diagonal fov

	// convert to mm
	double filmWidth = 25.4 * effectContext.camera->FilmSizeWidth;
	double filmHeight = 25.4 * effectContext.camera->FilmSizeHeight;

	double diag = sqrt(filmWidth*filmWidth + filmHeight*filmHeight);
	double focallen = effectContext.camera->FocalLength;

	float fov = 2.0 * atan(diag / (focallen * 2.0));

	float clipInfo[4];

	clipInfo[0] = znear * zfar;
	clipInfo[1] = znear - zfar;
	clipInfo[2] = zfar;
	clipInfo[3] = (perspective) ? 1.0f : 0.0f;


	FBMatrix dproj, dinvProj, dinvModelview;
	effectContext.camera->GetCameraMatrix(dproj, kFBProjection);
	effectContext.camera->GetCameraMatrix(dinvProj, kFBProjInverse);

	effectContext.camera->GetCameraMatrix(dinvModelview, kFBModelViewProj);
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
		projScale = float(effectContext.h) / (projInfoOrtho[1]);
	}
	else {
		projScale = float(effectContext.h) / (tanf(fov * 0.5f) * 2.0f);
	}

	// radius

//	float meters2viewspace = 1.0f;
//	float R = (float)pData->SSAO_Radius * meters2viewspace;
//	float R2 = R * R;
//	float negInvR2 = -1.0f / R2;
//	float RadiusToScreen = R * 0.5f * projScale;

	// resolution
	int quarterWidth = ((effectContext.w + 3) / 4);
	int quarterHeight = ((effectContext.h + 3) / 4);

	GLSLShaderProgram* mShader = GetShaderPtr();
	if (nullptr != mShader)
	{
		mShader->Bind();

		if (mLoc.zNear >= 0)
			glUniform1f(mLoc.zNear, znear);
		if (mLoc.zFar >= 0)
			glUniform1f(mLoc.zFar, zfar);

		UpdateUniforms(pData);

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
			glUniform2f(mLoc.InvFullResolution, 1.0f / float(effectContext.w), 1.0f / float(effectContext.h));

		int localFrame = effectContext.localFrame; 
		
		if (0 == localFrame || (localFrame != mLastLocalFrame))
		{
			// store modelview proj for the next frame
			effectContext.camera->GetCameraMatrix(mLastModelViewProj, kFBModelViewProj);

			if (mLoc.dt >= 0)
			{
				float dt = static_cast<float>(effectContext.localTimeDT);
				dt *= 0.01f * (float)_amount;
				glUniform1f(mLoc.dt, dt);
			}

			mLastLocalFrame = effectContext.localFrame;
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
