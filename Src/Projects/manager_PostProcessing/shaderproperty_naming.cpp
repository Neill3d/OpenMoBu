
/**	\file	shaderproperty_naming.cpp

Sergei <Neill3d> Solokhin 2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

#include "shaderproperty_naming.h"

namespace
{
	constexpr std::string_view OBJECT_COMPANION_INFIX{ "_object_" };

	bool EndsWith(std::string_view name, std::string_view postfix)
	{
		return name.size() > postfix.size()
			&& name.compare(name.size() - postfix.size(), postfix.size(), postfix) == 0;
	}

	struct CompanionRule
	{
		std::string_view field;
		ObjectSlotField kind;
		UniformBaseType type;
		UniformBaseType alternativeType;
	};

	constexpr CompanionRule companionRules[] = {
		{ "color", ObjectSlotField::Color, UniformBaseType::Vec3, UniformBaseType::Vec3 },
		{ "intensity", ObjectSlotField::Intensity, UniformBaseType::Float, UniformBaseType::Float },
		{ "screen", ObjectSlotField::Screen, UniformBaseType::Vec3, UniformBaseType::Vec3 },
		{ "connected", ObjectSlotField::Connected, UniformBaseType::Float, UniformBaseType::Bool }
	};

	struct PostfixRule
	{
		UniformBaseType type;
		std::string_view postfix;
		bool UniformNameInfo::* flag;
	};

	const PostfixRule postfixRules[] = {
		{ UniformBaseType::Float, "_flag", &UniformNameInfo::isFlag },
		{ UniformBaseType::Bool, "_flag", &UniformNameInfo::isFlag },
		{ UniformBaseType::Float, "_slider", &UniformNameInfo::isClamped100 },
		{ UniformBaseType::Vec2, "_wstoss", &UniformNameInfo::isWorldToScreen },
		{ UniformBaseType::Vec3, "_color", &UniformNameInfo::isColor },
		{ UniformBaseType::Vec4, "_color", &UniformNameInfo::isColor },
		{ UniformBaseType::Vec3, "_object", &UniformNameInfo::isObjectSlot }
	};
};

UniformNameInfo ParseUniformName(std::string_view uniformName, UniformBaseType type)
{
	UniformNameInfo info;

	// 1. object slot companion, <slot>_object_<field>
	// NOTE: it is checked before postfix rules, otherwise <slot>_object_color would be taken as a color picker

	const size_t infixPos = uniformName.rfind(OBJECT_COMPANION_INFIX);
	if (infixPos != std::string_view::npos && infixPos > 0)
	{
		const std::string_view field = uniformName.substr(infixPos + OBJECT_COMPANION_INFIX.size());

		for (const CompanionRule& rule : companionRules)
		{
			if (field != rule.field)
				continue;

			info.propertyName.assign(uniformName.data(), uniformName.size());
			info.slotPropertyName.assign(uniformName.data(), infixPos);

			if (type == rule.type || type == rule.alternativeType)
			{
				info.isObjectCompanion = true;
				info.companionField = rule.kind;
			}
			else
			{
				info.isInvalidCompanion = true;
			}
			return info;
		}
	}

	// 2. postfix rules

	std::string_view name = uniformName;

	for (const PostfixRule& rule : postfixRules)
	{
		if (rule.type == type && EndsWith(uniformName, rule.postfix))
		{
			info.*(rule.flag) = true;
			name = uniformName.substr(0, uniformName.size() - rule.postfix.size());
			break;
		}
	}

	// a glsl bool is always a checkbox, with or without the _flag postfix
	if (type == UniformBaseType::Bool)
	{
		info.isFlag = true;
	}

	info.propertyName.assign(name.data(), name.size());
	return info;
}
