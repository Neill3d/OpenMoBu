//********************************************************************
// Newton Game dynamics 
// copyright 2000
// By Julio Jerez
// VC: 6.0
// simple 4d matrix class
//********************************************************************

#ifndef __dMatrix__
#define __dMatrix__

#include "stdafx.h"
#include "Newton.h"
#include "dVector.h"

class dQuaternion;

// small but fully operational 4x4 matrix class
class dQuaternion;

class dMatrix
{
	public:
	dMatrix ();
	dMatrix (const dVector &front, const dVector &up, const dVector &right, const dVector &posit);
	dMatrix (const dQuaternion &rotation, const dVector &position);

	dVector& operator[] (int i);
	const dVector& operator[] (int i) const;

	dMatrix Inverse () const;
	dMatrix Transpose () const;
	dMatrix Transpose4X4 () const;
	dVector RotateVector (const dVector &v) const;
	dVector UnrotateVector (const dVector &v) const;
	dVector TransformVector (const dVector &v) const;
	dVector UntransformVector (const dVector &v) const;

	void TransformTriplex (void* const dst, int dstStrideInBytes,
						   void* const src, int srcStrideInBytes, int count) const;

	dMatrix operator* (const dMatrix &B) const;


	dVector m_front;
	dVector m_up;
	dVector m_right;
	dVector m_posit;
};


inline dMatrix GetIdentityMatrix()
{
	return dMatrix (dVector (1.0f, 0.0f, 0.0f, 0.0f),
				    dVector (0.0f, 1.0f, 0.0f, 0.0f),
				    dVector (0.0f, 0.0f, 1.0f, 0.0f),
				    dVector (0.0f, 0.0f, 0.0f, 1.0f));
}

inline dMatrix GetZeroMatrix ()
{
	return dMatrix (dVector (0.0f, 0.0f, 0.0f, 0.0f),
					dVector (0.0f, 0.0f, 0.0f, 0.0f),
					dVector (0.0f, 0.0f, 0.0f, 0.0f),
					dVector (0.0f, 0.0f, 0.0f, 0.0f));
}


inline dMatrix::dMatrix ()
{
}

inline dMatrix::dMatrix (
	const dVector &front, 
	const dVector &up,
	const dVector &right,
	const dVector &posit)
	:m_front (front), m_up(up), m_right(right), m_posit(posit)
{
}



inline dVector& dMatrix::operator[] (int  i)
{
	return (&m_front)[i];
}

inline const dVector& dMatrix::operator[] (int  i) const
{
	return (&m_front)[i];
}

inline dMatrix dMatrix::Inverse () const
{
	return dMatrix (dVector (m_front.m_x, m_up.m_x, m_right.m_x, 0.0f),
					dVector (m_front.m_y, m_up.m_y, m_right.m_y, 0.0f),
		            dVector (m_front.m_z, m_up.m_z, m_right.m_z, 0.0f),
		            dVector (- (m_posit % m_front), - (m_posit % m_up), - (m_posit % m_right), 1.0f));
}

inline dMatrix dMatrix::Transpose () const
{
	return dMatrix (dVector (m_front.m_x, m_up.m_x, m_right.m_x, 0.0f),
					dVector (m_front.m_y, m_up.m_y, m_right.m_y, 0.0f),
					dVector (m_front.m_z, m_up.m_z, m_right.m_z, 0.0f),
					dVector (0.0f, 0.0f, 0.0f, 1.0f));
}

inline dMatrix dMatrix::Transpose4X4 () const
{
	return dMatrix (dVector (m_front.m_x, m_up.m_x, m_right.m_x, m_posit.m_x),
					dVector (m_front.m_y, m_up.m_y, m_right.m_y, m_posit.m_y),
					dVector (m_front.m_z, m_up.m_z, m_right.m_z, m_posit.m_z),
					dVector (m_front.m_w, m_up.m_w, m_right.m_w, m_posit.m_w));
							
}

inline dVector dMatrix::RotateVector (const dVector &v) const
{
	return dVector (v.m_x * m_front.m_x + v.m_y * m_up.m_x + v.m_z * m_right.m_x,
					 v.m_x * m_front.m_y + v.m_y * m_up.m_y + v.m_z * m_right.m_y,
					 v.m_x * m_front.m_z + v.m_y * m_up.m_z + v.m_z * m_right.m_z);
}


inline dVector dMatrix::UnrotateVector (const dVector &v) const
{
	return dVector (v % m_front, v % m_up, v % m_right);
}


inline dVector dMatrix::TransformVector (const dVector &v) const
{
	return m_posit + RotateVector(v);
}

inline dVector dMatrix::UntransformVector (const dVector &v) const
{
	return UnrotateVector(v - m_posit);
}


inline void dMatrix::TransformTriplex (
	void* const dstPtr, 
	int dstStrideInBytes,
	void* const srcPtr, 
	int srcStrideInBytes, 
	int count) const
{
	int i;
	dFloat x;
	dFloat y;
	dFloat z;
	dFloat* dst;
	dFloat* src;

	dst = (dFloat*) dstPtr;
	src = (dFloat*) srcPtr;

	dstStrideInBytes /= sizeof (dFloat);
	srcStrideInBytes /= sizeof (dFloat);

	for (i = 0 ; i < count; i ++ ) {
		x = src[0];
		y = src[1];
		z = src[2];
		dst[0] = x * m_front.m_x + y * m_up.m_x + z * m_right.m_x + m_posit.m_x;
		dst[1] = x * m_front.m_y + y * m_up.m_y + z * m_right.m_y + m_posit.m_y;
		dst[2] = x * m_front.m_z + y * m_up.m_z + z * m_right.m_z + m_posit.m_z;
		dst += dstStrideInBytes;
		src += srcStrideInBytes;
	}
}


