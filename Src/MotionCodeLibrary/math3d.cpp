
/////////////////////////////////////////////////////////////////////////////////////////
//
// math3d.cpp
//
// Sergei <Neill3d> Solokhin 2018
//
// GitHub page - https://github.com/Neill3d/OpenMoBu
// Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
/////////////////////////////////////////////////////////////////////////////////////////

#include <math.h>
#include "math3d.h"

#define		EPSILON		10e-6

//
// Splines
//

bool apprLSS_H ( int n, const double * x, const double * y, int m, double * c )
{
    if ( n <= 0 || m <= 0 ) return false;
    if ( n < m )
    {
        for ( int j = n; j < m; ++j ) c[j] = 0.;
        m = n;
    }
    Matrix a ( n, m );
    for ( int i = 0; i < n; ++i )
    {
        a[i][0] = 1.;
        for ( int j = 1; j < m; ++j ) a[i][j] = x[i] * a[i][j-1];
    }
    return lss_h ( a, y, c );
}

double CalcBezier(double p0, double p1, double p2, double p3, double t)
{
	double p;
	double t2, t3;

	t2=t*t;
	t3=t2*t;
	p =  (((1-t)*p0 + 3*t*p1)*(1-t)+3*t2*p2)*(1-t)+t3*p3;

	return p;
}

double CalcBetaPoint(double p0, double p1, double p2, double p3, double t, double beta1, double beta2)
{
	double s=1.0-t,
	t2=t*t,
	t3=t2*t,
	b12=beta1*beta1,
	b13=b12*beta1,
	delta=2.0*b13+4.0*b12+4.0*beta1+beta2+2.0,
	d=1.0/delta,
	b0=2*b13*d*s*s*s,
	b3=2*t3*d,
	b1=d*(2*b13*t*(t2-3*t+3)+2*b12*(t3-3*t2+2)+
		2*beta1*(t3-3*t+2)+beta2*(2*t3-3*t2+1)),
	b2=d*(2*b12*t2*(-t+3)+2*beta1*t*(-t2+3)+
		beta2*t2*(-2*t+3)+2*(-t3+1) );

	double res = b0*p0+b1*p1+b2*p2+b3*p3+0.5;
	return res;
};

void polint (double xa[], double ya[], int n, double x, double *y, double *dy)
{
  int i, m, ns=1;
  double den,dif,dift,ho,hp,w;
  double *c, *d;

  dif = fabs(x-xa[1]);
  c = new double[n];
  d = new double[n];
  for (i = 1; i <= n; i++) {
    dift = fabs (x - xa[i]);
    if (dift < dif) {
      ns = i;
      dif = dift;
    }
    c[i] = ya[i];
    d[i] = ya[i];
  }
  *y = ya[ns--];
  for (m = 1; m < n; m++) {
    for (i = 1; i <= n-m; i++) {
      ho = xa[i] - x;
      hp = xa[i+m] - x;
      w = c[i+1] - d[i];
      den = ho - hp;
      den = w / den;
      d[i] = hp * den;
      c[i] = ho * den;
    }
    *y += (*dy = (2*ns < (n-m) ? c[ns+1] : d[ns--]));
	}
	if (d) {
		delete [] d;
		d = NULL;
	}
	if (c) {
		delete [] c;
		c = NULL;
	}
}

double Lagr(int n, double xt, double *p)
{

	// x axis - time
	double tx = n+1;
	double *t = new double[n];
	for (int i=0; i<n; i++)
		t[i] = i+1;

	double S, P;
	S = 0.0;
	for (int i=0; i<n; i++)
	{
		P = 1.0;
		for (int j=0; j<n; j++)
			if (i != j) P = P * (tx-t[j])/(t[i]-t[j]);
		S = S + (p[i])*P;
	}

	if (t) {
		delete [] t;
		t = NULL;
	}

	return S;
}

//
// Vectors
//

double VectorAngle(const FBVector3d &u, const FBVector3d &v)
{
	return acos(DotProduct(u, v));
}

