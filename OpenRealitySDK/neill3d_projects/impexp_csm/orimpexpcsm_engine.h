#ifndef __ORIMPEXP_TOOL_ENGINE_H__
#define __ORIMPEXP_TOOL_ENGINE_H__


/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/MoBu/blob/master/LICENSE
//
// GitHub repository - https://github.com/Neill3d/MoBu
//
// Author Sergey Solohin (Neill3d) 2014
//  e-mail to: s@neill3d.com
//		www.neill3d.com
/////////////////////////////////////////////////////////////////////////////////////////

/**	\file	orimpexpcsv_engine.h
*/

//--- SDK include
#include <fbsdk/fbsdk.h>
#include <fbsdk/fbarray.h>

#include <math.h>

#define MAX_MARKERS		70

enum csm_mode { csm_common, csm_markers, csm_points };

struct csm_data {

	int		firstframe;
	int		lastframe;
	float	framerate;

	struct	marker_data {
		int		frame;
		float	pos[MAX_MARKERS*3];	//
	};

	FBArrayTemplate<FBString>		markers;
	FBArrayTemplate<marker_data>	points;

	csm_data()
	{
		firstframe = 0;
		lastframe = 0;
		framerate = 30.0;
	}
	~csm_data()
	{}
};

class CSMEngine
{
public:
    bool	    ImportFile		( const char* pFileName );
	bool		ExportFile		( const char* pFileName );

private:
	
	FBModelOptical		*CreateOpticalModel(int frameCount);

	void				CreateOpticalRigidBody( char* line );
	void				ImportFrameData( char* line );
	void				FilterFrameData();

	FBAnimationNode		*FindAnimationNode	( FBAnimationNode *pNode, char* pName );

private:
    FBSystem    mSystem;

	csm_mode	mode;
	csm_data	csm;
};


#endif /* __ORIMPEXP_TOOL_ENGINE_H__ */
