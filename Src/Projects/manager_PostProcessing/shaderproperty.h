#pragma once

/**	\file	shaderproperty.h

Sergei <Neill3d> Solokhin 2025-2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- SDK include
#include <fbsdk/fbsdk.h>

#include <GL/glew.h>
#include "shaderproperty_value.h"
#include <array>
#include <string_view>
#include <bitset>

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
class PostEffectContextProxy;

struct ShaderPropertyProxy
{
	int32_t index{ -1 }; // index in the property array (in case array is not changed or sorted)
	uint32_t nameHash{ 0 }; // hash key of a name to double check that we reference to a correct property
};

// represents a single shader property, its type, name, value, etc.
struct MANAGER_POSTPROCESSING_API ShaderProperty
{
	constexpr static int MAX_NAME_LENGTH{ 64 };
	constexpr static int HASH_SEED{ 123 };

	ShaderProperty() = default;
	ShaderProperty(const ShaderProperty& other);

	// constructor to associate property with fbProperty, recognize the type
	ShaderProperty(std::string_view nameIn, std::string_view uniformNameIn, FBProperty* fbPropertyIn = nullptr);
	ShaderProperty(std::string_view nameIn, std::string_view uniformNameIn, EPropertyType typeIn, FBProperty* fbPropertyIn = nullptr);

	void SetGeneratedByUniform(bool isGenerated) { isGeneratedByUniform = isGenerated; }
	bool IsGeneratedByUniform() const { return isGeneratedByUniform; }

	// get proxy of this property for a quick access from a property scheme
	ShaderPropertyProxy GetProxy() const
	{
		return { indexInArray, GetNameHash() };
	}

	void SetName(std::string_view nameIN);
	void SetNameHash(uint32_t nameHashIn);
	inline const char* GetName() const noexcept;
	inline uint32_t GetNameHash() const;
	void SetUniformName(std::string_view uniformNameIN);
	void SetUniformNameHash(uint32_t hashIn);
	inline const char* GetUniformName() const noexcept;
	inline uint32_t GetUniformNameHash() const;
	
	void SetLocation(GLint locationIN) {
		mDefaultValue.SetLocation(locationIN);
	}
	inline GLint GetLocation() const { return mDefaultValue.GetLocation(); }

public:
	//
	// method to chain calls

	ShaderProperty& SetType(EPropertyType newType);
	inline EPropertyType GetType() const { return mDefaultValue.GetType();  }
	ShaderProperty& SetFlag(PropertyFlag testFlag, bool setValue=true);
	bool HasFlag(PropertyFlag testFlag) const;

	// toggle a check if glsl location is found
	ShaderProperty& SetRequired(bool isRequired);

	ShaderProperty& SetScale(float scaleIn);
	float GetScale() const;
		
	ShaderProperty& SetDefaultValue(int valueIn);
	ShaderProperty& SetDefaultValue(bool valueIn);
	ShaderProperty& SetDefaultValue(float valueIn);
	ShaderProperty& SetDefaultValue(double valueIn);

	ShaderProperty& SetDefaultValue(float x, float y);
	ShaderProperty& SetDefaultValue(float x, float y, float z);
	ShaderProperty& SetDefaultValue(float x, float y, float z, float w);
		
public:
	inline void SetFBProperty(FBProperty* fbPropertyIN) { fbProperty = fbPropertyIN; }
	inline FBProperty* GetFBProperty() const { return fbProperty; }

	inline void SetFBComponent(FBComponent* fbComponentIN) { fbComponent = fbComponentIN; }
	inline FBComponent* GetFBComponent() const { return fbComponent; }

	const float* GetDefaultFloatData() const;
	ShaderPropertyValue& GetDefaultValue() { return mDefaultValue; }
	const ShaderPropertyValue& GetDefaultValue() const { return mDefaultValue; }

	static void ReadFBPropertyValue(
		FBProperty* fbProperty,
		ShaderPropertyValue& value, 
		const ShaderProperty& shaderProperty, 
		const PostEffectContextProxy* effectContext,
		int maskIndex);
		
	// when shader property comes from FBPropertyListObject
	//  we read first object in the list and can have either texture of shader user object type from it
	static void ReadTextureConnections(ShaderPropertyValue& value, FBProperty* fbProperty);

	void SetIndexInArray(int32_t indexInArrayIn) { indexInArray = indexInArrayIn; }
	int32_t GetIndexInArray() const { return indexInArray; }

public:

	static FBPropertyType ShaderPropertyToFBPropertyType(const ShaderProperty& prop);
	static EPropertyType FBPropertyToShaderPropertyType(const FBPropertyType& fbType);

private:
		
	ShaderPropertyValue mDefaultValue;
		
	uint32_t nameHash{ 0 };
	uint32_t uniformNameHash{ 0 };

	int32_t indexInArray{ -1 }; // for a quick access in the property array

	std::bitset<PROPERTY_BITSET_SIZE> flags;

	bool isGeneratedByUniform{ false };

	// TODO: move that into an evaluator cache (per effect shader)
	FBProperty* fbProperty{ nullptr };
	FBComponent* fbComponent{ nullptr }; // the owner of the property

	// calculate a hash and add it into a hash server
	uint32_t ComputeNameHash(std::string_view s) const;
};