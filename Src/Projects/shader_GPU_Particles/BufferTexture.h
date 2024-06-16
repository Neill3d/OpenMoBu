#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: BufferTexture.h
//
//	Author Sergei Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "GL\glew.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
//
class BufferTexture
{
private:
	GLuint						mSize;		//!< size of buffer array in bytes
	GLuint						mCount;		//!< number of vertices which generate particles
	GLuint						mBufferId;
	GLuint						mTextureId;

	int							mTempChannelId;
public:
	//! a constructor
	BufferTexture();

	//! a destructor
	~BufferTexture();

	GLuint		GetCount();

	bool SetUp(float *data, unsigned int Size);

	void Bind(int channelId);
	void UnBind();
};