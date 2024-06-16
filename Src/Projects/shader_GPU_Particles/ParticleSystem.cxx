
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: ParticleSystem.cxx
//
//	Author Sergei Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "ParticleSystem.h"
#include "checkglerror.h"
#include "FileUtils.h"
#include "nv_math.h"
#include "math3d.h"

#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>
/*
#include "model_collision_sphere.h"
#include "model_collision_terrain.h"
#include "model_force_wind.h"
#include "model_force_drag.h"
#include "model_force_motor.h"
*/
using namespace GPUParticles;

#define		USE_FORCES			mEvaluateData.gFlags.x
#define		USE_COLLISIONS		mEvaluateData.gFlags.y
#define		EMITTER_TYPE		mEvaluateData.gFlags.z

#define		COMPUTE_SHADER_GROUP_SIZE	1024

#define RANDOM_TEXTURE_SIZE		1024

#define INVALID_UNIFORM_LOCATION	-1

extern void LOGE(const char* pFormatString, ...);

float RandomFloat()
{
    float Max = RAND_MAX;
    return ((float)rand() / Max);
}

const char *CollisionNames[]=
{
	"position",
	"velocity"
	"radius",
	"friction",
	0
};

const char *ForceNames[]=
{
	"position",
	"magnitude",
	"radius",
	"turbulence",
	0
};