double DotProduct(const FBVector3d v1, double *v2)
{
  return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

FBVector3d CrossProduct(const FBVector3d &left, const FBVector3d &right)
{
	FBVector3d	dest;

	dest[0]=	left[1]*right[2]-
								left[2]*right[1];
	dest[1]=	left[2]*right[0]-
								left[0]*right[2];
	dest[2]=	left[0]*right[1]-
								left[1]*right[0];
	return dest;
}

void VectorMult( FBVector3d	&v, double factor)
{
	v[0] *= factor;
	v[1] *= factor;
	v[2] *= factor;
}

void VectorMultArray( FBVector3d*	v[], int count, double factor )
{
	for (int i=0; i<count; i++)
	{
		(*v[i])[0] *= factor;
		(*v[i])[1] *= factor;
		(*v[i])[2] *= factor;
	}
}

FBVector3d VectorCenter( FBVector3d *v, int count )
{
	FBVector3d c(0.0, 0.0, 0.0);
	if (!count) return c;
	for (int i=0; i<count; i++)
	{
		c = VectorAdd(c, v[i]);
	}
	VectorScale( c, count );
	return c;
}

void VectorScale( FBVector3d &v,	double factor )
{
	v[0] /= factor;
	v[1] /= factor;
	v[2] /= factor;
}

FBVector3d VectorAdd( const FBVector3d &v1, const FBVector3d &v2 )
{
	return FBVector3d( v1[0] + v2[0], v1[1] + v2[1], v1[2] + v2[2] );
}

void VectorAddArray( FBVector3d* v[], int count, const FBVector3d &v2 )
{
	for (int i=0; i<count; i++)
	{
		(*v[i])[0] += v2[0];
		(*v[i])[1] += v2[1];
		(*v[i])[2] += v2[2];
	}
}

FBVector3d VectorSubtract( const FBVector3d &v1, const FBVector3d &v2)
{
	return FBVector3d( v1[0]-v2[0], v1[1]-v2[1], v1[2]-v2[2] );
}

double	VectorLength( const FBVector3d &v )
{
	/*
	double len = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	len = sqrt(len);
	return len;
	*/
	return sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

void VectorNormalize( FBVector3d	&v )
{
	double len = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);

	if (len != 0.0)
	{
		len = 1.0 / len;
		v[0] *= len;
		v[1] *= len;
		v[2] *= len;
	}
}
void VectorNormalizeArray( FBVector3d*	v[], int count )
{
	double len;
	for (int i=0; i<count; i++)
	{
		len = sqrt( (*v[i])[0] * (*v[i])[0] + (*v[i])[1] * (*v[i])[1] + (*v[i])[2] * (*v[i])[2]);

		if (len > 0)
		{
			len = 1.0 / len;
			(*v[i])[0] *= len;
			(*v[i])[1] *= len;
			(*v[i])[2] *= len;
		}
	}
}


void VectorTransform ( const FBVector3d in1, const FBMatrix in2, FBVector3d &res )
{
  res[0] = DotProduct(in1, FBVector3d(in2[0], in2[4], in2[8])) + in2[12];
	res[1] = DotProduct(in1, FBVector3d(in2[1], in2[5], in2[9])) +	in2[13];
	res[2] = DotProduct(in1, FBVector3d(in2[2], in2[6], in2[10])) +	in2[14];
}

void VectorTransform33 ( const FBVector3d in1, const FBMatrix in2, FBVector3d &res )
{
  res[0] = DotProduct(in1, FBVector3d(in2[0], in2[4], in2[8]) );
	res[1] = DotProduct(in1, FBVector3d(in2[1], in2[5], in2[9]) );
	res[2] = DotProduct(in1, FBVector3d(in2[2], in2[6], in2[10]) );
}

void VectorTransform33_id (const FBVector3d &in1, const FBMatrix &mat, FBVector3d &res )
{
	res[0] = mat[mNdx(0,0)] * in1[0] + mat[mNdx(1,0)] * in1[1] + mat[mNdx(2,0)] * in1[2];
	res[1] = mat[mNdx(0,1)] * in1[0] + mat[mNdx(1,1)] * in1[1] + mat[mNdx(2,1)] * in1[2];
	res[2] = mat[mNdx(0,2)] * in1[0] + mat[mNdx(1,2)] * in1[1] + mat[mNdx(2,2)] * in1[2];
}

void VectorRotate( const FBVector3d in1, const FBVector3d angles, FBVector3d &res )
{
	FBMatrix my, mp, mr;
	my = rotateX( DEG2RAD(angles[0]) );
	mp = rotateY( DEG2RAD(angles[1]) );
	mr = rotateZ( DEG2RAD(angles[2]) );

	VectorTransform33( in1, mr, res );
	VectorTransform33( res, mp, res );
	VectorTransform33( res, my, res );
}

FBVector3d LinearInterpolation( FBVector3d	&v1, FBVector3d	&v2, double t )
{
	FBVector3d	result;
	result = FBVector3d( (v2[0] - v1[0]) * t + v1[0],
							(v2[1] - v1[1]) * t + v1[1],
							(v2[2] - v1[2]) * t + v1[2] );
	return result;
}

bool MatrixExtract(const FBMatrix	&tm, FBVector3d &forward, FBVector3d &up, FBVector3d &right)
{
	forward = FBVector3d( tm[0], tm[1], tm[2] );
	up = FBVector3d( tm[4], tm[5], tm[6] );
	right = FBVector3d( tm[8], tm[9], tm[10] );
	return true;
}

double PlaneDistance(const FBVector4d &plane, const FBVector3d &point)
{
	return plane[0] * point[0] + plane[1] * point[1] + plane[2] * point[2] + plane[3];
}

//If you work with a plane defined by point and normal
FBVector3d ClosestPointOnPlane(FBVector4d plane, FBVector3d point)
{
	FBVector3d	normal(plane[0], plane[1], plane[2]);
	double distance = PlaneDistance(plane, point);
  return FBVector3d(point[0] - distance*normal[0], point[1] - distance*normal[1], point[2] - distance*normal[2]);
}

FBVector3d ClosestPointOnLine(const FBVector3d A, const FBVector3d B, const FBVector3d P, double *t)
{
	FBVector3d a_to_p = VectorSubtract(P, A);
	FBVector3d a_to_b = VectorSubtract(B, A);

	double atb2 = VectorLength(a_to_b);
	double atp2 = VectorLength(a_to_p);
	VectorNormalize(a_to_p);
	VectorNormalize(a_to_b);
	double atp_dot_atb = DotProduct(a_to_b, a_to_p);

	assert(atb2 != 0.0);
	double _t = atp_dot_atb * atp2 / atb2;
	a_to_b = VectorSubtract(B, A);
	FBVector3d res( A[0] + a_to_b[0] * _t, A[1] + a_to_b[1] * _t, A[2] + a_to_b[2] * _t );
	if (t) *t = _t;
	return res;
}

void rotateWithVector(const FBVector3d x, FBVector3d u, double ang, FBVector3d &Res) {
  FBVector3d h,v,uxx;

  //u.normalize();

	uxx[0] = u[0] / x[0] * sin(ang);
	uxx[1] = u[1] / x[1] * sin(ang);
	uxx[2] = u[2] / x[2] * sin(ang);

	h[0] = u[0] * (x[0] * u[0]);
	h[1] = u[1] * (x[1] * u[1]);
	h[2] = u[2] * (x[2] * u[2]);

	v[0] = (x[0] - h[0]) * cos(ang);
	v[1] = (x[1] - h[1]) * cos(ang);
	v[2] = (x[2] - h[2]) * cos(ang);

  Res[0] = (h[0] + v[0]) + uxx[0];
	Res[1] = (h[1] + v[1]) + uxx[1];
	Res[2] = (h[2] + v[2]) + uxx[2];
}

int		mNdx(int row, int col)
{
	return (row * 4 + col);
}

FBMatrix rotateX ( double angle )
{
	FBMatrix res;
	res.Identity();
  double cosine = cos(angle);
	double sine   = sin(angle);

	res[mNdx(1,1)] = cosine;
	res[mNdx(1,2)] = -sine;
	res[mNdx(2,1)] = sine;
	res[mNdx(2,2)] = cosine;

	return res;
}

FBMatrix       rotateY ( double angle )
{
	FBMatrix res;
	res.Identity();
  double cosine = cos(angle);
	double sine   = sin(angle);

	res[mNdx(0,0)] = cosine;
	res[mNdx(0,2)] = -sine;
	res[mNdx(2,0)] = sine;
	res[mNdx(2,2)] = cosine;

	return res;
}

FBMatrix       rotateZ ( double angle )
{
	FBMatrix res;
	res.Identity();
  double cosine = cos(angle);
	double sine   = sin(angle);

	res[mNdx(0,0)] = cosine;
	res[mNdx(0,1)] = -sine;
	res[mNdx(1,0)] = sine;
	res[mNdx(1,1)] = cosine;

	return res;
}

FBMatrix	MatrixMult(const FBMatrix &M1, const FBMatrix &M2)
{
	FBMatrix dest;

	double*  L;
	double*  R;

	L = &M1[0];
	R = &M2[0];

	double* D = &dest[0];
	for( int i=0; i<4; i++, D+=4, R+=4 )
	{
		D[0] = L[0] * R[0] + L[4] * R[1] + L[8]  * R[2] + L[12] * R[3];
		D[1] = L[1] * R[0] + L[5] * R[1] + L[9]  * R[2] + L[13] * R[3];
		D[2] = L[2] * R[0] + L[6] * R[1] + L[10] * R[2] + L[14] * R[3];
		D[3] = L[3] * R[0] + L[7] * R[1] + L[11] * R[2] + L[15] * R[3];
	}
	return dest;
}

FBMatrix		MatrixFastInverse(const FBMatrix &in)
{
	FBMatrix	M;
	/*
	//
	// Calculate the determinant.
	//
	Determinant = ( in[0,0] * ( in[1,1] * in[2,2] - in[1,2] * in[2,1] ) -
		in[0,1] * ( in[1,0] * in[2,2] - in[1,2] * in[2,0] ) +
		in[0,2] * ( in[1,0] * in[2,1] - in[1,1] * in[2,0] ) );

	if( fabs( Determinant ) < 0.0001 )
	{
		M.Identity();
		//return Matrix;
	}

	Determinant = 1.0 / Determinant;

	//
	// Find the inverse of the matrix.
	//
	M(0,0) =  Determinant * ( in(1,1) * in(2,2) - in(1,2) * in(2,1) );
	M(0,1) = -Determinant * ( in(0,1) * in(2,2) - in(0,2) * in(2,1) );
	M(0,2) =  Determinant * ( in(0,1) * in(1,2) - in(0,2) * in(1,1) );
	M(0,3) = 0.0;

	M(1,0) = -Determinant * ( in(1,0) * in(2,2) - in(1,2) * in(2,0) );
	M(1,1) =  Determinant * ( in(0,0) * in(2,2) - in(0,2) * in(2,0) );
	M(1,2) = -Determinant * ( in(0,0) * in(1,2) - in(0,2) * in(1,0) );
	M(1,3) = 0.0;

	M(2,0) =  Determinant * ( in(1,0) * in(2,1) - in(1,1) * in(2,0) );
	M(2,1) = -Determinant * ( in(0,0) * in(2,1) - in(0,1) * in(2,0) );
	M(2,2) =  Determinant * ( in(0,0) * in(1,1) - in(0,1) * in(1,0) );
	M(2,3) = 0.0;

	M(3,0) = -( in(3,0) * M(0,0) + in(3,1) * M(1,0) + in(3,2) * M(2,0) );
	M(3,1) = -( in(3,0) * M(0,1) + in(3,1) * M(1,1) + in(3,2) * M(2,1) );
	M(3,2) = -( in(3,0) * M(0,2) + in(3,1) * M(1,2) + in(3,2) * M(2,2) );
	M(3,3) = 1.0;
	*/
	return M;
}

FBMatrix MatrixInvert(FBMatrix	&m)
{
	FBMatrix  out;
	out.Identity();

	for(int i=0;i<4;i++)
	{
		float d = m(i,i);

	  if(d != 1.0)
		{
			for(int j=0; j<4; j++)
			{
				out(i,j) /=d;
				m(i,j)     /=d;
			}
		}

	  for(int j=0; j<4; j++)
		{
			if (j!=i)
			{
				if( m(j,i)!=0.0)
				{
					float  mulBy = m(j,i);

					for(int k=0; k<4; k++)
					{
						m(j,k)     -=mulBy*m(i,k);
						out(j,k) -= mulBy*out(i,k);
					}
				}
			}
		}
	}
	return out;
}

FBVector3d MatrixToEuler(const FBMatrix &matrix)
{
#if !defined(FLT_EPSILON)
	constexpr float FLT_EPSILON{ 0.00001f };
#endif
	FBVector3d	angles;
	double		theta;
	double		cp;
	float		sp;

	sp = matrix[mNdx(0,2)];

	// cap off our sin value so that we don't get any NANs
	if ( sp > 1.0f ) {
		sp = 1.0f;
	} else if ( sp < -1.0f ) {
		sp = -1.0f;
	}

	theta = -asin( sp );
	cp = cos( theta );

	if ( cp > 8192.0f * FLT_EPSILON ) {
		angles[1]	= RAD2DEG( theta );
		angles[0]	= RAD2DEG( atan2( matrix[mNdx(0,1)], matrix[mNdx(0,0)] ) );
		angles[2]	= RAD2DEG( atan2( matrix[mNdx(1,2)], matrix[mNdx(2,2)] ) );
	} else {
		angles[1]	= RAD2DEG( theta );
		angles[0]	= RAD2DEG( -atan2( matrix[mNdx(1,0)], matrix[mNdx(1,1)] ) );
		angles[2]	= 0;
	}
	return angles;
}


enum QUAT_INDEX
{
   X_INDEX=0,
   Y_INDEX=1,
   Z_INDEX=2
};
static QUAT_INDEX QIndex [] = {Y_INDEX, Z_INDEX, X_INDEX};

FBVector4d MatrixToQuaternion (const FBMatrix &matrix)
{
	FBVector4d	res;

	double *ptr;
	double trace;
	QUAT_INDEX i;
	QUAT_INDEX j;
	QUAT_INDEX k;

	trace = matrix[mNdx(0,0)] + matrix[mNdx(1,1)] + matrix[mNdx(2,2)];

	if (trace > 0.0) {
		trace = 1.0 / sqrt (trace + 1.0);
		res[3] = 0.5 * trace;
		trace = 0.5 / trace;

		res[0] = (matrix[mNdx(2,1)] - matrix[mNdx(1,2)]) * trace;
		res[1] = (matrix[mNdx(0,2)] - matrix[mNdx(2,0)]) * trace;
		res[2] = (matrix[mNdx(1,0)] - matrix[mNdx(0,1)]) * trace;

	} else {
		i = X_INDEX;
		if (matrix[mNdx(Y_INDEX,Y_INDEX)] > matrix[mNdx(X_INDEX,X_INDEX)]) {
			i = Y_INDEX;
		}
		if (matrix[mNdx(Z_INDEX,Z_INDEX)] > matrix[mNdx(i,i)]) {
			i = Z_INDEX;
		}
		j = QIndex [i];
		k = QIndex [j];

		trace = 1.0 + matrix[mNdx(i,i)] - matrix[mNdx(j,j)] - matrix[mNdx(k,k)];
		trace = 1.0 / sqrt (trace);

		ptr = &res[0];
		ptr[i] = 0.5 * trace;
		trace = 0.5 / trace;
		ptr[3] = (matrix[mNdx(k,j)] - matrix[mNdx(j,k)]) * trace;
		ptr[j] = (matrix[mNdx(j,i)] + matrix[mNdx(i,j)]) * trace;
		ptr[k] = (matrix[mNdx(k,i)] + matrix[mNdx(i,k)]) * trace;
	}

	res[0] *= -1;
	res[1] *= -1;
	res[2] *= -1;

	return	res;
}

FBMatrix FBMatrixFromAnimationNode( FBModel	*pModel, FBTime pTime )
{
	FBMatrix	lResult;

	FBAnimationNode	*lTranslationNode = pModel->Translation.GetAnimationNode();
	FBAnimationNode	*lRotationNode = pModel->Rotation.GetAnimationNode();
	FBAnimationNode	*lScaleNode = pModel->Scaling.GetAnimationNode();

	FBVector3d	lRotationV(
		lRotationNode->Nodes[0]->FCurve->Evaluate(pTime) * 0.017453292519943295769236907684886,
		lRotationNode->Nodes[1]->FCurve->Evaluate(pTime) * 0.017453292519943295769236907684886,
		lRotationNode->Nodes[2]->FCurve->Evaluate(pTime) * 0.017453292519943295769236907684886);

	FBVector3d	lScaleV(
		lScaleNode->Nodes[0]->FCurve->Evaluate(pTime),
		lScaleNode->Nodes[1]->FCurve->Evaluate(pTime),
		lScaleNode->Nodes[2]->FCurve->Evaluate(pTime));

	double sphi = sin(lRotationV[0]);
	double cphi = cos(lRotationV[0]);
	double stheta = sin(lRotationV[1]);
	double ctheta = cos(lRotationV[1]);
	double spsi = sin(lRotationV[2]);
	double cpsi = cos(lRotationV[2]);

	lResult[0] = (cpsi*ctheta)*lScaleV[0];
	lResult[1] = (spsi*ctheta)*lScaleV[0];
	lResult[2] = (-stheta)*lScaleV[0];

	lResult[4] = (cpsi*stheta*sphi - spsi*cphi)*lScaleV[1];
	lResult[5] = (spsi*stheta*sphi + cpsi*cphi)*lScaleV[1];
	lResult[6] = (ctheta*sphi)*lScaleV[1];

	lResult[8] = (cpsi*stheta*cphi + spsi*sphi)*lScaleV[2];
	lResult[9] = (spsi*stheta*cphi - cpsi*sphi)*lScaleV[2];
	lResult[10] = (ctheta*cphi)*lScaleV[2];

	lResult[12] = lTranslationNode->Nodes[0]->FCurve->Evaluate(pTime);
	lResult[13] = lTranslationNode->Nodes[1]->FCurve->Evaluate(pTime);
	lResult[14] = lTranslationNode->Nodes[2]->FCurve->Evaluate(pTime);

	return lResult;
}

FBVector4d from_matrix( const FBMatrix &m )
{
	FBVector4d	res;
    float tr = m[mNdx(0,0)] + m[mNdx(1,1)] + m[mNdx(2,2)]; // trace of martix
    if (tr > 0.0){     // if trace positive than "w" is biggest component
        res = FBVector4d( m[mNdx(1,2)] - m[mNdx(2,1)], m[mNdx(2,0)] - m[mNdx(0,2)], m[mNdx(0,1)] - m[mNdx(1,0)], tr + 1.0 );
    }else                 // Some of vector components is bigger
    if( (m[mNdx(0,0)] > m[mNdx(1,1)] ) && ( m[mNdx(0,0)] > m[mNdx(2,2)]) ) {
        res = FBVector4d( 1.0 + m[mNdx(0,0)] - m[mNdx(1,1)] - m[mNdx(2,2)], m[mNdx(1,0)] + m[mNdx(0,1)],
             m[mNdx(2,0)] + m[mNdx(0,2)], m[mNdx(1,2)] - m[mNdx(2,1)] );
    }else
    if ( m[mNdx(1,1)] > m[mNdx(2,2)] ){
        res= FBVector4d ( m[mNdx(1,0)] + m[mNdx(0,1)], 1.0 + m[mNdx(1,1)] - m[mNdx(0,0)] - m[mNdx(2,2)],
             m[mNdx(2,1)] + m[mNdx(1,2)], m[mNdx(2,0)] - m[mNdx(0,2)] );
    }else{
        res = FBVector4d( m[mNdx(2,0)] + m[mNdx(0,2)], m[mNdx(2,1)] + m[mNdx(1,2)],
             1.0 + m[mNdx(2,2)] - m[mNdx(0,0)] - m[mNdx(1,1)], m[mNdx(0,1)] - m[mNdx(1,0)] );
    }
		return res;
}

FBVector4d	UnitAxisToQuaternion (const FBVector3d &unitAxis, double Angle)
{
	FBVector4d	res;

	double sinAng;

	Angle *= 0.5;
	res[3] = cos (Angle);
	sinAng = sin (Angle);

	res[0] = unitAxis[0] * sinAng;
	res[1] = unitAxis[1] * sinAng;
	res[2] = unitAxis[2] * sinAng;

	return res;
}

double	QuaternionNorm( const FBVector4d	&q)
{
	return (q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3]);
}

