
#pragma once

// posteffectbase
/*
Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "GL/glew.h"

#include "graphics_framebuffer.h"

#include "glslShaderProgram.h"
#include "Framebuffer.h"

#include <variant>
#include <array>
#include <vector>
#include <memory>
#include <bitset>

// forward
class PostEffectBuffers;
class PostPersistentData;
class ScopedEffectBind;
class EffectShaderUserObject;

namespace FBSDKNamespace
{
	class FBCamera;
}

//////////////////////////////

enum class BuildInEffect
{
	FISHEYE,
	COLOR,
	VIGNETTE,
	FILMGRAIN,
	LENSFLARE,
	SSAO,
	DOF,
	DISPLACEMENT,
	MOTIONBLUR,
	COUNT
};

enum class CompositionMask : uint32_t
{
	NO_MASK = 0,
	MASK_A = 0 >> 1,
	// TODO:
};

/// <summary>
/// uniforms needed for a common effect functionality, masking, clipping, etc.
/// </summary>
class CommonEffectUniforms
{
public:

	static GLint GetColorSamplerSlot() { return 0; }

	static GLint GetDepthSamplerSlot() { return 1; }

	static GLint GetLinearDepthSamplerSlot() { return 2; }

	/// <summary>
	/// glsl sampler slot binded for a mask texture
	/// </summary>
	static GLint GetMaskSamplerSlot() { return 4; }


	static GLint GetWorldNormalSamplerSlot() { return 3; }

protected:

	void PrepareCommonLocations(GLSLShaderProgram* shader);

	/// <summary>
	/// collect common data values into a buffer, /see UploadCommonData to upload values into gpu shader
	/// </summary>
	void CollectCommonData(PostPersistentData* data, FBComponent* userObject, const char* enableMaskingPropertyName);

	/// <summary>
	/// glsl shader must be binded, upload uniforms from collected common data values
	/// </summary>
	void UploadCommonData();

	/// <summary>
	/// a property name in @sa PostPersistentData
	///  the property will be used to update useMasking glsl uniform value
	/// </summary>
	//virtual const char* GetEnableMaskPropertyName() const = 0;

protected:
	/*
	// common functionality of effects needs common uniforms
	GLint lowerClipLoc{ -1 };
	GLint upperClipLoc{ -1 };
	GLint useMaskLoc{ -1 };

	struct CommonShaderData
	{
		float useMasking{ 0.0f };
		float lowerClip{ 0.0f };
		float upperClip{ 0.0f };
		float pad{ 0.0f };
	} mCommonShaderData;
	*/
};


class IPostEffectContext
{
public:
	virtual ~IPostEffectContext() = default;

	struct Parameters
	{
		int w{ 1 }; //!< viewport width
		int h{ 1 }; //!< viewport height
		int localFrame{ 0 }; //!< playback frame number

		double sysTime{ 0.0 }; //!< system time (in seconds)
		double sysTimeDT{ 0.0 };

		double localTime{ 0.0 }; //!< playback time (in seconds)
		double localTimeDT{ 0.0 };
	};
	
	// interface to query the needed data

	int GetViewWidth() const { return mParameters.w; }
	int GetViewHeight() const { return mParameters.h; }

	int GetLocalFrame() const { return mParameters.localFrame; }
	double GetSystemTime() const { return mParameters.sysTime; }
	double GetLocalTime() const { return mParameters.localTime; }

	virtual double* GetCameraPosition() const = 0;

	virtual double* GetModelViewMatrix() const = 0;
	virtual double* GetProjectionMatrix() const = 0;
	virtual double* GetModelViewProjMatrix() const = 0;

	virtual FBCamera* GetCamera() const = 0;
	virtual FBComponent* GetComponent() const = 0;
	virtual PostPersistentData* GetPostProcessData() const = 0;

protected:
	Parameters mParameters;
};

class PostEffectContextMoBu : public IPostEffectContext
{
public:

	PostEffectContextMoBu(FBCamera* cameraIn, FBComponent* userObjectIn, PostPersistentData* postProcessDataIn, const Parameters& parametersIn)
		: camera(cameraIn)
		, userObject(userObjectIn)
		, postProcessData(postProcessDataIn)
	{
		mParameters = parametersIn;
		PrepareCache();
	}

