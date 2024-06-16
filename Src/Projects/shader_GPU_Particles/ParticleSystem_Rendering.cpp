
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: ParticleSystem_Rendering.cpp
//
//	Author Sergei Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "ParticleSystem.h"
#include <vector>

using namespace GPUParticles;


// const mat4 &VP, const mat4 &MV, const vec3 &CameraPos, float size, bool useSizeCurve, vec4 color, bool useColorCurve, float AlphaFactor, const GLuint tex 

renderBlock	&ParticleSystem::GetRenderData()
{
	return mRenderData;
}

bool ParticleSystem::UploadRenderDataOnGPU()
{
	if (mShader->IsInitialized() == false)
	if (false == mShader->Initialize() )
			return false;

	mShader->UploadRenderDataBlock(mRenderData);

	return true;

	/*
	mRenderData.gCameraPos = vec4(CameraPos.x, CameraPos.y, CameraPos.z, 1.0f);
	mRenderData.gVP = VP;
	mRenderData.gMV = MV;

	mRenderData.gColor = color;
	mRenderData.gUseColorCurve = (useColorCurve) ? 1 : 0;

	mRenderData.gBillboardSize = size;
	mRenderData.gUseSizeCurve = (useSizeCurve) ? 1 : 0;
	*/
}


void ParticleSystem::SetRenderSizeAndColorCurves( GLuint sizeTextureId, GLuint colorTextureId )
{
	mSizeTextureId = sizeTextureId;
	mColorTextureId = colorTextureId;
}

void ParticleSystem::RenderParticles(int type, int lighting, const bool pointSmooth, const bool pointFalloff)
{
	//GLuint lSizeTexId = mSizeCurve.GetTextureId();
	//GLuint lColorTexId = mColorCurve.GetTextureId();

	glDisable(GL_RASTERIZER_DISCARD);
	/*
	// TODO: depricated, size is computed inside the simulation cycle
	if (mSizeTextureId > 0 && mRenderData.gUseSizeCurve > 0.0f)
	{
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_1D, mSizeTextureId);
		glActiveTexture(GL_TEXTURE0);
	}
	*/
	if (mColorTextureId > 0 && mRenderData.gUseColorCurve > 0.0f)
	{
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_1D, mColorTextureId);
		glActiveTexture(GL_TEXTURE0);
	}

	
	//
	switch(type)
	{
	case 0: // points
		{
			if (pointFalloff || mRenderData.gUseColorMap > 0.0f)
				glEnable(GL_POINT_SPRITE);
			else if (pointSmooth)
				glEnable(GL_POINT_SMOOTH);
	
			RenderPoints();

			if (pointFalloff || mRenderData.gUseColorMap > 0.0f)
				glDisable(GL_POINT_SPRITE);
			else if (pointSmooth)
				glDisable(GL_POINT_SMOOTH);
	
		} break;
	case 1:
		RenderQuads();
		break;
	case 2: // billboard
		RenderBillboards();
		break;
	case 3: // stretched billboard
		RenderStretchedBillboards();
		break;
	case 4:
		RenderInstances(lighting);
		break;
	}
	
	/*
	if (mSizeTextureId > 0 && mRenderData.gUseSizeCurve > 0.0f)
	{
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_1D, 0);
		glActiveTexture(GL_TEXTURE0);
	}
			*/
	if (mColorTextureId > 0 && mRenderData.gUseColorCurve > 0.0f)
	{
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_1D, 0);
		glActiveTexture(GL_TEXTURE0);
	}
}


void ParticleSystem::RenderPoints()
{
	
	//glPointSize(mPointSize);
	//glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glEnable(GL_PROGRAM_POINT_SIZE);

	mShader->BindRenderPoints();

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	glBindBuffer(GL_ARRAY_BUFFER, mParticleBuffer[mCurrTFB]);
	
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)0);  // position
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)16);  // velocity
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)32);  // color
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)48);  // rotation
	
	//glDrawTransformFeedback(GL_POINTS, mTransformFeedback[mCurrTFB]);
	glDrawArrays( GL_POINTS, 0, mInstanceCount );

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);

	mShader->UnBindRenderPoints();

	//glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glDisable(GL_PROGRAM_POINT_SIZE);
	
}