void	MatrixInit (const FBVector4d &rotation, FBMatrix &m)
{
	double wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;
	double s  = 2.0/QuaternionNorm(rotation);  // 4 mul 3 add 1 div
  x2 = rotation[0] * s;    y2 = rotation[1] * s;    z2 = rotation[2] * s;
  xx = rotation[0] * x2;   xy = rotation[0] * y2;   xz = rotation[0] * z2;
  yy = rotation[1] * y2;   yz = rotation[1] * z2;   zz = rotation[2] * z2;
  wx = rotation[3] * x2;   wy = rotation[3] * y2;   wz = rotation[3] * z2;

  m[mNdx(0,0)] = 1.0 - (yy + zz);
  m[mNdx(1,0)] = xy - wz;
  m[mNdx(2,0)] = xz + wy;

  m[mNdx(0,1)] = xy + wz;
  m[mNdx(1,1)] = 1.0 - (xx + zz);
  m[mNdx(2,1)] = yz - wx;

  m[mNdx(0,2)] = xz - wy;
  m[mNdx(1,2)] = yz + wx;
  m[mNdx(2,2)] = 1.0 - (xx + yy);
}

bool equivalent(double a, double b)
{
  if ((a > b + EPSILON) && (a < b + EPSILON))
    return true;

  return false;
}

