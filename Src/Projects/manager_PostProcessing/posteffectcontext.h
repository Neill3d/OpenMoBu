#pragma once

//--- SDK include
#include <fbsdk/fbsdk.h>

// forward
class PostPersistentData;


class IPostEffectContext
{
public:
	virtual ~IPostEffectContext() = default;

	struct Parameters
	{
		int w{ 1 }; //!< viewport width
		int h{ 1 }; //!< viewport height
		int localFrame{ 0 }; //!< playback frame number

		double sysTime{ 0.0 }; //!< system time (in seconds)
		double sysTimeDT{ 0.0 };

		double localTime{ 0.0 }; //!< playback time (in seconds)
		double localTimeDT{ 0.0 };

		FBMatrix prevModelViewProjMatrix; //!< modelview-projection matrix of the previous frame
	};

	// interface to query the needed data

	[[nodiscard]] int GetViewWidth() const noexcept { return parameters.w; }
	[[nodiscard]] int GetViewHeight() const noexcept { return parameters.h; }

	[[nodiscard]] int GetLocalFrame() const noexcept { return parameters.localFrame; }
	[[nodiscard]] double GetSystemTime() const noexcept { return parameters.sysTime; }
	[[nodiscard]] double GetLocalTime() const noexcept { return parameters.localTime; }

	[[nodiscard]] double GetLocalTimeDT() const noexcept { return parameters.localTimeDT; }
	[[nodiscard]] double GetSystemTimeDT() const noexcept { return parameters.sysTimeDT; }

	[[nodiscard]] virtual double* GetCameraPosition() const noexcept = 0;
	[[nodiscard]] virtual const float* GetCameraPositionF() const noexcept = 0;

	[[nodiscard]] virtual float GetCameraNearDistance() const noexcept = 0;
	[[nodiscard]] virtual float GetCameraFarDistance() const noexcept = 0;

	[[nodiscard]] virtual double* GetModelViewMatrix() const noexcept = 0;
	[[nodiscard]] virtual const float* GetModelViewMatrixF() const noexcept = 0;
	[[nodiscard]] virtual double* GetProjectionMatrix() const noexcept = 0;
	[[nodiscard]] virtual const float* GetProjectionMatrixF() const noexcept = 0;
	[[nodiscard]] virtual double* GetModelViewProjMatrix() const noexcept = 0;
	[[nodiscard]] virtual const float* GetModelViewProjMatrixF() const noexcept = 0;
	// returns the modelview-projection matrix of the previous frame
	[[nodiscard]] virtual const float* GetPrevModelViewProjMatrixF() const noexcept = 0;
	// returns the inverse of the modelview-projection matrix
	[[nodiscard]] virtual const float* GetInvModelViewProjMatrixF() const noexcept = 0;

	// 4 floats in format - year + 1900, month + 1, day, seconds since midnight
	[[nodiscard]] virtual const float* GetIDate() const noexcept = 0;

	[[nodiscard]] virtual FBCamera* GetCamera() const = 0;
	[[nodiscard]] virtual FBComponent* GetComponent() const = 0;
	[[nodiscard]] virtual PostPersistentData* GetPostProcessData() const = 0;

	virtual void OverrideComponent(FBComponent* component) const = 0;

protected:
	Parameters parameters;
};