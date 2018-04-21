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

struct csv_data {
	// info
	int  framecount;
	int	 rigidbodycount;
	int  markercount;

	// marker
	struct marker_data {
		int  frame;
		FBVector3d	pos;
		bool occulusion;

		marker_data() {
			frame = 0;
			occulusion = false;
		}
		marker_data(int &_frame, FBVector3d &_pos)
			: frame(_frame)
			, pos(_pos)
		{ occulusion = false; }
		void print() {
			printf( "marker data - %f; %f; %f\n", pos[0], pos[1], pos[2] );
		}
	};
	struct marker {
		int firstframe;
		FBArrayTemplate<marker_data>	data;
		marker() {
			firstframe=0;
		}
		marker(int &_frame)
			: firstframe(_frame)
		{}
		bool GetFramePos(int frame, FBVector3d *pos)
		{
			for (int i=0; i<data.GetCount(); i++)
				if (frame == data[i].frame)
				{
					pos = &(data[i].pos);
					return true;
				}
		
			return false;
		}
	};
	struct marker_filtered : public marker {
		bool  active;
		int activeFrame;

		marker_filtered()
		{
			firstframe = 0;
			active = false;
			activeFrame = 0;
		}
		bool test(int frame, FBVector3d pos)
		{
			bool result = false;
			if (firstframe > frame) return result;

			for(int i=0; i<data.GetCount(); i++)
				if (frame==data[i].frame)
				{
					double dist = sqrt( (data[i].pos[0] + pos[0]) * (data[i].pos[0] + pos[0]) +
										(data[i].pos[1] + pos[1]) * (data[i].pos[1] + pos[1]) +
										(data[i].pos[2] + pos[2]) * (data[i].pos[2] + pos[2]) );
					if (dist < 1.0) {
						result = true;
/*
						if (!active) {
							active = true;
							activeFrame = frame;

							csv_data::marker_data	framedata(frame, pos);
							data.Add(framedata);
						}
						*/
						active = true;
						activeFrame = frame;
						csv_data::marker_data	framedata(frame, pos);
						data.Add(framedata);
					}
				}
			return result;
		}
		void insert(int frame, FBVector3d pos)
		{
			firstframe = frame;
			csv_data::marker_data	framedata(frame, pos);
			data.Add(framedata);
			active=true;
			activeFrame = frame;
		}
		void insertOcculuded(int frame)
		{
			if (!active) {
				FBVector3d pos;
				memset( &pos[0], 0, sizeof(double)*3);
				csv_data::marker_data	framedata(frame, pos);
				framedata.occulusion = true;
				data.Add(framedata);
			}
		}
	};

	FBArrayTemplate<marker>  markers;
	FBArrayTemplate<marker_filtered>	 markers_filtered;

	// rigidbody
	struct rigidbody_data {
		int frame;
		FBVector3d	pos;
		FBVector4d	quat; // quaternion rotation
		FBVector3d	rot;  // euler angles

		rigidbody_data() {
			frame = 0;
		}
		rigidbody_data(int &_frame, FBVector3d &_pos, FBVector4d &_quat, FBVector3d &_rot)
			: frame(_frame)
			, pos(_pos)
			, quat(_quat)
			, rot(_rot)
		{}
	};
	struct rigidbody {
		char	name[80];

		int markercount;
		FBArrayTemplate<marker_data>	marker_setup;

		int firstframe;
		FBArrayTemplate<rigidbody_data>		data;
		rigidbody() {
			firstframe = 0;
			markercount = 0;
		}
		rigidbody(int &_frame, int &_markercount)
			: firstframe(_frame)
			, markercount(_markercount)
		{}
		void print_marker_setup()
		{
			if (marker_setup.GetCount() != markercount)
				printf("!! > error: marker_setup count is not equal to variable!\n" );
			for(int i=0; i<markercount; i++)
				marker_setup[i].print();
		}
	};

	FBArrayTemplate<rigidbody*>	rigidbodies;

	// constructor
	csv_data() {
		framecount = 0;
		rigidbodycount = 0;
		markercount = 0;
	}
	void FreeData() {
		for(int i=0; i<rigidbodycount; i++) {
			if (rigidbodies[i]) {
				delete rigidbodies[i];
				rigidbodies[i] = NULL;
			}
		}
		rigidbodies.Clear();
		rigidbodycount = 0;
		framecount = 0;
		markercount = 0;

		markers.Clear();
		markers_filtered.Clear();
	}
	~csv_data() {
		FreeData();
	}
};

class ORImportCsvEngine
{
public:
    bool	    ImportFile		( const char* pFileName );

private:
	
	FBModelOptical		*CreateOpticalModel(int frameCount);

	void				CreateOpticalRigidBody( char* line );
	void				ImportFrameData( char* line );
	void				FilterFrameData();

	FBAnimationNode		*FindAnimationNode	( FBAnimationNode *pNode, char* pName );

private:
    FBSystem    mSystem;

	csv_data	csv;
};


#endif /* __ORIMPEXP_TOOL_ENGINE_H__ */