	virtual double* GetCameraPosition() const override { return cameraPosition; }
	
	virtual double* GetModelViewMatrix() const override { return modelView; }
	virtual double* GetProjectionMatrix() const override { return projection; }
	virtual double* GetModelViewProjMatrix() const override { return modelViewProj; }
	
	FBCamera* GetCamera() const { return camera; }
	FBComponent* GetComponent() const { return userObject; }
	PostPersistentData* GetPostProcessData() const { return postProcessData; }

private:

	FBCamera* camera{ nullptr }; //!< current camera that we are drawing with
	FBComponent* userObject{ nullptr }; //!< this is a component where all ui properties are exposed
	PostPersistentData* postProcessData{ nullptr }; //!< this is a main post process object for common effects properties

	// cache values

	FBVector3d	cameraPosition;
	FBMatrix	modelView;
	FBMatrix	projection;
	FBMatrix	modelViewProj;

	void PrepareCache()
	{
		if (!camera)
			return;
		camera->GetVector(cameraPosition, kModelTranslation, true);
		camera->GetCameraMatrix(modelView, FBCameraMatrixType::kFBModelView);
		camera->GetCameraMatrix(projection, FBCameraMatrixType::kFBProjection);
		camera->GetCameraMatrix(modelViewProj, FBCameraMatrixType::kFBModelViewProj);
	}
};



enum class ShaderSystemUniform
{
	INPUT_COLOR_SAMPLER_2D, //!< this is an input image that we read from
	iCHANNEL0, //!< this is an input image, compatible with shadertoy
	INPUT_DEPTH_SAMPLER_2D, //!< this is a scene depth texture sampler in case shader will need it for processing
	LINEAR_DEPTH_SAMPLER_2D, //!< a depth texture converted into linear space (used in SSAO)
	INPUT_MASK_SAMPLER_2D, //!< binded mask for a shader processing
	WORLD_NORMAL_SAMPLER_2D,

	USE_MASKING, //!< float uniform [0; 1] to define if the mask have to be used
	UPPER_CLIP, //!< this is an upper clip image level. defined in a texture coord space to skip processing
	LOWER_CLIP, //!< this is a lower clip image level. defined in a texture coord space to skip processing

	RESOLUTION, //!< vec2 that contains processing absolute resolution, like 1920x1080
	iRESOLUTION, //!< vec2 absolute resolution, compatible with shadertoy
	TEXEL_SIZE, //!< vec2 of a texel size, computed as 1/resolution

	iTIME, //!< compatible with shadertoy, float, shader playback time (in seconds)
	iDATE, //!< compatible with shadertoy, vec4, (year, month, day, time in seconds)

	CAMERA_POSITION, //!< world space camera position
	MODELVIEW,	//!< current camera modelview matrix
	PROJ,		//!< current camera projection matrix
	MODELVIEWPROJ,	//!< current camera modelview-projection matrix

	COUNT
};

constexpr size_t PROPERTY_BITSET_SIZE = 8;

class IEffectShaderConnections
{
public:

	enum class EPropertyType : uint8_t
	{
		INT,
		BOOL,
		FLOAT,
		VEC2,
		VEC3,
		VEC4,
		MAT4,
		TEXTURE
	};

	enum class PropertyFlag
	{
		SYSTEM = 1,	// flag that the property is a system one, like masking, upper/lower clip, etc.
		IsClamped100 = 1,
		IsClamped1 = 2,
		IsFlag = 3,
		IsColor = 4,
		ConvertWorldToScreenSpace = 5,
		ShouldSkip = 6 //!< this is for manual processing of property (like manual reading and setting value)
	};

	class IUserData {
	public:
		virtual ~IUserData() = default;
	};

	class FBPropertyUserData : public IUserData {
		FBProperty* property;

		// extracted value from reference object property
		FBTexture* texture;
		EffectShaderUserObject* shaderUserObject;
	};

	

	struct ShaderProperty
	{
		char name[64]{ 0 };
		char uniformName[64]{ 0 };
		int length{ 0 };

		EPropertyType type{ EPropertyType::FLOAT };
		
		std::bitset<PROPERTY_BITSET_SIZE> flags;

		GLint location{ -1 }; //!< GLSL shader location holder
		
		//std::unique_ptr<IUserData> userData;

