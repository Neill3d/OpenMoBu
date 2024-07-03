
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: ParticleSystem_Generation.cpp
//
//	Author Sergei Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include <Windows.h>
#include "ParticleSystem.h"
#include "checkglerror.h"
#include "FileUtils.h"
#include "nv_math.h"
#include "math3d.h"

#include "ParticleSystem_types.h"

using namespace GPUParticles;

#define _USE_MATH_DEFINES
#include <math.h>

////////////////////////////////////////////////////////////////////////////
// declaration

struct SurfaceColor
{
	BYTE		red;
	BYTE		green;
	BYTE		blue;
	BYTE		alpha;
};

///////////////////////////////////////////////////////////
//
// C++ offers `modf (...)`, which does the same thing, but this is simpler.
float fract (float f) {
  return f-(long)f;
}

nv::vec4 fract(const nv::vec4 &v)
{
	return nv::vec4( fract(v.x), fract(v.y), fract(v.z), fract(v.w) );
}

nv::vec4 Color_UnPack (float x)
{
	float a,b,c,d;
	a = floor(x*255.0/64.0)*64.0/255.0;
	x -= a;
	b = floor(x*255.0/16.0)*16.0/255.0;
	x -= b;
	b *= 4.0;
	c = floor(x*255.0/4.0)*4.0/255.0;
	x -= c;
	c *= 16.0;
	d = x*255.0 * 64.0 / 255.0; // scan be simplified to just x*64.0
			
	return nv::vec4(a,b,c,d);
}