/*
source: DEBUG_SOURCE_X where X may be API, 
SHADER_COMPILER, WINDOW_SYSTEM, 
THIRD_PARTY, APPLICATION, OTHER
type: DEBUG_TYPE_X where X may be ERROR, 
MARKER, OTHER, DEPRECATED_BEHAVIOR, 
UNDEFINED_BEHAVIOR, PERFORMANCE, 
PORTABILITY, {PUSH, POP}_GROUP
severity: DEBUG_SEVERITY_{HIGH, MEDIUM}, 
DEBUG_SEVERITY_{LOW, NOTIFICATION}
*/
void DebugOGL_Callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, const void*userParam)
{
	if (type == GL_DEBUG_TYPE_ERROR)
		LOGE( ">> ERROR!\n" );
	LOGE( "debug message - %s\n", message );
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// particle system connections

ParticleSystemConnections::ParticleSystemConnections()
{
}

ParticleSystemConnections::~ParticleSystemConnections()
{
}

void ParticleSystemConnections::SetCollisionsCount(const int count)
{
	mCollisionData.resize(count);
}

void ParticleSystemConnections::SetCollisionData(const int index, const TCollision &data)
{
	mCollisionData[index] = data;
}

void ParticleSystemConnections::UploadCollisionsToGPU()
{
	mBufferCollisions.UpdateData( sizeof(TCollision), (int)mCollisionData.size(), mCollisionData.data() );
}

void ParticleSystemConnections::SetForcesCount(const int count)
{
	mForcesData.resize(count);
}

void ParticleSystemConnections::SetForceData(const int index, const TForce &data)
{
	mForcesData[index] = data;
}

void ParticleSystemConnections::UploadForcesToGPU()
{
	mBufferForces.UpdateData( sizeof(TForce), (int)mForcesData.size(), mForcesData.data() );
}


void ParticleSystemConnections::SetInstanceVertexStream( const TInstanceVertexStream &stream, unsigned int patchCount )
{
	mInstanceVertex = stream;
	mInstanceMesh.resize(patchCount);
}

void ParticleSystemConnections::SetInstancePatchData( const int index, unsigned int offset, unsigned int size, unsigned int texId )
{
	TMeshPatch patch = { texId, offset, size };
	mInstanceMesh[index] = patch;
}

////////////////////////////////////////////////////////////////////////////////////////////// PARTICLE SYSTEM
//

ParticleSystem::ParticleSystem(unsigned int maxparticles)
	: e2(rd())
	, dist(0, 1.0)
	, mConnections(nullptr)
{
	mMaxParticles = maxparticles;

	mCurrVB = 0;
	mCurrTFB = 1;

	mSurfaceBack = 0;
	mSurfaceFront = 1;

	mIsFirst = true;
	mTime = 0;

	mNeedReset = true;

	mParticleCount = 0;
	mUseRate = false;
	mParticleRate = 0;

	mSurfaceTextureId = 0;
	mSurfaceMaskId = 0;
	mTexture = 0;

	mQuery = 0;

	mTotalCycles = 0;

	mTransformFeedback[0] = mTransformFeedback[1] = 0;
	mParticleBuffer[0] = mParticleBuffer[1] = 0;
	
#ifdef _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback( DebugOGL_Callback, nullptr );
#endif
	
	mNoiseTexture = 0;

	mColorTextureId = 0;
	mSizeTextureId = 0;

	mShader = QueryShader();

	mPerModelUserData.isFirst = true;
	mPerModelUserData.isResetDone = false;
	mPerModelUserData.lastFrameTime = 0.0;

	mUseColor2 = false;
	mUseColor3 = false;
}

ParticleSystem::~ParticleSystem()
{
    ClearParticleSystem();

	FreeShader();
}

void ParticleSystem::ClearParticleSystem()
{
	if (mQuery)
	{
		glDeleteQueries(1, &mQuery);
		mQuery = 0;
	}

	if (mTransformFeedback[0] != 0) {
        glDeleteTransformFeedbacks(2, mTransformFeedback);
		mTransformFeedback[0] = mTransformFeedback[1] = 0;
    }
    
    if (mParticleBuffer[0] != 0) {
        glDeleteBuffers(2, mParticleBuffer);
		mParticleBuffer[0] = mParticleBuffer[1] = 0;
    }

	mBufferSurface[0].Free();
	mBufferSurface[1].Free();

	FreeNoiseTexture();
}

void ParticleSystem::PrepNoiseTexture()
{
	if (mNoiseTexture == 0)
	{
		glGenTextures(1, &mNoiseTexture);
	}
}

void ParticleSystem::FreeNoiseTexture()
{
	if (mNoiseTexture > 0)
	{
		glDeleteTextures(1, &mNoiseTexture);
		mNoiseTexture = 0;
	}
}

bool ParticleSystem::ReloadShaders()
{
	mShader->ChangeContext();
	return true; // mShader->Initialize();
}

void ParticleSystem::ChangeDisplayContext()
{
	ClearParticleSystem();

	ReloadShaders();
	NeedReset();

	mBufferSurface[0].Free();
	mBufferSurface[1].Free();
}

void ParticleSystem::NeedReset()
{
	mNeedReset = true;
}

bool ParticleSystem::InitParticleSystem(const vec3 &Pos)
{
	
	if (mShader->IsInitialized() == false)
		if (false == mShader->Initialize() )
			return false;

	if (mQuery == 0)
	{
		glGenQueries(1, &mQuery);
	}

	// TODO: one code for array assignment

	
	Particle *Particles = new Particle[mMaxParticles];
	memset( Particles, 0, sizeof(Particle)*mMaxParticles );

    //Particles[0].OldPos = vec4(0.0f, 0.0f, 0.0f, PARTICLE_TYPE_LAUNCHER);
    Particles[0].Pos = vec4(Pos[0], Pos[1], Pos[2], 0.0);	// in w - size
    Particles[0].Vel = vec4(0.0f, 0.1f, 0.0f, 0.0f);
	Particles[0].Color = vec4(0.0f, 10000.0f, 0.0f, 1.0f); // g-total lifetime, b-Age, -Index
    Particles[0].Rot = vec4(0.0, 0.0f, 0.0f, 0.0f); 
	Particles[0].RotVel = vec4(0.0f, 0.0f, 0.0f, 0.0f); 
	

	if (mTransformFeedback[0] == 0)
	{
		glGenTransformFeedbacks(2, mTransformFeedback);    
		glGenBuffers(2, mParticleBuffer);
	}
    
    for (unsigned int i = 0; i < 2 ; i++) {
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, mTransformFeedback[i]);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mParticleBuffer[i]);        
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
		
		glBindBuffer(GL_ARRAY_BUFFER, mParticleBuffer[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Particle)*mMaxParticles, Particles, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);   
		
    }

	// free mem from use dyn array
	
	if (Particles)
	{
		delete [] Particles;
		Particles = nullptr;
	}
	
	return true;
}

float myclampf(const float value, const float minValue, const float maxValue)
{
	if (value < minValue)
		return minValue;
	else if (value > maxValue)
		return maxValue;

	return value;
}

