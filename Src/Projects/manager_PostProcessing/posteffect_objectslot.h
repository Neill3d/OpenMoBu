#pragma once

/**	\file	posteffect_objectslot.h

Sergei <Neill3d> Solokhin 2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- SDK include
#include <fbsdk/fbsdk.h>
#include "shaderproperty_naming.h"

#include <utility>
#include <vector>

// forward
class PostEffectContextProxy;
struct ShaderPropertyScheme;
struct ShaderPropertyValue;

/**
* values of a scene model connected to an object slot property (uniform vec3 <name>_object)
* NOTE: defaults describe an empty slot, color and intensity are neutral for a multiplication
*/
struct ObjectSlotData
{
	bool connected{ false };
	float position[3]{ 0.0f, 0.0f, 0.0f };	//!< world position
	float color[3]{ 1.0f, 1.0f, 1.0f };		//!< light diffuse color, white for a non-light
	float intensity{ 1.0f };				//!< light intensity / 100, 1.0 for a non-light
	float screen[3]{ 0.0f, 0.0f, 0.0f };	//!< (u, v) in [0; 1] and a view depth (negative behind the camera), 0 without a camera
};

/// read the first model connected to an object slot property, values are evaluated for the current frame
void ReadObjectSlot(ObjectSlotData& out, FBProperty* slotProperty, const PostEffectContextProxy* effectContext);

/// write a slot value (Position) or a value of a companion uniform into a shader property value
void WriteObjectSlotValue(ShaderPropertyValue& value, const ObjectSlotData& data, ObjectSlotField field);

/**
* object slots read once per effect while collecting values, a slot and its companions share the data
*/
class ObjectSlotCache
{
public:
	/// @param lookupComponent when set, the slot property is found by name in it instead of using the associated FBProperty
	///  (the scheme association is not current, see ShaderPropertyScheme::IsAssociationCurrent)
	ObjectSlotData Get(const ShaderPropertyScheme* scheme, uint32_t slotNameHash, const PostEffectContextProxy* effectContext,
		FBComponent* lookupComponent = nullptr);

private:
	std::vector<std::pair<uint32_t, ObjectSlotData>> mEntries;
};