		FBProperty* fbProperty{ nullptr };

		// extracted value from reference object property
		FBTexture* texture{ nullptr };
		EffectShaderUserObject* shaderUserObject{ nullptr };

		// Type-safe dynamic storage for float values
		std::variant<std::array<float, 1>, std::array<float, 2>, std::array<float, 3>, std::array<float, 4>, std::vector<float>> value;

		float scale{ 1.0f };

		ShaderProperty() : value(std::array<float, 1>{ 0.0f }) {}

		// constructor to associate property with fbProperty, recognize the type
		ShaderProperty(const char* nameIn, const char* uniformNameIn, FBProperty* fbPropertyIn = nullptr);
		ShaderProperty(const char* nameIn, const char* uniformNameIn, IEffectShaderConnections::EPropertyType typeIn, FBProperty* fbPropertyIn = nullptr);

		ShaderProperty& SetType(IEffectShaderConnections::EPropertyType newType);
		ShaderProperty& SetFlag(PropertyFlag testFlag, bool setValue);
		
		ShaderProperty& SetScale(float scaleIn);
		float GetScale() const;

		ShaderProperty& SetValue(int valueIn);
		ShaderProperty& SetValue(float valueIn);
		ShaderProperty& SetValue(double valueIn);

		float* GetFloatData();

		bool HasFlag(PropertyFlag testFlag) const;

		void ReadFBPropertyValue(const IPostEffectContext* effectContext, int maskIndex);
	};

	virtual ~IEffectShaderConnections() = default;

	virtual ShaderProperty& AddProperty(const ShaderProperty& property) = 0;
	virtual ShaderProperty& AddProperty(ShaderProperty&& property) = 0;

	virtual int GetNumberOfProperties() = 0;
	virtual ShaderProperty& GetProperty(int index) = 0;

	virtual ShaderProperty* FindProperty(const std::string& name) = 0;

	//virtual int GetNumberOfOutputConnections() = 0;
	//virtual void GetOutputConnectionType() = 0;
	//virtual void GetOutputProperty(int index) = 0;

	// look for a UI interface, and read properties and its values
	// should it be a separate class
	virtual bool CollectUIValues(const IPostEffectContext* effectContext, int maskIndex) = 0;

	// use uniformName to track down some type casts
	static FBPropertyType ShaderPropertyToFBPropertyType(const ShaderProperty& prop);

	static EPropertyType FBPropertyToShaderPropertyType(const FBPropertyType& fbType);

	static EPropertyType UniformTypeToShaderPropertyType(GLenum type);
};



/// <summary>
/// one single fragment shader that we do one number of passes to process the input
/// </summary>
class PostEffectBufferShader : public CommonEffectUniforms, public IEffectShaderConnections
{
public:

	PostEffectBufferShader();
	virtual ~PostEffectBufferShader();

	/// number of variations of the same effect, but with a different algorithm (for instance, 3 ways of making a lens flare effect)
	virtual int GetNumberOfVariations() const abstract;

	//! an effect public name
	virtual const char* GetName() const abstract;
	//! get a filename of vertex shader, for this effect. returns a relative filename
	virtual const char* GetVertexFname(const int variationIndex) const abstract;
	//! get a filename of a fragment shader, for this effect, returns a relative filename
	virtual const char* GetFragmentFname(const int variationIndex) const abstract;

	// does shader uses the scene depth sampler (part of a system input)
	virtual bool IsDepthSamplerUsed() const;
	// does shader uses the scene linear depth sampler (part of a system input)
	virtual bool IsLinearDepthSamplerUsed() const;

	virtual bool IsMaskSamplerUsed() const;
	virtual bool IsWorldNormalSamplerUsed() const;

	/// load and initialize shader from a specified location, vname and fname are computed absolute path
	bool Load(const int variationIndex, const char* vname, const char* fname);

	/// <summary>
	/// use \ref GetVertexFname and \ref GetFragmentFname to load a shader variance
	///  the given shaderLocation is used to make an absolute path
	/// </summary>
	bool Load(const char* shaderLocation);

	//! is being called after \ref Load is succeed
	//!  so we could initialized some property or system uniform locations
	bool PrepUniforms(const int variationIndex);

