
#pragma once

// glm_utils.h
/*
Sergei <Neill3d> Solokhin 2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include <GL\glew.h>
#include <glm/glm.hpp>

namespace Graphics
{

	inline void FBMatrixToGLM(glm::mat4& matrixOut, const double** matrixIn)
	{
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				matrixOut[i][j] = static_cast<float>(matrixIn[i][j]);
			}
		}
	}


	inline void FBMatrixToGLM(glm::mat4& matrixOut, const double* matrixIn)
	{
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				matrixOut[i][j] = static_cast<float>(matrixIn[i * 4 + j]);
			}
		}
	}

	inline void FBVectorToGLM(glm::vec3& v, FBVector4d vIn)
	{
		v[0] = static_cast<float>(vIn[0]);
		v[1] = static_cast<float>(vIn[1]);
		v[2] = static_cast<float>(vIn[2]);
	}
}