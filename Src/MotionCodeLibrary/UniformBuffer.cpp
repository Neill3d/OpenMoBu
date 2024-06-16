/*
	Sergei Solokhin (Neill3d)
		
	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
*/

#include <stdio.h>
#include "UniformBuffer.h"

#include "CheckGLError.h"
#include "Logger.h"

//////////////////////////////////////////////////////////////////////////////////////////////////
// UniformBufferCG

#ifdef USE_CG

UniformBufferCG::UniformBufferCG()
	: CGPUBuffer()
{
	param = nullptr;
	buffer = nullptr;
}

UniformBufferCG::~UniformBufferCG()
{
	Free();
}

bool UniformBufferCG::InitGL(CGcontext cgcontext, CGprogram program, const char *paramname, const GLsizeiptr size)
{
	param = cgGetNamedProgramUniformBuffer(program, paramname);
	if (param == nullptr)
		return false;

	//
	glGenBuffers(1, &mBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, mBuffer);
	glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	buffer = cgGLCreateBufferFromObject(cgcontext, mBuffer, GL_FALSE);
	cgSetUniformBufferParameter( param, buffer );

	return true;
}

void UniformBufferCG::UpdateData( const GLintptr offset, const GLsizeiptr size, const void *buffer )
{
	glBindBuffer(GL_UNIFORM_BUFFER, mBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, size, buffer );
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBufferCG::Free()
{
	CGPUBuffer::Free();

	if (buffer != nullptr)
	{
		cgDestroyBuffer( buffer );
		buffer = nullptr;
	}
	/*
	if (bufferGL > 0)
	{
		glDeleteBuffers( 1, &bufferGL );
		bufferGL = 0;
	}
	*/
}

#endif

//////////////////////////////////////////////////////////////////////////////////////////////////

CGPUBuffer::CGPUBuffer()
{
	mBuffer = 0;
	mBufferSize = 0;
	mBufferCount = 0;
}

void CGPUBuffer::Free()
{
	if (mBuffer > 0)
	{
		glDeleteBuffers(1, &mBuffer);
		mBuffer = 0;
	}
}

CGPUBuffer::~CGPUBuffer()
{
	Free();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// SSBO

CGPUBufferSSBO::CGPUBufferSSBO()
	: CGPUBuffer()
{
}

CGPUBufferSSBO::~CGPUBufferSSBO()
{
	Free();
}

void CGPUBufferSSBO::UpdateData(const size_t _size, const size_t _count, const void *data)
{

	if (mBuffer == 0)
	{
		glGenBuffers( 1, &mBuffer );
		CHECK_GL_ERROR();
	}

	// update data in SSBO
	if (mBuffer > 0 && _size > 0 && _count > 0)
	{
		GLsizeiptr size = (GLsizeiptr) (_size * _count);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, mBuffer);
		glBufferData( GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_STATIC_DRAW );
		glBufferData( GL_SHADER_STORAGE_BUFFER, size, data, GL_STATIC_DRAW );
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0 );
		CHECK_GL_ERROR();
	}

	mBufferSize = _size;
	mBufferCount = _count;
}

void CGPUBufferSSBO::Bind(const GLuint unitId)
{
	if (mBuffer > 0)
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, unitId, mBuffer);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// NV

CGPUBufferNV::CGPUBufferNV()
	: CGPUBuffer()
{
	mBufferPtr = 0;
}

CGPUBufferNV::~CGPUBufferNV()
{
	Free();
}

void CGPUBufferNV::UnBind()
{
	/*
	if (mBufferPtr > 0)
	{
		glBindBuffer(GL_UNIFORM_BUFFER, mBuffer);
		glMakeBufferNonResidentNV(GL_UNIFORM_BUFFER);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
	*/
}

void CGPUBufferNV::MakeBufferResident()
{
	/*
	if (mBufferPtr > 0)
	{
		glBindBuffer(GL_UNIFORM_BUFFER, mBuffer);
		glMakeBufferResidentNV(GL_UNIFORM_BUFFER, GL_READ_ONLY);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
	*/
}