vec4 ParticleSystem::GenerateParticleColor(const vec4 &color, const float variation)
{
	if (variation <= 0.0)
		return color;

	float redV = 2.0f * dist(e2) * variation;
	float greenV = 2.0f * dist(e2) * variation;
	float blueV = 2.0f * dist(e2) * variation;

	vec4 newcolor(color);

	newcolor.x = myclampf(newcolor.x + redV - variation, 0.0f, 1.0f);
	newcolor.y = myclampf(newcolor.y + greenV - variation, 0.0f, 1.0f);
	newcolor.z = myclampf(newcolor.z + blueV - variation, 0.0f, 1.0f);

	return newcolor;
}

// variation should be in [0; 1]
float ParticleSystem::GenerateParticleSize(const float size, const float variation)
{
	float f = 2.0f * size * dist(e2) * variation;
	return size + (f - size * variation);
}



bool ParticleSystem::ReadSurfaceTextureData()
{
	mSurfaceTextureData.resize(0);

	if (mSurfaceTextureId == 0)
		return false;

	glBindTexture(GL_TEXTURE_2D, mSurfaceTextureId);

	const int miplevel = 0;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_WIDTH, &mSurfaceTextureInfo.width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_HEIGHT, &mSurfaceTextureInfo.height);

	GLint	format;
	GLint internalFormat;
	GLint compressed = 0;
	//GLint compressed_size;
	
	glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED, &compressed );

	// get a compression result			
	glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_RED_SIZE, &mSurfaceTextureInfo.red);
	glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_GREEN_SIZE, &mSurfaceTextureInfo.green );
	glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_BLUE_SIZE, &mSurfaceTextureInfo.blue );
	glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_ALPHA_SIZE, &mSurfaceTextureInfo.alpha );

	glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat );
	glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED, &compressed );

	format = (mSurfaceTextureInfo.alpha>0) ? GL_RGBA : GL_RGB;
//	int pixelMemorySize = mSurfaceTextureInfo.GetPixelMemorySize();
	
	if (compressed == GL_TRUE || mSurfaceTextureInfo.width <= 0 || mSurfaceTextureInfo.height <= 0)
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		return false;
	}

	int imageSize = mSurfaceTextureInfo.GetImageSize();
	mSurfaceTextureData.resize(imageSize);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glGetTexImage( GL_TEXTURE_2D, 0, format, GL_UNSIGNED_BYTE, mSurfaceTextureData.data() );

	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

void ParticleSystem::SetParticleSize(const double size, const double size_variation)
{
	mPointSize = (float)size;
	mPointSizeVariation = (float)size_variation;

	if (mPointSize <= 0.0f)
		mPointSize = abs(mPointSize) + 0.001f;
}

void ParticleSystem::SetParticleColor(const bool inheritSurfaceColor, const vec4 color, const double color_variation,
	const bool useColor2, const vec4 color2,
		const bool useColor3, const vec4 color3)
{
	mInheritSurfaceColor = inheritSurfaceColor;
	mPointColor = color;
	mPointColorVariation = (float) color_variation;
	mUseColor2 = useColor2;
	mPointColor2 = color2;
	mUseColor3 = useColor3;
	mPointColor3 = color3;
}


void ParticleSystem::PrepareParticles(unsigned int maxparticles, const int randomSeed, unsigned int particleCount, bool useRate, unsigned int rate, const double extrudeDist)
{
	if (mParticleBuffer[0] == 0 || mTransformFeedback[0] == 0)
		InitParticleSystem( vec3(0.0f, 0.0f, 0.0f) );

	const unsigned int localRate = (useRate) ? rate : 0;

	if ( true == mNeedReset )
	{
		mUseRate = useRate;
		mParticleRate = localRate;
		mParticleCount = particleCount;

		mIsFirst = true;
		ResetParticles(maxparticles, randomSeed, localRate, particleCount, extrudeDist);

		mTime = 0.0;

		mNeedReset = false;
	}
}