void NormalizeQuat(FBVector4d &dest)
{
  double rnorm = 1.0 / sqrt(dest[3] * dest[3] + dest[0] * dest[0] + dest[1] * dest[1] + dest[2] * dest[2]);
  if (equivalent(rnorm, 0.0))
      return;

  dest[0] = dest[0] * rnorm;
  dest[1] = dest[1] * rnorm;
  dest[2] = dest[2] * rnorm;
  dest[3] = dest[3] * rnorm;
}

FBVector4d QuaternionMult (const FBVector4d &ql, const FBVector4d &qr)
{
	FBVector4d	q;

	q[3] = ql[3] * qr[3] - ql[0] * qr[0] - ql[1] * qr[1] - ql[2] * qr[2];
  q[0] = ql[3] * qr[0] + ql[0] * qr[3] + ql[1] * qr[2] - ql[2] * qr[1];
  q[1] = ql[3] * qr[1] + ql[1] * qr[3] + ql[2] * qr[0] - ql[0] * qr[2];
  q[2] = ql[3] * qr[2] + ql[2] * qr[3] + ql[0] * qr[1] - ql[1] * qr[0];

	NormalizeQuat(q);
	return q;
}


FBVector4d CreateQuat(const FBVector3d &rotateFrom, const FBVector3d &rotateTo)
{
	FBVector3d	p1,p2;
	double alpha;
	FBVector3d v;
	FBVector3d	tmp;

	FBVector4d	res(0.0, 0.0, 0.0, 1.0);

  p1 = rotateFrom;
	VectorNormalize(p1);
  p2 = rotateTo;
  VectorNormalize(p2);

  alpha = DotProduct(p1, p2);

  if (equivalent(alpha, 1.0))
	{
    return res;
	}

  if (equivalent(alpha, -1.0))
	{
		if ((p1[0] != p1[1]) || (p1[0] != p1[2]))
		{
      v[0] = p1[1];
      v[1] = p1[2];
      v[2] = p1[0];
		}
		else
		{
			v[0] = -p1[0];
			v[1] = p1[1];
			v[2] = p1[2];
		}

    tmp = p1;
    VectorScale(tmp, DotProduct(p1, v));
    v = VectorSubtract(tmp, v);
    VectorNormalize(v);

		return UnitAxisToQuaternion(v, 1.0);
	}

  p1 = CrossProduct(p1, p2);
  VectorNormalize(p1);
  return UnitAxisToQuaternion(p1, acos(alpha));
}



