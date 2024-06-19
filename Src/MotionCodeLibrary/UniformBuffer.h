
#pragma once

/*
	Sergei Solokhin (Neill3d)
	
	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
*/

//-- 
#include <GL\glew.h>

#ifdef USE_CG
#include <Cg\cgGL.h>
#endif

//////////////////////////////////////////////////////////////////////////////////////

///
/// There are two variant to share buffer in shader - SSBO or nVidia pointer
///
class CGPUBuffer
{
public:

	/// a constructor
	CGPUBuffer();

	/// a destructor
	virtual ~CGPUBuffer();

	virtual void Free();

	/// size in bytes, data is a pointer to the data struct
	virtual void UpdateData(const size_t elemSize, const size_t count, const void *buffer)
	{}

	virtual void UpdateData(const size_t elemSize, const size_t count1, const void *buffer1,
		const size_t count2, const void *buffer2)
	{
	}

	virtual void Bind(const GLuint unitId)
	{}
	virtual void UnBind()
	{}

	const size_t	GetSize() const
	{
		return mBufferSize;
	}
	const size_t	GetCount() const
	{
		return mBufferCount;
	}

	const GLuint GetBufferId() const {
		return mBuffer;
	}

protected:
	/// SSBO for texture addresses
	GLuint							mBuffer{ 0 }; //!< TODO: SSBO or texture buffer pointer

	size_t			mBufferSize{ 0 };	//!< one element size
	size_t			mBufferCount{ 0 };	//!< number of elements in the buffer
};

#ifdef USE_CG

// this buffer is auto binded to the specified cg parameter
//	only you need to do is to update buffer subdata

class UniformBufferCG : public CGPUBuffer
{
public:
	//! a constructor
	UniformBufferCG();

	//! a destructor
	~UniformBufferCG();

	// 
	bool InitGL(CGcontext cgcontext, CGprogram program, const char *paramname, const GLsizeiptr size);
	//void UploadBuffer( const GLintptr offset, const GLsizeiptr size, const void *buffer );
	void UpdateData(const GLintptr offset, const GLsizeiptr size, const void *buffer);

private:

	CGparameter					param{ nullptr };
	CGbuffer					buffer{ nullptr };
	//GLuint						bufferGL;

protected:

	void Free();

};

#endif

////////////////////////////////////////////////////////////////////////////////////////////////
/// ssbo array
class CGPUBufferSSBO : public CGPUBuffer
{
public:

	//! a constructor
	CGPUBufferSSBO();

	// a destructor
	virtual ~CGPUBufferSSBO();

	// size in bytes, data is a pointer to the data struct
	virtual void UpdateData(const size_t elemSize, const size_t count, const void *buffer) override;

	virtual void Bind(const GLuint unitId) override;
	virtual void UnBind() override
	{}

};

//////////////////////////////////////////////////////////////////////////////////////////////////
/// NV unified memory (pointer)
class CGPUBufferNV : public CGPUBuffer
{
public:
	//! a constructor
	CGPUBufferNV();

	// a destructor
	virtual ~CGPUBufferNV();

	// size in bytes, data is a pointer to the data struct
	virtual void UpdateData(const size_t size, const size_t count, const void *data) override;
	virtual void UpdateData(const size_t elemSize, const size_t count1, const void *buffer1,
		const size_t count2, const void *buffer2) override;

	virtual void BindAsAttribute(const GLuint attribIndex, const GLuint offset) const;
	virtual void BindAsUniform(const GLuint programId, const GLint uniformLoc, const GLuint offset) const;
	virtual void UnBind() override;

	void MakeBufferResident();

	const GLuint64 GetGPUPtr() const
	{
		return mBufferPtr;
	}

private:

	GLuint64		mBufferPtr{ 0 };
};

//////////////////////////////////////////////////////////////////////////////////////////////////
/// triple buffer order - evaluation, render, ready to use
class CGPUBufferDoubleNV
{
public:
	//! a constructor
	CGPUBufferDoubleNV();

	//! a destuctor
	virtual ~CGPUBufferDoubleNV();

	// size in bytes, data is a pointer to the data struct
	virtual void UpdateData(const size_t size, const size_t count, const void *data);

	void	UpdateGPUPtr();

	virtual void BindAsAttribute(const GLuint attribIndex, const GLuint offset);
	virtual void BindAsUniform(const GLuint programId, const GLint uniformLoc, const GLuint offset) const;
	virtual void UnBind()
	{}

	const GLuint64 GetGPUPtr() const
	{
		return mBufferPtr;
	}

	const int GetCount() const {
		return mCount;
	}

private:

	int				mCount{ 0 };

	long			mEvaluationId{ 0 };		//!< head buffer for evaluation
	long			mRenderId{ 1 };
	

	bool			mHasUpdated{ false };

	GLuint			mBuffers[2]{ 0 };

	GLuint64		mBufferPtr{ 0 };

	void		Free();
};