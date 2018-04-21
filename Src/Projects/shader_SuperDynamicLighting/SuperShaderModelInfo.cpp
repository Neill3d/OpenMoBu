
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: SuperShaderModelInfo.cpp
//
// Sergei <Neill3d> Solokhin 2018
//
// GitHub page - https://github.com/Neill3d/OpenMoBu
// Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "SuperShaderModelInfo.h"

#define VERTEX_ATTRIBUTE_POSITION		0
#define VERTEX_ATTRIBUTE_NORMAL			2
#define VERTEX_ATTRIBUTE_UV				1
#define VERTEX_ATTRIBUTE_TANGENT		3
#define VERTEX_ATTRIBUTE_UV2			4

typedef unsigned char       BYTE;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

void SuperShaderModelInfo::BufferData::Bind()
{
	if (id > 0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, id);
		glVertexAttribPointer(location, components, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)pOffset);
	}
}

void SuperShaderModelInfo::BufferData::UnBind()
{
	if (id > 0)
		glBindBuffer(GL_ARRAY_BUFFER, id);
}

SuperShaderModelInfo::VertexData::VertexData()
:position(VERTEX_ATTRIBUTE_POSITION, 4)
, normal(VERTEX_ATTRIBUTE_NORMAL, 4)
, uv(VERTEX_ATTRIBUTE_UV, 2)
, tangent(VERTEX_ATTRIBUTE_TANGENT, 4)
, uv2(4, 2)
{

}

void SuperShaderModelInfo::VertexData::Bind()
{
	position.Bind();
	normal.Bind();
	uv.Bind();
	tangent.Bind();
	uv2.Bind();
}

void SuperShaderModelInfo::VertexData::UnBind()
{
	
}

bool SuperShaderModelInfo::VertexData::IsReady()
{
	return (position.id > 0);
}

