#pragma once

#include "nv_math.h"
#include <string_view>

namespace PostProcessingEffects
{
	// forward
	class CEffectPropertyList;

	/// <summary>
	/// supported types for effect property
	/// </summary>
	enum class EEffectType
	{
		NUMBER,
		INT,
		BOOL,
		VECTOR,
		VECTOR2,
		VECTOR4,
		STRING,
		ENUM,
		COUNT
	};

	// generate such properties / attributes from json or python or cpp code
	//  is used to create UI elements and read values from them, also to redirect values into uniform in case of straight logic
	//  in case uniform have to be assigned in a special logic, then value have to be empty and another rule will be applied
	struct SEffectProperty
	{
	private:
		SEffectProperty() {}
		friend class CEffectPropertyList;
	public:
		// no copy for the property, only move
		SEffectProperty(const SEffectProperty& other)
		{
			name = other.name;
			tooltip = other.tooltip;
			uniform = other.uniform;

			valueType = other.valueType;
			std::copy(other.str, other.str + STR_SIZE, str);

			minValue = other.minValue;
			maxValue = other.maxValue;
			isClamped = other.isClamped;
		}
		SEffectProperty& operator= (const SEffectProperty& other)
		{
			name = other.name;
			tooltip = other.tooltip;
			uniform = other.uniform;

			valueType = other.valueType;
			std::copy(other.str, other.str + STR_SIZE, str);

			minValue = other.minValue;
			maxValue = other.maxValue;
			isClamped = other.isClamped;
			return *this;
		}
		SEffectProperty(SEffectProperty&& other) noexcept
		{
			name = other.name;
			tooltip = other.tooltip;
			uniform = other.uniform;

			valueType = other.valueType;
			std::copy(other.str, other.str + STR_SIZE, str);

			minValue = other.minValue;
			maxValue = other.maxValue;
			isClamped = other.isClamped;
		}
		SEffectProperty& operator= (SEffectProperty&& other) noexcept
		{
			name = other.name;
			tooltip = other.tooltip;
			uniform = other.uniform;

			valueType = other.valueType;
			std::copy(other.str, other.str + STR_SIZE, str);

			minValue = other.minValue;
			maxValue = other.maxValue;
			isClamped = other.isClamped;
			return *this;
		}

		// string view from a string pool
		std::string_view name;
		std::string_view tooltip;
		std::string_view uniform;

		EEffectType valueType;

		static const int STR_SIZE{ 32 };

		// can we pack into pool ?! some memory stream and store only offset from it
		union
		{
			int intValue;
			float floatValue;
			bool boolValue;
			vec2 vec2;
			vec3 vec3;
			vec4 vec4;
			char str[STR_SIZE]; // in case of enum, we store string of elements and one char of default value
		};

		// could be used for sliders
		float minValue{ 0.0f };
		float maxValue{ 100.0f };
		bool isClamped{ false };
	};
}

