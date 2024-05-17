
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: BlendShapeToolkit_brushes.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "BlendShapeToolkit_brushes.h"
#include "math3d.h"
#include "BlendShapeToolkit_Helper.h"

////////////////////////////////////////////////////////////////////////////////////
// MOVE BRUSH
////////////////////////////////////////////////////////////////////////////////////

BrushDrag::BrushDrag( FBComponent *pMaster )
	: BaseBrush( "Drag", pMaster )
{
}

BrushDrag::~BrushDrag()
{
}

void BrushDrag::Process( const BrushData &brushData, BrushCameraData *pCameraData, const OperationBuffer &bufferZero, OperationBuffer &buffer )
{
	if (pCameraData == nullptr)
		return;

	const double strength = brushData.strength;
	const int count = buffer.vertices.size();

	FBVector3d initdelta = pCameraData->deltaView;
	VectorMult( initdelta, strength );

	for (int i=0; i<count; ++i)
	{
		if ( (buffer.vertices[i].weight > 0.0) && (buffer.vertices[i].freeze < 1.0) )
		{
			FBVector3d pos(buffer.vertices[i].position);
			FBVector3d delta(initdelta);

			VectorMult( delta, buffer.vertices[i].weight * (1.0f - buffer.vertices[i].freeze) );
			pos = VectorAdd( pos, delta );

			buffer.vertices[i].position = pos;
		}
	}
}


////////////////////////////////////////////////////////////////////////////////////
// PUSH BRUSH
////////////////////////////////////////////////////////////////////////////////////

BrushPush::BrushPush( FBComponent *pMaster )
	: BaseBrush( "Push", pMaster )
{
}

BrushPush::~BrushPush()
{
}

