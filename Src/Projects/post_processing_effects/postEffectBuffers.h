#pragma once

#include "libraryMain.h"
#include "GL/glew.h"
#include "glslShader.h"
#include "Framebuffer.h"

#include "postEffectUtils.h"

#include <memory>

namespace PostProcessingEffects
{

    class POST_PROCESSING_EFFECTS_API PostEffectBuffers
    {
    public:

        //!< a constructor
        PostEffectBuffers();
        //!< a destructor
        ~PostEffectBuffers();

        void ChangeContext();

        bool ReSize(const int w, const int h, bool useScale, double scaleFactor, bool filterMips = false);

        bool Ok();

        const GLuint PrepAndGetBufferObject();

        SharedGraphicsLibrary::FrameBuffer* GetSrcBufferPtr();
        SharedGraphicsLibrary::FrameBuffer* GetDstBufferPtr();

        SharedGraphicsLibrary::FrameBuffer* GetBufferDepthPtr();
        SharedGraphicsLibrary::FrameBuffer* GetBufferBlurPtr();

        SharedGraphicsLibrary::FrameBuffer* GetBufferDownscalePtr();

        void SwapBuffers();

        const int GetWidth() const { return mWidth; }
        const int GetHeight() const { return mHeight; }
        const unsigned int GetPreviewWidth() const { return mPreviewWidth; }
        const unsigned int GetPreviewHeight() const { return mPreviewHeight; }

        // get a result of effect computation
        const GLuint GetFinalColor();
        const GLuint GetFinalFBO();

        const GLuint GetPreviewColor();
        const GLuint GetPreviewFBO();

        void		PreviewSignal() { mPreviewSignal = true; }

        //bool		PreviewCompressBegin();
        //bool		PreviewCompressEnd();
        //void		PrepPreviewCompressed();

        bool		PreviewOpenGLCompress(EImageCompression	compressionType, GLint& compressionCode);

        const GLuint GetPreviewCompressedColor();

        //void MapCompressedData(const float timestamp, Network::CPacketImageHeader &header);

        const size_t GetCompressedSize() const { return mCompressedSize; }
        const size_t GetUnCompressedSize() const { return mUnCompressSize; }
    protected:

        // DONE: double local buffer
        std::unique_ptr<SharedGraphicsLibrary::FrameBuffer>			mBufferPost0;
        std::unique_ptr<SharedGraphicsLibrary::FrameBuffer>			mBufferPost1;

        std::unique_ptr<SharedGraphicsLibrary::FrameBuffer>			mBufferDepth;	// buffer to store a linearize depth
        std::unique_ptr<SharedGraphicsLibrary::FrameBuffer>			mBufferBlur;

        std::unique_ptr<SharedGraphicsLibrary::FrameBuffer>			mBufferDownscale;	// output for a preview

        // last local buffers resize
        int								mWidth;
        int								mHeight;
        GLuint							mOutputColorObject;

        bool							mPreviewSignal;
        bool							mPreviewRunning;

        // downscaled size
        unsigned int					mPreviewWidth;
        unsigned int					mPreviewHeight;

        int			mSrc;
        int			mDst;

        // compressed ETC1 output texture
        GLenum							mCompressionInternal;
        GLenum							mCompressionFormat;
        GLenum							mCompressionType;
        GLuint							mCompressedPreviewId;

        GLuint							mCompressOnFlyId;

        int								mCurPBO{ 0 };
        GLuint							mPBOs[2]{ 0 };

        // temp
        int								mCurUnPack{ 0 };
        GLuint							mUnPackPBOs[2]{ 0 };

        size_t							mUnCompressSize{ 0 };
        size_t							mCompressedSize{ 0 };

        //CompressImageHeader			mCompressHeader;

        void		FreeBuffers();
        void        AllocPreviewTexture(int w, int h);
        void		FreeTextures();
    };
}