bool ParticleSystem::ResetParticles(unsigned int maxparticles, const int randomSeed, const int rate, const int preCount, const double extrudeDist)
{
	CHECK_GL_ERROR();

	//
	// read surface data from gpu
	CGPUBufferNV &buf = mBufferSurface[mSurfaceFront];
	if ( 0 == buf.GetBufferId() )
		return false;

	// copy data back to surface data on cpu
	glBindBuffer(GL_UNIFORM_BUFFER, buf.GetBufferId() );
	glGetBufferSubData(GL_UNIFORM_BUFFER, 0, buf.GetSize() * buf.GetCount(), mSurfaceData.data() );
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	bool newAssignment = (mMaxParticles != maxparticles);

	mMaxParticles = maxparticles;

	const int totalCount = (newAssignment) ? maxparticles : (rate + preCount);

	if (totalCount == 0)
		return false;

	Particle empty;
	memset( &empty, 0, sizeof(Particle) );

	std::vector<Particle> particles(totalCount, empty);
	
	srand(randomSeed);

	//
	if (true == mInheritSurfaceColor && PARTICLE_EMIT_FROM_VOLUME != EMITTER_TYPE )
	{
		ReadSurfaceTextureData();
		CHECK_GL_ERROR();
	}
	else if (mSurfaceTextureData.size() > 0)
	{
		mSurfaceTextureData.resize(0);
	}

	// assign launchers (pre generated pos, vel, color and size)
	auto iter = begin(particles);
	auto ratestop = (totalCount > rate) ? (iter + rate) : end(particles);

	for ( ; iter!=ratestop; ++iter)
	{
		GenerateParticle(EMITTER_TYPE, true, extrudeDist, *iter);

		iter->Pos.w = -1.0f * iter->Pos.w; // negative size value for launcher !
		iter->Color.y = -dist(e2) - 0.001f;	// negative lifetime value for launcher !! 
		iter->Color.z = 0.0f; // AgeMillis
		iter->Color.w = 0.0f; // Index
		//iter->Rot = vec4(0.0f, 0.0f, 0.0f, 0.0f); // AgeMillis = 1.0f; // Particles[i].Vel.w - 1000.0f;		// one launch per second for this launcher
		//iter->RotVel = vec4(0.0f, 0.0f, 0.0f, 0.0f); // Index = 1.0f;

		iter->Vel.w = dist(e2);	// random factor used when constraint to a surface
	}
	
	// assign pre particles
	// DONE: calculate particles emit parameters (on surface position, color, etc.)
	
	if (totalCount > rate)
	{
		for ( iter=ratestop; iter!=end(particles); ++iter )
		{
			GenerateParticle(EMITTER_TYPE, false, extrudeDist, *iter);
			
			iter->Color.y = mEvaluateData.gShellLifetime + (dist(e2) * 2.0 - 1.0) * mEvaluateData.gShellLifetime * mEvaluateData.gShellLifetimeVariation;	// lifetime
			if (iter->Color.y < 0.0f)
				iter->Color.y = 0.0;	// this defines the particle type (launcher or shell)

			iter->Rot = iter->Pos;	// TODO: temproary this is a constrained position
			iter->Rot.w = 0.0f;

			iter->Vel.w = dist(e2);
		}
	}

	//
	if (newAssignment)
	{
		for (unsigned int i = 0; i < 2 ; i++) {
			glBindBuffer(GL_ARRAY_BUFFER, mParticleBuffer[i]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(Particle)*particles.size(), particles.data(), GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);   
		}
	}
	else
	{
		for (unsigned int i = 0; i < 2 ; i++) {
			glBindBuffer(GL_ARRAY_BUFFER, mParticleBuffer[i]);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Particle)*particles.size(), particles.data());
			glBindBuffer(GL_ARRAY_BUFFER, 0);   
		}
	}

	CHECK_GL_ERROR();
	return true;
}


evaluateBlock		&ParticleSystem::GetSimulationData()
{
	return mEvaluateData;
}

bool ParticleSystem::UploadSimulationDataOnGPU()
{
	if (mShader->IsInitialized() == false)
		if (false == mShader->Initialize() )
			return false;

	mShader->UploadEvaluateDataBlock(mEvaluateData);

	return true;
}

