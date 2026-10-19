
/**	\file	posteffect_objectslot.cpp

Sergei <Neill3d> Solokhin 2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

#include "posteffect_objectslot.h"
#include "posteffect_context.h"
#include "shaderproperty_scheme.h"
#include "shaderproperty_value.h"

#include <cmath>

void ReadObjectSlot(ObjectSlotData& out, FBProperty* slotProperty, const PostEffectContextProxy* effectContext)
{
	out = ObjectSlotData();

	if (!slotProperty || !effectContext || !FBIS(slotProperty, FBPropertyListObject))
		return;

	FBPropertyListObject* listProperty = FBCast<FBPropertyListObject>(slotProperty);
	FBComponent* component = (listProperty && listProperty->GetCount() > 0) ? listProperty->GetAt(0) : nullptr;
	// NOTE: FBCast doesn't validate the type, the slot has no class filter and any object could be connected
	if (!FBIS(component, FBModel))
		return;

	FBModel* model = static_cast<FBModel*>(component);

	FBEvaluateInfo* evaluateInfo = effectContext->GetEvaluateInfo();

	out.connected = true;

	FBVector3d position;
	model->GetVector(position, kModelTranslation, true, evaluateInfo);
	for (int i = 0; i < 3; ++i)
	{
		out.position[i] = static_cast<float>(position[i]);
	}

	if (FBIS(model, FBLight))
	{
		FBLight* light = static_cast<FBLight*>(model);
		double color[3]{ 1.0, 1.0, 1.0 };
		light->DiffuseColor.GetData(color, sizeof(double) * 3, evaluateInfo);

		double intensity{ 100.0 };
		light->Intensity.GetData(&intensity, sizeof(double), evaluateInfo);

		for (int i = 0; i < 3; ++i)
		{
			out.color[i] = static_cast<float>(color[i]);
		}
		// MotionBuilder default light intensity 100 is 1.0
		out.intensity = static_cast<float>(0.01 * intensity);
	}

	// screen position, there is no screen without a scene camera (texture user effect)
	if (effectContext->GetCamera())
	{
		// NOTE: opengl column-major matrices of the current camera
		const double* mvp = effectContext->GetModelViewProjMatrix();
		const double* mv = effectContext->GetModelViewMatrix();
		const double x = position[0], y = position[1], z = position[2];

		const double clipX = mvp[0] * x + mvp[4] * y + mvp[8] * z + mvp[12];
		const double clipY = mvp[1] * x + mvp[5] * y + mvp[9] * z + mvp[13];
		const double clipW = mvp[3] * x + mvp[7] * y + mvp[11] * z + mvp[15];

		if (std::abs(clipW) > 1e-8)
		{
			out.screen[0] = static_cast<float>(0.5 * (clipX / clipW + 1.0));
			out.screen[1] = static_cast<float>(0.5 * (clipY / clipW + 1.0));
		}

		// the camera looks along -Z in view space, a positive depth is in front of the camera
		const double viewZ = mv[2] * x + mv[6] * y + mv[10] * z + mv[14];
		out.screen[2] = static_cast<float>(-viewZ);
	}
}

void WriteObjectSlotValue(ShaderPropertyValue& value, const ObjectSlotData& data, ObjectSlotField field)
{
	switch (field)
	{
	case ObjectSlotField::Position:
		value.SetValue(data.position[0], data.position[1], data.position[2]);
		break;
	case ObjectSlotField::Color:
		value.SetValue(data.color[0], data.color[1], data.color[2]);
		break;
	case ObjectSlotField::Intensity:
		value.SetValue(data.intensity);
		break;
	case ObjectSlotField::Screen:
		value.SetValue(data.screen[0], data.screen[1], data.screen[2]);
		break;
	case ObjectSlotField::Connected:
		value.SetValue(data.connected ? 1.0f : 0.0f);
		break;
	default:
		break;
	}
}

ObjectSlotData ObjectSlotCache::Get(const ShaderPropertyScheme* scheme, uint32_t slotNameHash, const PostEffectContextProxy* effectContext,
	FBComponent* lookupComponent)
{
	for (const auto& entry : mEntries)
	{
		if (entry.first == slotNameHash)
			return entry.second;
	}

	// NOTE: a slot without an associated FBProperty (or without a connection) gives values of an empty slot
	const ShaderProperty* slot = (scheme) ? scheme->FindPropertyByHash(slotNameHash) : nullptr;
	FBProperty* slotProperty = nullptr;
	if (slot && slot->HasFlag(PropertyFlag::IsObjectSlot))
	{
		slotProperty = (lookupComponent) ? ShaderPropertyScheme::FindFBProperty(lookupComponent, *slot) : slot->GetFBProperty();
	}

	ObjectSlotData data;
	ReadObjectSlot(data, slotProperty, effectContext);

	mEntries.emplace_back(slotNameHash, data);
	return data;
}
