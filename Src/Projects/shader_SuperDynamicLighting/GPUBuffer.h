
#pragma once

// GPUBuffer.h
/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "glslShaderProgram.h"
#include <cstddef>

namespace Graphics
{
	
    /**
     * @class GpuBuffer
     * @brief A base class for managing GPU buffer resources.
     *
     * The GpuBuffer class provides a common interface for GPU buffer operations,
     * such as binding, unbinding, and updating data. It serves as a base for
     * specialized buffer classes, allowing them to manage data in GPU memory.
     */
    class GpuBuffer {
    public:
        /**
         * @brief Constructs a new GpuBuffer instance.
         */
        GpuBuffer();

        /**
         * @brief Destroys the GpuBuffer, freeing GPU resources.
         */
        virtual ~GpuBuffer();

        /**
         * @brief Frees the allocated GPU buffer resources.
         */
        virtual void Free();

        /**
         * @brief Updates the buffer with new data.
         *
         * @param elemSize Size of each element in bytes.
         * @param count Number of elements to update.
         * @param buffer Pointer to the data to upload to the GPU buffer.
         */
        virtual void UpdateData(std::size_t elemSize, std::size_t count, const void* buffer);

        /**
         * @brief Updates the buffer with two separate data sources.
         *
         * This method can be used when data is split into two buffers, for example
         * for partial or structured updates.
         *
         * @param elemSize Size of each element in bytes.
         * @param count1 Number of elements in the first buffer.
         * @param buffer1 Pointer to the first data buffer.
         * @param count2 Number of elements in the second buffer.
         * @param buffer2 Pointer to the second data buffer.
         */
        virtual void UpdateData(std::size_t elemSize, std::size_t count1, const void* buffer1,
            std::size_t count2, const void* buffer2);

        /**
         * @brief Binds the buffer to a GPU unit.
         *
         * @param unitId GPU unit identifier.
         */
        virtual void Bind(GLuint unitId) const = 0;

        /**
         * @brief Unbinds the buffer from the GPU unit.
         */
        virtual void UnBind() const = 0;

        /**
         * @brief Gets the total size of the buffer in bytes.
         *
         * @return Size of the buffer in bytes.
         */
        std::size_t GetSize() const { return bufferSize; }

        /**
         * @brief Gets the count of elements in the buffer.
         *
         * @return Number of elements in the buffer.
         */
        std::size_t GetCount() const { return bufferCount; }

        /**
         * @brief Gets the GPU buffer ID.
         *
         * @return OpenGL buffer ID.
         */
        GLuint GetBufferId() const { return bufferId; }

    protected:
        GLuint bufferId;    ///< GPU buffer ID.
        std::size_t bufferSize;   ///< Size of one element in bytes.
        std::size_t bufferCount;  ///< Number of elements in the buffer.
    };


    /**
     * @class GPUBufferSSBO
     * @brief A class representing a Shader Storage Buffer Object (SSBO) for GPU data storage and management.
     *
     * This class provides an interface for managing GPU-side buffer objects used in
     * shader programs for handling large amounts of structured data. The class encapsulates
     * OpenGL buffer operations such as allocation, data updates, and binding.
     *
     * Key functionalities include:
     * - Creating and freeing GPU buffers.
     * - Updating buffer data with new content.
     * - Binding and unbinding the buffer to specific OpenGL binding points.
     *
     * Example usage:
     * @code
     * GPUBufferSSBO buffer;
     * buffer.UpdateData(sizeof(Vertex), vertexCount, vertexData);
     * buffer.Bind(bindingPoint);
     * @endcode
     *
     * @note This class assumes that an OpenGL context is properly initialized before use.
     *
     * @see https://www.khronos.org/opengl/wiki/Shader_Storage_Buffer_Object for more details on SSBOs.
     */
	class GPUBufferSSBO : public GpuBuffer
	{
	public:

        /**
         * @brief Constructs a new GPUBufferSSBO instance.
         */
		GPUBufferSSBO();

        /**
         * @brief Destroys the GpuBuffer, freeing GPU resources.
         */
		virtual ~GPUBufferSSBO();

        /**
         * @brief Updates the buffer with new data.
         *
         * @param elemSize Size of each element in bytes.
         * @param count Number of elements to update.
         * @param buffer Pointer to the data to upload to the GPU buffer.
         */
		virtual void UpdateData(std::size_t elemSize, std::size_t count, const void* buffer) override;

        /**
         * @brief Binds the buffer to a GPU unit.
         *
         * @param unitId GPU unit identifier.
         */
		virtual void Bind(const GLuint unitId) const override;

        /**
         * @brief Unbinds the buffer from the GPU unit.
         */
		virtual void UnBind() const override
		{}

    private:

        GLsizeiptr lastBufferSize{ 0 }; //!< compare with previous data size in order to do a partial update

	};
};