// PlaneMake (point + normal, hmg)
//
FBVector4d PlaneMake(const FBVector3d &point, const FBVector3d &normal)
{
	FBVector4d res;
	res[0] = normal[0];
	res[1] = normal[1];
	res[2] = normal[2];
	res[3] = -DotProduct(point, normal);
	return res;
}

// PlaneMake (3 points, hmg)
//
FBVector4d PlaneMake(const FBVector3d &p1, const FBVector3d &p2, FBVector3d &p3)
{
	FBVector3d	v1,v2, normal;

	v1 = VectorSubtract(p2, p1);
  v2 = VectorSubtract(p3, p1);
	normal = CrossProduct(v1, v2);
	VectorNormalize(normal);

	return FBVector4d( normal[0], normal[1], normal[2], -DotProduct(p1, normal) );
}


/*
================
PlaneIntersection
================
*/
bool PlaneIntersection( const FBVector4d &plane1, const FBVector4d &plane2, FBVector3d &start, FBVector3d &dir )
{
	double n00, n01, n11, det, invDet, f0, f1;

	//n00 = Normal().LengthSqr();
	FBVector3d normal(plane1[0], plane1[1], plane1[2]);
	n00 = VectorLength( normal );
	//n01 = Normal() * plane.Normal();
	n01 = DotProduct( normal, &plane2[0] );
	//n11 = plane.Normal().LengthSqr();
	FBVector3d normal2(plane2[0], plane2[1], plane2[2]);
	n11 = VectorLength( normal2 );
	det = n00 * n11 - n01 * n01;

	if ( fabs(det) < 1e-6f ) {
		return false;
	}

	invDet = 1.0 / det;
	f0 = ( n01 * plane2[3] - n11 * plane1[3] ) * invDet;
	f1 = ( n01 * plane1[3] - n00 * plane2[3] ) * invDet;

	//dir = Normal().Cross( plane.Normal() );
	dir = CrossProduct( normal, normal2 );
	//start = f0 * Normal() + f1 * plane.Normal();
	VectorMult( normal, f0 );
	VectorMult( normal2, f1 );
	start = VectorAdd( normal, normal2 );
	return true;
}

// plane & ray intersection
bool RayIntersection( const FBVector4d &plane, const FBVector3d &start, const FBVector3d &dir, double &scale )
{
	double d1, d2;
	FBVector3d	normal(plane[0], plane[1], plane[2]);

	d1 = DotProduct(normal, start) + plane[3];
	d2 = DotProduct(normal, dir);
	if ( d2 == 0.0 ) {
		return false;
	}
	scale = -( d1 / d2 );
	return true;
}

/*
 * Transform a point (column vector) by a 4x4 matrix.  I.e.  out = m * in
 * Input:  m - the 4x4 matrix
 *         in - the 4x1 vector
 * Output:  out - the resulting 4x1 vector.
 */
static void transform_point(GLdouble out[4], const GLdouble m[16], const GLdouble in[4])
{
#define M(row,col)  m[col*4+row]
   out[0] =
      M(0, 0) * in[0] + M(0, 1) * in[1] + M(0, 2) * in[2] + M(0, 3) * in[3];
   out[1] =
      M(1, 0) * in[0] + M(1, 1) * in[1] + M(1, 2) * in[2] + M(1, 3) * in[3];
   out[2] =
      M(2, 0) * in[0] + M(2, 1) * in[1] + M(2, 2) * in[2] + M(2, 3) * in[3];
   out[3] =
      M(3, 0) * in[0] + M(3, 1) * in[1] + M(3, 2) * in[2] + M(3, 3) * in[3];
#undef M
}

/*
 * Perform a 4x4 matrix multiplication  (product = a x b).
 * Input:  a, b - matrices to multiply
 * Output:  product - product of a and b
 */
static void matmul(GLdouble * product, const GLdouble * a, const GLdouble * b)
{
   /* This matmul was contributed by Thomas Malik */
   GLdouble temp[16];
   GLint i;

#define A(row,col)  a[(col<<2)+row]
#define B(row,col)  b[(col<<2)+row]
#define T(row,col)  temp[(col<<2)+row]

   /* i-te Zeile */
   for (i = 0; i < 4; i++) {
      T(i, 0) =
	 A(i, 0) * B(0, 0) + A(i, 1) * B(1, 0) + A(i, 2) * B(2, 0) + A(i,
								       3) *
	 B(3, 0);
      T(i, 1) =
	 A(i, 0) * B(0, 1) + A(i, 1) * B(1, 1) + A(i, 2) * B(2, 1) + A(i,
								       3) *
	 B(3, 1);
      T(i, 2) =
	 A(i, 0) * B(0, 2) + A(i, 1) * B(1, 2) + A(i, 2) * B(2, 2) + A(i,
								       3) *
	 B(3, 2);
      T(i, 3) =
	 A(i, 0) * B(0, 3) + A(i, 1) * B(1, 3) + A(i, 2) * B(2, 3) + A(i,
								       3) *
	 B(3, 3);
   }

#undef A
#undef B
#undef T
   memcpy(product, temp, 16 * sizeof(GLdouble));
}

/*
 * Compute inverse of 4x4 transformation matrix.
 * Code contributed by Jacques Leroy jle@star.be
 * Return GL_TRUE for success, GL_FALSE for failure (singular matrix)
 */
