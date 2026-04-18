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

namespace _ShaderPropertySchemeInternal
{
	inline std::string_view RemovePostfix(std::string_view name, std::string_view postfix)
	{
		if (postfix.empty()) {
			return name;  // No postfix to remove
		}

		if (name.size() >= postfix.size() &&
			name.compare(name.size() - postfix.size(), postfix.size(), postfix) == 0)
		{
			return name.substr(0, name.size() - postfix.size());  // Remove postfix
		}

		return name;
	}

	// TABLE OF POSTFIX RULES
	struct Rule
	{
		GLenum glType;
		std::string_view postfix;
		PropertyFlag flag;
	};

	static constexpr Rule rules[] = {
			{ GL_FLOAT,       "_flag",  PropertyFlag::IsFlag },
			{ GL_FLOAT,		  "_slider", PropertyFlag::IsClamped100 },
			{ GL_FLOAT_VEC2,  "_wstoss", PropertyFlag::ConvertWorldToScreenSpace },
			{ GL_FLOAT_VEC3,  "_color", PropertyFlag::IsColor },
			{ GL_FLOAT_VEC4,  "_color", PropertyFlag::IsColor }
	};

	std::string_view GetNameFromUniformName(std::string_view name, GLenum type)
	{
		std::string_view finalName = name;

		for (const Rule& r : rules)
		{
			if (r.glType == type)
			{
				if (name.size() > r.postfix.size() &&
					name.compare(name.size() - r.postfix.size(), r.postfix.size(), r.postfix) == 0)
				{
					finalName = name.substr(0, name.size() - r.postfix.size());
					break;
				}
			}
		}

		return finalName;
	}

	void SetNameAndFlagsFromUniformNameAndType(ShaderProperty& prop, const char* uniformNameIn, GLenum type)
	{
		std::string_view name(uniformNameIn);

		// default: no change
		std::string_view finalName = name;

		for (const Rule& r : rules)
		{
			if (r.glType == type)
			{
				if (name.size() > r.postfix.size() &&
					name.compare(name.size() - r.postfix.size(), r.postfix.size(), r.postfix) == 0)
				{
					prop.SetFlag(r.flag, true);
					finalName = name.substr(0, name.size() - r.postfix.size());
					break;
				}
			}
		}

		prop.SetName(finalName);
	}
};

///////////////////////////////////////////////////////////////////////////
// System Uniforms

void ShaderPropertyScheme::ResetSystemUniformLocations()
{
	systemUniformLocations.fill(-1);
}

const char* ShaderPropertyScheme::gSystemUniformNames[static_cast<int>(ShaderSystemUniform::COUNT)] =
{
	"inputSampler", //!< this is an input image that we read from
	"iChannel0", //!< this is an input image, compatible with shadertoy
	"depthSampler", //!< this is a scene depth texture sampler in case shader will need it for processing
	"linearDepthSampler",
	"maskSampler", //!< binded mask for a shader processing (system run-time texture)
	"normalSampler", //!< binded World-space normals texture (system run-time texture)

	"useMasking", //!< float uniform [0; 1] to define if the mask have to be used
	"upperClip", //!< this is an upper clip image level. defined in a texture coord space to skip processing
	"lowerClip", //!< this is a lower clip image level. defined in a texture coord space to skip processing

	"gResolution", //!< vec2 that contains processing absolute resolution, like 1920x1080
	"iResolution", //!< vec2 image absolute resolution, compatible with shadertoy naming
	"uInvResolution", //!< inverse resolution
	"texelSize", //!< vec2 of a texel size, computed as 1/resolution

	"iTime", //!< compatible with shadertoy, float, shader playback time (in seconds)
	"iDate",  //!< compatible with shadertoy, vec4, (year, month, day, time in seconds)

	"cameraPosition", //!< world space camera position
	"modelView", //!< current camera modelview matrix
	"projection", //!< current camera projection matrix
	"modelViewProj", //!< current camera modelview-projection matrix
	"invModelViewProj",
	"prevModelViewProj",

	"zNear", //!< camera near plane
	"zFar"	//!< camera far plane
};


//////////////////////////////////////////////////////////////////////////
// PropertyScheme

ShaderProperty& ShaderPropertyScheme::AddProperty(const ShaderProperty& property)
{
	const uint32_t nameHash = property.GetNameHash();
	VERIFY(nameHash != 0);
	VERIFY(!FindPropertyByHash(nameHash));

	auto& newProp = properties.emplace_back(property);
	newProp.SetIndexInArray(static_cast<int>(properties.size()) - 1);
	return newProp;
}

