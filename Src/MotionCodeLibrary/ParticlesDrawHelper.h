
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: ParticlesDrawHelper.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <GL\glew.h>

//--- SDK include
#include <fbsdk/fbsdk.h>


//void DrawString(int pX, int pY, const char* pText);

void DrawCircle(const float radius, const int segs);
void DrawCircleBillboard(FBCamera *pCamera, const float radius, const int segs, const FBVector3d pos);
void DrawCircleBillboard(const float radius, const int segs, const FBMatrix m);
void DrawCircleBillboardFill(FBCamera *pCamera, const float radius, const int segs, const FBVector3d pos, const FBVector4d innerColor, const FBVector4d outerColor);

void DrawSphere(float radius, int segcount );
void DrawHemisphere(double r, int lats, int longs);
void DrawCone(float base_radius, float top_radius, float height, int segcount);

////////////////////////////////////////////////////////////////////////////////////////////////////////
// generate 1D texture from animatable color and alpha property
class ColorPropertyTexture
{
public:
	//! a constructor
	ColorPropertyTexture(const int textureWidth=128, FBPropertyAnimatableColorAndAlpha	*prop=nullptr);
	//! a destructor
	~ColorPropertyTexture();

	// generate
	bool Generate(FBPropertyAnimatableColorAndAlpha	*prop=nullptr);
	void Free();

	GLuint		GetTextureId() {
		return mTexId;
	}
	FBPropertyAnimatableColorAndAlpha *GetProperty()
	{
		return mProp;
	}
	void SetUp(FBPropertyAnimatableColorAndAlpha	*prop)
	{
		mProp = prop;
	}

private:
	int			mTextureWidth;	// number of samplers evaluated into the texture

	int			mWidth;
	GLuint		mTexId;

	FBPropertyAnimatableColorAndAlpha	*mProp;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////
// generate 1D texture from animatable double property
class DoublePropertyTexture
{
public:
	//! a constructor
	DoublePropertyTexture(const int textureWidth=128, FBPropertyAnimatableDouble	*prop=nullptr);
	//! a destructor
	~DoublePropertyTexture();

	// generate
	bool Generate(FBPropertyAnimatableDouble	*prop=nullptr);
	void Free();

	GLuint		GetTextureId() {
		return mTexId;
	}
	FBPropertyAnimatableDouble *GetProperty()
	{
		return mProp;
	}
	void SetUp(FBPropertyAnimatableDouble	*prop)
	{
		mProp = prop;
	}

private:
	int			mTextureWidth;

	int			mWidth;
	GLuint		mTexId;

	FBPropertyAnimatableDouble		*mProp;
};