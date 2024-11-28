
// FBModelProxy.cpp
/*
Sergei <Neill3d> Solokhin 2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "FBModelProxy.h"


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

	void RenderModel(FBModel* model, const bool useNormalAttrib, const GLint modelMatrixLoc)
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
		if (lSubRegionCount)
		{
			//Set up vertex buffer object (VBO) or vertex array
			vertexData->EnableOGLVertexData();

			const GLuint id = vertexData->GetVertexArrayVBOId(kFBGeometryArrayID_Point);
			const GLvoid* positionOffset = vertexData->GetVertexArrayVBOOffset(kFBGeometryArrayID_Point);
			
			const GLuint indexId = vertexData->GetIndexArrayVBOId();
			
			
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexId);
			
			bool useDrawingFallback = false;

			for (int i = 0; i < vertexData->GetSubPatchCount(); ++i)
			{
				bool isOptimized = false;
				const FBGeometryPrimitiveType primitiveType = vertexData->GetSubPatchPrimitiveType(i, &isOptimized);

				if (primitiveType != FBGeometryPrimitiveType::kFBGeometry_TRIANGLES
					&& primitiveType != FBGeometryPrimitiveType::kFBGeometry_TRIANGLE_FAN
					&& primitiveType != FBGeometryPrimitiveType::kFBGeometry_TRIANGLE_STRIP)
				{
					useDrawingFallback = true;
					break;
				}
			}

			glBindBuffer(GL_ARRAY_BUFFER, id);
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, positionOffset);
			glEnableVertexAttribArray(0);
			
			if (useNormalAttrib)
			{
				const GLuint normalId = vertexData->GetVertexArrayVBOId(kFBGeometryArrayID_Normal);
				const GLvoid* normalOffset = vertexData->GetVertexArrayVBOOffset(kFBGeometryArrayID_Normal);

				glBindBuffer(GL_ARRAY_BUFFER, normalId);
				glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, normalOffset);
				glEnableVertexAttribArray(2);
			}
			
			int* indexArray = nullptr;
			FBVertex* posArray = nullptr;

			if (useDrawingFallback)
			{
				vertexData->VertexArrayMappingRequest();

				indexArray = vertexData->GetIndexArray();
				posArray = static_cast<FBVertex*>(vertexData->GetVertexArray(kFBGeometryArrayID_Point));
			}

			for (int lSubPatchIndex = 0; lSubPatchIndex < vertexData->GetSubPatchCount(); ++lSubPatchIndex)
			{
				bool isOptimized = false;
				const FBGeometryPrimitiveType primitiveType = vertexData->GetSubPatchPrimitiveType(lSubPatchIndex, &isOptimized);

				const int offset = vertexData->GetSubPatchIndexOffset(lSubPatchIndex);
				const int size = vertexData->GetSubPatchIndexSize(lSubPatchIndex);

				switch (primitiveType)
				{
				case FBGeometryPrimitiveType::kFBGeometry_TRIANGLES:
					glDrawRangeElements(GL_TRIANGLES, offset, offset + size, size, GL_UNSIGNED_INT, nullptr);
					break;
				case FBGeometryPrimitiveType::kFBGeometry_TRIANGLE_FAN:
					glDrawRangeElements(GL_TRIANGLE_FAN, offset, offset + size, size, GL_UNSIGNED_INT, nullptr);
					break;
				case FBGeometryPrimitiveType::kFBGeometry_TRIANGLE_STRIP:
					glDrawRangeElements(GL_TRIANGLE_STRIP, offset, offset + size, size, GL_UNSIGNED_INT, nullptr);
					break;
				case FBGeometryPrimitiveType::kFBGeometry_QUADS:
				{
					assert(indexArray != nullptr);
					assert(posArray != nullptr);

					// fallback
					glBegin(GL_QUADS);

					for (int j = 0; j < size; j += 4)
					{
						glVertex3fv(posArray[indexArray[offset + j]]);
						glVertex3fv(posArray[indexArray[offset + j + 1]]);
						glVertex3fv(posArray[indexArray[offset + j + 2]]);
						glVertex3fv(posArray[indexArray[offset + j + 3]]);
					}

					glEnd();
				} break;
				default:
					FBTrace("Not supported primitive type %d for color renderer\n", static_cast<int>(primitiveType));
				}
			}
			
			glDisableVertexAttribArray(0);
			if (useNormalAttrib)
			{
				glDisableVertexAttribArray(2);
			}
			
			if (useDrawingFallback)
			{
				vertexData->VertexArrayMappingRelease();
			}

			vertexData->DisableOGLVertexData();
		}
	}


	void FBModelProxy::Render(bool useNormalAttrib, GLint modelMatrixLoc, GLint normalMatrixLoc)
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
				
				RenderModel(model, useNormalAttrib, modelMatrixLoc);
			}
		}
	}

	
};