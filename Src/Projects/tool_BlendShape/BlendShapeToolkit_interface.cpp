
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: BlendShapeToolkit_interface.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//#include "BlendShapeToolkit_interface.h"
#include "BlendShapeToolkit_manip.h"

extern "C"
{
	void AddManualBlendShape()
	{
		FBSystem	mSystem;

		int lCnt = mSystem.Manipulators.GetCount();
		for (int i = 0; i < lCnt; i++)
		{
			if (FBIS(mSystem.Manipulators[i], ORManip_Sculpt))
			{
				ORManip_Sculpt* pManip = static_cast<ORManip_Sculpt*>(mSystem.Manipulators[i]);
				ORManip_Sculpt::GlobalAddBlendShape(pManip, false);
				break;
			}

		}
	}
}

