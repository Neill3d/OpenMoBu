#pragma once

/**	\file	shaderproperty_naming.h

Sergei <Neill3d> Solokhin 2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

#include <cstdint>
#include <string>
#include <string_view>

// NOTE: the parser has no GL or MotionBuilder dependencies, so it can be unit tested

// a glsl type of a uniform, as much as the naming rules need to know
enum class UniformBaseType : uint8_t
{
	Float,
	Bool,
	Int,
	Vec2,
	Vec3,
	Vec4,
	Sampler2D,
	Other
};

// a value that an object slot companion uniform receives
enum class ObjectSlotField : uint8_t
{
	None,
	Position,	//!< vec3, the slot uniform itself, world position of the connected object
	Color,		//!< vec3, <slot>_object_color, light diffuse color (white for a non-light)
	Intensity,	//!< float, <slot>_object_intensity, light intensity / 100 (1.0 for a non-light)
	Screen,		//!< vec3, <slot>_object_screen, (u, v, view depth) on screen
	Connected	//!< float, <slot>_object_connected, 1 when an object is connected
};

// what a uniform name and type mean for a user property
struct UniformNameInfo
{
	std::string propertyName; //!< a property name with a postfix removed (for a companion, the uniform name as is)

	bool isFlag{ false };			//!< _flag or any bool, a checkbox
	bool isClamped100{ false };		//!< _slider, [0; 100]
	bool isColor{ false };			//!< _color, a color picker
	bool isWorldToScreen{ false };	//!< _wstoss, a world position converted into a screen position

	bool isObjectSlot{ false };		//!< vec3 <name>_object, an object slot property <name>

	bool isObjectCompanion{ false };	//!< <name>_object_<field>, filled from the object slot <name>
	ObjectSlotField companionField{ ObjectSlotField::None };
	std::string slotPropertyName;		//!< for a companion, a property name of its object slot

	bool isInvalidCompanion{ false };	//!< a known companion field with a wrong uniform type
};

// split a uniform name into a property name and flags, according to postfix rules and a uniform type
UniformNameInfo ParseUniformName(std::string_view uniformName, UniformBaseType type);
