
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: Shader_ParticlesSystem.cpp
//
//	Author Sergei Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "CheckGLError.h"
#include "Shader_ParticleSystem.h"

#include <iostream>

using namespace GPUParticles;

extern void LOGE(const char* pFormatString, ...);

/////////////////////////////////////// Global vars

char				fx_location[256];
int					fx_locationStrLen;

char				fx_computeLocation[256];
int					fx_computeStrLen;

char				fx_computeSelfCollisionsLocation[256];
int					fx_computeSelfCollisionsStrLen;

char				fx_integrateLocation[256];
int					fx_integrateStrLen;

char				g_computeSurfacePath[256];
int					g_computeSurfacePathLen;

/////////////////////////////////////////////////////////////////////////////////////////////////////
//

int numberOfShaderInstances = 0;
ParticleShaderFX	*gShader = nullptr;

ParticleShaderFX *GPUParticles::QueryShader()
{
	numberOfShaderInstances++;

	if (gShader == nullptr)
	{
		gShader = new ParticleShaderFX();
	}

	return gShader;
}

void ReloadShader()
{

}

void GPUParticles::FreeShader()
{
	numberOfShaderInstances--;

	if (numberOfShaderInstances == 0)
	{
		if (gShader != nullptr)
		{
			delete gShader;
			gShader = nullptr;
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
//

ParticleShaderFX::ParticleShaderFX()
{
	fx_Effect = nullptr;
    
	for (int i=0; i<eTechEmitCount; ++i)
		fx_TechEmit[i] = nullptr;

	fx_passEmitCurrent = nullptr;
/*
	fx_TechSimulate = nullptr;
	fx_passSimulate = nullptr;
*/
	fx_TechRenderPoints = nullptr;
	fx_TechRenderQuads = nullptr;
	fx_TechRenderBillboards = nullptr;
	fx_TechRenderStretchedBillboards = nullptr;
	fx_TechRenderInstancesSimple = nullptr;
	fx_TechRenderInstancesFlat = nullptr;
	fx_TechRenderInstancesDynamic = nullptr;
	fx_passRender = nullptr;

	fx_gTime = nullptr;
	fx_gDeltaTime = nullptr;

	//
	fx_evaluateBlock = nullptr;
	fx_renderBlock = nullptr;

	techTerrainPreviewFragment = 0;
	locTerrainTextureAddress = 0;

	//shaderCompute = 0;
	programCompute = 0;

	locEmitterTM = 0;
	locComputeDynamic = 0;
	locComputeFloor = 0;
	locComputeGravity = 0;
	locComputeNumCollisions = 0;
	locComputeNumForces = 0;
	locComputeTurbulence = 0;
	locComputeUseSizeAtten = 0;
	locComputeEmitterPointCount = 0;

	//shaderSelfCollisions = 0;
	programSelfCollisions = 0;

	programIntegrate = 0;

	bindlessTexturesSupported = true;
}

ParticleShaderFX::~ParticleShaderFX()
{
	clearResources();
}


bool ParticleShaderFX::IsInitialized()
{
	return (fx_Effect != nullptr);
}

bool ParticleShaderFX::SetShaderEffectLocation(const char *effectLocation, const int effectStrLen)
{
	memcpy_s( fx_location, sizeof(char)*256, effectLocation, effectStrLen );
	fx_locationStrLen = effectStrLen;

	return (fx_locationStrLen > 0);
}

bool ParticleShaderFX::SetComputeShaderLocation(const char *shaderLocation, const int shaderLen)
{
	memcpy_s( fx_computeLocation, sizeof(char)*256, shaderLocation, shaderLen );
	fx_computeStrLen = shaderLen;

	return (fx_computeStrLen > 0);
}

bool ParticleShaderFX::SetComputeSelfCollisionsShaderLocation(const char *shaderLocation, const int shaderLen)
{
	memcpy_s( fx_computeSelfCollisionsLocation, sizeof(char)*256, shaderLocation, shaderLen );
	fx_computeSelfCollisionsStrLen = shaderLen;

	return (fx_computeSelfCollisionsStrLen > 0);
}

bool ParticleShaderFX::SetComputeIntegrateLocation(const char *shaderLocation, const int shaderLen)
{
	memcpy_s( fx_integrateLocation, sizeof(char)*256, shaderLocation, shaderLen );
	fx_integrateStrLen = shaderLen;

	return (fx_integrateStrLen > 0);
}

bool ParticleShaderFX::SetComputeSurfaceDataPath(const char *path, const int pathLen)
{
	memset( g_computeSurfacePath, 0, sizeof(char) * 256 );
	memcpy_s( g_computeSurfacePath, sizeof(char)*256, path, pathLen );
	g_computeSurfacePathLen = pathLen;

	return (g_computeSurfacePathLen > 0);
}

bool ParticleShaderFX::Initialize()
{
	// check for bindless extensions

	bindlessTexturesSupported = glewIsExtensionSupported("GL_ARB_bindless_texture") 
								&& glewIsExtensionSupported("GL_ARB_gpu_shader_int64");

	bindlessTexturesSupported = false;

	//
	// load glslfx shader
	bool lSuccess = false;
	try
	{

		lSuccess = loadEffect( fx_location );

		if (false == lSuccess)
			throw std::exception( "failed to load particles effect" );

		//
		programCompute = loadComputeShader(fx_computeLocation);
	
		if (0 == programCompute)
			throw std::exception( "failed to load particles program compute" );

		locNumParticles = glGetUniformLocation(programCompute, "gNumParticles" );
		locComputeDeltaTime = glGetUniformLocation(programCompute, "DeltaTimeSecs");
		locComputeTime = glGetUniformLocation(programCompute, "gTime");

		locEmitterTM = glGetUniformLocation(programCompute, "gTM");
		locComputeDynamic = glGetUniformLocation(programCompute, "gDynamic");
		locComputeTurbulence = glGetUniformLocation(programCompute, "gTurbulence");
		locComputeGravity = glGetUniformLocation(programCompute, "gGravity");
		locComputeFloor = glGetUniformLocation(programCompute, "gFloor");
		locComputeNumForces = glGetUniformLocation(programCompute, "gNumForces");
		locComputeNumCollisions	= glGetUniformLocation(programCompute, "gNumCollisions");
		locComputeUseSizeAtten = glGetUniformLocation(programCompute, "gUseSizeAttenuation");

		locComputeUpdatePosition = glGetUniformLocation(programCompute, "gUpdatePosition");
		locComputeEmitterPointCount = glGetUniformLocation(programCompute, "gEmitterPointCount");

		// self collisions shader
		programSelfCollisions = loadComputeShader(fx_computeSelfCollisionsLocation);

		if ( 0 == programSelfCollisions )
			throw std::exception( "failed to load particles self collisions shader" );
	
		locSelfCollisionsNumParticles = glGetUniformLocation(programSelfCollisions, "gNumParticles");
		locSelfCollisionsDeltaTime = glGetUniformLocation(programSelfCollisions, "DeltaTimeSecs");
	
		// integrate shader
		programIntegrate = loadComputeShader(fx_integrateLocation);

		if ( 0 == programIntegrate )
			throw std::exception( "failed to load particles integrate shader" );
		
		locIntegrateNumParticles = glGetUniformLocation(programIntegrate, "gNumParticles");
		locIntegrateDeltaTime = glGetUniformLocation(programIntegrate, "DeltaTimeSecs");
		locIntegrateNumCollisions = glGetUniformLocation(programIntegrate, "gNumCollisions");
		locIntegrateFloor = glGetUniformLocation(programIntegrate, "gFloor");

		// surface data prep
		lSuccess = LoadSurfaceComputeShaders(g_computeSurfacePath, g_computeSurfacePathLen);

		if (false == lSuccess)
			throw std::exception( "failed to load surface data shader" );
	}
	catch( const std::exception &e )
	{
		LOGE ("[ERROR particles] - %s\n", e.what() );
		clearResources();
	}

	return lSuccess;
}

void ParticleShaderFX::ChangeContext()
{
	clearResources();
}

void ParticleShaderFX::clearResources()
{
	if(fx_Effect)
    {
		if (OnMessageEvent != 0)
			OnMessageEvent("Desroying previous material Effect\n");
        //fx_EffectMaterial->destroy();
        //or
		if (OnMessageEvent != 0)
			OnMessageEvent("=========> Destroying effect\n");
        nvFX::IContainer::destroy(fx_Effect);
        fx_Effect = nullptr;

		fx_passEmitCurrent = nullptr;

		//fx_passSimulate = nullptr;
		fx_passTerrainPrep = nullptr;
		fx_passTerrainPreview = nullptr;
    }

	if (programCompute)
	{
		glDeleteProgram(programCompute);
		programCompute = 0;
	}
	/*
	if (shaderCompute)
	{
		glDeleteShader(shaderCompute);
		shaderCompute = 0;
	}
	*/
	if (programSelfCollisions)
	{
		glDeleteProgram(programSelfCollisions);
		programSelfCollisions = 0;
	}
	/*
	if (shaderSelfCollisions)
	{
		glDeleteShader(shaderSelfCollisions);
		shaderSelfCollisions = 0;
	}
	*/
	if (programIntegrate)
	{
		glDeleteProgram(programIntegrate);
		programIntegrate = 0;
	}

	//
	mComputeSurface.reset(nullptr);
}


const GLuint nvGetProgramId(nvFX::IPass *pass, const int programPipeline, const int shaderProgram)
{
	if (pass == nullptr)
		return 0;
	
	GLuint fragmentId = 0;	

	nvFX::IProgram *glslProgram = nullptr;
    
	nvFX::IProgramPipeline *glslProgramPipeline = pass->getExInterface()->getProgramPipeline(programPipeline);
    if(glslProgramPipeline)
    {
		glslProgram = glslProgramPipeline->getShaderProgram(shaderProgram);	// Fragment glsl program !
    
    } else {
		glslProgram = pass->getExInterface()->getProgram(shaderProgram);
    
    }
    fragmentId = (glslProgram != nullptr) ? glslProgram->getProgram() : 0;

	return fragmentId;
}


//-----------------------------------------------------------------------------
// Load scene effect
//-----------------------------------------------------------------------------
bool ParticleShaderFX::loadEffect(const char *effectFileName)
{
    if(fx_Effect)
    {
		if (OnMessageEvent != 0)
			OnMessageEvent("Desroying previous material Effect\n");
        //fx_EffectMaterial->destroy();
        //or
		if (OnMessageEvent != 0)
			OnMessageEvent("=========> Destroying effect\n");
        nvFX::IContainer::destroy(fx_Effect);
        fx_Effect = nullptr;
    }
	
	if (OnMessageEvent != 0)
		OnMessageEvent("Creating Effect (material)\n");
    fx_Effect = nvFX::IContainer::create("material");
    bool bRes = nvFX::loadEffectFromFile(fx_Effect, effectFileName);
        
    if(!bRes)
    {
		if (OnMessageEvent != 0)
			OnMessageEvent("Failed\n");
        return false;
    }
	if (OnMessageEvent != 0)
		OnMessageEvent("Loaded\n");
    //
    // Let's keep track in interface pointers everything, for purpose of clarity
    //

	const char *emitTechNames[eTechEmitCount] = {
		"volumeEmitter",
		"surfaceEmitter",
		"verticesEmitter",
		"emitPreGenerated"
	};

	fx_passEmitCurrent = nullptr;
	for (int i=0; i<eTechEmitCount; ++i)
	{
		auto pTech = fx_Effect->findTechnique( emitTechNames[i] );
		
		if (nullptr == pTech || (pTech && !pTech->validate() ))
			return false;

		fx_TechEmit[i] = pTech;

		if (i != eTechEmitPreGenerated)
		{
			emitGeometryProgram[i] = nvGetProgramId(pTech->getPass(0), 0, 1);	// geometry shader
			locEmitMesh[i] = glGetUniformLocation( emitGeometryProgram[i], "gEmitMesh" );
			locEmitPrevMesh[i] = glGetUniformLocation( emitGeometryProgram[i], "gEmitPrevMesh" );
		}
		else
		{
			emitGeometryProgram[i] = 0;
			locEmitMesh[i] = 0;
			locEmitPrevMesh[i] = 0;
		}
	}

	CHECK_GL_ERROR();

	// simulate technique
	/*
	fx_TechSimulate = fx_Effect->findTechnique("simulate0");
	if (fx_TechSimulate && (!fx_TechSimulate->validate() ))
		return false;
	fx_passSimulate = fx_TechSimulate->getPass(0);
	*/
	// render technique

	fx_TechRenderPoints = fx_Effect->findTechnique("renderPoints");
	if(fx_TechRenderPoints && (!fx_TechRenderPoints->validate()))
        return false;
	
	fx_TechRenderQuads = fx_Effect->findTechnique("renderQuads");
	if(fx_TechRenderQuads && (!fx_TechRenderQuads->validate()))
        return false;
	
	fx_TechRenderBillboards = fx_Effect->findTechnique("renderBillboards");
	if(fx_TechRenderBillboards && (!fx_TechRenderBillboards->validate()))
        return false;

	fx_TechRenderStretchedBillboards = fx_Effect->findTechnique("renderStretchedBillboards");
	if(fx_TechRenderStretchedBillboards && (!fx_TechRenderStretchedBillboards->validate()))
        return false;

	fx_TechRenderInstancesSimple = fx_Effect->findTechnique("renderInstancesSimple");
	if(fx_TechRenderInstancesSimple && (!fx_TechRenderInstancesSimple->validate()))
        return false;
	
	fx_TechRenderInstancesFlat = fx_Effect->findTechnique("renderInstancesFlat");
	if (fx_TechRenderInstancesFlat && (!fx_TechRenderInstancesFlat->validate()))
		return false;

	fx_TechRenderInstancesDynamic = fx_Effect->findTechnique("renderInstancesDynamic");
	if (fx_TechRenderInstancesDynamic && (!fx_TechRenderInstancesDynamic->validate()))
		return false;

	fx_passRender = fx_TechRenderPoints->getPass(0);

	// terrain shaders

	fx_TechTerrainPrep = fx_Effect->findTechnique("terrainPrepare");
	if(fx_TechTerrainPrep && (!fx_TechTerrainPrep->validate()))
        return false;
	fx_passTerrainPrep = fx_TechTerrainPrep->getPass(0);
	if (fx_passTerrainPrep == nullptr)
		return false;

	if (bindlessTexturesSupported)
	{
		fx_TechTerrainPreview = fx_Effect->findTechnique("terrainPreviewBindless");
		if(fx_TechTerrainPreview && (!fx_TechTerrainPreview->validate()))
			return false;
		fx_passTerrainPreview = fx_TechTerrainPreview->getPass(0);
		if (fx_passTerrainPreview == nullptr)
			return false;
	}
	else
	{
		fx_TechTerrainPreview = fx_Effect->findTechnique("terrainPreview");
		if(fx_TechTerrainPreview && (!fx_TechTerrainPreview->validate()))
			return false;
		fx_passTerrainPreview = fx_TechTerrainPreview->getPass(0);
		if (fx_passTerrainPreview == nullptr)
			return false;
	}

	//
	//
	fx_gTime = fx_Effect->findUniform( "gTime" );
	if (fx_gTime == nullptr)
		return false;
	fx_gDeltaTime = fx_Effect->findUniform( "gDeltaTimeMillis" );
	if (fx_gDeltaTime == nullptr)
		return false;

	fx_terrainModelTM = fx_Effect->findUniform( "gTerrainModelTM" );
	if (fx_terrainModelTM == nullptr)
		return false;

	//
	//
	fx_evaluateBlock = fx_Effect->findCstBuffer("evaluateBlock");
	if (fx_evaluateBlock)
		fx_evaluateBlock->buildGLBuffer();
	
	fx_renderBlock = fx_Effect->findCstBuffer("renderBlock");
	if (fx_renderBlock)
		fx_renderBlock->buildGLBuffer();

	fx_terrainBlock = fx_Effect->findCstBuffer("terrainBlock");
	if (fx_terrainBlock)
		fx_terrainBlock->buildGLBuffer();

	//
	//
	/*
	if (fx_TechEmitDynamic)
	{
		techEmitGeometry = nvGetProgramId( fx_passEmitDynamic, 0, 1 );	// geometry shader
		locEmitMesh = glGetUniformLocation( techEmitGeometry, "gEmitMesh" );
		locEmitTexture = glGetUniformLocation( techEmitGeometry, "gEmitTexture" );
	}
	*/


	// terrain texture address
	if (fx_TechTerrainPreview)
	{
		techTerrainPreviewFragment = nvGetProgramId( fx_TechTerrainPreview->getPass(0), 0, 1 ); // fragment shader
		locTerrainTextureAddress = glGetUniformLocation( techTerrainPreviewFragment, "gTerrainColorAddress" );
	}

	CHECK_GL_ERROR();

	return true;
}

void ParticleShaderFX::UploadEvaluateDataBlock(const evaluateBlock &_evaluateBlock)
{
	// copy the block to OGL
	if(fx_evaluateBlock)
    {
        void* p;
        if (true == fx_evaluateBlock->mapBuffer(&p) )
		{
			memcpy(p, &_evaluateBlock, sizeof(evaluateBlock));
			fx_evaluateBlock->unmapBuffer();
		}
    }

	CHECK_GL_ERROR();


	// update the compute shader uniforms
	if (programCompute > 0)
	{
		glUseProgram( programCompute );

		glUniformMatrix4fv( locEmitterTM, 1, GL_FALSE, _evaluateBlock.gTM.mat_array );
		glUniform4f( locComputeDynamic, _evaluateBlock.gDynamic.x, _evaluateBlock.gDynamic.y, _evaluateBlock.gDynamic.z, _evaluateBlock.gDynamic.w );
		glUniform4f( locComputeTurbulence, _evaluateBlock.gTurbulence.x, _evaluateBlock.gTurbulence.y, _evaluateBlock.gTurbulence.z, _evaluateBlock.gTurbulence.w );
		glUniform4f( locComputeGravity, _evaluateBlock.gGravity.x, _evaluateBlock.gGravity.y, _evaluateBlock.gGravity.z, _evaluateBlock.gGravity.w );
		glUniform4f( locComputeFloor, _evaluateBlock.gFloor.x, _evaluateBlock.gFloor.y, _evaluateBlock.gFloor.z, _evaluateBlock.gFloor.w );
		glUniform1i( locComputeNumForces, _evaluateBlock.gNumForces );
		glUniform1i( locComputeNumCollisions, _evaluateBlock.gNumCollisions );
		glUniform1i( locComputeUseSizeAtten, _evaluateBlock.gUseSizeAttenuation );
		glUniform1i( locComputeEmitterPointCount, _evaluateBlock.gPositionCount );
		glUseProgram( 0 );

		CHECK_GL_ERROR();
	}

	if (programIntegrate > 0)
	{
		glUseProgram( programIntegrate );

		glUniform1i( locIntegrateNumCollisions, _evaluateBlock.gNumCollisions );
		glUniform4f( locIntegrateFloor, _evaluateBlock.gFloor.x, _evaluateBlock.gFloor.y, _evaluateBlock.gFloor.z, _evaluateBlock.gFloor.w );

		glUseProgram(0);
	}
}

void ParticleShaderFX::UploadRenderDataBlock(const renderBlock	&_renderBlock)
{
	// copy the block to OGL
	if(fx_renderBlock)
    {
        void* p;
        if (true == fx_renderBlock->mapBuffer(&p) )
		{
			memcpy(p, &_renderBlock, sizeof(renderBlock));
			fx_renderBlock->unmapBuffer();
		}
    }

	CHECK_GL_ERROR();
}


void ParticleShaderFX::UploadTerrainDataBlock(const terrainBlock &data)
{
	// copy the block to OGL
	if(fx_terrainBlock)
    {
        void* p;
        if (true == fx_terrainBlock->mapBuffer(&p) )
		{
			memcpy(p, &data, sizeof(terrainBlock));
			fx_terrainBlock->unmapBuffer();
		}
    }

	CHECK_GL_ERROR();
}

void ParticleShaderFX::BindEmit(const ETechEmitType	type)
{
	switch (type)
	{
	case eTechEmitVolume:
		fx_passEmitCurrent = fx_TechEmit[eTechEmitVolume]->getPass(0);
		break;
	case eTechEmitVertices:
		fx_passEmitCurrent = fx_TechEmit[eTechEmitVertices]->getPass(0);
		break;
	case eTechEmitSurface:
		fx_passEmitCurrent = fx_TechEmit[eTechEmitSurface]->getPass(0);
		break;
	default:
		fx_passEmitCurrent = fx_TechEmit[eTechEmitPreGenerated]->getPass(0);
		break;
	}

	CHECK_GL_ERROR();

	if (fx_passEmitCurrent)
		fx_passEmitCurrent->execute();

	CHECK_GL_ERROR();
}

void ParticleShaderFX::UnBindEmit()
{
	if (fx_passEmitCurrent)
		fx_passEmitCurrent->unbindProgram();

	fx_passEmitCurrent = nullptr;
}

void ParticleShaderFX::BindSimulation(const bool updatePosition)
{
	if (programCompute > 0)
	{
		glUseProgram(programCompute);

		glUniform1i( locComputeUpdatePosition, (updatePosition) ? 1 : 0 );
	}

}

void ParticleShaderFX::DispatchSimulation(const float dt, const float gTime, const int size, const int group_x, const int group_y, const int group_z)
{
	if (programCompute > 0)
	{
		glUniform1f( locComputeDeltaTime, dt );
		glUniform1f( locComputeTime, gTime );
		glUniform1i( locNumParticles, size );

		//
		glDispatchCompute( (size / group_x) + 1, group_y, group_z );
	}

}

void ParticleShaderFX::UnBindSimulation()
{
	if (programCompute > 0)
		glUseProgram(0);
}

void ParticleShaderFX::BindSelfCollisions()
{
	if (programSelfCollisions > 0)
	{
		glUseProgram(programSelfCollisions);
	}

}

void ParticleShaderFX::DispatchSelfCollisions(const float dt, const int size, const int group_x, const int group_y, const int group_z)
{
	if (programSelfCollisions > 0)
	{
		glUniform1f( locSelfCollisionsDeltaTime, dt );
		glUniform1i( locSelfCollisionsNumParticles, size );

		//
		glDispatchCompute( (size / group_x) + 1, group_y, group_z );
	}

}

void ParticleShaderFX::UnBindSelfCollisions()
{
	if (programSelfCollisions > 0)
		glUseProgram(0);
}

void ParticleShaderFX::BindIntegrate()
{
	if (programIntegrate > 0)
	{
		glUseProgram(programIntegrate);
	}

}

void ParticleShaderFX::DispatchIntegrate(const float dt, const int size, const int group_x, const int group_y, const int group_z)
{
	if (programIntegrate > 0)
	{
		glUniform1f( locIntegrateDeltaTime, dt );
		glUniform1i( locIntegrateNumParticles, size );

		//
		glDispatchCompute( (size / group_x) + 1, group_y, group_z );
	}

}

void ParticleShaderFX::UnBindIntegrate()
{
	if (programIntegrate > 0)
		glUseProgram(0);
}

void ParticleShaderFX::BindRenderPoints()
{
	fx_passRender = (fx_TechRenderPoints) ? fx_TechRenderPoints->getPass(0) : nullptr;

	if (fx_passRender)
		fx_passRender->execute();
}

void ParticleShaderFX::UnBindRenderPoints()
{
	if (fx_passRender)
		fx_passRender->unbindProgram();
}

void ParticleShaderFX::BindRenderQuads()
{
	fx_passRender = (fx_TechRenderQuads) ? fx_TechRenderQuads->getPass(0) : nullptr;

	if (fx_passRender)
		fx_passRender->execute();
}

void ParticleShaderFX::UnBindRenderQuads()
{
	if (fx_passRender)
		fx_passRender->unbindProgram();
}

void ParticleShaderFX::BindRenderBillboards()
{
	fx_passRender = (fx_TechRenderBillboards) ? fx_TechRenderBillboards->getPass(0) : nullptr;

	if (fx_passRender)
		fx_passRender->execute();
}

void ParticleShaderFX::UnBindRenderBillboards()
{
	if (fx_passRender)
		fx_passRender->unbindProgram();
}

void ParticleShaderFX::BindRenderStretchedBillboards()
{
	fx_passRender = (fx_TechRenderStretchedBillboards) ? fx_TechRenderStretchedBillboards->getPass(0) : nullptr;

	if (fx_passRender)
		fx_passRender->execute();
}

void ParticleShaderFX::UnBindRenderStretchedBillboards()
{
	if (fx_passRender)
		fx_passRender->unbindProgram();
}

void ParticleShaderFX::BindRenderInstances(const int lighting)
{
	switch (lighting)
	{
	case 0:
		fx_passRender = (fx_TechRenderInstancesSimple) ? fx_TechRenderInstancesSimple->getPass(0) : nullptr;
		break;
	case 1:
		fx_passRender = (fx_TechRenderInstancesFlat) ? fx_TechRenderInstancesFlat->getPass(0) : nullptr;
		break;
	case 2:
		fx_passRender = (fx_TechRenderInstancesDynamic) ? fx_TechRenderInstancesDynamic->getPass(0) : nullptr;
		break;
	default:
		fx_passRender = nullptr;
	}

	if (fx_passRender)
		fx_passRender->execute();
}

void ParticleShaderFX::UnBindRenderInstances()
{
	if (fx_passRender)
		fx_passRender->unbindProgram();
}

void ParticleShaderFX::BindTerrainDepth()
{
	if (fx_passTerrainPrep)
		fx_passTerrainPrep->execute();
}

void ParticleShaderFX::UnBindTerrainDepth()
{
	if (fx_passTerrainPrep)
		fx_passTerrainPrep->unbindProgram();
}

void ParticleShaderFX::BindTerrainPreview()
{
	if (fx_passTerrainPreview)
		fx_passTerrainPreview->execute();
}

void ParticleShaderFX::UnBindTerrainPreview()
{
	if (fx_passTerrainPreview)
		fx_passTerrainPreview->unbindProgram();
}

void ParticleShaderFX::SetTime(const float value)
{
	fx_gTime->setValue1f(value);
}

void ParticleShaderFX::SetDeltaTime(const float value)
{
	fx_gDeltaTime->setValue1f(value);
}

void ParticleShaderFX::UpdateTerrainModelTM(const nv::mat4 &tm)
{
	fx_terrainModelTM->updateMatrix4f( (float*)tm.mat_array, fx_passTerrainPrep);
}

void ParticleShaderFX::SetUniformTerrainTextureAddress(const GLuint64 address)
{
	//if (bind && fx_passTerrainPreview)
	//	fx_passTerrainPreview->execute();

	if (locTerrainTextureAddress > 0)
		glProgramUniformHandleui64ARB(techTerrainPreviewFragment, locTerrainTextureAddress, address);
		//glUniformHandleui64ARB(locTerrainTextureAddress, address);

	//if (bind && fx_passTerrainPreview)
	//	fx_passTerrainPreview->unbindProgram();
}


void ParticleShaderFX::checkCompileStatus(GLuint shader, const char *shadername)
{
    GLint  compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled)
    {
        std::cerr << shadername << " failed to compile:" << std::endl;
        GLint  logSize;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
        char* logMsg = new char[logSize];
        glGetShaderInfoLog(shader, logSize, NULL, logMsg);
        std::cerr << logMsg << std::endl;
        delete[] logMsg;
    }
}

void ParticleShaderFX::checkLinkStatus(GLuint program, const char * programName)
{
    GLint  linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        std::cerr << "Shader program " << programName << " failed to link" << std::endl;
        GLint  logSize;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);
        char* logMsg = new char[logSize];
        glGetProgramInfoLog(program, logSize, NULL, logMsg);
        std::cerr << logMsg << std::endl;
        delete[] logMsg;
        system("Pause");
        exit(0);
    }
}

