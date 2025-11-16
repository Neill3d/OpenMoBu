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
	INV_RESOLUTION, //!< inverse resolution
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

	// a generic value holder for different property types
	struct MANAGER_POSTPROCESSING_API ShaderPropertyValue
	{
		ShaderPropertyValue() : value(std::array<float, 1>{ 0.0f }) {}

		// Type-safe dynamic storage for float values
		std::variant<std::array<float, 1>, std::array<float, 2>, std::array<float, 3>, std::array<float, 4>, std::vector<float>> value;

		uint32_t	key=0; //!< unique key to identify property

		EPropertyType type{ EPropertyType::FLOAT };

		// extracted value from reference object property
		FBTexture* texture{ nullptr };
		EffectShaderUserObject* shaderUserObject{ nullptr };

		// change type and apply a default value according to a given type
		void SetType(IEffectShaderConnections::EPropertyType newType);

		inline void SetValue(int valueIn) {
			value = std::array<float, 1>{ static_cast<float>(valueIn) };
		}
		inline void SetValue(bool valueIn) {
			value = std::array<float, 1>{ valueIn ? 1.0f : 0.0f };
		}
		inline void SetValue(float valueIn) {
			value = std::array<float, 1>{ valueIn };
		}
		inline void SetValue(double valueIn) {
			value = std::array<float, 1>{ static_cast<float>(valueIn) };
		}

		inline void SetValue(float x, float y) {
			value = std::array<float, 2>{ x, y };
		}
		inline void SetValue(float x, float y, float z) {
			value = std::array<float, 3>{ x, y, z };
		}
		inline void SetValue(float x, float y, float z, float w) {
			value = std::array<float, 4>{ x, y, z, w };
		}

		inline const float* GetFloatData() const {
			return std::visit([](auto&& arg) -> const float* {
				return arg.data();
				}, value);
		}

		void ReadTextureConnections(FBProperty* fbProperty);
	};

	// represents a single shader property, its type, name, value, etc.
	struct MANAGER_POSTPROCESSING_API ShaderProperty
	{
		constexpr static int MAX_NAME_LENGTH{ 64 };
		constexpr static int HASH_SEED{ 123 };

		char name[MAX_NAME_LENGTH]{ 0 };
		char uniformName[MAX_NAME_LENGTH]{ 0 };
		int length{ 0 };

		uint32_t key{ 0 };

		EPropertyType type{ EPropertyType::FLOAT };

		bool bIsLocationRequired{ true }; //!< should we treat missing location as an error or not
		GLint location{ -1 }; //!< GLSL shader location holder
		
		std::bitset<PROPERTY_BITSET_SIZE> flags;
		
		FBProperty* fbProperty{ nullptr };

		// pre-cache evaluated value
		ShaderPropertyValue value;

		// modify the output value that we upload on gpu
		float scale{ 1.0f };

		ShaderProperty() = default;

		// constructor to associate property with fbProperty, recognize the type
		ShaderProperty(const char* nameIn, const char* uniformNameIn, FBProperty* fbPropertyIn = nullptr);
		ShaderProperty(const char* nameIn, const char* uniformNameIn, IEffectShaderConnections::EPropertyType typeIn, FBProperty* fbPropertyIn = nullptr);

		ShaderProperty& SetType(IEffectShaderConnections::EPropertyType newType);
		ShaderProperty& SetFlag(PropertyFlag testFlag, bool setValue);

		ShaderProperty& SetRequired(bool isRequired);

		ShaderProperty& SetScale(float scaleIn);
		float GetScale() const;

		ShaderProperty& SetValue(int valueIn);
		ShaderProperty& SetValue(bool valueIn);
		ShaderProperty& SetValue(float valueIn);
		ShaderProperty& SetValue(double valueIn);

		ShaderProperty& SetValue(float x, float y);
		ShaderProperty& SetValue(float x, float y, float z);
		ShaderProperty& SetValue(float x, float y, float z, float w);

		const float* GetFloatData() const;

		bool HasFlag(PropertyFlag testFlag) const;

		static void ReadFBPropertyValue(ShaderPropertyValue& value, FBProperty* fbProperty, 
			const ShaderProperty& shaderProperty, const IPostEffectContext* effectContext, int maskIndex);
		
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