void ParticleSystem::EmitParticles(const double deltaTime, const ETechEmitType	type)
{
	if (mShader->IsInitialized() == false)
		mShader->Initialize();

	CHECK_GL_ERROR();

	// Disable rasterisation, vertices processing only!
    glEnable(GL_RASTERIZER_DISCARD);
    
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	//glEnableVertexAttribArray(5);

	//glEnableClientState(GL_VERTEX_ARRAY);

	mTime += deltaTime;

	mShader->SetDeltaTime( (float) deltaTime );
	mShader->SetTime( (float) mTime );
	mShader->BindEmit(type);

	CHECK_GL_ERROR();

	//
	if (type != eTechEmitPreGenerated)
	{
		// layout(location=128) 	uniform 	TTriangle 	*gEmitMesh;
		mBufferSurface[mSurfaceFront].BindAsUniform(mShader->GetEmitGeometryProgramId(type), 
			mShader->GetEmitMeshLocation(type), 0);
		mBufferSurface[mSurfaceBack].BindAsUniform(mShader->GetEmitGeometryProgramId(type), 
			mShader->GetEmitPrevMeshLocation(type), 0);

		if (mSurfaceMaskId > 0)
		{
			glActiveTexture( GL_TEXTURE1 );
			glBindTexture( GL_TEXTURE_2D, mSurfaceMaskId );
			glActiveTexture( GL_TEXTURE0 );
		}

		if (mSurfaceTextureId > 0)
		{
			glBindTexture(GL_TEXTURE_2D, mSurfaceTextureId);
		}
	}

	if (mIsFirst == false)
	{
		SwapBuffers();
	}

	CHECK_GL_ERROR();

	glBindBuffer(GL_ARRAY_BUFFER, mParticleBuffer[mCurrVB]);    
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, mTransformFeedback[mCurrTFB]);

	// 96 bytes in total
	//glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), 0);                        // oldpos, type
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)0);         // position, normalized lifetime
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)16);        // velocity, lifetime
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)32);        // color
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)48);        // age milliseconds
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)64);        // index

	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, mQuery);
	glBeginTransformFeedback(GL_POINTS);

	if (mIsFirst) {
		const int totalCount = mParticleRate + mParticleCount;
		glDrawArrays(GL_POINTS, 0, totalCount);		// needed startup particle count

		mIsFirst = false;
	}
	else {
		glDrawTransformFeedback(GL_POINTS, mTransformFeedback[mCurrVB]);
	}            
    
	glEndTransformFeedback();
	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

	// retrieve a number of processed points
	glGetQueryObjectuiv(mQuery, GL_QUERY_RESULT, &mInstanceCount);
	//LOGI( "processed points count - %d\n", mInstanceCount);
	
	//glDisableClientState(GL_VERTEX_ARRAY);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
	glDisableVertexAttribArray(4);
	//glDisableVertexAttribArray(5);

	mShader->UnBindEmit();
	
	if (type != eTechEmitPreGenerated)
	{
		mBufferSurface[mSurfaceFront].UnBind();
		mBufferSurface[mSurfaceBack].UnBind();

		if (mSurfaceMaskId > 0)
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, 0);
			glActiveTexture(GL_TEXTURE0);
		}

		if (mSurfaceTextureId > 0)
			glBindTexture(GL_TEXTURE_2D, 0);
	}

	glDisable(GL_RASTERIZER_DISCARD);
	CHECK_GL_ERROR();
}

