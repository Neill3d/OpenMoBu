
// FBModelProxy.cpp
/*
Sergei <Neill3d> Solokhin 2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "FBModelProxy.h"
#include "SuperShader.h"
#include "glm_utils.h"
#include <glm/gtc/type_ptr.hpp>

namespace Graphics
{
	void BindUniformMatrix(const int location, const FBMatrix& matrix)
	{

		const float tm[16] = { static_cast<float>(matrix[0]), static_cast<float>(matrix[1]), static_cast<float>(matrix[2]), static_cast<float>(matrix[3]),
						static_cast<float>(matrix[4]), static_cast<float>(matrix[5]), static_cast<float>(matrix[6]), static_cast<float>(matrix[7]),
						static_cast<float>(matrix[8]), static_cast<float>(matrix[9]), static_cast<float>(matrix[10]), static_cast<float>(matrix[11]),
						static_cast<float>(matrix[12]), static_cast<float>(matrix[13]), static_cast<float>(matrix[14]), static_cast<float>(matrix[15]) };
		glUniformMatrix4fv(location, 1, GL_FALSE, tm);
	}

	void RenderModel(FBModel* model, const bool useNormalAttrib, const GLint modelMatrixLoc, const GLuint programId)
	{
		if (modelMatrixLoc >= 0)
		{
			FBMatrix modelMatrix;
			model->GetMatrix(modelMatrix, kModelTransformation_Geometry);
			BindUniformMatrix(modelMatrixLoc, modelMatrix);
		}
		
		FBModelVertexData* vertexData = model->ModelVertexData;

		//Get number of region mapped by different materials.
		const int lSubRegionCount = vertexData->GetSubRegionCount();
		if (lSubRegionCount == 0)
			return;
		
		//Set up vertex buffer object (VBO) or vertex array
		vertexData->EnableOGLVertexData();

		const GLuint id = vertexData->GetVertexArrayVBOId(kFBGeometryArrayID_Point);
		const GLvoid* positionOffset = vertexData->GetVertexArrayVBOOffset(kFBGeometryArrayID_Point);
		const GLuint uvId = vertexData->GetUVSetVBOId();
		const GLvoid* uvOffset = vertexData->GetUVSetVBOOffset();

		glBindBuffer(GL_ARRAY_BUFFER, id);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, positionOffset);
		glEnableVertexAttribArray(0);
		
		if (uvId > 0)
		{
			glBindBuffer(GL_ARRAY_BUFFER, uvId);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, uvOffset);
			glEnableVertexAttribArray(1);
		}
		
		if (useNormalAttrib)
		{
			const GLuint normalId = vertexData->GetVertexArrayVBOId(kFBGeometryArrayID_Normal);
			const GLvoid* normalOffset = vertexData->GetVertexArrayVBOOffset(kFBGeometryArrayID_Normal);

			glBindBuffer(GL_ARRAY_BUFFER, normalId);
			glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, normalOffset);
			glEnableVertexAttribArray(2);
		}
		
		const GLuint SAMPLER_SLOT_TRANSPARENCY{ 1 };
		glm::mat4 textureMatrix;

		const GLint useTransparencyLocation = glGetUniformLocation(programId, "useTransparency");
		const GLint transparencyTransformLoc = glGetUniformLocation(programId, "transparencyTransform");

		for (int lSubPatchIndex = 0; lSubPatchIndex < vertexData->GetSubPatchCount(); ++lSubPatchIndex)
		{
			bool useTransparency = false;
			GLuint textureId = 0;
			
			if (FBMaterial* material = vertexData->GetSubPatchMaterial(lSubPatchIndex))
			{
				double lTransparencyFactor = material->TransparencyFactor;
				if (material->GetTexture(kFBMaterialTextureTransparent) && lTransparencyFactor > 0.0)
				{
					useTransparency = true;

					const double* tm = material->GetTexture(kFBMaterialTextureTransparent)->GetMatrix();
					FBMatrixToGLM(textureMatrix, tm);

					textureId = SuperShader::GetTextureId(material, kFBMaterialTextureTransparent, false);
				}
			}
			
			glUniform1f(useTransparencyLocation, useTransparency ? 1.0f : 0.0f);

			if (useTransparency)
			{
				glUniformMatrix4fv(transparencyTransformLoc, 1, GL_FALSE, glm::value_ptr(textureMatrix));
			}
			
			if (textureId > 0)
			{	
				glActiveTexture(GL_TEXTURE0 + SAMPLER_SLOT_TRANSPARENCY);
				glBindTexture(GL_TEXTURE_2D, textureId);
				glActiveTexture(GL_TEXTURE0);
			}
			
			vertexData->DrawSubPatch(lSubPatchIndex);
			
			if (textureId > 0)
			{
				glActiveTexture(GL_TEXTURE0 + SAMPLER_SLOT_TRANSPARENCY);
				glBindTexture(GL_TEXTURE_2D, 0);
				glActiveTexture(GL_TEXTURE0);
			}
		}
			
		glDisableVertexAttribArray(0);
		//glDisableVertexAttribArray(1); // NOTE: don't disable here, as it's an active attribute for the model shading rendering itself
		if (useNormalAttrib)
		{
			glDisableVertexAttribArray(2);
		}
		
		vertexData->DisableOGLVertexData();
		
	}

	bool FBModelProxy::IsCastsShadows() const
	{
		if (FBModel* model = modelPlug)
		{
			return model->CastsShadows;
		}
		return false;
	}

	bool FBModelProxy::IsReceiveShadows() const
	{
		if (FBModel* model = modelPlug)
		{
			return model->ReceiveShadows;
		}
		return false;
	}

	void FBModelProxy::Render(bool useNormalAttrib, GLint modelMatrixLoc, GLint normalMatrixLoc, GLuint programId)
	{
		if (FBModel* model = modelPlug)
		{
			FBModelVertexData* vertexData = model->ModelVertexData;
			
			if (vertexData && vertexData->IsDrawable())
			{
				if (normalMatrixLoc >= 0)
				{
					FBMatrix mv;
					FBMatrix normalMatrix;

					model->GetMatrix(normalMatrix, kModelTransformation_Geometry);
					FBMatrixMult(normalMatrix, mv, normalMatrix);
					FBMatrixInverse(normalMatrix, normalMatrix);
					FBMatrixTranspose(normalMatrix, normalMatrix);
					BindUniformMatrix(6, normalMatrix);
				}
				
				RenderModel(model, useNormalAttrib, modelMatrixLoc, programId);
			}
		}
	}

	
};