GLuint ParticleShaderFX::loadComputeShader(const char* computeShaderName)
{
	FILE *fp = nullptr;
	fopen_s(&fp, computeShaderName, "r");
	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		size_t fileLen = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		char  *buffer = new char[ fileLen + 1 ];
		const GLcharARB*  bufferARB = buffer;

		GLint   len = (GLint) fileLen;

		// read shader from file
		memset( &buffer[0], 0, sizeof(char)*(len + 1) );
		
		size_t readlen = fread(buffer, sizeof(char), fileLen, fp);
  
		// trick to zero all outside memory
		memset( &buffer[readlen], 0, sizeof(char)*(len + 1 - readlen) );

		if (readlen == 0) //(readlen != len)
		{
			//ERR("glsl shader file size" );
			fclose(fp);
			return 0;
		}

		GLuint shaderCompute = glCreateShader(GL_COMPUTE_SHADER);
		glShaderSource(shaderCompute, 1, &bufferARB, NULL);
		glCompileShader(shaderCompute);
		checkCompileStatus(shaderCompute, computeShaderName);

		GLuint programCompute = glCreateProgram();
		glAttachShader(programCompute, shaderCompute);
		glLinkProgram(programCompute);
		checkLinkStatus(programCompute, computeShaderName);

		if (buffer) {
			delete[] buffer;
			buffer = NULL;
		}

		fclose(fp);

		return programCompute;
	}

    return 0;
}