	/// <summary>
	/// is being called right before Render and when shader is binded
	/// </summary>
	void UploadUniforms(PostEffectBuffers* buffers, FrameBuffer* dstBuffer, int colorAttachment, const GLuint inputTextureId, int w, int h, bool generateMips, const IPostEffectContext* effectContext);

	/// repeated call of the shader (define iPass uniform to distinguish)
	virtual const int GetNumberOfPasses() const;
	
	//! get a pointer to a (current variance) shader program
	GLSLShaderProgram* GetShaderPtr();
	const GLSLShaderProgram* GetShaderPtr() const;

	/// <summary>
	/// the given buffer shader will process the given inputTextureId and write result into dst frame buffer
	/// </summary>
	void Render(PostEffectBuffers* buffers, FrameBuffer* dstBuffer, int colorAttachment, const GLuint inputTextureId, int w, int h, bool generateMips, const IPostEffectContext* effectContext);

	// means that processing will use smaller size of a buffer
	void SetDownscaleMode(const bool value);
	bool IsDownscaleMode() const { return isDownscale; }

	// shader version, increments on every shader reload
	int GetVersion() const { return version; }

	// binded textures for connected buffers starts from 5, then custom user textures will start from 10
	static int GetUserSamplerId() { return 5; }

public:
	//
	// IEffectShaderConnections
	virtual ShaderProperty& AddProperty(const ShaderProperty& property) override;
	virtual ShaderProperty& AddProperty(ShaderProperty&& property) override;

	virtual int GetNumberOfProperties() override;
	virtual ShaderProperty& GetProperty(int index) override;
	virtual ShaderProperty* FindProperty(const std::string& name) override;

	// TODO: search for locations

	void MakeCommonProperties();

	int MakePropertyLocationsFromShaderUniforms();
	int MakeSystemLocationsFromShaderUniforms();

	int PopulatePropertiesFromShaderUniforms();

	// TODO: upload properties values into uniforms

	void AutoUploadUniforms(PostEffectBuffers* buffers, const GLuint inputTextureId, int w, int h, bool generateMips, const IPostEffectContext* effectContext);

	// TODO: auto update values from fb properties

	// TODO: let's move it for buildin shaders only ?!
	//! grab from UI all needed parameters to update effect state (uniforms) during evaluation
	bool CollectUIValues(const IPostEffectContext* effectContext, int maskIndex) override;		//!< grab main UI values for the effect


	bool CollectUIValues(FBComponent* component, const IPostEffectContext* effectContext, int maskIndex);

protected:

	std::unordered_map<std::string, ShaderProperty>		mProperties;

	//! a callback event to process a property added, so that we could make and associate component's FBProperty with it
	virtual void OnPropertyAdded(ShaderProperty& property) 
	{}

protected:

	static const char* gSystemUniformNames[static_cast<int>(ShaderSystemUniform::COUNT)];
	GLint mSystemUniformLocations[static_cast<int>(ShaderSystemUniform::COUNT)];

	void	ResetSystemUniformLocations();
	int		IsSystemUniform(const char* uniformName); // -1 if not found, or return an index of a system uniform in the ShaderSystemUniform enum
	void	BindSystemUniforms(const IPostEffectContext* effectContext) const;

protected:
	bool isDownscale{ false };
	int version; //!< keep track of resolution modifications, inc version everytime we change resolution
	int mCurrentShader{ 0 }; //!< current variance of a shader
	std::vector<std::unique_ptr<GLSLShaderProgram>>	mShaders; //!< store a list of all variances

	void SetCurrentShader(const int index) { mCurrentShader = index; }
	void FreeShaders();

	//!< TODO: masking property in the UI, should we move it into input connection ?!
	virtual const char* GetUseMaskingPropertyName() const = 0;
	virtual const char* GetMaskingChannelPropertyName() const = 0;
	//!< if true, once shader is loaded, let's inspect all the uniforms and make properties from them
	virtual bool DoPopulatePropertiesFromUniforms() const = 0;

	virtual bool OnPrepareUniforms(const int variationIndex) { return true; }
	virtual bool OnCollectUI(const IPostEffectContext* effectContext, int maskIndex) { return true; }
	virtual void OnUploadUniforms(PostEffectBuffers* buffers, FrameBuffer* dstBuffer, int colorAttachment, const GLuint inputTextureId, int w, int h, bool generateMips, const IPostEffectContext* effectContext)
	{}

