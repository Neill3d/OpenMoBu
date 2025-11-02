#pragma once

#include "posteffectcontext.h"

//--- SDK include
#include <fbsdk/fbsdk.h>

#include <limits>
#include <ctime>

class PostEffectContextMoBu : public IPostEffectContext
{
public:

	PostEffectContextMoBu(FBCamera* cameraIn, FBComponent* userObjectIn, PostPersistentData* postProcessDataIn, const Parameters& parametersIn)
		: camera(cameraIn)
		, userObject(userObjectIn)
		, postProcessData(postProcessDataIn)
	{
		parameters = parametersIn;
		PrepareCache();
	}

	virtual double* GetCameraPosition() const noexcept override { return cameraPosition; }
	virtual const float* GetCameraPositionF() const noexcept override { return cameraPositionF; }

	virtual float GetCameraNearDistance() const noexcept override { return zNear; }
	virtual float GetCameraFarDistance() const noexcept override { return zFar; }

	virtual bool IsCameraOrthogonal() const noexcept override { return isCameraOrtho; }

	virtual double* GetModelViewMatrix() const noexcept override { return modelView; }
	virtual const float* GetModelViewMatrixF() const noexcept override { return modelViewF; }
	virtual double* GetProjectionMatrix() const noexcept override { return projection; }
	virtual const float* GetProjectionMatrixF() const noexcept override { return projectionF; }
	virtual double* GetModelViewProjMatrix() const noexcept override { return modelViewProj; }
	virtual const float* GetModelViewProjMatrixF() const noexcept override { return modelViewProjF; }
	// returns the modelview-projection matrix of the previous frame
	virtual const float* GetPrevModelViewProjMatrixF() const noexcept override { return prevModelViewProjF; }
	// returns the inverse of the modelview-projection matrix
	virtual const float* GetInvModelViewProjMatrixF() const noexcept override { return invModelViewProjF; }
	
	// 4 floats in format - year + 1900, month + 1, day, seconds since midnight
	virtual const float* GetIDate() const noexcept override { return iDate; }

	FBCamera* GetCamera() const { return camera; }
	FBComponent* GetComponent() const { return userObject; }
	PostPersistentData* GetPostProcessData() const { return postProcessData; }

	void OverrideComponent(FBComponent* component) const { userObject = component; }

private:

	// cache values

	FBMatrix	modelView;
	FBMatrix	projection;
	FBMatrix	modelViewProj;
	FBMatrix	invModelViewProj;
	FBMatrix	prevModelViewProj;
	FBVector3d	cameraPosition;

	float zNear;
	float zFar;

	float modelViewF[16];	
	float projectionF[16];
	float modelViewProjF[16];
	float invModelViewProjF[16];
	float prevModelViewProjF[16];
	float cameraPositionF[3];
	float iDate[4];

	FBCamera* camera{ nullptr }; //!< current camera that we are drawing with
	mutable FBComponent* userObject{ nullptr }; //!< this is a component where all ui properties are exposed
	PostPersistentData* postProcessData{ nullptr }; //!< this is a main post process object for common effects properties

	bool isCameraOrtho{ false };

	void PrepareCache()
	{
		if (!camera)
			return;

		zNear = static_cast<float>(camera->NearPlaneDistance);
		zFar = static_cast<float>(camera->FarPlaneDistance);
		
		isCameraOrtho = (camera->Type == FBCameraType::kFBCameraTypeOrthogonal);

		camera->GetVector(cameraPosition, kModelTranslation, true);
		for (int i = 0; i < 3; ++i)
			cameraPositionF[i] = static_cast<float>(cameraPosition[i]);

		camera->GetCameraMatrix(modelView, FBCameraMatrixType::kFBModelView);
		for (int i = 0; i < 16; ++i)
		{
			modelViewF[i] = static_cast<float>(modelView[i]);
		}
		
		camera->GetCameraMatrix(projection, FBCameraMatrixType::kFBProjection);
		for (int i = 0; i < 16; ++i)
		{
			projectionF[i] = static_cast<float>(projection[i]);
		}
		camera->GetCameraMatrix(modelViewProj, FBCameraMatrixType::kFBModelViewProj);
		for (int i = 0; i < 16; ++i)
		{
			modelViewProjF[i] = static_cast<float>(modelViewProj[i]);
		}
		FBMatrixInverse(invModelViewProj, modelViewProj);
		for (int i = 0; i < 16; ++i)
		{
			invModelViewProjF[i] = static_cast<float>(invModelViewProj[i]);
		}
		prevModelViewProj = parameters.prevModelViewProjMatrix;
		for (int i = 0; i < 16; ++i)
		{
			prevModelViewProjF[i] = static_cast<float>(prevModelViewProj[i]);
		}

		std::time_t now = std::time(nullptr);
		std::tm localTime;
		localtime_s(&localTime, &now);  // now should be of type std::time_t

		const float secondsSinceMidnight = static_cast<float>(localTime.tm_hour * 3600 + localTime.tm_min * 60 + localTime.tm_sec);
		iDate[0] = static_cast<float>(localTime.tm_year + 1900);
		iDate[1] = static_cast<float>(localTime.tm_mon + 1);
		iDate[2] = static_cast<float>(localTime.tm_mday);
		iDate[3] = secondsSinceMidnight;
	}
};