static GLboolean invert_matrix(const GLdouble * m, GLdouble * out)
{
/* NB. OpenGL Matrices are COLUMN major. */
#define SWAP_ROWS(a, b) { GLdouble *_tmp = a; (a)=(b); (b)=_tmp; }
#define MAT(m,r,c) (m)[(c)*4+(r)]

   GLdouble wtmp[4][8];
   GLdouble m0, m1, m2, m3, s;
   GLdouble *r0, *r1, *r2, *r3;

   r0 = wtmp[0], r1 = wtmp[1], r2 = wtmp[2], r3 = wtmp[3];

   r0[0] = MAT(m, 0, 0), r0[1] = MAT(m, 0, 1),
      r0[2] = MAT(m, 0, 2), r0[3] = MAT(m, 0, 3),
      r0[4] = 1.0, r0[5] = r0[6] = r0[7] = 0.0,
      r1[0] = MAT(m, 1, 0), r1[1] = MAT(m, 1, 1),
      r1[2] = MAT(m, 1, 2), r1[3] = MAT(m, 1, 3),
      r1[5] = 1.0, r1[4] = r1[6] = r1[7] = 0.0,
      r2[0] = MAT(m, 2, 0), r2[1] = MAT(m, 2, 1),
      r2[2] = MAT(m, 2, 2), r2[3] = MAT(m, 2, 3),
      r2[6] = 1.0, r2[4] = r2[5] = r2[7] = 0.0,
      r3[0] = MAT(m, 3, 0), r3[1] = MAT(m, 3, 1),
      r3[2] = MAT(m, 3, 2), r3[3] = MAT(m, 3, 3),
      r3[7] = 1.0, r3[4] = r3[5] = r3[6] = 0.0;

   /* choose pivot - or die */
   if (fabs(r3[0]) > fabs(r2[0]))
      SWAP_ROWS(r3, r2);
   if (fabs(r2[0]) > fabs(r1[0]))
      SWAP_ROWS(r2, r1);
   if (fabs(r1[0]) > fabs(r0[0]))
      SWAP_ROWS(r1, r0);
   if (0.0 == r0[0])
      return GL_FALSE;

   /* eliminate first variable     */
   m1 = r1[0] / r0[0];
   m2 = r2[0] / r0[0];
   m3 = r3[0] / r0[0];
   s = r0[1];
   r1[1] -= m1 * s;
   r2[1] -= m2 * s;
   r3[1] -= m3 * s;
   s = r0[2];
   r1[2] -= m1 * s;
   r2[2] -= m2 * s;
   r3[2] -= m3 * s;
   s = r0[3];
   r1[3] -= m1 * s;
   r2[3] -= m2 * s;
   r3[3] -= m3 * s;
   s = r0[4];
   if (s != 0.0) {
      r1[4] -= m1 * s;
      r2[4] -= m2 * s;
      r3[4] -= m3 * s;
   }
   s = r0[5];
   if (s != 0.0) {
      r1[5] -= m1 * s;
      r2[5] -= m2 * s;
      r3[5] -= m3 * s;
   }
   s = r0[6];
   if (s != 0.0) {
      r1[6] -= m1 * s;
      r2[6] -= m2 * s;
      r3[6] -= m3 * s;
   }
   s = r0[7];
   if (s != 0.0) {
      r1[7] -= m1 * s;
      r2[7] -= m2 * s;
      r3[7] -= m3 * s;
   }

   /* choose pivot - or die */
   if (fabs(r3[1]) > fabs(r2[1]))
      SWAP_ROWS(r3, r2);
   if (fabs(r2[1]) > fabs(r1[1]))
      SWAP_ROWS(r2, r1);
   if (0.0 == r1[1])
      return GL_FALSE;

   /* eliminate second variable */
   m2 = r2[1] / r1[1];
   m3 = r3[1] / r1[1];
   r2[2] -= m2 * r1[2];
   r3[2] -= m3 * r1[2];
   r2[3] -= m2 * r1[3];
   r3[3] -= m3 * r1[3];
   s = r1[4];
   if (0.0 != s) {
      r2[4] -= m2 * s;
      r3[4] -= m3 * s;
   }
   s = r1[5];
   if (0.0 != s) {
      r2[5] -= m2 * s;
      r3[5] -= m3 * s;
   }
   s = r1[6];
   if (0.0 != s) {
      r2[6] -= m2 * s;
      r3[6] -= m3 * s;
   }
   s = r1[7];
   if (0.0 != s) {
      r2[7] -= m2 * s;
      r3[7] -= m3 * s;
   }

   /* choose pivot - or die */
   if (fabs(r3[2]) > fabs(r2[2]))
      SWAP_ROWS(r3, r2);
   if (0.0 == r2[2])
      return GL_FALSE;

   /* eliminate third variable */
   m3 = r3[2] / r2[2];
   r3[3] -= m3 * r2[3], r3[4] -= m3 * r2[4],
      r3[5] -= m3 * r2[5], r3[6] -= m3 * r2[6], r3[7] -= m3 * r2[7];

   /* last check */
   if (0.0 == r3[3])
      return GL_FALSE;

   s = 1.0 / r3[3];		/* now back substitute row 3 */
   r3[4] *= s;
   r3[5] *= s;
   r3[6] *= s;
   r3[7] *= s;

   m2 = r2[3];			/* now back substitute row 2 */
   s = 1.0 / r2[2];
   r2[4] = s * (r2[4] - r3[4] * m2), r2[5] = s * (r2[5] - r3[5] * m2),
      r2[6] = s * (r2[6] - r3[6] * m2), r2[7] = s * (r2[7] - r3[7] * m2);
   m1 = r1[3];
   r1[4] -= r3[4] * m1, r1[5] -= r3[5] * m1,
      r1[6] -= r3[6] * m1, r1[7] -= r3[7] * m1;
   m0 = r0[3];
   r0[4] -= r3[4] * m0, r0[5] -= r3[5] * m0,
      r0[6] -= r3[6] * m0, r0[7] -= r3[7] * m0;

   m1 = r1[2];			/* now back substitute row 1 */
   s = 1.0 / r1[1];
   r1[4] = s * (r1[4] - r2[4] * m1), r1[5] = s * (r1[5] - r2[5] * m1),
      r1[6] = s * (r1[6] - r2[6] * m1), r1[7] = s * (r1[7] - r2[7] * m1);
   m0 = r0[2];
   r0[4] -= r2[4] * m0, r0[5] -= r2[5] * m0,
      r0[6] -= r2[6] * m0, r0[7] -= r2[7] * m0;

   m0 = r0[1];			/* now back substitute row 0 */
   s = 1.0 / r0[0];
   r0[4] = s * (r0[4] - r1[4] * m0), r0[5] = s * (r0[5] - r1[5] * m0),
      r0[6] = s * (r0[6] - r1[6] * m0), r0[7] = s * (r0[7] - r1[7] * m0);

   MAT(out, 0, 0) = r0[4];
   MAT(out, 0, 1) = r0[5], MAT(out, 0, 2) = r0[6];
   MAT(out, 0, 3) = r0[7], MAT(out, 1, 0) = r1[4];
   MAT(out, 1, 1) = r1[5], MAT(out, 1, 2) = r1[6];
   MAT(out, 1, 3) = r1[7], MAT(out, 2, 0) = r2[4];
   MAT(out, 2, 1) = r2[5], MAT(out, 2, 2) = r2[6];
   MAT(out, 2, 3) = r2[7], MAT(out, 3, 0) = r3[4];
   MAT(out, 3, 1) = r3[5], MAT(out, 3, 2) = r3[6];
   MAT(out, 3, 3) = r3[7];

   return GL_TRUE;

#undef MAT
#undef SWAP_ROWS
}