void CGPUBufferNV::UpdateData(const size_t _size, const size_t _count, const void *data)
{

	if (mBuffer == 0)
	{
		glGenBuffers( 1, &mBuffer );
		CHECK_GL_ERROR();
	}

	// update data in SSBO
	if (mBuffer > 0 && _size > 0 && _count > 0)
	{
		GLsizeiptr	size = (GLsizeiptr) (_size * _count);
		// Give the uniform data to the GPU
		
		//glNamedBufferDataEXT(mBuffer, size, data, GL_STREAM_DRAW);


		// Get the GPU pointer for the per mesh uniform buffer and make the buffer resident on the GPU
		// For bindless uniforms, this GPU pointer will later be passed to the vertex shader via a 
		//   vertex attribute. The vertex shader will then directly use the GPU pointer to access the uniform data.

		GLint uniformsDataSize;
		glBindBuffer(GL_UNIFORM_BUFFER, mBuffer);
		
		// Orphaning
		glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STREAM_DRAW);
		glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STREAM_DRAW);

		glGetBufferParameterui64vNV(GL_UNIFORM_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &mBufferPtr);
		glGetBufferParameteriv(GL_UNIFORM_BUFFER, GL_BUFFER_SIZE, &uniformsDataSize);
		glMakeBufferResidentNV(GL_UNIFORM_BUFFER, GL_READ_ONLY);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		if (mBufferPtr == 0 || uniformsDataSize != size)
		{
			LOGE( "> ERROR: per mesh uniform size is not equal to the struct size!\n" );
		}

		CHECK_GL_ERROR();
	}

	mBufferSize = _size;
	mBufferCount = _count;
}

void CGPUBufferNV::UpdateData(const size_t elemSize, const size_t count1, const void *buffer1,
		const size_t count2, const void *buffer2)
{
	bool firstTime = false;

	// update data in SSBO
	if (elemSize == 0 || count1 == 0)
	{
		mBufferPtr = 0;

		glDeleteBuffers(1, &mBuffer);
		mBuffer = 0;
		mBufferCount = 0;
	}
	else
	if (elemSize > 0 && count1 > 0)
	{

		if (mBufferCount != (count1+count2) )
		{
			mBufferPtr = 0;

			glDeleteBuffers(1, &mBuffer);
			mBuffer = 0;
			mBufferCount = 0;
		}

		if (mBuffer == 0)
		{
			glGenBuffers( 1, &mBuffer );
			CHECK_GL_ERROR();

			firstTime = true;
		}


		GLsizeiptr	size1 = (GLsizeiptr) (elemSize * count1);
		GLsizeiptr	size2 = (GLsizeiptr) (elemSize * count2);

		
		// Get the GPU pointer for the per mesh uniform buffer and make the buffer resident on the GPU
		// For bindless uniforms, this GPU pointer will later be passed to the vertex shader via a 
		//   vertex attribute. The vertex shader will then directly use the GPU pointer to access the uniform data.

		//GLint uniformsDataSize;
		glBindBuffer(GL_UNIFORM_BUFFER, mBuffer);

		// Give the uniform data to the GPU
		if (firstTime)
		{
			//glBufferStorage(GL_UNIFORM_BUFFER, size1+size2, nullptr, GL_STREAM_DRAW);
			glBufferData(GL_UNIFORM_BUFFER, size1+size2, nullptr, GL_STREAM_DRAW);
			glGetBufferParameterui64vNV(GL_UNIFORM_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &mBufferPtr);
		}
		else
		{
			glMakeBufferNonResidentNV(GL_UNIFORM_BUFFER);
		}

		glBufferSubData(GL_UNIFORM_BUFFER, 0, size1, buffer1);

		if (count2 > 0)
			glBufferSubData(GL_UNIFORM_BUFFER, size1, size2, buffer2);

		//glGetBufferParameteriv(GL_UNIFORM_BUFFER, GL_BUFFER_SIZE, &uniformsDataSize);
		glMakeBufferResidentNV(GL_UNIFORM_BUFFER, GL_READ_ONLY);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		if (mBufferPtr == 0)
		{
			LOGE( "> ERROR: empty nv buffer pointer!\n" );
		}

		CHECK_GL_ERROR();

		mBufferSize = elemSize;
		mBufferCount = count1 + count2;
	}
}

void CGPUBufferNV::BindAsAttribute(const GLuint attribIndex, const GLuint offset) const
{
	if (mBufferPtr > 0)
	{
		const GLuint64 uniformGPUPtr = mBufferPtr + offset;
		glVertexAttribI2iEXT( attribIndex, (int)(uniformGPUPtr & 0xFFFFFFFF), (int) (uniformGPUPtr>>32) & (0xFFFFFFFF) );
	}
}