inline dMatrix dMatrix::operator* (const dMatrix &B) const
{
	const dMatrix& A = *this;
	return dMatrix (dVector (A[0][0] * B[0][0] + A[0][1] * B[1][0] + A[0][2] * B[2][0] + A[0][3] * B[3][0],
							 A[0][0] * B[0][1] + A[0][1] * B[1][1] + A[0][2] * B[2][1] + A[0][3] * B[3][1],
							 A[0][0] * B[0][2] + A[0][1] * B[1][2] + A[0][2] * B[2][2] + A[0][3] * B[3][2],
	                         A[0][0] * B[0][3] + A[0][1] * B[1][3] + A[0][2] * B[2][3] + A[0][3] * B[3][3]),
					dVector (A[1][0] * B[0][0] + A[1][1] * B[1][0] + A[1][2] * B[2][0] + A[1][3] * B[3][0],
						     A[1][0] * B[0][1] + A[1][1] * B[1][1] + A[1][2] * B[2][1] + A[1][3] * B[3][1],
							 A[1][0] * B[0][2] + A[1][1] * B[1][2] + A[1][2] * B[2][2] + A[1][3] * B[3][2],
							 A[1][0] * B[0][3] + A[1][1] * B[1][3] + A[1][2] * B[2][3] + A[1][3] * B[3][3]),
					dVector (A[2][0] * B[0][0] + A[2][1] * B[1][0] + A[2][2] * B[2][0] + A[2][3] * B[3][0],
							 A[2][0] * B[0][1] + A[2][1] * B[1][1] + A[2][2] * B[2][1] + A[2][3] * B[3][1],
							 A[2][0] * B[0][2] + A[2][1] * B[1][2] + A[2][2] * B[2][2] + A[2][3] * B[3][2],
							 A[2][0] * B[0][3] + A[2][1] * B[1][3] + A[2][2] * B[2][3] + A[2][3] * B[3][3]),
					dVector (A[3][0] * B[0][0] + A[3][1] * B[1][0] + A[3][2] * B[2][0] + A[3][3] * B[3][0],
							 A[3][0] * B[0][1] + A[3][1] * B[1][1] + A[3][2] * B[2][1] + A[3][3] * B[3][1],
							 A[3][0] * B[0][2] + A[3][1] * B[1][2] + A[3][2] * B[2][2] + A[3][3] * B[3][2],
							 A[3][0] * B[0][3] + A[3][1] * B[1][3] + A[3][2] * B[2][3] + A[3][3] * B[3][3]));
}


// calculate an orthonormal matrix with the front vector pointing on the 
// dir direction, and the up and right are determined by using the Gramm Schidth procedure
inline dMatrix dgGrammSchmidt(const dVector& dir)
{
	dVector up;
	dVector right;
	dVector front (dir); 

	front = front.Scale(1.0f / dSqrt (front % front));
	if (dAbs (front.m_z) > 0.577f) {
		right = front * dVector (-front.m_y, front.m_z, 0.0f);
	} else {
	  	right = front * dVector (-front.m_y, front.m_x, 0.0f);
	}
  	right = right.Scale (1.0f / dSqrt (right % right));
  	up = right * front;

	front.m_w = 0.0f;
	up.m_w = 0.0f;
	right.m_w = 0.0f;
	return dMatrix (front, up, right, dVector (0.0f, 0.0f, 0.0f, 1.0f));
}

inline dMatrix dgPitchMatrix(dFloat ang)
{
	dFloat cosAng;
	dFloat sinAng;
	sinAng = dSin (ang);
	cosAng = dCos (ang);
	return dMatrix (dVector (1.0f,    0.0f,    0.0f, 0.0f), 
					dVector (0.0f,  cosAng,  sinAng, 0.0f),
					dVector (0.0f, -sinAng,  cosAng, 0.0f), 
					dVector (0.0f,    0.0f,    0.0f, 1.0f)); 

}

inline dMatrix dgYawMatrix(dFloat ang)
{
	dFloat cosAng;
	dFloat sinAng;
	sinAng = dSin (ang);
	cosAng = dCos (ang);
	return dMatrix (dVector (cosAng, 0.0f, -sinAng, 0.0f), 
					dVector (0.0f,   1.0f,    0.0f, 0.0f), 
					dVector (sinAng, 0.0f,  cosAng, 0.0f), 
					dVector (0.0f,   0.0f,    0.0f, 1.0f)); 
}

inline dMatrix dgRollMatrix(dFloat ang)
{
	dFloat cosAng;
	dFloat sinAng;
	sinAng = dSin (ang);
	cosAng = dCos (ang);
	return dMatrix (dVector ( cosAng, sinAng, 0.0f, 0.0f), 
					dVector (-sinAng, cosAng, 0.0f, 0.0f),
					dVector (   0.0f,   0.0f, 1.0f, 0.0f), 
					dVector (   0.0f,   0.0f, 0.0f, 1.0f)); 
}																		 





#endif