const unsigned int ParticleSystem::SimulateParticles(const bool emitEachStep, const ETechEmitType type, const double timeStep, 
	double &DeltaTime, const double limit, const int SubSteps, const bool selfCollisions, bool surfaceConstraint)
{
	if (false == mShader->IsInitialized() )
	{
		mShader->Initialize();

		if (false == mShader->IsInitialized() )
			return 0;
	}

	unsigned int cycles = 0;

	double ltime = DeltaTime;
	double globalTime = mTime - DeltaTime;

	//
	

	if (selfCollisions)
	{
		while(ltime > timeStep)
		{
			globalTime += timeStep;

			if (true == emitEachStep)
			{
				EmitParticles( timeStep, type );
			}

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mParticleBuffer[mCurrTFB]);  
	
			mConnections->BindForces(1);
			mConnections->BindCollisions(2);

			if (true == surfaceConstraint)
			{
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, mBufferSurface[mSurfaceFront].GetBufferId() );
			}

			// TODO: bind terrain 2d texture if bindless is not supported
			const GLuint terrainId = mConnections->GetTextureTerrain();
			if (terrainId > 0 && mShader->IsBindlessTexturesSupported() == false)
			{
				glBindTexture(GL_TEXTURE_2D, terrainId);
			}
			

			//
			//

			mShader->BindSimulation(false);
			mShader->DispatchSimulation( (float)timeStep, (float)globalTime, mInstanceCount, COMPUTE_SHADER_GROUP_SIZE, 1, 1);
			//glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
			mShader->UnBindSimulation();
			glFinish();

			//
			mShader->BindSelfCollisions();
			mShader->DispatchSelfCollisions( (float) timeStep, mInstanceCount, 256, 1, 1 );
			mShader->UnBindSelfCollisions();

			// GL_ALL_BARRIER_BITS
			glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
			//glFinish();

			//
			mShader->BindIntegrate();
			mShader->DispatchIntegrate( (float) timeStep, mInstanceCount, COMPUTE_SHADER_GROUP_SIZE, 1, 1 );
			//mShader->UnBindIntegrate();

			glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
			//glFinish();

			ltime -= timeStep;
			cycles += 1;
		}

		mShader->UnBindSimulation();
	}
	else
	{
		//mShader->BindSimulation(true);



		while(ltime > timeStep)
		{
			globalTime += timeStep;

			if (true == emitEachStep)
			{
				EmitParticles( timeStep, type );
			}
		
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mParticleBuffer[mCurrTFB]);  
	
			mConnections->BindForces(1);
			mConnections->BindCollisions(2);

			if (true == surfaceConstraint)
			{
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, mBufferSurface[mSurfaceFront].GetBufferId() );
			}

			// TODO: bind terrain 2d texture if bindless is not supported
			const GLuint terrainId = mConnections->GetTextureTerrain();
			if (terrainId > 0 && mShader->IsBindlessTexturesSupported() == false)
			{
				glBindTexture(GL_TEXTURE_2D, terrainId);
			}
			
			//
			//
			mShader->BindSimulation(true);

			mShader->DispatchSimulation( (float)timeStep, (float)globalTime, mInstanceCount, COMPUTE_SHADER_GROUP_SIZE, 1, 1);
			glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
		
			mShader->UnBindSimulation();
			if (terrainId > 0 && mShader->IsBindlessTexturesSupported() == false)
				glBindTexture(GL_TEXTURE_2D, 0);

			ltime -= timeStep;
			cycles += 1;
		}
		//mShader->UnBindSimulation();
	}

	
	/*
	if (terrainId > 0 && mShader->IsBindlessTexturesSupported() == false)
		glBindTexture(GL_TEXTURE_2D, 0);
		*/
	DeltaTime = ltime;

	CHECK_GL_ERROR();
	mTotalCycles += cycles;
	return cycles;
}
    
bool ParticleSystem::EmitterSurfaceUpdateOnCPU(const int vertexCount, float *positionsArray, 
	float *normalArray, float *uvArray, const int indexCount, const int *indexArray, const GLuint textureId )
{
	if (indexCount < 3 || vertexCount < 1)
		return false;

	vec4 *positions4 = (vec4*) positionsArray;
	vec4 *normals4 = (vec4*) normalArray;
	vec2 *uv2 = (vec2*) uvArray;

	const int triCount = indexCount / 3;
	mSurfaceData.resize(triCount);

	auto triIter = begin(mSurfaceData);

	vec4 n0, n1, n2;

	for (int i=0; i<indexCount; i+=3)
	{
		triIter->p0 = positions4[ indexArray[i] ];
		triIter->p1 = positions4[ indexArray[i + 1] ];
		triIter->p2 = positions4[ indexArray[i + 2] ];

		triIter->p0.w = 1.0f;
		triIter->p1.w = 1.0f;
		triIter->p2.w = 1.0f;

		n0 = normals4[ indexArray[i] ];
		n1 = normals4[ indexArray[i + 1] ];
		n2 = normals4[ indexArray[i + 2] ];

		// TODO: compute average from n0, n1, n2
		triIter->n = 1.0 / 3.0 * (n0 + n1 + n2);
		triIter->n.w = 1.0f;
		normalize(triIter->n);

		triIter->uv0 = uv2[ indexArray[i] ];
		triIter->uv1 = uv2[ indexArray[i + 1] ];
		triIter->uv2 = uv2[ indexArray[i + 2] ];

		triIter++;
	}

	//
	//
	mEvaluateData.gPositionCount = triCount;
	mEvaluateData.gUseEmitterTexture = (textureId > 0);
	mEvaluateData.gUseEmitterMask = 0;
	mSurfaceTextureId = textureId;

	return true;
}

