#pragma once

/**	\file	shaderproperty_scheme.h

Sergei <Neill3d> Solokhin 2025-2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

#include <GL/glew.h>
#include "shaderproperty.h"

#include <array>
#include <atomic>
#include <bitset>
#include <variant>
#include <vector>
#include <filesystem>

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

enum class ShaderSystemUniform : uint8_t
{
	INPUT_COLOR_SAMPLER_2D, //!< this is an input image that we read from
	iCHANNEL0, //!< this is an input image, compatible with shadertoy
	INPUT_DEPTH_SAMPLER_2D, //!< this is a scene depth texture sampler in case shader will need it for processing
	LINEAR_DEPTH_SAMPLER_2D, //!< a depth texture converted into linear space (used in SSAO)
	INPUT_MASK_SAMPLER_2D, //!< binded mask for a shader processing
	WORLD_NORMAL_SAMPLER_2D,

	USE_MASKING, //!< float uniform [0; 1] to define if the mask have to be used (value comes from the common property, see MakeCommonProperties)
	UPPER_CLIP, //!< this is an upper clip image level. defined in a texture coord space to skip processing (value comes from the common property)
	LOWER_CLIP, //!< this is a lower clip image level. defined in a texture coord space to skip processing (value comes from the common property)

	RESOLUTION, //!< vec2 that contains processing absolute resolution (size of the render target), like 1920x1080
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

	CAMERA_FOCUS_DISTANCE,	//!< float, camera focus distance in scene units (specific distance, interest or focus model)
	CAMERA_FOCUS_ANGLE,		//!< float, camera focus angle in degrees
	CAMERA_FOCAL_LENGTH,	//!< float, camera focal length in mm
	CAMERA_FOV,				//!< float, camera FieldOfView property in degrees
	CAMERA_FOCAL_LENGTH_PX,	//!< float, focal length in pixels of the view height, 0 for an orthogonal camera
	CAMERA_IS_ORTHO,		//!< float [0; 1], 1 for an orthogonal camera

	COUNT
};

// name and expected glsl type of a system uniform
struct SystemUniformDesc
{
	const char* name;
	GLenum		type;
};

// result of glsl uniforms reflection
struct ShaderPropertyScheme
{
	ShaderPropertyScheme()
	{
		ResetSystemUniformLocations();
	}

	ShaderProperty& AddProperty(const ShaderProperty& property);
	ShaderProperty& AddProperty(ShaderProperty&& property);
	ShaderProperty& AddProperty(std::string_view nameIn, std::string_view uniformNameIn, FBProperty* fbPropertyIn = nullptr);
	ShaderProperty& AddProperty(std::string_view nameIn, std::string_view uniformNameIn, EPropertyType typeIn, FBProperty* fbPropertyIn = nullptr);

	bool IsEmpty() const { return properties.empty(); }

	ShaderProperty* FindPropertyByHash(uint32_t nameHash);
	const ShaderProperty* FindPropertyByHash(uint32_t nameHash) const;

	ShaderProperty* FindProperty(const std::string_view name);
	const ShaderProperty* FindProperty(const std::string_view name) const;
	ShaderProperty* FindPropertyByUniform(const std::string_view name);
	const ShaderProperty* FindPropertyByUniform(const std::string_view name) const;

	ShaderProperty* GetProperty(const ShaderPropertyProxy proxy);
	const ShaderProperty* GetProperty(const ShaderPropertyProxy proxy) const;

	size_t GetNumberOfProperties() const { return properties.size(); }

	// non-critical problems found while the scheme was made (skipped uniforms, not associated properties, etc.)
	int GetNumberOfWarnings() const { return warningCount; }
	void AddWarning() { ++warningCount; }

	const std::vector<ShaderProperty>& GetProperties() const { return properties; }

	void	ResetSystemUniformLocations();

	void SetSystemUniformLoc(ShaderSystemUniform u, GLint location) noexcept {
		systemUniformLocations[static_cast<uint32_t>(u)] = location;
	}
	inline GLint GetSystemUniformLoc(ShaderSystemUniform u) const noexcept {
		return systemUniformLocations[static_cast<uint32_t>(u)];
	}

	// @see PostEffectBufferShader::Render
	void AssociateFBProperties(FBComponent* component);

	bool ExportToJSON(const std::filesystem::path& filePath) const;

	int ReflectUniforms(const GLuint programId, bool doPopulatePropertiesFromUniforms);

	// make object slot properties for companion uniforms (<name>_object_<field>) that have no active slot uniform
	void AddImplicitObjectSlots();

	int		FindSystemUniform(const char* uniformName) const; // -1 if not found, or return an index of a system uniform in the ShaderSystemUniform enum
	bool	IsInternalGLSLUniform(const char* uniformName) const;

	static const SystemUniformDesc& GetSystemUniformDesc(ShaderSystemUniform u) noexcept;
	// a glsl bool is accepted where a float is expected (flags like useMasking)
	static bool IsSystemUniformTypeCompatible(ShaderSystemUniform u, GLenum type) noexcept;

	EPropertyType UniformTypeToShaderPropertyType(GLenum type) const;

private:
	std::vector<ShaderProperty> properties;
	int warningCount{ 0 };
	std::array<GLint, static_cast<int>(ShaderSystemUniform::COUNT)> systemUniformLocations;
};