bool ParticleShaderFX::LoadSurfaceComputeShaders(const char *path, const int pathLen)
{
	if ( nullptr == mComputeSurface.get() )
	{
		CComputeProgram *pNewProgram = nullptr; 
		try
		{
			pNewProgram = new CComputeProgram();

			if (nullptr == pNewProgram)
				throw std::exception("not enough memory");

			bool lSuccess;
			
			lSuccess = pNewProgram->PrepProgram(path);

			if (false == lSuccess)
				throw std::exception("prep program failed");
		}
		catch (const std::exception &e)
		{
			LOGE ("Failed to load a shader - %s\n", e.what() );

			if (nullptr != pNewProgram)
			{
				delete pNewProgram;
				pNewProgram = nullptr;
			}
		}
		
		//
		mComputeSurface.reset(pNewProgram);
	}

	return (nullptr != mComputeSurface.get() );
}

bool ParticleShaderFX::RunSurfaceComputeShader(const int numberOfTriangles)
{
	if (nullptr == mComputeSurface.get() )
		return false;

	const GLuint programId = mComputeSurface->GetProgramId();
	if (programId == 0)
		return false;

	mComputeSurface->Bind();

	GLint loc = glGetUniformLocation( programId, "numberOfTriangles" );
	if (loc >= 0)
		glProgramUniform1i( programId, loc, numberOfTriangles );
	
	const int computeLocalX = 1024;
	const int x = numberOfTriangles / computeLocalX + 1;

	mComputeSurface->DispatchPipeline( x, 1, 1 );

	mComputeSurface->UnBind();

//	CHECK_GL_ERROR_MOBU();

	glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );
	return true;
}