void SuperShaderModelInfo::VertexData::AssignBuffers(GLuint _pos, GLuint _nor, GLuint _tangent, GLuint _uv, GLuint _uv2, GLuint _index)
{
	position.id = _pos;
	normal.id = _nor;
	tangent.id = _tangent;
	uv.id = _uv;
	uv2.id = _uv2;
	indexId = _index;
}
void SuperShaderModelInfo::VertexData::AssignBufferOffsets(void *_pos, void *_nor, void *_tangent, void *_uv, void *_uv2, void *_index)
{
	position.pOffset = _pos;
	normal.pOffset = _nor;
	tangent.pOffset = _tangent;
	uv.pOffset = _uv;
	uv2.pOffset = _uv2;
	indexOffset = _index;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

SuperShaderModelInfo::SuperShaderModelInfo(FBShader* pShader, HKModelRenderInfo pInfo, int pSubRegionIndex)
	: FBShaderModelInfo(pShader, pInfo, pSubRegionIndex)
	, mShader(pShader)
{
	mBufferCount = 0;
	mBufferId = 0;
	mLocationId = 0;

	//mMeshIndex = 0;
}

//! a destructor
SuperShaderModelInfo::~SuperShaderModelInfo()
{
	if (mBufferId)
	{
		glDeleteBuffers( 1, &mBufferId );
		mBufferId = 0;
	}

	//FBShaderModelInfo::~FBShaderModelInfo();
}

bool SuperShaderModelInfo::IsSecondUVSetNeeded()
{
	FBModel *pModel = GetFBModel();
	if (pModel == nullptr) return false;

	FBGeometry *pGeometry = pModel->Geometry;
	FBStringList uvSets = pGeometry->GetUVSets();

	if (uvSets.GetCount() < 2 ) return false;

	for (int i=0; i<pModel->Textures.GetCount(); ++i)
	{
		FBTexture *pTexture = pModel->Textures[i];
		FBProperty *lProp = pTexture->PropertyList.Find( "UVSet" );
		if (lProp)
		{
			FBString str( lProp->AsString() );
			
			for (int j=1; j<uvSets.GetCount(); ++j)
			{
				FBString uvSetName(uvSets[j]);

				if (str == uvSetName)
				{
					return true;
				}
			}
		}
	}

	return false;
}


bool SuperShaderModelInfo::PrepareUVSets()
{
	FBModel *pModel = GetFBModel();
	if (pModel == nullptr) return false;

	FBModelVertexData *lModelVertexData = pModel->ModelVertexData;
	FBGeometry *pGeometry = pModel->Geometry;
	const GLuint uvId = lModelVertexData->GetUVSetVBOId();

	FBString uvset("");
	FBStringList uvSets = pGeometry->GetUVSets();

	if (uvSets.GetCount() < 2) 
	{
		mBufferId = uvId;
		return false;
	}

	for (int i=0; i<pModel->Textures.GetCount(); ++i)
	{
		FBTexture *pTexture = pModel->Textures[i];
		FBProperty *lProp = pTexture->PropertyList.Find( "UVSet" );
		if (lProp)
		{
			FBString str(lProp->AsString() );

			for (int j=0; j<uvSets.GetCount(); ++j)
				if ( str == uvSets[j] )
				{
					uvset = lProp->AsString();
					break;
				}
		}
	}

	if (uvset == "")
	{
		mBufferId = uvId;
		return false;
	}

	//
	// lets manually prepare second uvset
	
	// TODO: include two uv sets in one buffer !
				
	int uvIndCount = 0;
	int uvCount = 0;
			
	FBGeometryReferenceMode refMode = pGeometry->GetUVSetReferenceMode( uvset );
	FBGeometryMappingMode mapping = pGeometry->GetUVSetMappingMode( uvset );
			
	int *uvIndices = pGeometry->GetUVSetIndexArray(uvIndCount, uvset );
	FBUV *uvs = pGeometry->GetUVSetDirectArray( uvCount, uvset );

	//int *indices = lModelVertexData->GetIndexArray();

	int vertCount = pGeometry->VertexCount();
	int vertCountRenderable = lModelVertexData->GetVertexCount();

	int dublicatedCount = 0;
	const int *dublicatedIndices = lModelVertexData->GetVertexArrayDuplicationMap( (unsigned int &) dublicatedCount );

	if (dublicatedCount < 0)
	{
		dublicatedCount = 0;
		dublicatedIndices = nullptr;
	}

	int numPolyIndices = 0;
	const int *polyIndices = ( (FBMesh*) pGeometry)->PolygonVertexArrayGet( numPolyIndices );

		
	if (mBufferId && (uvIndCount != mBufferCount) )
	{
		glDeleteBuffers(1, &mBufferId);
		mBufferId = 0;
	}

	if (mBufferId == 0)
	{
				
		switch(mapping)
		{
		case kFBGeometryMapping_BY_CONTROL_POINT:
			break;

		case kFBGeometryMapping_BY_POLYGON_VERTEX:

			//printf ("polygon vertex" );

			break;
		}

		FBUV *temp = new FBUV[vertCountRenderable];
		BYTE *flags = new BYTE[vertCount];
		BYTE *dubFlags = (dublicatedCount>0) ? new BYTE[dublicatedCount] : nullptr;

		memset( flags, 0, sizeof(BYTE) * vertCount );
		
		if (dublicatedCount > 0)
			memset( dubFlags, 0, sizeof(BYTE) * dublicatedCount );

		//int lastUpperIndex = vertCount;

		for (int i=0; i<uvIndCount; ++i)
		{
			switch(refMode)
			{
			case kFBGeometryReference_DIRECT:
				temp[i] = uvs[i];
				break;
			case kFBGeometryReference_INDEX_TO_DIRECT:

				// i - index in polygon vertex space
				// idx = index in control point space
				int idx = polyIndices[i];

				FBUV uv = uvs[ uvIndices[i] ];

				if (flags[idx] == 0)
				{
					temp[idx] = uv;
					flags[idx] = 1;
				}
				else
				{
					for (unsigned int j=0; j<dublicatedCount; ++j)
					{
						if ( (dublicatedIndices[j] == idx) && (dubFlags[j] == 0) )
						{
							temp[ vertCount + j ] = uv;

							dubFlags[j] = 1;
							break;
						}
					}

				}

				break;
			}
		}

		glGenBuffers(1, &mBufferId);
		glBindBuffer(GL_ARRAY_BUFFER, mBufferId);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * uvIndCount, temp, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		mBufferCount = uvIndCount;
			
		// free mem
		if (nullptr != dubFlags)
		{
			delete [] dubFlags;
			dubFlags = nullptr;
		}

		if (nullptr != flags)
		{
			delete [] flags;
			flags = nullptr;
		}

		if (nullptr !=temp)
		{
			delete [] temp;
			temp = nullptr;
		}
	}
	
	return true;
}

void SuperShaderModelInfo::BindUVBuffer(const GLuint locationId)
{
	mLocationId = 0;
	// Finally, bind buffer to use it in a shader
	if (mBufferId > 0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, mBufferId);
		glEnableVertexAttribArray(locationId);
		glVertexAttribPointer(locationId, 2, GL_FLOAT, GL_FALSE, sizeof(FBUV), (const GLvoid*) 0 );  // uv coords
		
		mLocationId = locationId;
	}
}