void CGPUBufferNV::BindAsUniform(const GLuint programId, const GLint uniformLoc, const GLuint offset) const
{
	if (uniformLoc >= 0)
	{
		GLint lprogram = programId;
		if (lprogram == 0)
			glGetIntegerv( GL_CURRENT_PROGRAM, &lprogram );

		if (mBufferPtr > 0)
		{
			GLuint64 uniformGPUPtr = mBufferPtr + offset;
			glProgramUniform1ui64NV( lprogram, uniformLoc, uniformGPUPtr );		
		}
		else
		{
			CHECK_GL_ERROR();
			glProgramUniform1ui64NV( lprogram, uniformLoc, 0 );		
			CHECK_GL_ERROR();
		}
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////
// Triple NV

CGPUBufferDoubleNV::CGPUBufferDoubleNV()
{
	mEvaluationId = 0;
	mRenderId = 1;

	mBuffers[0]=0;
	mBuffers[1]=0;

	mHasUpdated = false;

	mBufferPtr = 0;
	mCount = 0;

}

CGPUBufferDoubleNV::~CGPUBufferDoubleNV()
{
	Free();
}

void CGPUBufferDoubleNV::Free()
{
	if (mBuffers[0] > 0)
	{
		glDeleteBuffers(2, &mBuffers[0]);
		mBuffers[0] = 0;
		mBuffers[1] = 0;
	}
	mCount = 0;
	mBufferPtr = 0;
}

void CGPUBufferDoubleNV::UpdateData(const size_t _size, const size_t _count, const void *data)
{

	if (mBuffers[0] == 0)
	{
		glGenBuffers( 2, &mBuffers[0] );
		CHECK_GL_ERROR();
	}

	// swap free and evaluation buffer
	
	if (mBuffers[mEvaluationId] > 0 && _count > 0 && _size > 0)
	{
		GLsizeiptr size = (GLsizeiptr) (_count * _size);
		// Give the uniform data to the GPU
		//glNamedBufferDataEXT(mBuffers[mEvaluationId], size, data, GL_STREAM_DRAW);

		// Orphaning
		glNamedBufferData(mBuffers[mEvaluationId], size, nullptr, GL_STREAM_DRAW);
		glNamedBufferData(mBuffers[mEvaluationId], size, data, GL_STREAM_DRAW);
	}

	mHasUpdated = true;
	mCount = (int) _count;
}

void CGPUBufferDoubleNV::UpdateGPUPtr()
{
	if (mBuffers[0] == 0) return;

	// swap free and render buffer
	
	//mCriticalSection.Enter();
	
	if (mHasUpdated)
	{
		long temp = mEvaluationId;
		mEvaluationId = mRenderId;
		mRenderId = temp;
	}

	mHasUpdated = false;

	//mCriticalSection.Leave();

	// Get the GPU pointer for the per mesh uniform buffer and make the buffer resident on the GPU
	// For bindless uniforms, this GPU pointer will later be passed to the vertex shader via a 
	//   vertex attribute. The vertex shader will then directly use the GPU pointer to access the uniform data.

	GLint uniformsDataSize;
	glBindBuffer(GL_UNIFORM_BUFFER, mBuffers[mRenderId]);
	glGetBufferParameterui64vNV(GL_UNIFORM_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &mBufferPtr);
	glGetBufferParameteriv(GL_UNIFORM_BUFFER, GL_BUFFER_SIZE, &uniformsDataSize);
	glMakeBufferResidentNV(GL_UNIFORM_BUFFER, GL_READ_ONLY);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	/*
	if (uniformsDataSize != size)
	{
		LOGE( "> ERROR: per mesh uniform size is not equal to the struct size!\n" );
	}
	*/
	CHECK_GL_ERROR();
}

void CGPUBufferDoubleNV::BindAsAttribute(const GLuint attribIndex, const GLuint offset)
{
	if (mBufferPtr > 0)
	{
		const GLuint64 uniformGPUPtr = mBufferPtr + offset;
		glVertexAttribI2iEXT( attribIndex, (int)(uniformGPUPtr & 0xFFFFFFFF), (int) (uniformGPUPtr>>32) & (0xFFFFFFFF) );
	}
}

void CGPUBufferDoubleNV::BindAsUniform(const GLuint programId, const GLint uniformLoc, const GLuint offset) const
{
	if (mBufferPtr > 0 && uniformLoc >= 0)
	{
		GLint lprogram = programId;
		if (lprogram == 0)
			glGetIntegerv( GL_CURRENT_PROGRAM, &lprogram );

		GLuint64 uniformGPUPtr = mBufferPtr + offset;
		glProgramUniform1ui64NV( lprogram, uniformLoc, uniformGPUPtr );		
	}
}