void BrushPush::Process( const BrushData &brushData, BrushCameraData *pCameraData, const OperationBuffer &bufferZero, OperationBuffer &buffer )
{
	if (pCameraData == nullptr)
		return;

	const int count = buffer.vertices.size();
	if (count == 0) return;

	FBVector3d dir;

	switch( pCameraData->direction )
	{
	case kFBBrushScreen:
		dir = FBVector3d(pCameraData->viewPlane);
		break;
	case kFBBrushAverageNormal:
		{
			for (int i=0; i<count; ++i)
			{
				FBVector3d f = FBVector3d(buffer.vertices[i].normal[0], buffer.vertices[i].normal[1], buffer.vertices[i].normal[2]);
				dir = VectorAdd( dir, f );
			}
			
			VectorMult( dir, 1.0 / count );
			VectorNormalize( dir );

		} break;
	case kFBBrushFirstNormal:
		dir = FBVector3d(buffer.vertices[0].normal[0], buffer.vertices[0].normal[1], buffer.vertices[0].normal[2]);
		break;
	case kFBBrushX:
		dir = FBVector3d( 1.0, 0.0, 0.0 );
		break;
	case kFBBrushY:
		dir = FBVector3d( 0.0, 1.0, 0.0 );
		break;
	case kFBBrushZ:
		dir = FBVector3d( 0.0, 0.0, 1.0 );
		break;
	}

	for (int i=0; i<count; ++i)
	{
		if ( (buffer.vertices[i].weight > 0.0) && (buffer.vertices[i].freeze < 1.0) )
		{

			FBVector3d pos = buffer.vertices[i].position;
		
			FBVector3d f(dir);

			if (pCameraData->direction == kFBBrushVertexNormal)
			{
				f = FBVector3d(buffer.vertices[i].normal[0], buffer.vertices[i].normal[1], buffer.vertices[i].normal[2]);
			}

			VectorMult( f, -1.0 * brushData.strength * buffer.vertices[i].weight * (1.0f - buffer.vertices[i].freeze) );

			buffer.vertices[i].position = VectorAdd( pos, f );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////
// FREEZE BRUSH
////////////////////////////////////////////////////////////////////////////////////

BrushFreeze::BrushFreeze( FBComponent *pMaster )
	: BaseBrush( "Freeze", pMaster )
{
}

BrushFreeze::~BrushFreeze()
{
}

void BrushFreeze::Process( const BrushData &brushData, BrushCameraData *pCameraData, const OperationBuffer &bufferZero, OperationBuffer &buffer )
{
	int count = buffer.vertices.size();
	const double strength = brushData.strength;

	for (int i=0; i<count; ++i)
	{
		double freeze = buffer.vertices[i].freeze + strength * buffer.vertices[i].weight;
		if (freeze < 0.0) freeze = 0.0;
		if (freeze > 1.0) freeze = 1.0;
		buffer.vertices[i].freeze = freeze;
	}
}

////////////////////////////////////////////////////////////////////////////////////
// SMOOTH BRUSH
////////////////////////////////////////////////////////////////////////////////////

BrushSmooth::BrushSmooth( FBComponent *pMaster )
	: BaseBrush( "Smooth", pMaster )
{
}

BrushSmooth::~BrushSmooth()
{
}

void BrushSmooth::Process( const BrushData &brushData, BrushCameraData *pCameraData, const OperationBuffer &bufferZero, OperationBuffer &buffer )
{
	int count = buffer.vertices.size();

	double strength = brushData.strength;
	if (strength < 0.0) strength = 0.0;
	if (strength > 1.0) strength = 1.0;

	for (int i=0; i<count; ++i)
	{

		if ( (buffer.vertices[i].weight > 0.0) && (buffer.vertices[i].freeze < 1.0) )
		{
			// compute neighbories average position
		
			BrushData &data = (BrushData&) brushData;
			IntSet &lSet = data.GetVertexNeighbores(i);

			FBVector3d avg, pos;
			int total = 0;

			for (auto iter=lSet.begin(); iter!=lSet.end(); ++iter)
			{
				int index = *iter;

				pos = buffer.vertices[index].position;

				avg = VectorAdd(avg, pos);
				total++;
			}

			if (total > 0) 
			{
				VectorMult( avg, 1.0 / total );

				pos = VectorSubtract( avg, buffer.vertices[i].position );
				VectorMult( pos, strength * buffer.vertices[i].weight * (1.0f - buffer.vertices[i].freeze) );
				buffer.vertices[i].position = VectorAdd( buffer.vertices[i].position, pos );
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////
// ERASE BRUSH
////////////////////////////////////////////////////////////////////////////////////

BrushErase::BrushErase( FBComponent *pMaster )
	: BaseBrush( "Erase", pMaster )
{
}

BrushErase::~BrushErase()
{
}

void BrushErase::Process( const BrushData &brushData, BrushCameraData *pCameraData, const OperationBuffer &bufferZero, OperationBuffer &buffer )
{
	int count = buffer.vertices.size();

	FBVector3d zero, pos;

	double strength = brushData.strength;
	if (strength < 0.0) strength = 0.0;
	if (strength > 1.0) strength = 1.0;

	for (int i=0; i<count; ++i)
	{
		if ( (buffer.vertices[i].weight > 0.0) && (buffer.vertices[i].freeze < 1.0) )
		{
			zero = bufferZero.vertices[i].position;
			pos = buffer.vertices[i].position;

			zero = VectorSubtract( zero, pos );
			VectorMult( zero, strength * buffer.vertices[i].weight * (1.0f - buffer.vertices[i].freeze) );

			buffer.vertices[i].position = VectorAdd( pos, zero );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////
// PAINT BRUSH
////////////////////////////////////////////////////////////////////////////////////

BrushPaint::BrushPaint( FBComponent *pMaster )
	: BaseBrush( "Paint", pMaster )
{
}

BrushPaint::~BrushPaint()
{
}

FBColorF	Mix(const FBColorF color1, const FBColorF color2, const double f)
{
	FBColorF result;

	result[0] = color1[0] * (1.0 - f) + color2[0] * f;
	result[1] = color1[1] * (1.0 - f) + color2[1] * f;
	result[2] = color1[2] * (1.0 - f) + color2[2] * f;

	if (result[0] > 1.0f) result[0] = 1.0f;
	if (result[1] > 1.0f) result[1] = 1.0f;
	if (result[2] > 1.0f) result[2] = 1.0f;

	return result;
}

void BrushPaint::Process( const BrushData &brushData, BrushCameraData *pCameraData, const OperationBuffer &bufferZero, OperationBuffer &buffer )
{
	int count = buffer.vertices.size();
	double strength = brushData.strength;

	if (strength < 0.0) strength = 0.0;
	if (strength > 1.0) strength = 1.0;

	for (int i=0; i<count; ++i)
	{
		double f = strength;
		if (brushData.fillMode == false) 
			f *= buffer.vertices[i].weight;

		buffer.vertices[i].color = Mix( buffer.vertices[i].color, brushData.color, f );
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//

double FalloffHard::Calculate(const double t)
{
	return (t < 0.1) ? smoothstep(0.0, 0.1, t) : 1.0;
}

double FalloffPoint::Calculate(const double t)
{
	return (t > 0.9) ? smoothstep(0.9, 1.0, t) : 0.0;
}

double FalloffSmooth::Calculate(const double t)
{
	return smoothstep(0.0, 1.0, t);
}