
/**	\file	effectshaderconnections.cxx

Sergei <Neill3d> Solokhin 2018-2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "effectshaderconnections.h"
#include "mobu_logging.h"

#include "posteffect_shader_userobject.h"

/////////////////////////////////////////////////////////////////////////
// IEffectShaderConnections

FBPropertyType IEffectShaderConnections::ShaderPropertyToFBPropertyType(const IEffectShaderConnections::ShaderProperty& prop)
{
	switch (prop.type)
	{
	case IEffectShaderConnections::EPropertyType::FLOAT:
		return (prop.HasFlag(IEffectShaderConnections::PropertyFlag::IsFlag)) ? FBPropertyType::kFBPT_bool : FBPropertyType::kFBPT_double;
	case IEffectShaderConnections::EPropertyType::INT:
		return FBPropertyType::kFBPT_int;
	case IEffectShaderConnections::EPropertyType::BOOL:
		return FBPropertyType::kFBPT_bool;
	case IEffectShaderConnections::EPropertyType::VEC2:
		return (prop.HasFlag(IEffectShaderConnections::PropertyFlag::ConvertWorldToScreenSpace)) ? FBPropertyType::kFBPT_Vector3D : FBPropertyType::kFBPT_Vector2D;
	case IEffectShaderConnections::EPropertyType::VEC3:
		return (prop.HasFlag(IEffectShaderConnections::PropertyFlag::IsColor)) ? FBPropertyType::kFBPT_ColorRGB : FBPropertyType::kFBPT_Vector3D;
	case IEffectShaderConnections::EPropertyType::VEC4:
		return (prop.HasFlag(IEffectShaderConnections::PropertyFlag::IsColor)) ? FBPropertyType::kFBPT_ColorRGBA : FBPropertyType::kFBPT_Vector4D;
	case IEffectShaderConnections::EPropertyType::MAT4:
		return FBPropertyType::kFBPT_Vector4D; // TODO:
	case IEffectShaderConnections::EPropertyType::TEXTURE:
		return FBPropertyType::kFBPT_object; // reference to a texture object that we could bind to a property
	default:
		return FBPropertyType::kFBPT_double;
	}
}

IEffectShaderConnections::EPropertyType IEffectShaderConnections::FBPropertyToShaderPropertyType(const FBPropertyType& fbType)
{
	switch (fbType)
	{
	case FBPropertyType::kFBPT_int:
		return IEffectShaderConnections::EPropertyType::INT;
	case FBPropertyType::kFBPT_float:
		return IEffectShaderConnections::EPropertyType::FLOAT;
	case FBPropertyType::kFBPT_bool:
		return IEffectShaderConnections::EPropertyType::FLOAT;
	case FBPropertyType::kFBPT_Vector2D:
		return IEffectShaderConnections::EPropertyType::VEC2;
	case FBPropertyType::kFBPT_ColorRGB:
	case FBPropertyType::kFBPT_Vector3D:
		return IEffectShaderConnections::EPropertyType::VEC3;
	case FBPropertyType::kFBPT_ColorRGBA:
	case FBPropertyType::kFBPT_Vector4D:
		return IEffectShaderConnections::EPropertyType::VEC4;
	}
	return IEffectShaderConnections::EPropertyType::FLOAT;
}

IEffectShaderConnections::EPropertyType IEffectShaderConnections::UniformTypeToShaderPropertyType(GLenum type)
{
	switch (type)
	{
	case GL_FLOAT:
		return IEffectShaderConnections::EPropertyType::FLOAT;
	case GL_INT:
		return IEffectShaderConnections::EPropertyType::INT;
	case GL_BOOL:
		return IEffectShaderConnections::EPropertyType::FLOAT;
	case GL_FLOAT_VEC2:
		return IEffectShaderConnections::EPropertyType::VEC2;
	case GL_FLOAT_VEC3:
		return IEffectShaderConnections::EPropertyType::VEC3;
	case GL_FLOAT_VEC4:
		return IEffectShaderConnections::EPropertyType::VEC4;
	case GL_FLOAT_MAT4:
		return IEffectShaderConnections::EPropertyType::MAT4;
	case GL_SAMPLER_2D:
		return IEffectShaderConnections::EPropertyType::TEXTURE;
	default:
		LOGE("unsupported uniform type %d", type);
		return IEffectShaderConnections::EPropertyType::FLOAT;
	}
}

IEffectShaderConnections::ShaderProperty::ShaderProperty(const char* nameIn, const char* uniformNameIn, FBProperty* fbPropertyIn)
{
	if (nameIn)
		strcpy_s(name, sizeof(char) * 64, nameIn);
	if (uniformNameIn)
		strcpy_s(uniformName, sizeof(char) * 64, uniformNameIn);
	if (fbPropertyIn)
	{
		const auto propertyType = FBPropertyToShaderPropertyType(fbPropertyIn->GetPropertyType());
		SetType(propertyType);
		fbProperty = fbPropertyIn;
	}
}

IEffectShaderConnections::ShaderProperty::ShaderProperty(const char* nameIn, const char* uniformNameIn, IEffectShaderConnections::EPropertyType typeIn, FBProperty* fbPropertyIn)
{
	if (nameIn)
		strcpy_s(name, sizeof(char) * 64, nameIn);
	if (uniformNameIn)
		strcpy_s(uniformName, sizeof(char) * 64, uniformNameIn);

	SetType(typeIn);

	if (fbPropertyIn)
		fbProperty = fbPropertyIn;
}

IEffectShaderConnections::ShaderProperty& IEffectShaderConnections::ShaderProperty::SetType(IEffectShaderConnections::EPropertyType newType) {
	type = newType;
	switch (newType) {
	case IEffectShaderConnections::EPropertyType::INT:
	case IEffectShaderConnections::EPropertyType::BOOL:
	case IEffectShaderConnections::EPropertyType::FLOAT:
	case IEffectShaderConnections::EPropertyType::TEXTURE:
		value = std::array<float, 1>{ 0.0f };
		break;
	case IEffectShaderConnections::EPropertyType::VEC2:
		value = std::array<float, 2>{0.0f, 0.0f};
		break;
	case IEffectShaderConnections::EPropertyType::VEC3:
		value = std::array<float, 3>{0.0f, 0.0f, 0.0f};
		break;
	case IEffectShaderConnections::EPropertyType::VEC4:
		value = std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f};
		break;
	case IEffectShaderConnections::EPropertyType::MAT4:
		value = std::vector<float>(15, 0.0f);
		break;
	}
	return *this;
}

IEffectShaderConnections::ShaderProperty& IEffectShaderConnections::ShaderProperty::SetRequired(bool isRequired)
{
	bIsLocationRequired = isRequired;
	return *this;
}

IEffectShaderConnections::ShaderProperty& IEffectShaderConnections::ShaderProperty::SetValue(int valueIn)
{
	assert((type == IEffectShaderConnections::EPropertyType::INT)
		|| (type == IEffectShaderConnections::EPropertyType::FLOAT)
		|| (type == IEffectShaderConnections::EPropertyType::TEXTURE));

	value = std::array<float, 1>{ static_cast<float>(valueIn) };
	return *this;
}

IEffectShaderConnections::ShaderProperty& IEffectShaderConnections::ShaderProperty::SetValue(bool valueIn)
{
	assert(type == IEffectShaderConnections::EPropertyType::BOOL);

	value = std::array<float, 1>{ static_cast<float>(valueIn) };
	return *this;
}

IEffectShaderConnections::ShaderProperty& IEffectShaderConnections::ShaderProperty::SetValue(float valueIn)
{
	assert(type == IEffectShaderConnections::EPropertyType::FLOAT);

	value = std::array<float, 1>{ valueIn };
	return *this;
}

IEffectShaderConnections::ShaderProperty& IEffectShaderConnections::ShaderProperty::SetValue(double valueIn)
{
	assert(type == IEffectShaderConnections::EPropertyType::FLOAT);

	value = std::array<float, 1>{ static_cast<float>(valueIn) };
	return *this;
}

IEffectShaderConnections::ShaderProperty& IEffectShaderConnections::ShaderProperty::SetValue(float x, float y)
{
	assert(type == IEffectShaderConnections::EPropertyType::VEC2);

	value = std::array<float, 2>{ x, y };
	return *this;
}

IEffectShaderConnections::ShaderProperty& IEffectShaderConnections::ShaderProperty::SetValue(float x, float y, float z)
{
	assert(type == IEffectShaderConnections::EPropertyType::VEC3);

	value = std::array<float, 3>{ x, y, z };
	return *this;
}

IEffectShaderConnections::ShaderProperty& IEffectShaderConnections::ShaderProperty::SetValue(float x, float y, float z, float w)
{
	assert(type == IEffectShaderConnections::EPropertyType::VEC4);

	value = std::array<float, 4>{ x, y, z, w };
	return *this;
}

IEffectShaderConnections::ShaderProperty& IEffectShaderConnections::ShaderProperty::SetFlag(PropertyFlag testFlag, bool setValue) {
	flags.set(static_cast<size_t>(testFlag), setValue);
	return *this;
}

IEffectShaderConnections::ShaderProperty& IEffectShaderConnections::ShaderProperty::SetScale(float scaleIn)
{
	scale = scaleIn;
	return *this;
}

float IEffectShaderConnections::ShaderProperty::GetScale() const
{
	return scale;
}

float* IEffectShaderConnections::ShaderProperty::GetFloatData() {
	return std::visit([](auto& data) -> float* {
		return data.data();
		}, value);
}

bool IEffectShaderConnections::ShaderProperty::HasFlag(PropertyFlag testFlag) const {
	return flags.test(static_cast<size_t>(testFlag));
}

void IEffectShaderConnections::ShaderProperty::ReadFBPropertyValue(const IPostEffectContext* effectContext, int maskIndex)
{
	if (fbProperty == nullptr)
		return;

	double v[4]{ 0.0 };

	switch (type)
	{
	case IEffectShaderConnections::EPropertyType::INT:
	{
		int ivalue = 0;
		fbProperty->GetData(&ivalue, sizeof(int));
		GetFloatData()[0] = static_cast<float>(ivalue);
	} break;

	case IEffectShaderConnections::EPropertyType::BOOL:
	{
		bool bvalue = false;
		fbProperty->GetData(&bvalue, sizeof(bool));
		GetFloatData()[0] = static_cast<float>(bvalue);
	} break;

	case IEffectShaderConnections::EPropertyType::FLOAT:
	{
		if (HasFlag(IEffectShaderConnections::PropertyFlag::IsFlag))
		{
			bool bvalue = false;
			fbProperty->GetData(&bvalue, sizeof(bool));
			GetFloatData()[0] = bvalue ? 1.0f : 0.0f;
		}
		else
		{
			fbProperty->GetData(v, sizeof(double));
			GetFloatData()[0] = static_cast<float>(v[0]);
		}
	} break;

	case IEffectShaderConnections::EPropertyType::VEC2:
	{
		if (!HasFlag(IEffectShaderConnections::PropertyFlag::ConvertWorldToScreenSpace))
		{
			fbProperty->GetData(v, sizeof(double) * 2);
			float* value = GetFloatData();
			value[0] = static_cast<float>(v[0]);
			value[1] = static_cast<float>(v[1]);
		}
		else
		{
			// world space to screen space
			fbProperty->GetData(v, sizeof(double) * 3);

			FBMatrix mvp(effectContext->GetModelViewProjMatrix());

			FBVector4d v4;
			FBVectorMatrixMult(v4, mvp, FBVector4d(v[0], v[1], v[2], 1.0));

			v4[0] = effectContext->GetViewWidth() * 0.5 * (v4[0] + 1.0);
			v4[1] = effectContext->GetViewHeight() * 0.5 * (v4[1] + 1.0);

			float* value = GetFloatData();
			value[0] = static_cast<float>(v4[0]) / static_cast<float>(effectContext->GetViewWidth());
			value[1] = static_cast<float>(v4[1]) / static_cast<float>(effectContext->GetViewHeight());
			//value[2] = static_cast<float>(v4[2]);
		}
		
	} break;

	case IEffectShaderConnections::EPropertyType::VEC3:
	{
		fbProperty->GetData(v, sizeof(double) * 3);

		float* value = GetFloatData();
		value[0] = static_cast<float>(v[0]);
		value[1] = static_cast<float>(v[1]);
		value[2] = static_cast<float>(v[2]);

	} break;

	case IEffectShaderConnections::EPropertyType::VEC4:
	{
		fbProperty->GetData(v, sizeof(double) * 4);

		float* value = GetFloatData();
		value[0] = static_cast<float>(v[0]);
		value[1] = static_cast<float>(v[1]);
		value[2] = static_cast<float>(v[2]);
		value[3] = static_cast<float>(v[3]);
	} break;

	case IEffectShaderConnections::EPropertyType::TEXTURE:
		ReadTextureConnections();
		
		if (shaderUserObject)
		{
			FBComponent* tempComponent = effectContext->GetComponent();
			effectContext->OverrideComponent(shaderUserObject);
			shaderUserObject->GetUserShaderPtr()->CollectUIValues(effectContext, maskIndex);
			effectContext->OverrideComponent(tempComponent);
		}
		break;
	}
}

void IEffectShaderConnections::ShaderProperty::ReadTextureConnections()
{
	texture = nullptr;
	shaderUserObject = nullptr;

	if (FBIS(fbProperty, FBPropertyListObject))
	{
		if (FBPropertyListObject* listObjProp = FBCast<FBPropertyListObject>(fbProperty))
		{
			if (listObjProp->GetCount() > 0)
			{
				if (FBIS(listObjProp->GetAt(0), FBTexture))
				{
					FBTexture* textureObj = FBCast<FBTexture>(listObjProp->GetAt(0));
					assert(textureObj != nullptr);

					texture = textureObj;
				}
				else if (FBIS(listObjProp->GetAt(0), EffectShaderUserObject))
				{
					EffectShaderUserObject* shaderObject = FBCast<EffectShaderUserObject>(listObjProp->GetAt(0));
					assert(shaderObject != nullptr);

					shaderUserObject = shaderObject;
				}
			}
		}
	}
}