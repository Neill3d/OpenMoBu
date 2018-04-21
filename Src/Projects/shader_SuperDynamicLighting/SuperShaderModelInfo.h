
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: SuperShaderModelInfo.h
//
// Sergei <Neill3d> Solokhin 2018
//
// GitHub page - https://github.com/Neill3d/OpenMoBu
// Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////
// Organize object data and second uv and store this information in shaderModelInfo struct
//

//-- 
#include <GL\glew.h>

//--- SDK include
#include <fbsdk/fbsdk.h>


////////////////////////////////////////////////////////////////////
// per model information for storing second lightmap uvs (if needed)

const unsigned int	kFBGeometryArrayID_SecondUVSet          =  1 << 5;     //!< ID to the Second UVSet Array

class SuperShaderModelInfo : public FBShaderModelInfo
{
public:

	//! a constructor
	SuperShaderModelInfo(FBShader* pShader, HKModelRenderInfo pInfo, int pSubRegionIndex);

	//! a destructor
	virtual ~SuperShaderModelInfo();

	//! To be overloaded, always be called when Model or Shader version out of date.
	virtual void UpdateModelShaderInfo(int pShader_Version) override;

	// VAO
	void	Bind();
	void	UnBind();

	// only uv buffer
	void	BindUVBuffer(const GLuint locationId);
	void	UnBindUVBuffer();
	/*
	const int GetCachedMeshIndex() const {
		return mMeshIndex;
	}
	*/
protected:

	FBShader			*mShader;

	// buffer for support 2 sets of UVs
	int					mBufferCount;
	GLuint				mBufferId;
	GLuint				mLocationId;
	
	// 
	struct BufferData
	{
		GLuint		location;
		GLuint		id;
		GLuint		components;
		void		*pOffset;

		BufferData(GLuint _location, GLuint _components)
			: location(_location)
			, components(_components)
		{
			id = 0;
			pOffset = nullptr;
		}

		void Bind();
		void UnBind();
	};

	struct VertexData
	{
		BufferData		position;
		BufferData		normal;
		BufferData		uv;
		BufferData		tangent;
		BufferData		uv2;
		
		GLuint			indexId;
		void			*indexOffset;

		//! a constructor
		VertexData();

		void Bind();
		void UnBind();

		bool IsReady();

		void AssignBuffers(GLuint _pos, GLuint _nor, GLuint _tangent, GLuint _uv, GLuint _uv2, GLuint _index);
		void AssignBufferOffsets(void *_pos, void *_nor, void *_tangent, void *_uv, void *_uv2, void *_index);
	};

	VertexData			mVertexData;

	// cached mesh index in the CGPUFBScene
	//int					mMeshIndex;

	// UV sets

	bool			IsSecondUVSetNeeded();
	bool			PrepareUVSets();	// make a buffer with two uvsets inside
	bool			VertexDataFromFBModel(bool processTangentBuffer);
};