/* transformation du point ecran (winx,winy,winz) en point objet */
GLint UnProject(GLdouble winx, GLdouble winy, GLdouble winz,
	     const GLdouble model[16], const GLdouble proj[16],
	     const GLint viewport[4],
	     GLdouble * objx, GLdouble * objy, GLdouble * objz)
{
   /* matrice de transformation */
   GLdouble m[16], A[16];
   GLdouble in[4], out[4];

   /* transformation coordonnees normalisees entre -1 et 1 */
   in[0] = (winx - viewport[0]) * 2 / viewport[2] - 1.0;
   in[1] = (winy - viewport[1]) * 2 / viewport[3] - 1.0;
   in[2] = 2 * winz - 1.0;
   in[3] = 1.0;

   /* calcul transformation inverse */
   matmul(A, proj, model);
   invert_matrix(A, m);

   /* d'ou les coordonnees objets */
   transform_point(out, m, in);
   if (out[3] == 0.0)
      return GL_FALSE;
   *objx = out[0] / out[3];
   *objy = out[1] / out[3];
   *objz = out[2] / out[3];
   return GL_TRUE;
}

bool MatrixExtract(const GLdouble model[16], const GLdouble proj[16],
	     const GLint viewport[4],
	     GLdouble * objx, GLdouble * objy, GLdouble * objz)
{
	/* matrice de transformation */
   GLdouble A[16];
   //GLdouble in[4], out[4];

	//invert_matrix(model, A);

	 glGetDoublev(GL_MODELVIEW_MATRIX,	&A[0] );

	*objx = -A[8];
  *objy = -A[9];
  *objz = -A[10];

	/*
	in[0] = 0.0;
	in[1] = 0.0;
	in[2] = 1.0;
	in[3] = 1.0;
	transform_point(out, m, in);
	if (out[3] == 0.0)
		return false;
	*objx = out[0] / out[3];
	*objy = out[1] / out[3];
	*objz = out[2] / out[3];
	*/
	return true;
}


//*********************** 07.03.2003 **************************//
//
//      Метод наименьших квадратов ( n >= m ).
//      Преобразования Хаусхолдера.
//		Источник - prografix.narod.ru
//
//*********************** 03.04.2005 **************************//

// \param x,
// \param y - arrays of point data (plotted data)
// \return a - matrix A for ssh
// \return b - vector B for ssh
void linBuild(unsigned n, unsigned m, const double *x, const double *y,
			  double *&A, double *&B)
{
	double p;
/*
	// matrix A

	for (int i=0; i<=m; i++)
		for (int j=0; j<=m; j++)
		{
			p = 0.0;
			for (int k=0; k<n; k++)
				p = p + pow( x(k), (i+j) );
			A(i,j) = p;
		}
	*/



	for (unsigned j=0; j<m; j++)
		for (unsigned k=j; k<m; k++)
		{
			A[j*m+k] = 0.0;
			for (unsigned i=0; i<n; i++)
			{
				p = 1.0;
				for (unsigned l=1; l<(j+k); l++)
					p = p * x[i];
				A[j*m+k] = A[j*m+k]+p;
			}
			A[k*m+j]=A[j*m+k];
		}

/*
	// B
	for (int i=0; i<=m; i++)
	{
		p = 0.0;
		for (int k=0; k<n; k++)
			p = p + y(k)*pow(x(k), i);
		B(i)=p;
	}*/

	for (unsigned k=0; k<m; k++)
	{
		B[k] = 0.0;
		for (unsigned i=0; i<n; i++)
		{
			p = y[i];
			for (unsigned l=1; l<k; l++)
				p = p * x[i];
			B[k] = B[k] + p;
		}
	}

}

//
// solve linear eq (Gauss method)
//
void linSolve(unsigned m, double *&A, double *&B, double *&c)
{
	//double p;
/*
	for (int i=0; i<m; i++)
		for (int k=i+1; k<=m; k++)
		{
			p = A[k*m+i]/A[i*m+i];
			for (int j=i+1; j<=m; j++)
				A[k*m+j] = A[k*m+j]-A[i*m+j]*p;
			B[k]=B[k]-B[i]*p;
		}

	for (int i=m; i>=0; i--)
		for (int j=i+1; j<=m; j++)
		{
			B[i] = B[i] - A[i*m+j]*c[j];
			c[i]=B[i]/A[i*m+i];
		}
*/
	// forward
	for (unsigned i=0; i < m; i++)
		for (unsigned j=i+1; j<m; j++)
		{
			double d = A[i*m+j]/A[i*m+i];
			A[i*m+j] = 0.0;
			for (unsigned k=i+1; k<m; k++)
				A[k*m+j] = A[k*m+j] - d * A[k*m+i];
			B[j] = B[j] - B[i] * d;
		}
	// backward
	for (uint32_t j=0; j<m; j++)
	{
		assert( A[j*m+j] != 0.0 );
		c[j] = B[j] / A[j*m+j];
	}

	for (uint32_t j=m-1; j>=0; j--)
	{
		//c[j]=B[j] / A[j,j];
		double s = 0.0;
		for (unsigned k=j+1; k<m; k++)
			s = s + A[k*m+j]*c[k];
		c[j] = (B[j] - s) / A[j*m+j];
	}

	/*
	// forward
	for (unsigned i=0; i < m; i++)
		for (unsigned j=i+1; j<m; j++)
		{
			for (unsigned k=i+1; k<m; k++)
				A[k*m+j] = A[k*m+j] - A[i*m+j]*A[k*m+i]/A[i*m+i];
			B[j] = B[j] - B[i] * A[i*m+j]/A[i*m+i];
		}
	// backward
	for (int j=m-1; j>=0; j--)
	{
		c[j]=B[j];
		for (unsigned k=j+1; k<m; k++)
			c[j] = c[j] - A[k*m+j]*c[k];
		c[j] = c[j] / A[j*m+j];
	}
	*/
}

typedef unsigned int nat;

Matrix::Matrix ( unsigned int r, unsigned int c ) :
nRow ( r > 0 && c > 0 ? r : 0 ),
nCol ( r > 0 && c > 0 ? c : 0 )
{
    if ( nRow == 0 )
    {
        p = 0;
        return;
    }
    p = new double*[nRow];
    *p = new double[nRow*nCol];
    for ( unsigned int i = 1; i < nRow; ++i ) p[i] = p[i-1] + nCol;
}

