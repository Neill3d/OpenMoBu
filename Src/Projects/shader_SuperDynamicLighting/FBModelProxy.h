
#pragma once

// FBModelProxy.h
/*
Sergei <Neill3d> Solokhin 2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "ShadowManager.h"
//--- SDK include
#include <fbsdk/fbsdk.h>

namespace Graphics
{
	
	// mobu implementation for model proxy
	class FBModelProxy : public ModelProxy
	{
	public:

		FBModelProxy(FBModel* modelIn)
			: modelPlug(modelIn)
		{}

		virtual ~FBModelProxy()
		{}

		/*
		*  render model under the current opengl context
		* 
		* vertex attributes
		*  0 - positions (vec4)
		*  2 - normals (vec4)
		* uniform matrix binding
		*  5 - model matrix (mat4)
		*  6 - normal matrix (mat4)
		*/
		virtual void Render(bool useNormalAttrib, GLint modelMatrixLoc, GLint normalMatrixLoc) override;

	private:

		HdlFBPlugTemplate<FBModel>	modelPlug;
	};

};