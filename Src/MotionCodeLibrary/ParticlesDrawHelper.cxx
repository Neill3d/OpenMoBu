
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

#include "ParticlesDrawHelper.h"
#include "math3d.h"

#define _USE_MATH_DEFINES
#include <math.h>



//
//WGLFont		g_Font( NULL );	// init with NULL device context
GLUquadric	*g_pSphere = nullptr;
/*
void DrawString(int pX, int pY, const char* pText)
{
	g_Font.PositionText((float)pX, (float)pY);
	g_Font.glDrawText( pText, strlen(pText) );
}
*/
void DrawSphere(float radius, int segcount)
{
	if (!g_pSphere) g_pSphere = gluNewQuadric();
	if (g_pSphere) {
		gluQuadricDrawStyle( g_pSphere, GLU_FILL );
		gluQuadricOrientation( g_pSphere, GLU_OUTSIDE );
		gluSphere(g_pSphere, radius, segcount, segcount);
	}
}

void DrawCone(float base_radius, float top_radius, float height, int segcount)
{
	if (!g_pSphere) g_pSphere = gluNewQuadric();
	if (g_pSphere) {
		gluQuadricDrawStyle( g_pSphere, GLU_FILL );
		gluQuadricOrientation( g_pSphere, GLU_OUTSIDE );
		gluCylinder(g_pSphere, base_radius, top_radius, height, segcount, 1);
	}
}