// DONE: run a computer shader
bool ParticleSystem::EmitterSurfaceUpdateOnGPU(void *pModelVertexData, const GLuint textureId, const double *tm, const GLuint maskId)
{	
	//
	// 
	FBModelVertexData *pData = (FBModelVertexData*) pModelVertexData;

	//FBGeometry *pGeometry = pModel->Geometry;
	//FBModelVertexData *pData = pModel->ModelVertexData;

	if ( nullptr == pData || false == pData->IsDrawable() )
		return false;

	const int numberOfVertices = pData->GetVertexCount();
	
	if (0 == numberOfVertices)
		return false;


	int numberOfIndices = 0;
	
	for (int i=0, count=pData->GetSubPatchCount(); i<count; ++i)
	{
		int offset = pData->GetSubPatchIndexOffset(i);
		int size = pData->GetSubPatchIndexSize(i);

		int localCount = offset+size;
		if ( localCount > numberOfIndices )
			numberOfIndices = localCount;
	}

	const int numberOfTriangles = numberOfIndices / 3;

	// output surface data - allocate and upload on gpu
	//	do only once - when number of triangles has beend changed
	if ( numberOfTriangles != (int)mSurfaceData.size() )
	{
		mSurfaceData.resize(numberOfTriangles);
		
		mBufferSurface[0].UpdateData( sizeof(TTriangle), (int)mSurfaceData.size(), mSurfaceData.data() );
		mBufferSurface[1].UpdateData( sizeof(TTriangle), (int)mSurfaceData.size(), mSurfaceData.data() );
	}

	//
	// run a compute program

	const GLuint posId = pData->GetVertexArrayVBOId( kFBGeometryArrayID_Point, true );
	const GLuint norId = pData->GetVertexArrayVBOId( kFBGeometryArrayID_Normal, true );
	const GLuint uvId = pData->GetUVSetVBOId();
	const GLuint indId = pData->GetIndexArrayVBOId();
	const GLuint surfaceId = mBufferSurface[mSurfaceBack].GetBufferId();

	if ( 0 == posId || 0 == norId || 0 == uvId || 0 == indId || 0 == surfaceId )
		return false;

	const GLvoid* positionOffset = pData->GetVertexArrayVBOOffset(kFBGeometryArrayID_Point);
	const GLvoid* normalOffset = pData->GetVertexArrayVBOOffset(kFBGeometryArrayID_Normal);
	const GLvoid* uvOffset = pData->GetUVSetVBOOffset();

	
	//DONE: not correct for GPU skinning, we should use ptr offset
	/*
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posId );
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, norId );
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, uvId );
	
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, indId );
	// and output into a triangles buffer
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, surfaceId );
	*/
	
	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, posId, (GLintptr) positionOffset, numberOfVertices * sizeof(vec4));
	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, norId, (GLintptr) normalOffset, numberOfVertices * sizeof(vec4));
	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 2, uvId, (GLintptr) uvOffset, numberOfVertices * sizeof(vec2));
	
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, indId );
	// and output into a triangles buffer
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, surfaceId );

	//
	// ACCUM NORMALS
	
	mShader->RunSurfaceComputeShader( numberOfTriangles );

	//
	//
	mEvaluateData.gPositionCount = numberOfTriangles;
	mEvaluateData.gUseEmitterTexture = (textureId > 0);
	mEvaluateData.gUseEmitterMask = (maskId > 0);

	for (int i=0; i<16; ++i)
		mEvaluateData.gTextureTM.mat_array[i] = (float) tm[i];

	mSurfaceTextureId = textureId;
	mSurfaceMaskId = maskId;

	SwapSurfaceBuffers();

	return true;

}

// that's only for the case when data is updating on CPU
void ParticleSystem::UploadSurfaceDataToGPU()
{
	mBufferSurface[mSurfaceFront].UpdateData( sizeof(TTriangle), (int)mSurfaceData.size(), mSurfaceData.data() );
	// TODO: do we support per-vertex velocity when computing on CPU ?!
	//mBufferSurface[mSurfaceBack].UpdateData( sizeof(TTriangle), (int)mSurfaceData.size(), mSurfaceData.data() );
}