void ParticleSystem::RenderQuads()
{
	

	mShader->BindRenderQuads();

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	glBindBuffer(GL_ARRAY_BUFFER, mParticleBuffer[mCurrTFB]);
	
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)0);  // position
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)16);  // velocity
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)32);  // color
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)48);  // rotation
	
	//glDrawTransformFeedback(GL_POINTS, mTransformFeedback[mCurrTFB]);
	glDrawArrays( GL_POINTS, 0, mInstanceCount );

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);

	mShader->UnBindRenderQuads();

}

void ParticleSystem::RenderBillboards()
{
	
	mShader->BindRenderBillboards();

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	glBindBuffer(GL_ARRAY_BUFFER, mParticleBuffer[mCurrTFB]);
	
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)0);  // position
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)16);  // velocity
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)32);  // color
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)48);  // rotation
	
	//glDrawTransformFeedback(GL_POINTS, mTransformFeedback[mCurrTFB]);
	glDrawArrays( GL_POINTS, 0, mInstanceCount );

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);

	mShader->UnBindRenderBillboards();

}

void ParticleSystem::RenderStretchedBillboards()
{
	mShader->BindRenderStretchedBillboards();

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	glBindBuffer(GL_ARRAY_BUFFER, mParticleBuffer[mCurrTFB]);
	
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)0);  // position
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)16);  // velocity
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)32);  // color
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)48);  // rotation
	
	//glDrawTransformFeedback(GL_POINTS, mTransformFeedback[mCurrTFB]);
	glDrawArrays( GL_POINTS, 0, mInstanceCount );

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);

	mShader->UnBindRenderStretchedBillboards();

}


void ParticleSystem::RenderInstances(const int lighting)
{
	if ( nullptr!=mConnections && mConnections->GetNumberOfInstanceSubPatches() > 0 )
	{
		const TInstanceVertexStream &stream = mConnections->GetInstanceVertexStream();
		std::vector<TMeshPatch> &mesh = mConnections->GetInstanceMeshVector();
		//pModelVertexData->EnableOGLVertexData();
		
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		glEnableVertexAttribArray(4);
		glEnableVertexAttribArray(5);
		glEnableVertexAttribArray(6);
		glEnableVertexAttribArray(7);

		glBindBuffer(GL_ARRAY_BUFFER, stream.positionId);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) stream.positionOffset); // position
		
		glBindBuffer(GL_ARRAY_BUFFER, stream.normalId);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) stream.normalOffset); // normals

		glBindBuffer(GL_ARRAY_BUFFER, stream.uvId);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) stream.uvOffset); // normals

		mShader->BindRenderInstances(lighting);

		glActiveTexture(GL_TEXTURE0);
		glBindBuffer(GL_ARRAY_BUFFER, mParticleBuffer[mCurrTFB]);
		
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)0);         // position, normalized lifetime
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)16);        // velocity, lifetime
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)32);        // color
		glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)48);        // age milliseconds

		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);
		glVertexAttribDivisor(7, 1);

		GLuint texId = 0;
		unsigned int baseVertex = 0;
		unsigned int indicesCount = 0;

		if (stream.indexId > 0)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stream.indexId);

			for (auto iter=begin(mesh); iter!=end(mesh); ++iter)
			{
				texId = iter->textureId;
				baseVertex = iter->offset;
				indicesCount = iter->size;

				if (texId > 0)
				{
					glBindTexture(GL_TEXTURE_2D, texId);
				}

				glDrawElementsInstanced(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, (void*) (sizeof(unsigned int) * baseVertex), mInstanceCount);
			}

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(4);
		glDisableVertexAttribArray(5);
		glDisableVertexAttribArray(6);
		glDisableVertexAttribArray(7);
		
		mShader->UnBindRenderInstances();

		glBindTexture(GL_TEXTURE_2D, 0);
		//pModelVertexData->DisableOGLVertexData();
	}
}

void ParticleSystem::SwapBuffers()
{
	//
	// swap buffers
	//
	mCurrVB = mCurrTFB;
    mCurrTFB = (mCurrTFB + 1) & 0x1;
}

void ParticleSystem::SwapSurfaceBuffers()
{
	//
	// swap buffers
	//
	mSurfaceBack = mSurfaceFront;
    mSurfaceFront = (mSurfaceFront + 1) & 0x1;
}