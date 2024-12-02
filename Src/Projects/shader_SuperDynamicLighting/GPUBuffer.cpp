
// GPUBuffer.cpp
/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "GPUBuffer.h"
#include "CheckGLError.h"
#include "mobu_logging.h"

namespace Graphics {

	//////////////////////////////////////////////////////////////////////////////////////////////////

	GpuBuffer::GpuBuffer()
	{
		bufferId = 0;
		bufferSize = 0;
		bufferCount = 0;
	}

	void GpuBuffer::Free()
	{
		if (bufferId > 0)
		{
			glDeleteBuffers(1, &bufferId);
			bufferId = 0;
		}
	}

	GpuBuffer::~GpuBuffer()
	{
		Free();
	}

	void GpuBuffer::UpdateData(std::size_t elemSize, std::size_t count, const void* buffer)
	{}

	void GpuBuffer::UpdateData(std::size_t elemSize, std::size_t count1, const void* buffer1,
		std::size_t count2, const void* buffer2)
	{}

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// SSBO

	GPUBufferSSBO::GPUBufferSSBO()
		: GpuBuffer()
	{
	}

	GPUBufferSSBO::~GPUBufferSSBO()
	{
		Free();
	}

	void GPUBufferSSBO::UpdateData(std::size_t elemSize, std::size_t count, const void* buffer)
	{
		if (elemSize == 0 || count == 0 || buffer == nullptr) {
			// Log a warning or handle invalid input as needed
			LOGE("[CGPUBufferSSBO] UpdateData received wrong arguments");
			return;
		}

		if (bufferId == 0)
		{
			glGenBuffers(1, &bufferId);
			CHECK_GL_ERROR();

			if (bufferId == 0) {
				LOGE("[CGPUBufferSSBO] UpdateData failed to create a buffer");
				return;
			}
		}

		// update data in SSBO
		if (bufferId > 0 && elemSize > 0 && count > 0)
		{
			const GLsizeiptr size = (GLsizeiptr)(elemSize * count);

			if (lastBufferSize != size)
			{
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferId);
				glBufferData(GL_SHADER_STORAGE_BUFFER, size, buffer, GL_STATIC_DRAW);
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

				lastBufferSize = size;
			}
			else
			{
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferId);
				glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, size, buffer);
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			}
			
			CHECK_GL_ERROR();
		}

		bufferSize = elemSize;
		bufferCount = count;
	}

	void GPUBufferSSBO::Bind(const GLuint unitId) const
	{
		if (bufferId > 0)
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, unitId, bufferId);
		}
	}

};