void DrawHemisphere(double r, int lats, int longs) 
{
	const double PI = 3.14159265;

    int i, j;
    int halfLats = lats / 4; 
    for(i = 0; i <= halfLats; i++) 
    {
        double lat0 = PI * (-0.5 + (double) (i - 1) / lats);
        double z0 = sin(lat0) * r;
        double zr0 = cos(lat0) * r;

        double lat1 = PI * (-0.5 + (double) i / lats);
        double z1 = sin(lat1) * r;
        double zr1 = cos(lat1) * r;

        glBegin(GL_QUAD_STRIP);
        for(j = 0; j <= longs; j++)
        {
            double lng = 2 * PI * (double) (j - 1) / longs;
            double x = cos(lng) * r;
            double y = sin(lng) * r;

            glNormal3d(x * zr0, y * zr0, z0);
            glVertex3d(x * zr0, y * zr0, z0);       

            glNormal3d(x * zr1, y * zr1, z1);
            glVertex3d(x * zr1, y * zr1, z1);
        }
        glEnd();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void DrawCircle(const float radius, const int segs)
{
	float t = 0.0f;
	float maxt = 2.0f * (float) M_PI;
	float step = maxt / segs;
	FBVector3d W;

	glBegin(GL_LINE_LOOP);
	while(t < maxt)
	{
		W = FBVector3d(radius*cos(t), radius*sin(t), 0.0);

		glVertex3dv(W);
		t += step;
	}
	glEnd();
	
}

void DrawCircleBillboard(FBCamera *pCamera, const float radius, const int segs, const FBVector3d pos)
{
	float t = 0.0f;
	float maxt = 2.0f * (float) M_PI;
	float step = maxt / segs;
	
	FBVector3d W;
	FBMatrix m;
#ifdef ORSDK2013
	m = pCamera->GetMatrix(kFBModelView);
#else
	pCamera->GetCameraMatrix(m, kFBModelView);
#endif
	W = MatrixToEuler(m);
	glPushMatrix();
	{
		glTranslatef( pos[0], pos[1], pos[2] );
	
		glRotatef( -W[2], 1.0f, 0.0f, 0.0f );
		glRotatef( -W[1], 0.0f, 1.0f, 0.0f );
		glRotatef( -W[0], 0.0f, 0.0f, 1.0f );

		glBegin(GL_LINE_LOOP);
		while(t < maxt)
		{
			W = FBVector3d(radius*cos(t), radius*sin(t), 0.0);

			glVertex3dv(W);
			t += step;
		}
		glEnd();
	}
	glPopMatrix();
}

void DrawCircleBillboard(const float radius, const int segs, const FBMatrix m)
{
	float t = 0.0f;
	float maxt = 2.0f * 3.1415f;
	float step = maxt / segs;
	
	FBVector3d W;

	glPushMatrix();
	{
		glMultMatrixd(m);

		glBegin(GL_LINE_LOOP);
		while(t < maxt)
		{
			W[0] = radius*cos(t);
			W[1] = radius*sin(t);

			glVertex3dv(W);
			t += step;
		}
		glEnd();
	}
	glPopMatrix();
}

void DrawCircleBillboardFill(FBCamera *pCamera, const float radius, const int segs, const FBVector3d pos, const FBVector4d innerColor, const FBVector4d outerColor)
{
	float t = 0.0f;
	float maxt = 2.0f * (float) M_PI;
	float step = maxt / segs;
	
	FBVector3d W;
	FBMatrix m;

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#ifdef ORSDK2013
	m = pCamera->GetMatrix(kFBModelView);
#else
	pCamera->GetCameraMatrix(m, kFBModelView);
#endif
	W = MatrixToEuler(m);
	glPushMatrix();
	{
		glTranslatef( pos[0], pos[1], pos[2] );
	
		glRotatef( -W[2], 1.0f, 0.0f, 0.0f );
		glRotatef( -W[1], 0.0f, 1.0f, 0.0f );
		glRotatef( -W[0], 0.0f, 0.0f, 1.0f );

		glBegin(GL_TRIANGLE_FAN);

		glColor4dv( innerColor );
		glVertex3d( 0.0, 0.0, 0.0 );

		glColor4dv( outerColor );
		while(t < maxt)
		{
			W = FBVector3d(radius*cos(t), radius*sin(t), 0.0);

			glVertex3dv(W);
			t += step;
		}
		glEnd();
	}
	glPopMatrix();

	glDisable(GL_BLEND);
}

void drawOrthoQuad2d(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, w, 0.0, h, -1.0, 1.0); 
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//-------------------------
	
	glColor3d(1.0, 1.0, 1.0);
	glBegin(GL_QUADS);

	glTexCoord2d(0.0,		0.0);
		glVertex2d(0.0,			0.0);
	glTexCoord2d(1.0,		0.0);
		glVertex2d(w,	0.0);
	glTexCoord2d(1.0,		1.0);
		glVertex2d(w,	h);
	glTexCoord2d(0.0,		1.0);
		glVertex2d(0.0,			h);

	glEnd();
}



/////////////////////////////////////////////////////////////////////////////////////// ColorPropertyTexture
//

//! a constructor
ColorPropertyTexture::ColorPropertyTexture(const int textureWidth, FBPropertyAnimatableColorAndAlpha	*prop)
	: mTextureWidth(textureWidth)
	, mProp(prop)
{
	mTexId = 0;
}

//! a destructor
ColorPropertyTexture::~ColorPropertyTexture()
{
	Free();
}

void ColorPropertyTexture::Free()
{
	if (mTexId)
	{
		glDeleteTextures(1, &mTexId);
		mTexId = 0;
	}
}

// generate
bool ColorPropertyTexture::Generate(FBPropertyAnimatableColorAndAlpha	*prop)
{
//	CHECK_GL_ERROR();

	if (prop) mProp = prop;
	if (!mProp) return false;

	FBAnimationNode *pNode = mProp->GetAnimationNode();
	if (!pNode || (pNode->Nodes.GetCount() != 4)) return false;

	unsigned char *pixels = new unsigned char[mTextureWidth * 4];
	FBColorAndAlpha color;
	FBTime time;
	unsigned char r,g,b,a;

	for (int i=0; i<mTextureWidth; ++i)
	{
		time.SetSecondDouble( 1.0 / mTextureWidth * i );
		pNode->Evaluate( color, time );

		color[0] = clamp(color[0], 0.0, 1.0);
		color[1] = clamp(color[1], 0.0, 1.0);
		color[2] = clamp(color[2], 0.0, 1.0);
		color[3] = clamp(color[3], 0.0, 1.0);
		
		r = (unsigned char) (color[0] * 255.0);
		g = (unsigned char) (color[1] * 255.0);
		b = (unsigned char) (color[2] * 255.0);
		a = (unsigned char) (color[3] * 255.0);

		pixels[i*4] = r;
		pixels[i*4+1] = g;
		pixels[i*4+2] = b;
		pixels[i*4+3] = a;
	}

	glEnable(GL_TEXTURE_1D);
	if (mTexId == 0)
	{

		glGenTextures(1, &mTexId);
		glBindTexture(GL_TEXTURE_1D, mTexId);

		glTexImage1D( GL_TEXTURE_1D, 0, GL_RGBA, mTextureWidth, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels );
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_1D, 0);
	}
	else
	{
		glBindTexture(GL_TEXTURE_1D, mTexId);
		glTexSubImage1D( GL_TEXTURE_1D, 0, 0, mTextureWidth, GL_RGBA, GL_UNSIGNED_BYTE, pixels );
		glBindTexture(GL_TEXTURE_1D, 0);
	}
	glDisable(GL_TEXTURE_1D);

//	CHECK_GL_ERROR();

	// free mem
	if (pixels)
	{
		delete [] pixels;
		pixels = nullptr;
	}
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////// DoublePropertyTexture
//

//! a constructor
DoublePropertyTexture::DoublePropertyTexture(const int textureWidth, FBPropertyAnimatableDouble	*prop)
	: mTextureWidth(textureWidth)
	, mProp(prop)
{
	mTexId = 0;
}

//! a destructor
DoublePropertyTexture::~DoublePropertyTexture()
{
	Free();
}

void DoublePropertyTexture::Free()
{
	if (mTexId)
	{
		glDeleteTextures(1, &mTexId);
		mTexId = 0;
	}
}

// generate
bool DoublePropertyTexture::Generate(FBPropertyAnimatableDouble	*prop)
{
//	CHECK_GL_ERROR();

	if (prop) mProp = prop;
	if (!mProp) return false;

	FBAnimationNode *pNode = mProp->GetAnimationNode();
	if (!pNode) return false;

	float *values = new float[mTextureWidth];
	double v;
	FBTime time;

	for (int i=0; i<mTextureWidth; ++i)
	{
		time.SetSecondDouble( 1.0 / mTextureWidth * i );
		pNode->Evaluate( &v, time );

		values[i] = (float) v;
	}

	if (mTexId == 0)
	{

		glGenTextures(1, &mTexId);
		glBindTexture(GL_TEXTURE_1D, mTexId);

		//glTexStorage1D(GL_TEXTURE_1D, 1, GL_R32F, mTextureWidth);
        //glTexSubImage1D(GL_TEXTURE_1D, 0, 0, mTextureWidth, GL_RED, GL_FLOAT, values);
		glTexImage1D( GL_TEXTURE_1D, 0, GL_R32F, mTextureWidth, 0, GL_RED, GL_FLOAT, values );

		//glTexImage1D( GL_TEXTURE_1D, 0, GL_LUMINANCE32F_ARB, mTextureWidth, 0, GL_RED, GL_FLOAT, values );
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		
		glBindTexture(GL_TEXTURE_1D, 0);
	}
	else
	{
//		CHECK_GL_ERROR();

		glBindTexture(GL_TEXTURE_1D, mTexId);
		glTexSubImage1D( GL_TEXTURE_1D, 0, 0, mTextureWidth, GL_RED, GL_FLOAT, values );
		glBindTexture(GL_TEXTURE_1D, 0);
	}

//	CHECK_GL_ERROR();

	// free mem
	if (values)
	{
		delete [] values;
		values = nullptr;
	}
	return true;
}