ShaderProperty& ShaderPropertyScheme::AddProperty(ShaderProperty&& property)
{
	const uint32_t nameHash = property.GetNameHash();
	VERIFY(nameHash != 0);
	VERIFY(!FindPropertyByHash(nameHash));

	auto& newProp = properties.emplace_back(std::move(property));
	newProp.SetIndexInArray(static_cast<int>(properties.size()) - 1);
	return newProp;
}

ShaderProperty& ShaderPropertyScheme::AddProperty(std::string_view nameIn, std::string_view uniformNameIn, FBProperty* fbPropertyIn)
{
	ShaderProperty property(nameIn, uniformNameIn, fbPropertyIn);
	const uint32_t nameHash = property.GetNameHash();
	VERIFY(nameHash != 0);
	VERIFY(!FindPropertyByHash(nameHash));

	auto& newProp = properties.emplace_back(std::move(property));
	newProp.SetIndexInArray(static_cast<int>(properties.size()) - 1);
	return newProp;
}

ShaderProperty& ShaderPropertyScheme::AddProperty(std::string_view nameIn, std::string_view uniformNameIn, EPropertyType typeIn, FBProperty* fbPropertyIn)
{
	ShaderProperty property(nameIn, uniformNameIn, typeIn, fbPropertyIn);
	const uint32_t nameHash = property.GetNameHash();
	VERIFY(nameHash != 0);
	VERIFY(!FindPropertyByHash(nameHash));

	auto& newProp = properties.emplace_back(std::move(property));
	newProp.SetIndexInArray(static_cast<int>(properties.size()) - 1);
	return newProp;
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
	for (auto& prop : properties)
	{
		FBProperty* fbProp = component->PropertyList.Find(prop.GetName());
		VERIFY_MSG(fbProp || !prop.IsGeneratedByUniform(), "%s\n", prop.GetName());
		if (fbProp)
		{
			prop.SetFBProperty(fbProp);
			prop.SetFBComponent(component);
		}
	}
}

bool ShaderPropertyScheme::ExportToJSON(const char* fileName) const
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
	std::string out;
	try
	{
		out = root.dump(4);
	}
	catch (...)
	{
		return false;
	}

	std::ofstream ofs(fileName, std::ios::out | std::ios::trunc);
	if (!ofs.is_open())
		return false;

	ofs << out;
	ofs.close();
	return true;
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
		VERIFY(location >= 0);

		// System uniform?
		const int sysId = FindSystemUniform(uniformName);
		const bool isSystemUniform = (sysId >= 0);
		if (isSystemUniform)
		{
			SetSystemUniformLoc(static_cast<ShaderSystemUniform>(sysId), location);
		}

		const auto shaderType = UniformTypeToShaderPropertyType(type);

		// 3) Create ShaderProperty
		if (auto prop = FindPropertyByUniform(uniformName))
		{
			// already exists, update location
			VERIFY(prop
				|| (prop->GetType() == shaderType)
				|| (prop->GetType() == EPropertyType::BOOL && shaderType == EPropertyType::FLOAT));
			prop->SetLocation(location);
		}
		else if (!isSystemUniform)
		{
			//VERIFY(DoPopulatePropertiesFromUniforms());

			if (doPopulatePropertiesFromUniforms)
			{
				ShaderProperty newProp;

				newProp.SetGeneratedByUniform(true);
				newProp.SetUniformName(uniformName);
				newProp.SetLocation(location);
				newProp.SetType(shaderType);

				// from a uniform name, let's extract special postfix and convert it into a flag bit, prepare a clean property name
				// metadata
				SetNameAndFlagsFromUniformNameAndType(newProp, newProp.GetUniformName(), type);
				AddProperty(std::move(newProp));
				added++;
			}
		}
	}

	return added;
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
	case GL_FLOAT_MAT4:
		return EPropertyType::MAT4;
	case GL_SAMPLER_2D:
		return EPropertyType::TEXTURE;
	default:
		LOGE("unsupported uniform type %d", type);
		return EPropertyType::FLOAT;
	}
}

int ShaderPropertyScheme::FindSystemUniform(const char* uniformName) const
{
	for (int i = 0; i < static_cast<int>(ShaderSystemUniform::COUNT); ++i)
	{
		if (std::strcmp(uniformName, gSystemUniformNames[i]) == 0)
			return i;
	}
	return -1;
}

bool ShaderPropertyScheme::IsInternalGLSLUniform(const char* uniformName) const
{
	return std::strncmp(uniformName, "gl_", 3) == 0;
}