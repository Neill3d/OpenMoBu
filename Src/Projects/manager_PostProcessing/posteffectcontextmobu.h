#pragma once

#include "posteffectcontext.h"

//--- SDK include
#include <fbsdk/fbsdk.h>


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

	virtual double* GetModelViewMatrix() const noexcept override { return modelView; }
	virtual double* GetProjectionMatrix() const noexcept override { return projection; }
	virtual double* GetModelViewProjMatrix() const noexcept override { return modelViewProj; }

	FBCamera* GetCamera() const { return camera; }
	FBComponent* GetComponent() const { return userObject; }
	PostPersistentData* GetPostProcessData() const { return postProcessData; }

	void OverrideComponent(FBComponent* component) const { userObject = component; }

private:

	// cache values

	FBMatrix	modelView;
	FBMatrix	projection;
	FBMatrix	modelViewProj;
	FBVector3d	cameraPosition;

	FBCamera* camera{ nullptr }; //!< current camera that we are drawing with
	mutable FBComponent* userObject{ nullptr }; //!< this is a component where all ui properties are exposed
	PostPersistentData* postProcessData{ nullptr }; //!< this is a main post process object for common effects properties


	void PrepareCache()
	{
		if (!camera)
			return;
		camera->GetVector(cameraPosition, kModelTranslation, true);
		camera->GetCameraMatrix(modelView, FBCameraMatrixType::kFBModelView);
		camera->GetCameraMatrix(projection, FBCameraMatrixType::kFBProjection);
		camera->GetCameraMatrix(modelViewProj, FBCameraMatrixType::kFBModelViewProj);
	}
};