	//! bind effect shader program
	virtual void Bind();
	//! unbind effect shader program
	virtual void UnBind();

	//! initialize a specific path for drawing
	virtual bool PrepPass(const int pass, int w, int h);

	void RenderPass(int passIndex, FrameBuffer* dstBuffer, int colorAttachment, const GLuint inputTextureId, int w, int h, bool generateMips);
};

/// <summary>
/// a set of effect buffer shaders to process the input and write output to the effects chain
///  chain processing is sequential, except scenarios
///  - one buffer shader request outputs from several other buffer shaders
///		- when buffer shader request one output from another buffer shader, that could be aligned in a sequntial order
///  - when buffer shader request a downscale, that could be grouped with connected buffer shaders with downscale and processed as a separate sequence and then mix into main chain
/// 
/// that means - effects chain have to be effects tree instead
/// </summary>
class PostEffectBase
{
public:

	//! a constructor
	PostEffectBase();
	//! a destructor
	virtual ~PostEffectBase();

	virtual bool IsActive() const abstract;

	//! an effect public name
	virtual const char* GetName() const abstract; // { return mName.c_str(); }

	bool Load(const char* shaderLocation);

	virtual bool IsReadyAndActive() const;

	// TODO: should it be a general FBComponent instead of pre-defined PostPersistentData user object ?!
	bool CollectUIValues(const IPostEffectContext* effectContext);

	// TODO: mask index is like a pre-defined input connection
	//! define internal mask channel index or -1 for default, it comes from a user input (UI)
	void SetMaskIndex(const int maskIndex) { mMaskIndex = maskIndex; }
	//! get defined mask channel index
	int GetMaskIndex() const { return mMaskIndex; }

	virtual bool IsDepthSamplerUsed() const;
	virtual bool IsLinearDepthSamplerUsed() const;
	virtual bool IsMaskSamplerUsed() const;
	virtual bool IsWorldNormalSamplerUsed() const;

	struct RenderEffectContext
	{
		PostEffectBuffers* buffers;

		// INPUT: input in the effects chain for this effect
		GLuint srcTextureId;
		GLuint depthTextureId;

		int viewWidth;
		int viewHeight;

		bool generateMips;

		// OUTPUT: write an effect composition to a given frame buffer
		FrameBuffer* dstFrameBuffer;
		int colorAttachment; //!< a way to define a color attachment in the dstFrameBuffer where we should render into
	};

	virtual void Process(const RenderEffectContext& renderContext, const IPostEffectContext* effectContext);

	virtual int GetNumberOfBufferShaders() const abstract;
	virtual PostEffectBufferShader* GetBufferShaderPtr(const int bufferShaderIndex) abstract;
	virtual const PostEffectBufferShader* GetBufferShaderPtr(const int bufferShaderIndex) const abstract;

protected:

	//std::string mName;

	int mMaskIndex{ -1 }; //!< which mask channel the effect is use (-1 for a default, globally defined mask channel)

	std::vector<std::unique_ptr<FrameBuffer>>	mFrameBuffers; // in case buffer shader is used to render it into texture
	std::vector<int> mBufferShaderVersions; // keep last processing buffer shader version, every resolution change is going to inc the version

	// in case of render to texture
	void InitializeFrameBuffers(int w, int h);

	void BindFrameBuffer(int bufferIndex);

	void UnBindFrameBuffer(int bufferIndex, bool generateMips);

	GLuint GetTextureTextureId(int bufferIndex) const;

	FrameBuffer* GetFrameBufferForBufferShader(const int shaderIndex);

	bool DoNeedIntermediateBuffers();

};

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
		, mBufferShader(std::make_unique<T>())
	{}
	virtual ~PostEffectSingleShader()
	{}

	virtual bool IsActive() const override
	{
		return true;
	}

	virtual const char* GetName() const override
	{
		return mBufferShader->GetName();
	}

	virtual int GetNumberOfBufferShaders() const override { return 1; }
	virtual PostEffectBufferShader* GetBufferShaderPtr(const int bufferShaderIndex) override { return mBufferShader.get(); }
	virtual const PostEffectBufferShader* GetBufferShaderPtr(const int bufferShaderIndex) const override { return mBufferShader.get(); }

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
