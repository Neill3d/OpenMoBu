/**	\file	shaderproperty.cxx

Sergei <Neill3d> Solokhin 2025-2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "shaderproperty.h"
#include "hashUtils.h"
#include "mobu_logging.h"
#include "posteffect_shader_userobject.h"
#include "posteffect_contextmobu.h"

ShaderProperty::ShaderProperty(const ShaderProperty& other)
{
	SetNameHash(other.GetNameHash());
	SetUniformNameHash(other.GetUniformNameHash());

	SetFBComponent(other.GetFBComponent());
	SetFBProperty(other.GetFBProperty());

	flags = other.flags;
	mDefaultValue = other.mDefaultValue;
	indexInArray = other.indexInArray;
	isGeneratedByUniform = other.isGeneratedByUniform;
}

ShaderProperty::ShaderProperty(std::string_view nameIn, std::string_view uniformNameIn, FBProperty* fbPropertyIn)
{
	SetName(nameIn);
	SetUniformName(uniformNameIn);

	if (fbPropertyIn)
	{
		const auto propertyType = FBPropertyToShaderPropertyType(fbPropertyIn->GetPropertyType());
		SetType(propertyType);
		SetFBProperty(fbPropertyIn);
	}
}

ShaderProperty::ShaderProperty(std::string_view nameIn, std::string_view uniformNameIn, EPropertyType typeIn, FBProperty* fbPropertyIn)
{
	SetType(typeIn);

	SetName(nameIn);
	SetUniformName(uniformNameIn);

	if (fbPropertyIn)
	{
		SetFBProperty(fbPropertyIn);
	}
}

uint32_t ShaderProperty::ComputeNameHash(std::string_view s) const
{
	return xxhash32(s.data(), s.size(), HASH_SEED);
}

void ShaderProperty::SetName(std::string_view nameIN) 
{ 
	nameHash = ComputeNameHash(nameIN);
	mDefaultValue.SetNameHash(nameHash);
}

void ShaderProperty::SetNameHash(uint32_t hashIn)
{
	nameHash = hashIn;
	mDefaultValue.SetNameHash(nameHash);
}

const char* ShaderProperty::GetName() const noexcept
{ 
	return ResolveHash32(nameHash); 
}

uint32_t ShaderProperty::GetNameHash() const
{
	return nameHash;
}

void ShaderProperty::SetUniformName(std::string_view uniformNameIN)
{
	uniformNameHash = ComputeNameHash(uniformNameIN.data());
}

void ShaderProperty::SetUniformNameHash(uint32_t hashIn)
{
	uniformNameHash = hashIn;
}

const char* ShaderProperty::GetUniformName() const noexcept
{ 
	return ResolveHash32(uniformNameHash);
}

uint32_t ShaderProperty::GetUniformNameHash() const
{
	return uniformNameHash;
}

ShaderProperty& ShaderProperty::SetType(EPropertyType newType) 
{
	mDefaultValue.SetType(newType);
	return *this;
}

ShaderProperty& ShaderProperty::SetRequired(bool isRequired)
{
	mDefaultValue.SetRequired(isRequired);
	return *this;
}

ShaderProperty& ShaderProperty::SetDefaultValue(int valueIn)
{
	assert((type == EPropertyType::INT)
		|| (type == EPropertyType::FLOAT)
		|| (type == EPropertyType::TEXTURE));

	mDefaultValue.SetValue(valueIn);
	return *this;
}

ShaderProperty& ShaderProperty::SetDefaultValue(bool valueIn)
{
	assert(type == EPropertyType::BOOL);
	mDefaultValue.SetValue(valueIn);
	return *this;
}

ShaderProperty& ShaderProperty::SetDefaultValue(float valueIn)
{
	assert(type == EPropertyType::FLOAT);
	mDefaultValue.SetValue(valueIn);
	return *this;
}

ShaderProperty& ShaderProperty::SetDefaultValue(double valueIn)
{
	assert(type == EPropertyType::FLOAT);
	mDefaultValue.SetValue(valueIn);
	return *this;
}

ShaderProperty& ShaderProperty::SetDefaultValue(float x, float y)
{
	assert(type == EPropertyType::VEC2);
	mDefaultValue.SetValue(x, y);
	return *this;
}

ShaderProperty& ShaderProperty::SetDefaultValue(float x, float y, float z)
{
	assert(type == EPropertyType::VEC3);
	mDefaultValue.SetValue(x, y, z);
	return *this;
}

ShaderProperty& ShaderProperty::SetDefaultValue(float x, float y, float z, float w)
{
	assert(type == EPropertyType::VEC4);
	mDefaultValue.SetValue(x, y, z, w);
	return *this;
}
const float* ShaderProperty::GetDefaultFloatData() const {
	return mDefaultValue.GetFloatData();
}

ShaderProperty& ShaderProperty::SetFlag(PropertyFlag testFlag, bool setValue) {
	flags.set(static_cast<size_t>(testFlag), setValue);
	if (testFlag == PropertyFlag::INVERT_VALUE)
		mDefaultValue.SetInvertValue(setValue);
	return *this;
}

ShaderProperty& ShaderProperty::SetScale(float scaleIn)
{
	mDefaultValue.SetScale(scaleIn);
	return *this;
}

float ShaderProperty::GetScale() const
{
	return mDefaultValue.GetScale();
}

bool ShaderProperty::HasFlag(PropertyFlag testFlag) const {
	return flags.test(static_cast<size_t>(testFlag));
}

void ShaderProperty::ReadFBPropertyValue(
	FBProperty* fbProperty,
	ShaderPropertyValue& value, 
	const ShaderProperty& shaderProperty, 
	const PostEffectContextProxy* effectContext,
	int maskIndex)
{
	if (fbProperty == nullptr)
		return;

	double v[4]{ 0.0 };
	const FBPropertyType fbType = fbProperty->GetPropertyType();

	switch (fbType)
	{
	case FBPropertyType::kFBPT_int:
	{
		VERIFY(value.GetType() == EPropertyType::INT);
		int ivalue = 0;
		fbProperty->GetData(&ivalue, sizeof(int), effectContext->GetEvaluateInfo());
		value.SetValue(ivalue);
	} break;

	case FBPropertyType::kFBPT_bool:
	{
		VERIFY((value.GetType() == EPropertyType::BOOL)
			|| (value.GetType() == EPropertyType::FLOAT));

		bool bvalue = false;
		fbProperty->GetData(&bvalue, sizeof(bool), effectContext->GetEvaluateInfo());
		value.SetValue(bvalue);
	} break;

	case FBPropertyType::kFBPT_double:
	{
		VERIFY(value.GetType() == EPropertyType::FLOAT); // , "%s | %s\n", fbProperty->GetName(), ResolveHash32(value.GetNameHash()));
		fbProperty->GetData(v, sizeof(double), effectContext->GetEvaluateInfo());
		value.SetValue(v[0]);
	} break;

	case FBPropertyType::kFBPT_float:
	{
		VERIFY(value.GetType() == EPropertyType::FLOAT);
		float fvalue = 0.0f;
		fbProperty->GetData(&fvalue, sizeof(float), effectContext->GetEvaluateInfo());
		value.SetValue(fvalue);
	} break;

	case FBPropertyType::kFBPT_Vector2D:
	{
		VERIFY(value.GetType() == EPropertyType::VEC2);
		fbProperty->GetData(v, sizeof(double) * 2, effectContext->GetEvaluateInfo());
		value.SetValue(static_cast<float>(v[0]), static_cast<float>(v[1]));
	} break;

	case FBPropertyType::kFBPT_Vector3D:
	case FBPropertyType::kFBPT_ColorRGB:
	{
		if (!shaderProperty.HasFlag(PropertyFlag::ConvertWorldToScreenSpace))
		{
			VERIFY(value.GetType() == EPropertyType::VEC3);
			fbProperty->GetData(v, sizeof(double) * 3, effectContext->GetEvaluateInfo());
			value.SetValue(static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]));
		}
		else
		{
			VERIFY(effectContext != nullptr);
			// convert world to screen shape, output VEC2
			VERIFY(value.GetType() == EPropertyType::VEC2);

			// world space to screen space
			fbProperty->GetData(v, sizeof(double) * 3, effectContext->GetEvaluateInfo());

			const FBMatrix mvp(effectContext->GetModelViewProjMatrix());

			FBVector4d v4;
			FBVectorMatrixMult(v4, mvp, FBVector4d(v[0], v[1], v[2], 1.0));

			v4[0] = effectContext->GetViewWidth() * 0.5 * (v4[0] + 1.0);
			v4[1] = effectContext->GetViewHeight() * 0.5 * (v4[1] + 1.0);

			value.SetValue(static_cast<float>(v4[0]) / static_cast<float>(effectContext->GetViewWidth()),
				static_cast<float>(v4[1]) / static_cast<float>(effectContext->GetViewHeight()));
		}

	} break;

	case FBPropertyType::kFBPT_Vector4D:
	case FBPropertyType::kFBPT_ColorRGBA:
	{
		VERIFY(value.GetType() == EPropertyType::VEC4);
		fbProperty->GetData(v, sizeof(double) * 4, effectContext->GetEvaluateInfo());
		value.SetValue(static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]), static_cast<float>(v[3]));
	} break;

	case FBPropertyType::kFBPT_object:
	{
		// processed in PostEffectBufferShader::CollectUIValues
	} break;

	default:
		LOGE("unsupported fb property type %d", static_cast<int>(fbType));
		break;
	}
}

void ShaderProperty::ReadTextureConnections(ShaderPropertyValue& value, FBProperty* fbProperty)
{
	bool isFound = false;

	if (FBIS(fbProperty, FBPropertyListObject))
	{
		if (FBPropertyListObject* listObjProp = FBCast<FBPropertyListObject>(fbProperty))
		{
			FBComponent* firstObject = (listObjProp->GetCount() > 0) ? listObjProp->GetAt(0) : nullptr;

			if (FBIS(firstObject, FBTexture))
			{
				FBTexture* textureObj = FBCast<FBTexture>(firstObject);

				value.SetType(EPropertyType::TEXTURE);
				value.SetTexture(textureObj);
				isFound = true;
			}
			else if (FBIS(firstObject, EffectShaderUserObject))
			{
				EffectShaderUserObject* shaderObject = FBCast<EffectShaderUserObject>(firstObject);

				value.SetType(EPropertyType::SHADER_USER_OBJECT);
				value.SetShaderUserObject(shaderObject);
				isFound = true;
			}
		}
	}

	if (!isFound)
	{
		// not assigned object, which could be just a procedural applied current source buffer's texture
		value.SetType(EPropertyType::TEXTURE);
		value.SetTexture<FBTexture*>(nullptr);
	}
}


FBPropertyType ShaderProperty::ShaderPropertyToFBPropertyType(const ShaderProperty& prop)
{
	switch (prop.GetType())
	{
	case EPropertyType::FLOAT:
		return (prop.HasFlag(PropertyFlag::IsFlag)) ? FBPropertyType::kFBPT_bool : FBPropertyType::kFBPT_double;

	case EPropertyType::INT: return FBPropertyType::kFBPT_int;
	case EPropertyType::BOOL: return FBPropertyType::kFBPT_bool;

	case EPropertyType::VEC2:
		return (prop.HasFlag(PropertyFlag::ConvertWorldToScreenSpace)) ? FBPropertyType::kFBPT_Vector3D : FBPropertyType::kFBPT_Vector2D;

	case EPropertyType::VEC3:
		return (prop.HasFlag(PropertyFlag::IsColor)) ? FBPropertyType::kFBPT_ColorRGB : FBPropertyType::kFBPT_Vector3D;

	case EPropertyType::VEC4:
		return (prop.HasFlag(PropertyFlag::IsColor)) ? FBPropertyType::kFBPT_ColorRGBA : FBPropertyType::kFBPT_Vector4D;

	case EPropertyType::MAT4: return FBPropertyType::kFBPT_Vector4D; // TODO:
		// reference to a texture object that we could bind to a property
	case EPropertyType::TEXTURE: return FBPropertyType::kFBPT_object;
	default:
		return FBPropertyType::kFBPT_double;
	}
}

EPropertyType ShaderProperty::FBPropertyToShaderPropertyType(const FBPropertyType& fbType)
{
	switch (fbType)
	{
	case FBPropertyType::kFBPT_int: return EPropertyType::INT;
	case FBPropertyType::kFBPT_float: return EPropertyType::FLOAT;
	case FBPropertyType::kFBPT_bool: return EPropertyType::FLOAT;
	case FBPropertyType::kFBPT_Vector2D: return EPropertyType::VEC2;

	case FBPropertyType::kFBPT_ColorRGB:
	case FBPropertyType::kFBPT_Vector3D:
		return EPropertyType::VEC3;

	case FBPropertyType::kFBPT_ColorRGBA:
	case FBPropertyType::kFBPT_Vector4D:
		return EPropertyType::VEC4;
	}
	return EPropertyType::FLOAT;
}
