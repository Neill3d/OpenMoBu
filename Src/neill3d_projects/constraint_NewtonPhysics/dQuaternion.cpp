//********************************************************************
// Newton Game dynamics 
// copyright 2000-2004
// By Julio Jerez
// VC: 6.0
// basic Hierarchical Scene Node Class
//********************************************************************

#include "dVector.h"
#include "dMatrix.h"
#include "dQuaternion.h"



enum QUAT_INDEX
{
   X_INDEX=0,
   Y_INDEX=1,
   Z_INDEX=2
};
static QUAT_INDEX QIndex [] = {Y_INDEX, Z_INDEX, X_INDEX};


dQuaternion::dQuaternion (const dMatrix &matrix)
{
	dFloat *ptr;
	dFloat trace;
	QUAT_INDEX i;
	QUAT_INDEX j;
	QUAT_INDEX k;

	trace = matrix[0][0] + matrix[1][1] + matrix[2][2];

	if (trace > dFloat(0.0f)) {
		trace = 1.0f / dSqrt (trace + 1.0f);
		m_q0 = 0.5f * trace;
		trace = 0.5f / trace;

		m_q1 = (matrix[2][1] - matrix[1][2]) * trace;
		m_q2 = (matrix[0][2] - matrix[2][0]) * trace;
		m_q3 = (matrix[1][0] - matrix[0][1]) * trace;

	} else {
		i = X_INDEX;
		if (matrix[Y_INDEX][Y_INDEX] > matrix[X_INDEX][X_INDEX]) {
			i = Y_INDEX;
		}
		if (matrix[Z_INDEX][Z_INDEX] > matrix[i][i]) {
			i = Z_INDEX;
		}
		j = QIndex [i];
		k = QIndex [j];

		trace = 1.0f + matrix[i][i] - matrix[j][j] - matrix[k][k];
		trace = 1.0f / dSqrt (trace);

		ptr = &m_q1;
		ptr[i] = 0.5f * trace;
		trace = 0.5f / trace;
		ptr[3] = (matrix[k][j] - matrix[j][k]) * trace;
		ptr[j] = (matrix[j][i] + matrix[i][j]) * trace;
		ptr[k] = (matrix[k][i] + matrix[i][k]) * trace;
	}

	m_q1 *= -1;
	m_q2 *= -1;
	m_q3 *= -1;

#if _DEBUG
	dFloat err = dAbs (DotProduct(*this) - 1.0f);
	_ASSERTE (err < dFloat(1.0e-3f));
#endif

}


dQuaternion::dQuaternion (const dVector &unitAxis, dFloat Angle)
{
	dFloat sinAng;

	Angle *= 0.5f;
	m_q0 = dCos (Angle);
	sinAng = dSin (Angle);

#ifdef _DEBUG
	if (dAbs (Angle) > dFloat(1.0e-6f)) {
		_ASSERTE (dAbs (1.0f - unitAxis % unitAxis) < dFloat(1.0e-3f));
	} 
#endif
	m_q1 = unitAxis.m_x * sinAng;
	m_q2 = unitAxis.m_y * sinAng;
	m_q3 = unitAxis.m_z * sinAng;
}


dVector dQuaternion::CalcAverageOmega (const dQuaternion &QB, dFloat dt) const
{
	dFloat dirMag;
	dFloat dirMag2;
	dFloat omegaMag;
	dFloat dirMagInv;

	_ASSERTE (0);
//	dQuaternion dq (Inverse() * QB);
	dQuaternion dq (QB * Inverse());
	dVector omegaDir (dq.m_q1, dq.m_q2, dq.m_q3);

	dirMag2 = omegaDir % omegaDir;
	if (dirMag2	< dFloat(dFloat (1.0e-5f) * dFloat (1.0e-5f))) {
		return dVector (dFloat(0.0f), dFloat(0.0f), dFloat(0.0f), dFloat(0.0f));
	}

	dirMagInv = 1.0f / dSqrt (dirMag2);
	dirMag = dirMag2 * dirMagInv;

	omegaMag = dFloat(2.0f) * dAtan2 (dirMag, dq.m_q0) / dt;

	return omegaDir.Scale (dirMagInv * omegaMag);
}


dQuaternion dQuaternion::Slerp (const dQuaternion &QB, dFloat t) const 
{
	dFloat dot;
	dFloat ang;
	dFloat Sclp;
	dFloat Sclq;
	dFloat den;
	dFloat sinAng;
	dQuaternion Q;

	dot = DotProduct (QB);

	if ((dot + 1.0f) > 1.0e-5f) {
		if (dot < 0.995f) {
			ang = dAcos (dot);

			sinAng = dSin (ang);
			den = 1.0f / sinAng;

			Sclp = dSin ((1.0f - t ) * ang) * den;
			Sclq = dSin (t * ang) * den;

		} else  {
			Sclp = 1.0f - t;
			Sclq = t;
		}

		Q.m_q0 = m_q0 * Sclp + QB.m_q0 * Sclq;
		Q.m_q1 = m_q1 * Sclp + QB.m_q1 * Sclq;
		Q.m_q2 = m_q2 * Sclp + QB.m_q2 * Sclq;
		Q.m_q3 = m_q3 * Sclp + QB.m_q3 * Sclq;

	} else {
		Q.m_q0 =  m_q3;
		Q.m_q1 = -m_q2;
		Q.m_q2 =  m_q1;
		Q.m_q3 =  m_q0;

		Sclp = dSin ((1.0f - t) * (3.1416f * 0.5f));
		Sclq = dSin (t * (3.1416f * 0.5f));

		Q.m_q0 = m_q0 * Sclp + Q.m_q0 * Sclq;
		Q.m_q1 = m_q1 * Sclp + Q.m_q1 * Sclq;
		Q.m_q2 = m_q2 * Sclp + Q.m_q2 * Sclq;
		Q.m_q3 = m_q3 * Sclp + Q.m_q3 * Sclq;
	}

	dot = Q.DotProduct (Q);
	if ((dot) < (1.0f - 1.0e-4f) ) {
		//dot = 1.0f / dSqrt (dot);
		dot = 1.0f / dSqrt (dot);
		Q.m_q0 *= dot;
		Q.m_q1 *= dot;
		Q.m_q2 *= dot;
		Q.m_q3 *= dot;
	}
	return Q;
}



