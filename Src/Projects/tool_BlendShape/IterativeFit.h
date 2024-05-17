
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: iterativeFit.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
#include <fbsdk/fbsdk.h>

//////////////////////////////


struct FitUnit
{
	int		shapeId;
	int		shapePointId;

	FBVertex pos;
	FBVertex target;

	enum STEP_TYPE
	{
		kStepInit,
		kStepDirection,
		kStepRange,
		kStepFit,
		kStepFinish
	};

	

	struct StepAxis
	{
		bool	possitive;
		float	min;
		float	max;

		float	current;

		float	step;
		float	bigStep;

		STEP_TYPE type;

		StepAxis()
		{
			possitive = false;
			min	= 0.0f;
			max = 0.0f;
			current = 0.0f;
			step = 0.0f;
			bigStep = 0.0f;

			type = kStepInit;
		}

		void Step()
		{
			switch (type)
			{
			case kStepInit:
				// generate random values
				break;
			case kStepDirection:
				break;

			};
		}
	};

	StepAxis		stepX;
	StepAxis		stepY;
	StepAxis		stepZ;

	double	target_error;	// when raise that value - we are finish
	double	error;

	//! a constructor
	FitUnit()
	{
		error = 0.0;
		target_error = 0.0;
		
	}

	// determine direction on each axis


	bool step ()
	{
		// calculate error
		error = FBLength( FBVertex( target[0]-pos[0], target[1]-pos[1], target[2]-pos[2] ) );
		error *= error;

		if (error <= target_error) return true;

		stepX.Step();
		stepY.Step();
		stepZ.Step();
	}
};


////////////////////////////////////////////////////////////////////////////////////////////////////////
//

bool BlendShapeIterativeFit( FBModel *pBaseModel, FBModel *pModel );