void SuperShaderModelInfo::UnBindUVBuffer()
{
	if (mLocationId)
		glDisableVertexAttribArray(mLocationId);
}

void SuperShaderModelInfo::UpdateModelShaderInfo(int pShader_Version)
{
	FBShaderModelInfo::UpdateModelShaderInfo(pShader_Version);

	//CHECK_GL_ERROR_MOBU();

	const unsigned int arrayIds = GetGeometryArrayIds();

	if ( (arrayIds & kFBGeometryArrayID_SecondUVSet) == kFBGeometryArrayID_SecondUVSet )
	{
		PrepareUVSets();
	}

	//CHECK_GL_ERROR_MOBU();

	// TODO: calculate mesh index for this shader-model combination
	//auto mGPUFBScene = &CGPUFBScene::instance();
	//mMeshIndex = mGPUFBScene->FindMeshIndex( GetFBModel(), GetFBMaterial(), mShader );

	// update render model, prepare VAO for fast attrib setup
	
	const bool processTangentBuffer = arrayIds & kFBGeometryArrayID_Tangent;
	VertexDataFromFBModel(processTangentBuffer);
}

void SuperShaderModelInfo::Bind()
{
	if (mVertexData.IsReady() == false)
	{
		const unsigned int arrayIds = GetGeometryArrayIds();
		const bool processTangentBuffer = arrayIds & kFBGeometryArrayID_Tangent;

		VertexDataFromFBModel(processTangentBuffer);
	}

	mVertexData.Bind();
	BindUVBuffer(VERTEX_ATTRIBUTE_UV2);
}

void SuperShaderModelInfo::UnBind()
{
	UnBindUVBuffer();
	mVertexData.UnBind();
}

bool SuperShaderModelInfo::VertexDataFromFBModel(bool processTangentBuffer)
{
	FBModel *pModel = GetFBModel();
	if (nullptr == pModel) 
		return false;

	FBModelVertexData *pVertexData = pModel->ModelVertexData;
	if (nullptr == pVertexData) 
		return false;

	FBGeometryArrayID	ids[5] = { kFBGeometryArrayID_Point, kFBGeometryArrayID_Normal, kFBGeometryArrayID_Tangent,
		kFBGeometryArrayID_Color, kFBGeometryArrayID_Color };

	void *poffsets[6];

	for (int i=0; i<3; ++i)
	{
		const auto id = ids[i];
		poffsets[i] = pVertexData->GetVertexArrayVBOOffset(id);
	}
	
	poffsets[3] = pVertexData->GetUVSetVBOOffset();
	poffsets[4] = pVertexData->GetUVSetVBOOffset();

	const GLuint positionId = pVertexData->GetVertexArrayVBOId( kFBGeometryArrayID_Point );
	const GLuint normalId = pVertexData->GetVertexArrayVBOId( kFBGeometryArrayID_Normal );
	const GLuint tangentId = (processTangentBuffer) ? pVertexData->GetVertexArrayVBOId( kFBGeometryArrayID_Tangent ) : 0;
//	const GLuint binormalId = (processTangentBuffer) ? pVertexData->GetVertexArrayVBOId( kFBGeometryArrayID_Binormal ) : 0;
	const GLuint uvId = pVertexData->GetUVSetVBOId();
	const GLuint uvId2 = 0;
	const GLuint indexId = 0; // pVertexData->GetIndexArrayVBOId();

	if (0 == positionId || 0 == normalId || 0 == uvId )
		return false;

	mVertexData.AssignBuffers( positionId, normalId, tangentId, uvId, uvId2, indexId );
	mVertexData.AssignBufferOffsets( poffsets[0], poffsets[1], poffsets[2], poffsets[3], poffsets[4], nullptr );

	// compute deformations on GPU
	pVertexData->VertexArrayMappingRelease();

	return true;
}