#pragma once

#include "posteffectbase.h"
#include "posteffectbuffershader.h"

/// <summary>
/// this is for cases when effect contains of one buffer shader execution and directly output to effects chain buffer
/// </summary>
/// <typeparam name="T"></typeparam>
template<typename T>
class PostEffectSingleShader : public PostEffectBase
{
public:
	PostEffectSingleShader()
		: PostEffectBase()
		, mBufferShader(std::make_unique<T>(nullptr)) // making it without an owner component
	{
	}
	virtual ~PostEffectSingleShader()
	{
	}

	virtual bool IsActive() const override
	{
		return true;
	}

	virtual const char* GetName() const override
	{
		return mBufferShader->GetName();
	}

	virtual int GetNumberOfBufferShaders() const override { return 1; }
	virtual PostEffectBufferShader* GetBufferShaderPtr(const int bufferShaderIndex) override { return static_cast<PostEffectBufferShader*>(mBufferShader.get()); }
	virtual const PostEffectBufferShader* GetBufferShaderPtr(const int bufferShaderIndex) const override { return static_cast<const PostEffectBufferShader*>(mBufferShader.get()); }

protected:

	std::unique_ptr<T>		mBufferShader;
};


// TODO: effect mix shaders with input - for effect which do processing on input and then a computed output mix it up with input source

/*
class ScopedEffectBind {
public:
	ScopedEffectBind(PostEffectBase* effect) : mEffect(effect) { mEffect->Bind(); }
	~ScopedEffectBind() { mEffect->UnBind(); }
private:
	PostEffectBase* mEffect;
};
*/