Matrix::Matrix ( unsigned int r, unsigned int c, const double * const * a ) :
nRow ( r > 0 && c > 0 && a != 0 ? r : 0 ),
nCol ( r > 0 && c > 0 && a != 0 ? c : 0 )
{
    if ( nRow == 0 )
    {
        p = 0;
        return;
    }
    p = new double*[nRow];
    *p = new double[nRow*nCol];
    for ( unsigned int i = 0; i < nRow; ++i )
    {
        if ( i ) p[i] = p[i-1] + nCol;
        double * t = p[i];
        const double * s = a[i];
        for ( unsigned int j = 0; j < nCol; ++j ) t[j] = s[j];
    }
}

Matrix::Matrix ( const Matrix & m ) : nRow ( m.nRow ), nCol ( m.nCol )
{
    if ( nRow == 0 )
    {
        p = 0;
        return;
    }
    p = new double*[nRow];
    *p = new double[nRow*nCol];
    for ( unsigned int i = 0; i < nRow; ++i )
    {
        if ( i ) p[i] = p[i-1] + nCol;
        double * t = p[i];
        const double * s = m.p[i];
        for ( unsigned int j = 0; j < nCol; ++j ) t[j] = s[j];
    }
}

Matrix::~Matrix()
{
    if ( p )
    {
        delete[] *p;
        delete[] p;
    }
}

void Matrix::fill ( double d )
{
    if ( p == 0 ) return;
    double * t = *p;
    for ( int i = nRow*nCol; --i >= 0; ) *t++ = d;
}

Matrix & Matrix::operator *= ( double d )
{
    if ( p )
    {
        double * t = *p;
        for ( int i = nRow*nCol; --i >= 0; ) *t++ *= d;
    }
    return *this;
}

LSS_H_Stor::LSS_H_Stor ( Matrix & p ) : a(p)
{
    if ( a.nRow < a.nCol || a.nCol <= 0 )
    {
        alpha = 0;
        return;
    }
    alpha = new double[a.nCol+a.nCol+a.nRow];
    pivot = new nat[a.nCol];
    nat i, j, k;
    for ( k = 0; k < a.nCol; ++k )
    {
        pivot[k] = k;
    }
    for ( k = 0; k < a.nCol; ++k )
    {
        double sigma = 0.;
        nat jmax=0;
        for ( j = k; j < a.nCol; ++j )
        {
            double s = 0.;
            for ( i = k; i < a.nRow; ++i )
            {
                s += a[i][j] * a[i][j];
            }
            if ( sigma < s ) sigma = s, jmax = j;
        }
        if ( sigma == 0 )
        {
            delete[] pivot;
            delete[] alpha;
            alpha = 0;
            return;
        }
        if ( jmax != k ) // перестановка столбцов
        {
            i = pivot[k];
            pivot[k] = pivot[jmax];
            pivot[jmax] = i;
            for ( i = 0; i < a.nRow; ++i )
            {
                const double t = a[i][k];
                a[i][k] = a[i][jmax];
                a[i][jmax] = t;
            }
        }
        double & akk = a[k][k];
        const double alphak = alpha[k] = akk < 0 ? sqrt ( sigma ) : - sqrt ( sigma );
        const double beta = 1. / ( sigma - akk * alphak );
        akk -= alphak;
        for ( j = k + 1; j < a.nCol; ++j )
        {
            double gamma = 0.;
            for ( i = k; i < a.nRow; ++i )
            {
                gamma += a[i][j] * a[i][k];
            }
            gamma *= beta;
            for ( i = k; i < a.nRow; ++i )
            {
                a[i][j] -= a[i][k] * gamma;
            }
        }
    }
}

bool LSS_H_Stor::solve ( const double * b, double * x ) const
{
    if ( alpha == 0 || b == 0 || x == 0 ) return false;
    double * z = alpha + a.nCol;
    nat i, j;
    for ( i = 0; i < a.nRow; ++i )
    {
        z[i] = b[i];
    }
    for ( j = 0; j < a.nCol; ++j )
    {
        double gamma = 0.;
        for ( i = j; i < a.nRow; ++i )
        {
            gamma += a[i][j] * z[i];
        }
        gamma /= alpha[j] * a[j][j];
        for ( i = j; i < a.nRow; ++i )
        {
            z[i] += a[i][j] * gamma;
        }
    }
    i = a.nCol - 1;
    double * y = z + a.nRow;
    y[i] = z[i] / alpha[i];
    while ( i > 0 )
    {
        --i;
        y[i] = z[i];
        for ( j = i + 1; j < a.nCol; ++j )
        {
            y[i] -= y[j] * a[i][j];
        }
        y[i] /= alpha[i];
    }
    for ( i = 0; i < a.nCol; ++i ) x[pivot[i]] = y[i];
    return true;
}

LSS_H_Stor::~LSS_H_Stor ()
{
	if (pivot)
	{
		delete[] pivot;
	}

    if ( alpha )
    {
        delete[] alpha;
    }
}

bool lss_h ( Matrix & a, const double * b, double * x )
{
    LSS_H_Stor lss ( a );
    return lss.solve ( b, x );
}

bool lss_h ( nat n, nat m, const double * const * a, const double * b, double * x )
{
    Matrix A ( n, m, a );
    LSS_H_Stor lss ( A );
    return lss.solve ( b, x );
}

//************************ 23.06.2005 *******************************//
//
//      Аппроксимация точек сферой
//      Минимум суммы квадратов расстояний
//      c - центр, r - радиус
//		Источник - prografix.narod.ru
//
//************************ 11.04.2008 *******************************//

bool getSphere2 ( nat n, const FBVector3d * p, FBVector3d & c, double & r )
{
    if ( n < 4 ) return false;
    double x = 0.;
    double y = 0.;
    double z = 0.;
    double xxyyzz = 0.;
    nat i;
    for ( i = 0; i < n; ++i )
    {
        const double & xi = p[i][0];
        const double & yi = p[i][1];
        const double & zi = p[i][2];
        x += xi;
        y += yi;
        z += zi;
        xxyyzz += xi * xi + yi * yi + zi * zi;
    }
    const double d = 1. / n;
    x *= d;
    y *= d;
    z *= d;
    xxyyzz *= d;
    Matrix a ( n, 3 );
    double * b = new double[n];
    for ( i = 0; i < n; ++i )
    {
        const double & xi = p[i][0];
        const double & yi = p[i][1];
        const double & zi = p[i][2];
        a[i][0] = xi - x;
        a[i][1] = yi - y;
        a[i][2] = zi - z;
        b[i] = 0.5 * ( xi * xi + yi * yi + zi * zi - xxyyzz );
    }
    const bool ok = lss_h ( a, b, &c[0] );
    delete[] b;
	if ( ok ) r = sqrt ( DotProduct(c, c) + xxyyzz - 2. * ( c[0] * x + c[1] * y + c[2] * z ) );
    return ok;
}
