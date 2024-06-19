
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: BufferTexture.cpp
//
//	Author Sergei Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <cstring>
#include "BufferTexture.h"

#include "checkglerror.h"

///////////////////////////////////////////////////////////////////////////////
//

//! a constructor
BufferTexture::BufferTexture()
{}

//! a destructor
BufferTexture::~BufferTexture()
{
	if (mBufferId)
	{
		glDeleteBuffers( 1, &mBufferId );
	}

	if (mTextureId)
	{
		glDeleteTextures( 1, &mTextureId );
	}
}

GLuint		BufferTexture::GetCount() 
{ 
	return mCount; 
}

bool BufferTexture::SetUp(float *data, unsigned int Size)
{
	CHECK_GL_ERROR();

	mCount = Size;

	float *lData = data;
	unsigned int lSize = Size * 4 * sizeof(float); // we have 1d buffer, each element has 4 components
	if (lData == nullptr)
	{
		lData = new float[lSize];
		memset( lData, 0, sizeof(float) * lSize );
	}
        
	if (lSize != mSize)
	{
		if (mBufferId)
		{
			glDeleteBuffers(1, &mBufferId);
			mBufferId = 0;
		}
		if (mTextureId)
		{
			glDeleteTextures( 1, &mTextureId );
			mTextureId = 0;
		}
		mSize = lSize;
	}

	if (mTextureId == 0)
	{
		glGenBuffers(1, &mBufferId);

		glBindBuffer(GL_TEXTURE_BUFFER, mBufferId);
		glBufferData(GL_TEXTURE_BUFFER, lSize, lData, GL_STATIC_DRAW);
		glBindBuffer(GL_TEXTURE_BUFFER, 0);

		glGenTextures(1, &mTextureId);

		glBindTexture(GL_TEXTURE_BUFFER, mTextureId);
		glTexBuffer( GL_TEXTURE_BUFFER, GL_RGBA32F, mBufferId );
		glBindTexture(GL_TEXTURE_BUFFER, 0);
	}
	else
	{
		glBindBuffer(GL_TEXTURE_BUFFER, mBufferId);
		glBufferSubData(GL_TEXTURE_BUFFER, 0, lSize, lData);
		glBindBuffer(GL_TEXTURE_BUFFER, 0);
	}

	if (data == nullptr)
	{
		if (lData) 
			delete [] lData;
	}
    
	CHECK_GL_ERROR();
	return true;
}

void BufferTexture::Bind(int channelId)
{
	mTempChannelId = channelId;

	glActiveTexture(GL_TEXTURE0 + channelId);
	glBindTexture(GL_TEXTURE_BUFFER, mTextureId);
}

void BufferTexture::UnBind()
{
	glActiveTexture(GL_TEXTURE0 + mTempChannelId);
	glBindTexture(GL_TEXTURE_BUFFER, 0);
	glActiveTexture(GL_TEXTURE0);
}