float Color_Pack (const nv::vec4 &colour)
{
	float x = 1.0/255.0 * (floor(colour.x*255.0/64.0)*64.0 + floor(colour.y*255.0/64.0)*16.0 + floor(colour.z*255.0/64.0)*4.0 + floor(colour.w*255.0/64.0));
	return x;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//

void ParticleSystem::GetRandomVolumePos(const bool local, nv::vec4 &pos)
{
	using namespace nv;
	vec3 lMax = mEvaluateData.gMax;
	vec3 lMin = mEvaluateData.gMin;
			
	// if inherit emitter velocity, put particle on some random position during the way
	/*
	if (mEvaluateData.gEmitterVelocity.w > 0.0)
	{
		double randomValue = dist(e2);
		vec3 delta = mEvaluateData.gEmitterVelocity;
		delta *= (float)randomValue;
		lMax -= delta;
		lMin -= delta;
	}
	*/
	vec3 rnd = vec3( (float)dist(e2), (float)dist(e2), (float)dist(e2)); 
			
	pos.x = (lMax.x - lMin.x) * rnd.x + lMin.x;
	pos.y = (lMax.y - lMin.y) * rnd.y + lMin.y;
	pos.z = (lMax.z - lMin.z) * rnd.z + lMin.z;

	if (local == false)
		pos = mEvaluateData.gTM * pos;
}

/*
 r >= 0
 0 <= theta <= 180 deg (Pi)
 0 <= phi <= 360 deg (2Pi)
*/

void ParticleSystem::ConvertUnitVectorToSpherical(const nv::vec4 &v, float &r, float &theta, float &phi)
{
	r = sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
	theta = atan2( v.y, v.x );
	phi = atan2( sqrt(v.x*v.x + v.y*v.y), v.z );
}

void ParticleSystem::ConvertSphericalToUnitVector(const float r, const float theta, const float phi, nv::vec4 &v)
{
	v.x = r * cos(theta) * sin(phi);
	v.y = r * sin(theta) * sin(phi);
	v.z = r * cos(phi);
}

void ParticleSystem::GetRandomDir(const nv::vec4 &dir, const float randomH, const float randomV, nv::vec4 &outdir)
{
	float r, theta, phi;

	ConvertUnitVectorToSpherical(dir, r, theta, phi);

	const float PiPi = 2.0f * (float)M_PI;

	theta += randomH * M_PI;
	phi += randomV * PiPi;
	
	ConvertSphericalToUnitVector(r, theta, phi, outdir);
}

float ParticleSystem::GetRandomSpeed()
{
	float rnd = 2.0 * mEvaluateData.gEmitSpeed * dist(e2) - mEvaluateData.gEmitSpeed;
	return (mEvaluateData.gEmitSpeed - rnd * mEvaluateData.gSpeedSpread);
}

void ParticleSystem::GetRandomVolumeDir(nv::vec4 &vel)
{
	const float randomH = mEvaluateData.gDirSpreadHor * (float) dist(e2);
	const float randomV = mEvaluateData.gDirSpreadVer * (float) dist(e2);
	GetRandomDir(mEvaluateData.gDirection, randomH, randomV, vel);
}  

void ParticleSystem::GetRandomVolumeColor(const nv::vec4 &pos, nv::vec4 &color)
{
	using namespace nv;
	vec3 lMax = mEvaluateData.gMax;
	vec3 lMin = mEvaluateData.gMin;
	
	color.x = (pos.x - lMin.x) / (lMax.x - lMin.x);
	color.y = (pos.y - lMin.y) / (lMax.y - lMin.y);
	color.z = (pos.z - lMin.z) / (lMax.z - lMin.z);
	color.w = 1.0f;
}

void ParticleSystem::GetRandomVerticesPos(const bool local, nv::vec4 &pos, int &vertIndex)
{
	if (mSurfaceData.size() == 0)
		return;

	const int triCount = mSurfaceData.size();

	float rnd = (float) triCount * dist(e2);
	int triIndex = (int) rnd;

	_ASSERT (triIndex >= 0 || triIndex < triCount);

	if (triIndex >= triCount)
		triIndex = 0;

	rnd = dist(e2);

	if (rnd < 0.33f)
	{
		pos = mSurfaceData[triIndex].p0;
		vertIndex = triIndex * 3;
	}
	else if (rnd < 0.66f)
	{
		pos = mSurfaceData[triIndex].p1;
		vertIndex = triIndex * 3 + 1;
	}
	else
	{
		pos = mSurfaceData[triIndex].p2;
		vertIndex = triIndex * 3 + 2;
	}
	
	if (local == false)
		pos = mEvaluateData.gTM * pos;
}

void ParticleSystem::GetRandomVerticesDir(const int vertIndex, nv::vec4 &vel)
{
	using namespace nv;
	const int triIndex = vertIndex / 3;
	vec4 indir = mSurfaceData[triIndex].n;

	const float randomH = mEvaluateData.gDirSpreadHor * (float) dist(e2);
	const float randomV = mEvaluateData.gDirSpreadVer * (float) dist(e2);
	GetRandomDir(indir, randomH, randomV, vel);
}   

void ParticleSystem::GetRandomVerticesColor(const int vertIndex, nv::vec4 &color)
{
	using namespace nv;
	if ( 0 == mSurfaceData.size() )
		return;

	int triIndex = vertIndex / 3;

	if (triIndex < 0)
		triIndex = 0;
	if (triIndex >= mSurfaceData.size() )
		triIndex = mSurfaceData.size()-1;

	if (mSurfaceTextureId == 0 )
	{
		color = mSurfaceData[triIndex].n;
	}
	else if (mSurfaceTextureData.size() > 0)
	{
		// read from texture image

		vec2 uv = mSurfaceData[triIndex].uv0;
		
		int x = (int) (uv.x * mSurfaceTextureInfo.width);
		int y = (int) (uv.y * mSurfaceTextureInfo.height);

		x -= 1;
		y -= 1;

		if (x < 0) 
		{
			x = 0;
		}
		else if (x >= mSurfaceTextureInfo.width)
		{
			x = mSurfaceTextureInfo.width - 1;
		}
		if (y < 0) 
		{
			y = 0;
		}
		else if (y >= mSurfaceTextureInfo.height)
		{
			y = mSurfaceTextureInfo.height - 1;
		}

		int pixelSize = mSurfaceTextureInfo.GetPixelMemorySize();

		SurfaceColor *pColor = (SurfaceColor*) &mSurfaceTextureData[ (y * mSurfaceTextureInfo.width + x) * pixelSize ];

		if (pixelSize > 3)
			color.w = 1.0f * pColor->alpha / 256.0f;
		else
			color.w = 1.0f;
		color.x = 1.0f * pColor->red / 256.0f;
		color.y = 1.0f * pColor->green / 256.0f;
		color.z = 1.0f * pColor->blue / 256.0f;
	}

	// TODO: emitter mask ?!
}

void ParticleSystem::GetRandomSurfacePos(const bool local, const double extrudeDist, nv::vec4 &pos, int &vertIndex, float &r1, float &r2, float &r3)
{
	using namespace nv;
	if (mSurfaceData.size() == 0)
		return;

	const int triCount = mSurfaceData.size();

	float randomF = dist(e2);
	float rnd = (float) triCount * randomF;
	int triIndex = (int) rnd;

	// barycentric coords
	float rnd1 = dist(e2);
	float rnd2 = dist(e2);

	r1 = 1.0f - sqrt(rnd1);
	r2 = sqrt(rnd1) * (1.0f - rnd2);
	r3 = sqrt(rnd1) * rnd2; 

	vec3 p0 = mSurfaceData[triIndex].p0;
	vec3 p1 = mSurfaceData[triIndex].p1;
	vec3 p2 = mSurfaceData[triIndex].p2;

	vec3 cp;
    //vec3 e0(p1.x - p0.x, p1.y - p0.y, p1.z - p0.z);
    //vec3 e1(p2.x - p0.x, p2.y - p0.y, p2.z - p0.z);

	cp = normalize(mSurfaceData[triIndex].n);

    //cross(cp,e0,e1);
	//normalize(cp);

	p0 *= r1;
	p1 *= r2;
	p2 *= r3;

	vec3 P = p0 + p1 + p2;
	vertIndex = triIndex * 3;

	pos = P;
	
	if (local == false)
	{
		pos = mEvaluateData.gTM * pos;
		cp = mEvaluateData.gNormalTM * cp;	
	}
	
	// extrudeDist direction, in local direction

	cp *= extrudeDist * dist(e2);
	pos += cp;
}

void ParticleSystem::GetRandomSurfaceDir(const int vertIndex, nv::vec4 &vel)
{
	using namespace nv;
	const int triIndex = vertIndex / 3;
	vec4 indir = mSurfaceData[triIndex].n;
	
	const float randomH = mEvaluateData.gDirSpreadHor * (float) dist(e2);
	const float randomV = mEvaluateData.gDirSpreadVer * (float) dist(e2);
	GetRandomDir(indir, randomH, randomV, vel);
}   



void ParticleSystem::GetRandomSurfaceColor(const int vertIndex, float r1, float r2, float r3, nv::vec4 &color)
{
	using namespace nv;
	if ( 0 == mSurfaceData.size() )
		return;

	int triIndex = vertIndex / 3;

	if (triIndex < 0)
		triIndex = 0;
	if (triIndex >= mSurfaceData.size() )
		triIndex = mSurfaceData.size()-1;

	if (mSurfaceTextureId == 0 )
	{
		color = mSurfaceData[triIndex].n;
	}
	else if (mSurfaceTextureData.size() > 0)
	{
		// read from texture image

		vec2 uv0 = mSurfaceData[triIndex].uv0;
		vec2 uv1 = mSurfaceData[triIndex].uv1;
		vec2 uv2 = mSurfaceData[triIndex].uv2;

		uv0 *= r1;
		uv1 *= r2;
		uv2 *= r3;

		vec2 uv = uv0 + uv1 + uv2;

		int x = (int) (uv.x * mSurfaceTextureInfo.width);
		int y = (int) (uv.y * mSurfaceTextureInfo.height);

		x -= 1;
		y -= 1;

		if (x < 0) 
		{
			x = 0;
		}
		else if (x >= mSurfaceTextureInfo.width)
		{
			x = mSurfaceTextureInfo.width - 1;
		}
		if (y < 0) 
		{
			y = 0;
		}
		else if (y >= mSurfaceTextureInfo.height)
		{
			y = mSurfaceTextureInfo.height - 1;
		}

		int pixelSize = mSurfaceTextureInfo.GetPixelMemorySize();

		SurfaceColor *pColor = (SurfaceColor*) &mSurfaceTextureData[ (y * mSurfaceTextureInfo.width + x) * pixelSize ];

		if (pixelSize > 3)
			color.w = 1.0f * pColor->alpha / 256.0f;
		else
			color.w = 1.0f;
		color.x = 1.0f * pColor->red / 256.0f;
		color.y = 1.0f * pColor->green / 256.0f;
		color.z = 1.0f * pColor->blue / 256.0f;
	}

	// emitter mask ?!
}


void ParticleSystem::GenerateParticle(const int emitType, const bool local, const double extrudeDist, Particle &particle)
{
	using namespace nv;
	if (false == mInheritSurfaceColor)
	{
		vec4 color = GenerateParticleColor(mPointColor, mPointColorVariation);

		if (mUseColor2)
		{
			vec4 color2 = GenerateParticleColor(mPointColor2, mPointColorVariation);
			float variance = dist(e2);
			if (variance > 0.66)
				color = color2;
		}
		if (mUseColor3)
		{
			vec4 color3 = GenerateParticleColor(mPointColor3, mPointColorVariation);
			float variance = dist(e2);
			if (variance < 0.33)
				color = color3;
		}

		particle.Color.x = Color_Pack(color);
	}

	if (mEvaluateData.gDirection.w < 1.0f)
	{
		const float randomH = mEvaluateData.gDirSpreadHor * (float) dist(e2);
		const float randomV = mEvaluateData.gDirSpreadVer * (float) dist(e2);
		GetRandomDir(mEvaluateData.gDirection, randomH, randomV, particle.Vel);
	}

	if (emitType == PARTICLE_EMIT_FROM_VERTICES)
	{
		int vertIndex = 0;
		GetRandomVerticesPos(local, particle.Pos, vertIndex);
		
		if (mEvaluateData.gDirection.w > 0.0f)
			GetRandomVerticesDir(vertIndex, particle.Vel);
	
		if (true == mInheritSurfaceColor)
		{
			vec4 color;
			GetRandomVerticesColor(vertIndex, color);
			particle.Color.x = Color_Pack(color);
		}
	}
	else if (emitType == PARTICLE_EMIT_FROM_SURFACE)
	{
		int vertIndex = 0;
		float r1, r2, r3;

		GetRandomSurfacePos(local, extrudeDist, particle.Pos, vertIndex, r1, r2, r3);
		
		if (mEvaluateData.gDirection.w > 0.0f)
			GetRandomSurfaceDir(vertIndex, particle.Vel);

		if (true == mInheritSurfaceColor)
		{
			vec4 color;
			GetRandomSurfaceColor(vertIndex, r1, r2, r3, color);
			
			const bool skipZeroAlpha = true;

			if (true == skipZeroAlpha)
			{
				float alpha = color.w;

				int trycount=0;
				while (alpha < 0.5f && trycount < 10)
				{
					GetRandomSurfacePos(local, extrudeDist, particle.Pos, vertIndex, r1, r2, r3);
					
					if (mEvaluateData.gDirection.w > 0.0f)
						GetRandomSurfaceDir(vertIndex, particle.Vel);
					GetRandomSurfaceColor(vertIndex, r1, r2, r3, color);

					alpha = color.w;

					trycount += 1;
				}
			}

			particle.Color.x = Color_Pack(color);
		}
	}
	else
	{
		// from volume
		GetRandomVolumePos(local, particle.Pos);
		
		if (mEvaluateData.gDirection.w > 0.0f)
			GetRandomVolumeDir(particle.Vel);

		if (true == mInheritSurfaceColor)
		{
			vec4 color;
			GetRandomVolumeColor(particle.Pos, color);
			particle.Color.x = Color_Pack(color);
		}
	}

	//
	particle.Pos = particle.Pos; // - mEvaluateData.gEmitterVelocity.w * mEvaluateData.gEmitterVelocity;
	particle.Pos.w = GenerateParticleSize(mPointSize, mPointSizeVariation); // negative size value for launcher !
	//particle.Vel.w = 1.0f;
	//particle.Vel = mEvaluateData.gRotationTM * particle.Vel;
	particle.Vel = GetRandomSpeed() * normalize(particle.Vel);
	//particle.Vel = particle.Vel; // + mEvaluateData.gEmitterVelocity.w * mEvaluateData.gEmitterVelocity;
	//particle.Vel.w = -dist(e2) - 0.001f;	// negative lifetime value for launcher !!
	particle.Rot = vec4(0.0f, 0.0f, 0.0f, 0.0f); // AgeMillis = 1.0f; // Particles[i].Vel.w - 1000.0f;		// one launch per second for this launcher
	particle.RotVel = vec4(0.0f, 0.0f, 0.0f, 0.0f); // Index = 1.0f;
}