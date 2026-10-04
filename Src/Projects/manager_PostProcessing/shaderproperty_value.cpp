
/**	\file	shaderproperty_value.cpp

Sergei <Neill3d> Solokhin 2025-2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

#include "shaderproperty_value.h"
#include <mobu_logging.h>


/////////////////////////////////////////////////////////////////////////
// ShaderPropertyValue

void ShaderPropertyValue::SetType(EPropertyType newType)
{
	VERIFY(newType != EPropertyType::NONE);

	type = newType;
	switch (newType) {
	case EPropertyType::INT:
	case EPropertyType::BOOL:
	case EPropertyType::FLOAT:
	case EPropertyType::TEXTURE:
		value = std::array<float, 1>{ 0.0f };
		break;
	case EPropertyType::VEC2:
		value = std::array<float, 2>{ 0.0f, 0.0f };
		break;
	case EPropertyType::VEC3:
		value = std::array<float, 3>{ 0.0f, 0.0f, 0.0f };
		break;
	case EPropertyType::VEC4:
		value = std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 1.0f };
		break;
	case EPropertyType::MAT4:
		value = std::vector<float>(15, 0.0f);
		break;
	}
}

