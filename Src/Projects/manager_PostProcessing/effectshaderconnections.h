#pragma once

#include <GL/glew.h>
#include "posteffectcontext.h"

#include <array>
#include <bitset>
#include <variant>
#include <vector>

// DLL export macro for Windows
#ifndef MANAGER_POSTPROCESSING_API
# if defined(_WIN32) || defined(_WIN64)
#  if defined(manager_PostProcessing_EXPORTS)
#   define MANAGER_POSTPROCESSING_API __declspec(dllexport)
#  else
#   define MANAGER_POSTPROCESSING_API __declspec(dllimport)
#  endif
# else
#  define MANAGER_POSTPROCESSING_API
# endif
#endif

// forward
class EffectShaderUserObject;

enum class ShaderSystemUniform : uint8_t
{
	INPUT_COLOR_SAMPLER_2D, //!< this is an input image that we read from
	iCHANNEL0, //!< this is an input image, compatible with shadertoy
	INPUT_DEPTH_SAMPLER_2D, //!< this is a scene depth texture sampler in case shader will need it for processing
	LINEAR_DEPTH_SAMPLER_2D, //!< a depth texture converted into linear space (used in SSAO)
	INPUT_MASK_SAMPLER_2D, //!< binded mask for a shader processing
	WORLD_NORMAL_SAMPLER_2D,

	USE_MASKING, //!< float uniform [0; 1] to define if the mask have to be used
	UPPER_CLIP, //!< this is an upper clip image level. defined in a texture coord space to skip processing
	LOWER_CLIP, //!< this is a lower clip image level. defined in a texture coord space to skip processing

	RESOLUTION, //!< vec2 that contains processing absolute resolution, like 1920x1080
	iRESOLUTION, //!< vec2 absolute resolution, compatible with shadertoy
	TEXEL_SIZE, //!< vec2 of a texel size, computed as 1/resolution

	iTIME, //!< compatible with shadertoy, float, shader playback time (in seconds)
	iDATE, //!< compatible with shadertoy, vec4, (year, month, day, time in seconds)

	CAMERA_POSITION, //!< world space camera position
	MODELVIEW,	//!< current camera modelview matrix
	PROJ,		//!< current camera projection matrix
	MODELVIEWPROJ,	//!< current camera modelview-projection matrix

	INV_MODELVIEWPROJ, // inverse of modelview-projection matrix
	PREV_MODELVIEWPROJ, // modelview-projection matrix from a previous frame

	ZNEAR, //!< camera near plane
	ZFAR,	//!< camera far plane

	COUNT
};

constexpr size_t PROPERTY_BITSET_SIZE = 8;

class IEffectShaderConnections
{
public:

	enum class EPropertyType : uint8_t
	{
		INT,
		BOOL,
		FLOAT,
		VEC2,
		VEC3,
		VEC4,
		MAT4,
		TEXTURE
	};

	enum class PropertyFlag : uint8_t
	{
		SYSTEM = 1,	// flag that the property is a system one, like masking, upper/lower clip, etc.
		IsClamped100 = 1,
		IsClamped1 = 2,
		IsFlag = 3,
		IsColor = 4,
		ConvertWorldToScreenSpace = 5,
		ShouldSkip = 6, //!< this is for manual processing of property (like manual reading and setting value)
		INVERT_VALUE = 7 //!< a given property value is going to be written as 1.0 - value to the uniform
	};

	class IUserData {
	public:
		virtual ~IUserData() = default;
	};

	class FBPropertyUserData : public IUserData {
		FBProperty* property;

		// extracted value from reference object property
		FBTexture* texture;
		EffectShaderUserObject* shaderUserObject;
	};



	struct MANAGER_POSTPROCESSING_API ShaderProperty
	{
		constexpr static int MAX_NAME_LENGTH{ 64 };
		
		char name[MAX_NAME_LENGTH]{ 0 };
		char uniformName[MAX_NAME_LENGTH]{ 0 };
		int length{ 0 };

		EPropertyType type{ EPropertyType::FLOAT };

		std::bitset<PROPERTY_BITSET_SIZE> flags;

		GLint location{ -1 }; //!< GLSL shader location holder

		//std::unique_ptr<IUserData> userData;

		FBProperty* fbProperty{ nullptr };

		// extracted value from reference object property
		FBTexture* texture{ nullptr };
		EffectShaderUserObject* shaderUserObject{ nullptr };

		// Type-safe dynamic storage for float values
		std::variant<std::array<float, 1>, std::array<float, 2>, std::array<float, 3>, std::array<float, 4>, std::vector<float>> value;

		float scale{ 1.0f };

		ShaderProperty() : value(std::array<float, 1>{ 0.0f }) {}

		// constructor to associate property with fbProperty, recognize the type
		ShaderProperty(const char* nameIn, const char* uniformNameIn, FBProperty* fbPropertyIn = nullptr);
		ShaderProperty(const char* nameIn, const char* uniformNameIn, IEffectShaderConnections::EPropertyType typeIn, FBProperty* fbPropertyIn = nullptr);

		ShaderProperty& SetType(IEffectShaderConnections::EPropertyType newType);
		ShaderProperty& SetFlag(PropertyFlag testFlag, bool setValue);

		ShaderProperty& SetScale(float scaleIn);
		float GetScale() const;

		ShaderProperty& SetValue(int valueIn);
		ShaderProperty& SetValue(float valueIn);
		ShaderProperty& SetValue(double valueIn);

		ShaderProperty& SetValue(float x, float y);
		ShaderProperty& SetValue(float x, float y, float z);
		ShaderProperty& SetValue(float x, float y, float z, float w);


		float* GetFloatData();

		bool HasFlag(PropertyFlag testFlag) const;

		void ReadFBPropertyValue(const IPostEffectContext* effectContext, int maskIndex);
		void ReadTextureConnections();
	};

	virtual ~IEffectShaderConnections() = default;

	virtual ShaderProperty& AddProperty(const ShaderProperty& property) = 0;
	virtual ShaderProperty& AddProperty(ShaderProperty&& property) = 0;

	virtual int GetNumberOfProperties() = 0;
	virtual ShaderProperty& GetProperty(int index) = 0;

	virtual ShaderProperty* FindProperty(const std::string& name) = 0;

	//virtual int GetNumberOfOutputConnections() = 0;
	//virtual void GetOutputConnectionType() = 0;
	//virtual void GetOutputProperty(int index) = 0;

	// look for a UI interface, and read properties and its values
	// should it be a separate class
	virtual bool CollectUIValues(const IPostEffectContext* effectContext, int maskIndex) = 0;

	// use uniformName to track down some type casts
	static FBPropertyType ShaderPropertyToFBPropertyType(const ShaderProperty& prop);

	static EPropertyType FBPropertyToShaderPropertyType(const FBPropertyType& fbType);

	static EPropertyType UniformTypeToShaderPropertyType(GLenum type);
};

