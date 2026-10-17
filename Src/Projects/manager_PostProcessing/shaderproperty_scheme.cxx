/**	\file	shaderproperty_scheme.cxx

Sergei <Neill3d> Solokhin 2025-2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "shaderproperty_scheme.h"
#include "hashUtils.h"
#include "mobu_logging.h"
#include <nlohmann/json.hpp>
#include <magic_enum.hpp>

#include <fstream>
#include <iterator>

namespace _ShaderPropertySchemeInternal
{
	UniformBaseType ToUniformBaseType(GLenum type)
	{
		switch (type)
		{
		case GL_FLOAT: return UniformBaseType::Float;
		case GL_BOOL: return UniformBaseType::Bool;
		case GL_INT: return UniformBaseType::Int;
		case GL_FLOAT_VEC2: return UniformBaseType::Vec2;
		case GL_FLOAT_VEC3: return UniformBaseType::Vec3;
		case GL_FLOAT_VEC4: return UniformBaseType::Vec4;
		case GL_SAMPLER_2D: return UniformBaseType::Sampler2D;
		default: return UniformBaseType::Other;
		}
	}

	// apply naming rules (see ParseUniformName) to a property made from a uniform
	void SetNameAndFlagsFromUniformInfo(ShaderProperty& prop, const UniformNameInfo& info)
	{
		prop.SetFlag(PropertyFlag::IsFlag, info.isFlag);
		prop.SetFlag(PropertyFlag::IsClamped100, info.isClamped100);
		prop.SetFlag(PropertyFlag::IsColor, info.isColor);
		prop.SetFlag(PropertyFlag::ConvertWorldToScreenSpace, info.isWorldToScreen);
		prop.SetFlag(PropertyFlag::IsObjectSlot, info.isObjectSlot);

		if (info.isObjectCompanion)
		{
			// no own FBProperty, the value is written from the object slot while collecting values
			prop.SetFlag(PropertyFlag::IsObjectCompanion, true);
			prop.SetFlag(PropertyFlag::SKIP, true);
			prop.SetObjectCompanion(info.companionField,
				xxhash32(info.slotPropertyName.data(), info.slotPropertyName.size(), ShaderProperty::HASH_SEED));
		}

		prop.SetName(info.propertyName);
	}
};

///////////////////////////////////////////////////////////////////////////
// System Uniforms

void ShaderPropertyScheme::ResetSystemUniformLocations()
{
	systemUniformLocations.fill(-1);
}

namespace _ShaderPropertySchemeInternal
{
	// NOTE: the order must match the ShaderSystemUniform enum
	constexpr SystemUniformDesc gSystemUniforms[] =
	{
		{ "inputSampler", GL_SAMPLER_2D }, //!< this is an input image that we read from
		{ "iChannel0", GL_SAMPLER_2D }, //!< this is an input image, compatible with shadertoy
		{ "depthSampler", GL_SAMPLER_2D }, //!< this is a scene depth texture sampler in case shader will need it for processing
		{ "linearDepthSampler", GL_SAMPLER_2D },
		{ "maskSampler", GL_SAMPLER_2D }, //!< binded mask for a shader processing (system run-time texture)
		{ "normalSampler", GL_SAMPLER_2D }, //!< binded World-space normals texture (system run-time texture)

		{ "useMasking", GL_FLOAT }, //!< float uniform [0; 1] to define if the mask have to be used
		{ "upperClip", GL_FLOAT }, //!< this is an upper clip image level. defined in a texture coord space to skip processing
		{ "lowerClip", GL_FLOAT }, //!< this is a lower clip image level. defined in a texture coord space to skip processing

		{ "gResolution", GL_FLOAT_VEC2 }, //!< vec2 that contains processing absolute resolution, like 1920x1080
		{ "iResolution", GL_FLOAT_VEC2 }, //!< vec2 image absolute resolution, compatible with shadertoy naming
		{ "uInvResolution", GL_FLOAT_VEC2 }, //!< inverse resolution
		{ "texelSize", GL_FLOAT_VEC2 }, //!< vec2 of a texel size, computed as 1/resolution

		{ "iTime", GL_FLOAT }, //!< compatible with shadertoy, float, shader playback time (in seconds)
		{ "iDate", GL_FLOAT_VEC4 },  //!< compatible with shadertoy, vec4, (year, month, day, time in seconds)

		{ "cameraPosition", GL_FLOAT_VEC3 }, //!< world space camera position
		{ "modelView", GL_FLOAT_MAT4 }, //!< current camera modelview matrix
		{ "projection", GL_FLOAT_MAT4 }, //!< current camera projection matrix
		{ "modelViewProj", GL_FLOAT_MAT4 }, //!< current camera modelview-projection matrix
		{ "invModelViewProj", GL_FLOAT_MAT4 },
		{ "prevModelViewProj", GL_FLOAT_MAT4 },

		{ "zNear", GL_FLOAT }, //!< camera near plane
		{ "zFar", GL_FLOAT },	//!< camera far plane

		{ "cameraFocusDistance", GL_FLOAT }, //!< camera focus distance in scene units
		{ "cameraFocusAngle", GL_FLOAT }, //!< camera focus angle in degrees
		{ "cameraFocalLength", GL_FLOAT }, //!< camera focal length in mm
		{ "cameraFov", GL_FLOAT }, //!< camera FieldOfView property in degrees
		{ "cameraFocalLengthPx", GL_FLOAT }, //!< focal length in pixels of the view height
		{ "cameraIsOrtho", GL_FLOAT } //!< 1 for an orthogonal camera
	};

	static_assert(std::size(gSystemUniforms) == static_cast<size_t>(ShaderSystemUniform::COUNT),
		"gSystemUniforms must have an entry for every ShaderSystemUniform");
};

const SystemUniformDesc& ShaderPropertyScheme::GetSystemUniformDesc(ShaderSystemUniform u) noexcept
{
	return _ShaderPropertySchemeInternal::gSystemUniforms[static_cast<int>(u)];
}

bool ShaderPropertyScheme::IsSystemUniformTypeCompatible(ShaderSystemUniform u, GLenum type) noexcept
{
	const GLenum expectedType = GetSystemUniformDesc(u).type;
	return (type == expectedType)
		|| (expectedType == GL_FLOAT && type == GL_BOOL);
}


//////////////////////////////////////////////////////////////////////////
// PropertyScheme

// NOTE: a duplicated property name is a data problem (for instance, user shader declares both "glow" and "glow_slider"
//  uniforms which end up with the same property name), we must not abort the application because of that.
//  In such case the existing property is returned and the duplicate is dropped.

ShaderProperty& ShaderPropertyScheme::AddProperty(const ShaderProperty& property)
{
	const uint32_t nameHash = property.GetNameHash();
	VERIFY(nameHash != 0);

	if (ShaderProperty* existing = FindPropertyByHash(nameHash))
	{
		LOGE("[ShaderPropertyScheme] skip a duplicated property %s\n", property.GetName());
		AddWarning();
		return *existing;
	}

	auto& newProp = properties.emplace_back(property);
	newProp.SetIndexInArray(static_cast<int>(properties.size()) - 1);
	return newProp;
}

ShaderProperty& ShaderPropertyScheme::AddProperty(ShaderProperty&& property)
{
	const uint32_t nameHash = property.GetNameHash();
	VERIFY(nameHash != 0);

	if (ShaderProperty* existing = FindPropertyByHash(nameHash))
	{
		LOGE("[ShaderPropertyScheme] skip a duplicated property %s\n", property.GetName());
		AddWarning();
		return *existing;
	}

	auto& newProp = properties.emplace_back(std::move(property));
	newProp.SetIndexInArray(static_cast<int>(properties.size()) - 1);
	return newProp;
}

ShaderProperty& ShaderPropertyScheme::AddProperty(std::string_view nameIn, std::string_view uniformNameIn, FBProperty* fbPropertyIn)
{
	return AddProperty(ShaderProperty(nameIn, uniformNameIn, fbPropertyIn));
}

ShaderProperty& ShaderPropertyScheme::AddProperty(std::string_view nameIn, std::string_view uniformNameIn, EPropertyType typeIn, FBProperty* fbPropertyIn)
{
	return AddProperty(ShaderProperty(nameIn, uniformNameIn, typeIn, fbPropertyIn));
}

ShaderProperty* ShaderPropertyScheme::FindPropertyByHash(uint32_t nameHash)
{
	auto iter = std::find_if(begin(properties), end(properties), [nameHash](ShaderProperty& prop) {
		return prop.GetNameHash() == nameHash;
		});

	return (iter != end(properties)) ? std::addressof(*iter) : nullptr;
}

const ShaderProperty* ShaderPropertyScheme::FindPropertyByHash(uint32_t nameHash) const
{
	auto iter = std::find_if(begin(properties), end(properties), [nameHash](const ShaderProperty& prop) {
		return prop.GetNameHash() == nameHash;
		});

	return (iter != end(properties)) ? std::addressof(*iter) : nullptr;
}

ShaderProperty* ShaderPropertyScheme::FindProperty(const std::string_view name)
{
	const uint32_t nameHash = xxhash32(name.data(), name.size(), ShaderProperty::HASH_SEED);
	return FindPropertyByHash(nameHash);
}

const ShaderProperty* ShaderPropertyScheme::FindProperty(const std::string_view name) const
{
	const uint32_t nameHash = xxhash32(name.data(), name.size(), ShaderProperty::HASH_SEED);
	return FindPropertyByHash(nameHash);
}

ShaderProperty* ShaderPropertyScheme::FindPropertyByUniform(const std::string_view name)
{
	const uint32_t nameHash = xxhash32(name.data(), name.size(), ShaderProperty::HASH_SEED);

	auto iter = std::find_if(begin(properties), end(properties), [nameHash](ShaderProperty& prop) {
		return prop.GetUniformNameHash() == nameHash;
		});

	return (iter != end(properties)) ? std::addressof(*iter) : nullptr;
}

const ShaderProperty* ShaderPropertyScheme::FindPropertyByUniform(const std::string_view name) const
{
	const uint32_t nameHash = xxhash32(name.data(), name.size(), ShaderProperty::HASH_SEED);

	auto iter = std::find_if(begin(properties), end(properties), [nameHash](const ShaderProperty& prop) {
		return prop.GetUniformNameHash() == nameHash;
		});

	return (iter != end(properties)) ? std::addressof(*iter) : nullptr;
}

ShaderProperty* ShaderPropertyScheme::GetProperty(const ShaderPropertyProxy proxy)
{
	if (proxy.index >= 0
		&& proxy.index < static_cast<int>(properties.size())
		&& properties[proxy.index].GetNameHash() == proxy.nameHash)
	{
		return &properties[proxy.index];
	}

	return FindPropertyByHash(proxy.nameHash);
}

const ShaderProperty* ShaderPropertyScheme::GetProperty(const ShaderPropertyProxy proxy) const
{
	if (proxy.index >= 0
		&& proxy.index < static_cast<int>(properties.size())
		&& properties[proxy.index].GetNameHash() == proxy.nameHash)
	{
		return &properties[proxy.index];
	}

	return FindPropertyByHash(proxy.nameHash);
}

void ShaderPropertyScheme::AssociateFBProperties(FBComponent* component)
{
	if (!component)
		return;

	for (auto& prop : properties)
	{
		FBProperty* fbProp = component->PropertyList.Find(prop.GetName());

		// a property that is generated from a shader uniform must have exactly the type that we expect,
		//  otherwise we are looking at some other (outdated or system) property with the same name
		if (fbProp && prop.IsGeneratedByUniform()
			&& fbProp->GetPropertyType() != ShaderProperty::ShaderPropertyToFBPropertyType(prop))
		{
			LOGE("[ShaderPropertyScheme] property %s has an unexpected type %s, skip association\n",
				prop.GetName(), fbProp->GetPropertyTypeName());
			AddWarning();
			fbProp = nullptr;
			prop.SetFBProperty(nullptr);
		}

		if (!fbProp && prop.IsGeneratedByUniform())
		{
			LOGE("[ShaderPropertyScheme] failed to find a property for uniform %s\n", prop.GetName());
			AddWarning();
		}

		if (fbProp)
		{
			prop.SetFBProperty(fbProp);
			prop.SetFBComponent(component);
		}
	}
}

bool ShaderPropertyScheme::ExportToJSON(const std::filesystem::path& filePath) const
{
	// for convenience
	using json = nlohmann::json;

	json root = json::object();
	root["properties"] = json::array();

	for (const auto& prop : properties)
	{
		json item = json::object();

		// Basic identifiers
		item["name"] = prop.GetName();
		item["nameHash"] = prop.GetNameHash();
		item["uniformName"] = prop.GetUniformName();
		item["uniformHash"] = prop.GetUniformNameHash();

		// flags
		json flagsItem = json::array();
		for (auto e : magic_enum::enum_values<PropertyFlag>()) {
			if (prop.HasFlag(e))
			{
				flagsItem.push_back(magic_enum::enum_name(e));
			}
		}
		item["flags"] = std::move(flagsItem);

		// Default float data (4 floats)
		json valueItem = json::object();

		const float* df = prop.GetDefaultFloatData();
		json defArr = json::array();
		for (int i = 0; i < 4; ++i)
		{
			float v = (df != nullptr) ? df[i] : 0.0f;
			defArr.push_back(v);
		}
		valueItem["type"] = magic_enum::enum_name(prop.GetDefaultValue().GetType()).data();
		valueItem["isLocationRequired"] = prop.GetDefaultValue().IsRequired();
		valueItem["location"] = prop.GetDefaultValue().GetLocation();
		valueItem["nameHash"] = prop.GetDefaultValue().GetNameHash();
		valueItem["defaultFloat"] = std::move(defArr);

		item["defaultValue"] = std::move(valueItem);

		// Scale
		item["scale"] = prop.GetScale();

		item["index"] = prop.GetIndexInArray();

		item["isGeneratedByUniform"] = prop.IsGeneratedByUniform();

		item["fbComponent"] = prop.GetFBComponent() ? prop.GetFBComponent()->GetFullName() : "Empty";
		item["fbProperty"] = prop.GetFBProperty() ? prop.GetFBProperty()->GetName() : "Empty";
		item["fbPropertyType"] = prop.GetFBProperty() ? prop.GetFBProperty()->GetPropertyTypeName() : "Empty";
		
		root["properties"].push_back(std::move(item));
	}

	// Serialize and write to file

	std::ofstream output(filePath, std::ios::out | std::ios::trunc);

	if (!output.is_open())
		return false;

	output << root.dump(4);
	return output.good();
}


int ShaderPropertyScheme::ReflectUniforms(const GLuint programId, bool doPopulatePropertiesFromUniforms)
{
	using namespace _ShaderPropertySchemeInternal;

	ResetSystemUniformLocations();
	
	if (!programId)
		return 0;

	GLint count = 0;
	glGetProgramiv(programId, GL_ACTIVE_UNIFORMS, &count);

	GLint maxNameLen = 0;
	glGetProgramiv(programId, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxNameLen);

	std::vector<char> name(maxNameLen);

	int added = 0;

	for (int i = 0; i < count; i++)
	{
		GLsizei length;
		GLint size;
		GLenum type;

		glGetActiveUniform(programId, i, maxNameLen, &length, &size, &type, name.data());
		const char* uniformName = name.data();

		// Skip GLSL internal
		if (IsInternalGLSLUniform(uniformName))
			continue;

		const GLint location = glGetUniformLocation(programId, name.data());
		if (location < 0)
		{
			// members of a uniform block don't have a location, we can't set them as separate uniforms
			LOGE("[ShaderPropertyScheme] uniform %s has no location, skip it\n", uniformName);
			AddWarning();
			continue;
		}

		// an array is reported as "name[0]" (an array of structs as "name[0].member"),
		//  a property drives a single value, so arrays are not supported
		if (size > 1 || std::strchr(uniformName, '[') != nullptr)
		{
			LOGE("[ShaderPropertyScheme] uniform %s is an array, arrays are not supported, skip it\n", uniformName);
			AddWarning();
			continue;
		}

		// System uniform?
		const int sysId = FindSystemUniform(uniformName);
		const bool isSystemUniform = (sysId >= 0);
		if (isSystemUniform)
		{
			const ShaderSystemUniform systemUniform = static_cast<ShaderSystemUniform>(sysId);
			if (IsSystemUniformTypeCompatible(systemUniform, type))
			{
				SetSystemUniformLoc(systemUniform, location);
			}
			else
			{
				// a system value can't be uploaded into a uniform of another type, leave the uniform as is
				LOGE("[ShaderPropertyScheme] system uniform %s has an unexpected type, skip it\n", uniformName);
				AddWarning();
				continue;
			}
		}

		const auto shaderType = UniformTypeToShaderPropertyType(type);

		// a value of a property can't be uploaded into a uniform of an unsupported type (samplerCube, ivec, mat3, etc.)
		//  NOTE: system uniforms have their own type check and upload (like mat4 matrices)
		if (shaderType == EPropertyType::NONE && !isSystemUniform)
		{
			LOGE("[ShaderPropertyScheme] uniform %s has an unsupported type 0x%x, skip it\n", uniformName, type);
			AddWarning();
			continue;
		}

		// 3) Create ShaderProperty
		if (auto prop = FindPropertyByUniform(uniformName))
		{
			// already exists, update location
			const bool isTypeMatched = (prop->GetType() == shaderType)
				|| (prop->GetType() == EPropertyType::BOOL && shaderType == EPropertyType::FLOAT);
			if (!isTypeMatched)
			{
				// a value of the property can't be uploaded into the uniform (GL rejects it), keep the property without a location
				LOGE("[ShaderPropertyScheme] uniform %s has a type that differs from the registered property, skip it\n", uniformName);
				AddWarning();
				continue;
			}
			prop->SetLocation(location);
		}
		else if (!isSystemUniform)
		{
			//VERIFY(DoPopulatePropertiesFromUniforms());

			if (doPopulatePropertiesFromUniforms)
			{
				// from a uniform name, let's extract special postfix and convert it into a flag bit, prepare a clean property name
				const UniformNameInfo nameInfo = ParseUniformName(uniformName, ToUniformBaseType(type));

				if (nameInfo.isInvalidCompanion)
				{
					LOGE("[ShaderPropertyScheme] object slot uniform %s has an unexpected type, skip it\n", uniformName);
					AddWarning();
					continue;
				}

				ShaderProperty newProp;

				// NOTE: a companion of an object slot has no own FBProperty, it is not a property for a user
				newProp.SetGeneratedByUniform(!nameInfo.isObjectCompanion);
				newProp.SetUniformName(uniformName);
				newProp.SetLocation(location);
				newProp.SetType(shaderType);

				SetNameAndFlagsFromUniformInfo(newProp, nameInfo);

				// different uniforms could end up with the same property name after a postfix is removed
				//  (like "glow" and "glow_slider")
				if (FindPropertyByHash(newProp.GetNameHash()))
				{
					LOGE("[ShaderPropertyScheme] uniform %s has the same property name as another uniform, skip it\n", uniformName);
					AddWarning();
					continue;
				}

				AddProperty(std::move(newProp));
				added++;
			}
		}
	}

	// companions of an object slot need the slot property, even if the slot uniform itself is not declared
	//  or stripped by the compiler as unused (a shader could use only <name>_object_screen, for example)
	if (doPopulatePropertiesFromUniforms)
	{
		AddImplicitObjectSlots();
	}

	// registered properties that are expected to drive a uniform, report once here (values without a location are skipped on upload)
	for (const ShaderProperty& prop : properties)
	{
		if (!prop.IsGeneratedByUniform() && prop.GetDefaultValue().IsRequired() && prop.GetLocation() < 0)
		{
			LOGE("[ShaderPropertyScheme] uniform %s of a property %s is not found in the shader\n", prop.GetUniformName(), prop.GetName());
			AddWarning();
		}
	}

	return added;
}

void ShaderPropertyScheme::AddImplicitObjectSlots()
{
	// NOTE: names are collected first, adding a property while iterating would invalidate the iteration
	std::vector<std::string> missingSlots;

	for (const ShaderProperty& prop : properties)
	{
		if (!prop.HasFlag(PropertyFlag::IsObjectCompanion))
			continue;

		const ShaderProperty* slot = FindPropertyByHash(prop.GetObjectSlotNameHash());
		if (slot && slot->HasFlag(PropertyFlag::IsObjectSlot))
			continue;

		if (slot)
		{
			LOGE("[ShaderPropertyScheme] uniform %s needs an object slot %s, but a property with that name is not an object slot\n",
				prop.GetUniformName(), slot->GetName());
			AddWarning();
			continue;
		}

		// <slot>_object_<field>, the slot property name is the part before "_object_"
		const std::string_view uniformName(prop.GetUniformName());
		const size_t infixPos = uniformName.rfind("_object_");
		if (infixPos == std::string_view::npos)
			continue;

		std::string slotName(uniformName.substr(0, infixPos));
		if (std::find(begin(missingSlots), end(missingSlots), slotName) == end(missingSlots))
		{
			missingSlots.push_back(std::move(slotName));
		}
	}

	for (const std::string& slotName : missingSlots)
	{
		ShaderProperty slotProp;
		slotProp.SetGeneratedByUniform(true);
		slotProp.SetUniformName(slotName + "_object");
		slotProp.SetType(EPropertyType::VEC3);
		slotProp.SetFlag(PropertyFlag::IsObjectSlot, true);
		slotProp.SetName(slotName);
		// NOTE: no location, the slot uniform is not active, the property exists for the companion uniforms

		AddProperty(std::move(slotProp));
	}
}

EPropertyType ShaderPropertyScheme::UniformTypeToShaderPropertyType(GLenum type) const
{
	switch (type)
	{
	case GL_FLOAT:
		return EPropertyType::FLOAT;
	case GL_INT:
		return EPropertyType::INT;
	case GL_BOOL:
		return EPropertyType::FLOAT;
	case GL_FLOAT_VEC2:
		return EPropertyType::VEC2;
	case GL_FLOAT_VEC3:
		return EPropertyType::VEC3;
	case GL_FLOAT_VEC4:
		return EPropertyType::VEC4;
	case GL_SAMPLER_2D:
		return EPropertyType::TEXTURE;
	default:
		// NOTE: mat4 is not supported as a property yet (no FBProperty type to edit it and no upload),
		//  matrices come as system uniforms only
		return EPropertyType::NONE;
	}
}

int ShaderPropertyScheme::FindSystemUniform(const char* uniformName) const
{
	using namespace _ShaderPropertySchemeInternal;

	for (int i = 0; i < static_cast<int>(ShaderSystemUniform::COUNT); ++i)
	{
		if (std::strcmp(uniformName, gSystemUniforms[i].name) == 0)
			return i;
	}
	return -1;
}

bool ShaderPropertyScheme::IsInternalGLSLUniform(const char* uniformName) const
{
	return std::strncmp(uniformName, "gl_", 3) == 0;
}
