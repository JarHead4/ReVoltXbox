
#include "revolt.h"
#include "main.h"
#include "geom.h"
#include "Gaussian.h"

// globals

REAL BaseGeomPers = 640;
REAL ScreenLeftClip, ScreenRightClip, ScreenTopClip, ScreenBottomClip;
REAL ScreenLeftClipGuard, ScreenRightClipGuard, ScreenTopClipGuard, ScreenBottomClipGuard;
MAT IdentityMatrix = {ONE, ZERO, ZERO, ZERO, ONE, ZERO, ZERO, ZERO, ONE};

MAT Identity = {ONE, ZERO, ZERO, ZERO, ONE, ZERO, ZERO, ZERO, ONE};
VEC ZeroVector = {ZERO, ZERO, ZERO};
VEC DownVec = {ZERO, ONE, ZERO};
VEC UpVec = {ZERO, -ONE, ZERO};
VEC RightVec = {ONE, ZERO, ZERO};
VEC LeftVec = {-ONE, ZERO, ZERO};
VEC LookVec = {ZERO, ZERO, ONE};
VEC NegLookVec = {ZERO, ZERO, -ONE};
QUATERNION IdentityQuat = {ZERO, ZERO, ZERO, ONE};

/////////////////////////////////////////////////////////////////////
//
// get the fraction along a line that is nearest to the passed point
//
// Inputs:
//		r0, r1:		start and end points of line
//		p:			point in space to find nearest point to
//
// Outputs:
//		rN:			nearest point on line
//		return:		fraction of distance along line that is nearest 
//					point
//
/////////////////////////////////////////////////////////////////////

#pragma optimize("", off)

REAL NearPointOnLine(VEC *r0, VEC *r1, VEC *p, VEC *rN)
{
	REAL	dRdR, rPdR, t;
	VEC	dR, rP;

	VecMinusVec(r0, p, &rP);
	VecMinusVec(r1, r0, &dR);

	dRdR = VecDotVec(&dR, &dR);
	rPdR = VecDotVec(&rP, &dR);

	t = - rPdR / dRdR;

	VecPlusScalarVec(r0, t, &dR, rN);

	return t;
}
#pragma optimize("", on)

////////////////////
// set x rotation //
////////////////////

void RotMatrixX(MAT *mat, REAL rot)
{
	REAL c, s;

	rot *= RAD;
	c = (REAL)cos(rot);
	s = (REAL)sin(rot);
	
	mat->m[RX] = 1;
	mat->m[RY] = 0;
	mat->m[RZ] = 0;

	mat->m[UX] = 0;
	mat->m[UY] = c;
	mat->m[UZ] = -s;

	mat->m[LX] = 0;
	mat->m[LY] = s;
	mat->m[LZ] = c;
}

////////////////////
// set y rotation //
////////////////////

void RotMatrixY(MAT *mat, REAL rot)
{
	REAL c, s;

	rot *= RAD;
	c = (REAL)cos(rot);
	s = (REAL)sin(rot);

	mat->m[RX] = c;
	mat->m[RY] = 0;
	mat->m[RZ] = s;

	mat->m[UX] = 0;
	mat->m[UY] = 1;
	mat->m[UZ] = 0;

	mat->m[LX] = -s;
	mat->m[LY] = 0;
	mat->m[LZ] = c;
}

////////////////////
// set z rotation //
////////////////////

void RotMatrixZ(MAT *mat, REAL rot)
{
	REAL c, s;

	rot *= RAD;
	c = (REAL)cos(rot);
	s = (REAL)sin(rot);

	mat->m[RX] = c;
	mat->m[RY] = -s;
	mat->m[RZ] = 0;

	mat->m[UX] = s;
	mat->m[UY] = c;
	mat->m[UZ] = 0;

	mat->m[LX] = 0;
	mat->m[LY] = 0;
	mat->m[LZ] = 1;
}

//////////////////////
// set zyx rotation //
//////////////////////

void RotMatrixZYX(MAT *mat, REAL x, REAL y, REAL z)
{
	REAL cx, cy, cz, sx, sy, sz;

	cx = (REAL)cos(x * RAD);
	cy = (REAL)cos(-y * RAD);
	cz = (REAL)cos(z * RAD);
	sx = (REAL)sin(x * RAD);
	sy = (REAL)sin(-y * RAD);
	sz = (REAL)sin(z * RAD);

	mat->m[RX] = cy * cz;
	mat->m[RY] = -cy * sz;
	mat->m[RZ] = -sy;

	mat->m[UX] = cx * sz - sx * sy * cz;
	mat->m[UY] = sx * sy * sz + cx * cz;
	mat->m[UZ] = -sx * cy;

	mat->m[LX] = cx * sy * cz + sx * sz;
	mat->m[LY] = sx * cz - cx * sy * sz;
	mat->m[LZ] = cx * cy;
}

//////////////////
// rot a vector //
//////////////////

void RotVector(MAT *mat, VEC *in, VEC *out)
{
	out->v[X] = in->v[X] * mat->m[RX] + in->v[Y] * mat->m[UX] + in->v[Z] * mat->m[LX];
	out->v[Y] = in->v[X] * mat->m[RY] + in->v[Y] * mat->m[UY] + in->v[Z] * mat->m[LY];
	out->v[Z] = in->v[X] * mat->m[RZ] + in->v[Y] * mat->m[UZ] + in->v[Z] * mat->m[LZ];
}

////////////////////////////
// transpose rot a vector //
////////////////////////////

void TransposeRotVector(MAT *mat, VEC *in, VEC *out)
{
	out->v[X] = in->v[X] * mat->m[RX] + in->v[Y] * mat->m[RY] + in->v[Z] * mat->m[RZ];
	out->v[Y] = in->v[X] * mat->m[UX] + in->v[Y] * mat->m[UY] + in->v[Z] * mat->m[UZ];
	out->v[Z] = in->v[X] * mat->m[LX] + in->v[Y] * mat->m[LY] + in->v[Z] * mat->m[LZ];
}

//////////////////////////
// rot / trans a vector //
//////////////////////////

void RotTransVector(MAT *mat, VEC *trans, VEC *in, VEC *out)
{
	out->v[X] = in->v[X] * mat->m[RX] + in->v[Y] * mat->m[UX] + in->v[Z] * mat->m[LX] + trans->v[X];
	out->v[Y] = in->v[X] * mat->m[RY] + in->v[Y] * mat->m[UY] + in->v[Z] * mat->m[LY] + trans->v[Y];
	out->v[Z] = in->v[X] * mat->m[RZ] + in->v[Y] * mat->m[UZ] + in->v[Z] * mat->m[LZ] + trans->v[Z];
}

/////////////////////////////////
// rot / trans / pers a vector //
/////////////////////////////////

void RotTransPersVector(MAT *mat, VEC *trans, VEC *in, REAL *out)
{
	REAL z;

	z = in->v[X] * mat->m[RZ] + in->v[Y] * mat->m[UZ] + in->v[Z] * mat->m[LZ] + trans->v[Z];
	if (z < 1) z = 1;

	out[0] = (in->v[X] * mat->m[RX] + in->v[Y] * mat->m[UX] + in->v[Z] * mat->m[LX] + trans->v[X]) / z + RenderSettings.GeomCentreX;
	out[1] = (in->v[X] * mat->m[RY] + in->v[Y] * mat->m[UY] + in->v[Z] * mat->m[LY] + trans->v[Y]) / z + RenderSettings.GeomCentreY;

	out[3] = 1 / z;
	out[2] = GET_ZBUFFER(z);
}

/////////////////////////////////
// rot / trans / pers a vector //
/////////////////////////////////

void RotTransPersVectorZleave(MAT *mat, VEC *trans, VEC *in, REAL *out)
{
	REAL z;

	z = in->v[X] * mat->m[RZ] + in->v[Y] * mat->m[UZ] + in->v[Z] * mat->m[LZ] + trans->v[Z];
	out[0] = (in->v[X] * mat->m[RX] + in->v[Y] * mat->m[UX] + in->v[Z] * mat->m[LX] + trans->v[X]) / z + RenderSettings.GeomCentreX;
	out[1] = (in->v[X] * mat->m[RY] + in->v[Y] * mat->m[UY] + in->v[Z] * mat->m[LY] + trans->v[Y]) / z + RenderSettings.GeomCentreY;

	out[3] = 1 / z;
	out[2] = GET_ZBUFFER(z);
}

/////////////////////////////////
// rot / trans / pers a vector //
/////////////////////////////////

void RotTransPersVectorZbias(MAT *mat, VEC *trans, VEC *in, REAL *out, REAL zbias)
{
	REAL z;

	z = in->v[X] * mat->m[RZ] + in->v[Y] * mat->m[UZ] + in->v[Z] * mat->m[LZ] + trans->v[Z];
	if (z < 1) z = 1;

	out[0] = (in->v[X] * mat->m[RX] + in->v[Y] * mat->m[UX] + in->v[Z] * mat->m[LX] + trans->v[X]) / z + RenderSettings.GeomCentreX;
	out[1] = (in->v[X] * mat->m[RY] + in->v[Y] * mat->m[UY] + in->v[Z] * mat->m[LY] + trans->v[Y]) / z + RenderSettings.GeomCentreY;

	out[3] = 1 / z;
	out[2] = GET_ZBUFFER(z + zbias);
}

///////////////////////////
// multiply two matrices //
///////////////////////////

void MulMatrix(MAT *one, MAT *two, MAT *out)
{
	char i, j, k;

	for (i = 0 ; i < 9 ; i++)
		out->m[i] = 0;

	for (i = 0 ; i < 3 ; i++)
		for (j = 0 ; j < 3 ; j++)
			for (k = 0 ; k < 3 ; k++)
				out->m[i * 3 + j] += one->m[k * 3 + j] * two->m[i * 3 + k];
}

////////////////////////
// transpose a matrix //
////////////////////////

void TransposeMatrix(MAT *in, MAT *out)
{
	out->m[RX] = in->m[RX];
	out->m[RY] = in->m[UX];
	out->m[RZ] = in->m[LX];
	out->m[UX] = in->m[RY];
	out->m[UY] = in->m[UY];
	out->m[UZ] = in->m[LY];
	out->m[LX] = in->m[RZ];
	out->m[LY] = in->m[UZ];
	out->m[LZ] = in->m[LZ];
}

/////////////////////////
// build a look matrix //
/////////////////////////

void BuildLookMatrixForward(VEC *pos, VEC *look, MAT *mat)
{

// get forward vector

	SubVector(look, pos, &mat->mv[Z]);
	NormalizeVector(&mat->mv[Z]);

// get right vector

	mat->m[RX] = mat->m[LZ];
	mat->m[RY] = 0;
	mat->m[RZ] = -mat->m[LX];
	NormalizeVector(&mat->mv[X]);

// get up vector

	CrossProduct(&mat->mv[Z], &mat->mv[X], &mat->mv[Y]);
}

/////////////////////////
// build a look matrix //
/////////////////////////

void BuildLookMatrixDown(VEC *pos, VEC *look, MAT *mat)
{

// get up vector

	SubVector(look, pos, &mat->mv[Y]);
	NormalizeVector(&mat->mv[Y]);

// get right vector

	mat->m[RX] = -mat->m[UZ];
	mat->m[RY] = 0;
	mat->m[RZ] = mat->m[UX];
	NormalizeVector(&mat->mv[X]);

// get forward vector

	CrossProduct(&mat->mv[X], &mat->mv[Y], &mat->mv[Z]);
}

///////////////////////////////////////
// Copy 3x3 matrix from src to dest  //
///////////////////////////////////////

void CopyMatrix(MAT *src, MAT *dest)
{
	dest->m[XX] = src->m[XX];
	dest->m[XY] = src->m[XY];
	dest->m[XZ] = src->m[XZ];

	dest->m[YX] = src->m[YX];
	dest->m[YY] = src->m[YY];
	dest->m[YZ] = src->m[YZ];

	dest->m[ZX] = src->m[ZX];
	dest->m[ZY] = src->m[ZY];
	dest->m[ZZ] = src->m[ZZ];
}

/////////////////////////////////////////////////////////////////////
//
// BuildMatrixFromLook: build a world matrix given the look vector
//
/////////////////////////////////////////////////////////////////////

void BuildMatrixFromLook(MAT *matrix)
{

	// Choose an up vector perpendicular to the look vector
	if (fabs(matrix->m[LX]) < fabs(matrix->m[LY])) {
		if (fabs(matrix->m[LX]) < fabs(matrix->m[LZ])) {	// either X or Z is smallest
			matrix->m[UX] = 0.0f;					// look[X] is smallest
			matrix->m[UY] = matrix->m[LZ];
			matrix->m[UZ] = -matrix->m[LY];
		} else {
			matrix->m[UX] = -matrix->m[LY];		// look[Z] is smallest
			matrix->m[UY] = matrix->m[LX];
			matrix->m[UZ] = 0.0f;
		}
	} else {										// either Y or Z is smallest
		if (fabs(matrix->m[LY]) < fabs(matrix->m[LZ])) {
			matrix->m[UX] = matrix->m[LZ];		// Y is smallest
			matrix->m[UY] = 0.0f;
			matrix->m[UZ] = -matrix->m[LX];
		} else {
			matrix->m[UX] = matrix->m[LY];		// Z is smallest
			matrix->m[UY] = -matrix->m[LX];
			matrix->m[UZ] = 0.0f;
		}
	}
	NormalizeVector(&matrix->mv[U])

	// Calculate a right vector from the up and look vectors
	CrossProduct(&matrix->mv[L], &matrix->mv[U], &matrix->mv[R]);
}


void BuildMatrixFromUp(MAT *matrix)
{

	// Choose a right vector perpendicular to the up vector
	if (fabs(matrix->m[UX]) < fabs(matrix->m[UY])) {
		if (fabs(matrix->m[UX]) < fabs(matrix->m[UZ])) {	// either X or Z is smallest
			matrix->m[RX] = 0.0f;					// look[X] is smallest
			matrix->m[RY] = matrix->m[UZ];
			matrix->m[RZ] = -matrix->m[UY];
		} else {
			matrix->m[RX] = -matrix->m[UY];		// look[Z] is smallest
			matrix->m[RY] = matrix->m[UX];
			matrix->m[RZ] = 0.0f;
		}
	} else {										// either Y or Z is smallest
		if (fabs(matrix->m[UY]) < fabs(matrix->m[UZ])) {
			matrix->m[RX] = matrix->m[UZ];		// Y is smallest
			matrix->m[RY] = 0.0f;
			matrix->m[RZ] = -matrix->m[UX];
		} else {
			matrix->m[RX] = matrix->m[UY];		// Z is smallest
			matrix->m[RY] = -matrix->m[UX];
			matrix->m[RZ] = 0.0f;
		}
	}
	NormalizeVector(&matrix->mv[R])

	// Calculate a right vector from the up and look vectors
	CrossProduct(&matrix->mv[R], &matrix->mv[U], &matrix->mv[L]);
}


/////////////////////////////////////////////////////////////////////
//
// Interpolate3D: use a quadric parametric fit to interpolate
// between the three vector values passed (assumed r1 is mid-point).
// See QuadInterp3D for full interpolation with no assumptions.
//
// Inputs:
//		r0, r1, r2	- the three vectors to interpolate between
//		t			- parameter for interpolation (between 0 and 1)
//		rt			- the result (position at parameter t)
//
/////////////////////////////////////////////////////////////////////

void Interpolate3D(VEC *r0, VEC *r1, VEC *r2, REAL t, VEC *rt)
{
	/*rt[X] = t2Sq * (r0[X] - r1[X] + r2[X]) -
		t * (3.0f * r0[X] - r1[X] + r2[X]) +
		r0[X];
	rt[Y] = t2Sq * (r0[Y] - r1[Y] + r2[Y]) -
		t * (3.0f * r0[Y] - r1[Y] + r2[Y]) +
		r0[Y];
	rt[Z] = t2Sq * (r0[Z] - r1[Z] + r2[Z]) -
		t * (3.0f * r0[Z] - r1[Z] + r2[Z]) +
		r0[Z];*/
	REAL tSq = t * t;

	rt->v[X] = r0->v[X] * (2.0f * tSq - 3.0f * t + 1.0f) +
		r1->v[X] * (-4.0f * tSq + 4.0f * t) +
		r2->v[X] * (2.0f * tSq - t);
	rt->v[Y] = r0->v[Y] * (2.0f * tSq - 3.0f * t + 1.0f) +
		r1->v[Y] * (-4.0f * tSq + 4.0f * t) +
		r2->v[Y] * (2.0f * tSq - t);
	rt->v[Z] = r0->v[Z] * (2.0f * tSq - 3.0f * t + 1.0f) +
		r1->v[Z] * (-4.0f * tSq + 4.0f * t) +
		r2->v[Z] * (2.0f * tSq - t);

}



/////////////////////////////////////////////////////////////////////
//
// QuadInterpVec: quadratic interpolation in 3D between three points
// r0, r1, r2 parameterised by t.
//
/////////////////////////////////////////////////////////////////////

void QuadInterpVec(VEC *r0, REAL t0, VEC *r1, REAL t1, VEC *r2, REAL t2, REAL t, VEC *rt)
{
	REAL a, b, c;

	a = ((t - t1) * (t - t2)) / ((t0 - t1) * (t0 - t2));
	b = ((t - t0) * (t - t2)) / ((t1 - t0) * (t1 - t2));
	c = ((t - t0) * (t - t1)) / ((t2 - t0) * (t2 - t1));

	rt->v[X] = a * r0->v[X] + b * r1->v[X] + c * r2->v[X];
	rt->v[Y] = a * r0->v[Y] + b * r1->v[Y] + c * r2->v[Y];
	rt->v[Z] = a * r0->v[Z] + b * r1->v[Z] + c * r2->v[Z];
}


/////////////////////////////////////////////////////////////////////
//
// LInterpVec: linear interpolation between two vectors
//
/////////////////////////////////////////////////////////////////////

void LInterpVec(VEC *r0, REAL t0, VEC *r1, REAL t1, REAL t, VEC *rt)
{
	REAL a, b;

	a = (t - t1) / (t0 - t1);
	b = (t - t0) / (t1 - t0);

	rt->v[X] = a * r0->v[X] + b * r1->v[X];
	rt->v[Y] = a * r0->v[Y] + b * r1->v[Y];
	rt->v[Z] = a * r0->v[Z] + b * r1->v[Z];
}

/////////////////////////////////////////////////////////////////////
// MatMulVec: matrix times vector
// vOut = mIn * vIn 
/////////////////////////////////////////////////////////////////////

void MatMulVec(MAT *mIn, VEC *vIn, VEC *vOut)
{
	vOut->v[X] =
		mIn->m[XX] * vIn->v[X] +
		mIn->m[XY] * vIn->v[Y] +
		mIn->m[XZ] * vIn->v[Z]; 
	vOut->v[Y] =
		mIn->m[YX] * vIn->v[X] +
		mIn->m[YY] * vIn->v[Y] +
		mIn->m[YZ] * vIn->v[Z]; 
	vOut->v[Z] =
		mIn->m[ZX] * vIn->v[X] +
		mIn->m[ZY] * vIn->v[Y] +
		mIn->m[ZZ] * vIn->v[Z]; 
}

void VecMulMat(VEC *vIn, MAT *mIn, VEC *vOut)
{
	vOut->v[X] =
		mIn->m[XX] * vIn->v[X] +
		mIn->m[YX] * vIn->v[Y] +
		mIn->m[ZX] * vIn->v[Z]; 
	vOut->v[Y] =
		mIn->m[XY] * vIn->v[X] +
		mIn->m[YY] * vIn->v[Y] +
		mIn->m[ZY] * vIn->v[Z]; 
	vOut->v[Z] =
		mIn->m[XZ] * vIn->v[X] +
		mIn->m[YZ] * vIn->v[Y] +
		mIn->m[ZZ] * vIn->v[Z]; 
}
	

/////////////////////////////////////////////////////////////////////
// MatMulThisVec: multiply the passed vector by the matrix and
// store result in the passed vector
/////////////////////////////////////////////////////////////////////
void MatMulThisVec(MAT *mIn, VEC *vInOut)
{
	VEC vecTemp;
	vecTemp.v[X] =
		mIn->m[XX] * vInOut->v[X] +
		mIn->m[XY] * vInOut->v[Y] +
		mIn->m[XZ] * vInOut->v[Z]; 
	vecTemp.v[Y] =
		mIn->m[YX] * vInOut->v[X] +
		mIn->m[YY] * vInOut->v[Y] +
		mIn->m[YZ] * vInOut->v[Z]; 
	vecTemp.v[Z] =
		mIn->m[ZX] * vInOut->v[X] +
		mIn->m[ZY] * vInOut->v[Y] +
		mIn->m[ZZ] * vInOut->v[Z];
	vInOut->v[X] = vecTemp.v[X];	
	vInOut->v[Y] = vecTemp.v[Y];	
	vInOut->v[Z] = vecTemp.v[Z];	
}

/////////////////////////////////////////////////////////////////////
// MatMulTranMat: multiply matrix on left by the trnaspose of the
// matrix on the right. Return in mOut
/////////////////////////////////////////////////////////////////////

extern void MatMulTransMat(MAT *mLeft, MAT *mRight, MAT *mOut)
{
	mOut->m[XX] = 
		mLeft->m[XX] * mRight->m[XX] + 
		mLeft->m[XY] * mRight->m[XY] + 
		mLeft->m[XZ] * mRight->m[XZ];
	mOut->m[XY] = 
		mLeft->m[XX] * mRight->m[YX] + 
		mLeft->m[XY] * mRight->m[YY] + 
		mLeft->m[XZ] * mRight->m[YZ];
	mOut->m[XZ] = 
		mLeft->m[XX] * mRight->m[ZX] + 
		mLeft->m[XY] * mRight->m[ZY] + 
		mLeft->m[XZ] * mRight->m[ZZ];

	mOut->m[YX] = 
		mLeft->m[YX] * mRight->m[XX] + 
		mLeft->m[YY] * mRight->m[XY] + 
		mLeft->m[YZ] * mRight->m[XZ];
	mOut->m[YY] = 
		mLeft->m[YX] * mRight->m[YX] + 
		mLeft->m[YY] * mRight->m[YY] + 
		mLeft->m[YZ] * mRight->m[YZ];
	mOut->m[YZ] = 
		mLeft->m[YX] * mRight->m[ZX] + 
		mLeft->m[YY] * mRight->m[ZY] + 
		mLeft->m[YZ] * mRight->m[ZZ];

	mOut->m[ZX] = 
		mLeft->m[ZX] * mRight->m[XX] + 
		mLeft->m[ZY] * mRight->m[XY] + 
		mLeft->m[ZZ] * mRight->m[XZ];
	mOut->m[ZY] = 
		mLeft->m[ZX] * mRight->m[YX] + 
		mLeft->m[ZY] * mRight->m[YY] + 
		mLeft->m[ZZ] * mRight->m[YZ];
	mOut->m[ZZ] = 
		mLeft->m[ZX] * mRight->m[ZX] + 
		mLeft->m[ZY] * mRight->m[ZY] + 
		mLeft->m[ZZ] * mRight->m[ZZ];
}

extern void TransMatMulMat(MAT *mLeft, MAT *mRight, MAT *mOut)
{
	mOut->m[XX] = 
		mLeft->m[XX] * mRight->m[XX] + 
		mLeft->m[YX] * mRight->m[YX] + 
		mLeft->m[ZX] * mRight->m[ZX];
	mOut->m[XY] = 
		mLeft->m[XX] * mRight->m[XY] + 
		mLeft->m[YX] * mRight->m[YY] + 
		mLeft->m[ZX] * mRight->m[ZY];
	mOut->m[XZ] = 
		mLeft->m[XX] * mRight->m[XZ] + 
		mLeft->m[YX] * mRight->m[YZ] + 
		mLeft->m[ZX] * mRight->m[ZZ];

	mOut->m[YX] = 
		mLeft->m[XY] * mRight->m[XX] + 
		mLeft->m[YY] * mRight->m[YX] + 
		mLeft->m[ZY] * mRight->m[ZX];
	mOut->m[YY] = 
		mLeft->m[XY] * mRight->m[XY] + 
		mLeft->m[YY] * mRight->m[YY] + 
		mLeft->m[ZY] * mRight->m[ZY];
	mOut->m[YZ] = 
		mLeft->m[XY] * mRight->m[XZ] + 
		mLeft->m[YY] * mRight->m[YZ] + 
		mLeft->m[ZY] * mRight->m[ZZ];

	mOut->m[ZX] = 
		mLeft->m[XZ] * mRight->m[XX] + 
		mLeft->m[YZ] * mRight->m[YX] + 
		mLeft->m[ZZ] * mRight->m[ZX];
	mOut->m[ZY] = 
		mLeft->m[XZ] * mRight->m[XY] + 
		mLeft->m[YZ] * mRight->m[YY] + 
		mLeft->m[ZZ] * mRight->m[ZY];
	mOut->m[ZZ] = 
		mLeft->m[XZ] * mRight->m[XZ] + 
		mLeft->m[YZ] * mRight->m[YZ] + 
		mLeft->m[ZZ] * mRight->m[ZZ];
}

extern void MatMulMat(MAT *mLeft, MAT *mRight, MAT *mOut)
{
	mOut->m[XX] = 
		mLeft->m[XX] * mRight->m[XX] + 
		mLeft->m[XY] * mRight->m[YX] + 
		mLeft->m[XZ] * mRight->m[ZX];
	mOut->m[XY] = 
		mLeft->m[XX] * mRight->m[XY] + 
		mLeft->m[XY] * mRight->m[YY] + 
		mLeft->m[XZ] * mRight->m[ZY];
	mOut->m[XZ] = 
		mLeft->m[XX] * mRight->m[XZ] + 
		mLeft->m[XY] * mRight->m[YZ] + 
		mLeft->m[XZ] * mRight->m[ZZ];

	mOut->m[YX] = 
		mLeft->m[YX] * mRight->m[XX] + 
		mLeft->m[YY] * mRight->m[YX] + 
		mLeft->m[YZ] * mRight->m[ZX];
	mOut->m[YY] = 
		mLeft->m[YX] * mRight->m[XY] + 
		mLeft->m[YY] * mRight->m[YY] + 
		mLeft->m[YZ] * mRight->m[ZY];
	mOut->m[YZ] = 
		mLeft->m[YX] * mRight->m[XZ] + 
		mLeft->m[YY] * mRight->m[YZ] + 
		mLeft->m[YZ] * mRight->m[ZZ];

	mOut->m[ZX] = 
		mLeft->m[ZX] * mRight->m[XX] + 
		mLeft->m[ZY] * mRight->m[YX] + 
		mLeft->m[ZZ] * mRight->m[ZX];
	mOut->m[ZY] = 
		mLeft->m[ZX] * mRight->m[XY] + 
		mLeft->m[ZY] * mRight->m[YY] + 
		mLeft->m[ZZ] * mRight->m[ZY];
	mOut->m[ZZ] = 
		mLeft->m[ZX] * mRight->m[XZ] + 
		mLeft->m[ZY] * mRight->m[YZ] + 
		mLeft->m[ZZ] * mRight->m[ZZ];
}

/////////////////////////////////////////////////////////////////////
// BuildRotation3D: Calculate the rotation matrix for rotation about
// the Vector passed by an angle given by the length of the vector
// (in radians)
// Taken from "Graphics Gems IV" pp. 557
/////////////////////////////////////////////////////////////////////

void BuildRotation3D(REAL axisX, REAL axisY, REAL axisZ, REAL angle, MAT *matOut)
{
	REAL c, s;

	c = (REAL) cos(angle);
	s = (REAL) sin(angle);

	matOut->m[XX] = axisX * axisX - c * axisX * axisX + c;
	matOut->m[XY] = axisX * axisY - c * axisX * axisY - s * axisZ;
	matOut->m[XZ] = axisX * axisZ - c * axisX * axisZ + s * axisY;
	matOut->m[YX] = axisY * axisX - c * axisY * axisX + s * axisZ;
	matOut->m[YY] = axisY * axisY - c * axisY * axisY + c;
	matOut->m[YZ] = axisY * axisZ - c * axisY * axisZ - s * axisX;
	matOut->m[ZX] = axisZ * axisX - c * axisZ * axisX - s * axisY;
	matOut->m[ZY] = axisZ * axisY - c * axisZ * axisY + s * axisX;
	matOut->m[ZZ] = axisZ * axisZ - c * axisZ * axisZ + c;
}


/////////////////////////////////////////////////////////////////////
//
// RotationX: build a rotation matrix about the X axis (angle in radians
//
/////////////////////////////////////////////////////////////////////

void RotationX(MAT *mat, REAL rot)
{
	REAL c, s;

	c = (REAL)cos(rot);
	s = (REAL)sin(rot);
	
	mat->m[RX] = 1;
	mat->m[RY] = 0;
	mat->m[RZ] = 0;

	mat->m[UX] = 0;
	mat->m[UY] = c;
	mat->m[UZ] = -s;

	mat->m[LX] = 0;
	mat->m[LY] = s;
	mat->m[LZ] = c;
}

/////////////////////////////////////////////////////////////////////
//
// RotationY: build a rotation matrix about the Y axis (angle in radians
//
/////////////////////////////////////////////////////////////////////

void RotationY(MAT *mat, REAL rot)
{
	REAL c, s;

	c = (REAL)cos(rot);
	s = (REAL)sin(rot);

	mat->m[RX] = c;
	mat->m[RY] = 0;
	mat->m[RZ] = s;

	mat->m[UX] = 0;
	mat->m[UY] = 1;
	mat->m[UZ] = 0;

	mat->m[LX] = -s;
	mat->m[LY] = 0;
	mat->m[LZ] = c;
}

/////////////////////////////////////////////////////////////////////
//
// RotationZ: build a rotation matrix about the Z axis (angle in radians
//
/////////////////////////////////////////////////////////////////////

void RotationZ(MAT *mat, REAL rot)
{
	REAL c, s;

	c = (REAL)cos(rot);
	s = (REAL)sin(rot);

	mat->m[RX] = c;
	mat->m[RY] = -s;
	mat->m[RZ] = 0;

	mat->m[UX] = s;
	mat->m[UY] = c;
	mat->m[UZ] = 0;

	mat->m[LX] = 0;
	mat->m[LY] = 0;
	mat->m[LZ] = 1;
}

/////////////////////////////////////////////////////////////////////
// CopyMat: copy MAT object from src to dest
/////////////////////////////////////////////////////////////////////
void CopyMat(MAT *src, MAT *dest) 
{
	dest->m[XX] = src->m[XX];
	dest->m[XY] = src->m[XY];
	dest->m[XZ] = src->m[XZ];

	dest->m[YX] = src->m[YX];
	dest->m[YY] = src->m[YY];
	dest->m[YZ] = src->m[YZ];

	dest->m[ZX] = src->m[ZX];
	dest->m[ZY] = src->m[ZY];
	dest->m[ZZ] = src->m[ZZ];
}

/////////////////////////////////////////////////////////////////////
// SetMat:
/////////////////////////////////////////////////////////////////////

void SetMat(MAT *mat, REAL xx, REAL xy, REAL xz, REAL yx, REAL yy, REAL yz, REAL zx, REAL zy, REAL zz)
{
	mat->m[XX] = xx;
	mat->m[XY] = xy;
	mat->m[XZ] = xz;

	mat->m[YX] = yx;
	mat->m[YY] = yy;
	mat->m[YZ] = yz;

	mat->m[ZX] = zx;
	mat->m[ZY] = zy;
	mat->m[ZZ] = zz;
}


/////////////////////////////////////////////////////////////////////
// SetMatUnit: set the passed amtrix to the unit matrix
/////////////////////////////////////////////////////////////////////
void SetMatUnit(MAT *mat)
{
	mat->m[XX] = mat->m[YY] = mat->m[ZZ] = (REAL)1.0f;
	mat->m[XY] = mat->m[XZ] = (REAL)0.0f;
	mat->m[YX] = mat->m[YZ] = (REAL)0.0f;
	mat->m[ZX] = mat->m[ZY] = (REAL)0.0f;
}

void SetMatZero(MAT *mat)
{
	mat->m[XX] = mat->m[YY] = mat->m[ZZ] = (REAL)0.0f;
	mat->m[XY] = mat->m[XZ] = (REAL)0.0f;
	mat->m[YX] = mat->m[YZ] = (REAL)0.0f;
	mat->m[ZX] = mat->m[ZY] = (REAL)0.0f;
}


/////////////////////////////////////////////////////////////////////
// MatMulScalar: multiply all elements of vector by a scalar
/////////////////////////////////////////////////////////////////////
void MatMulScalar(MAT *mat, REAL scalar)
{
	int iEl;

	for (iEl = 0; iEl < 9; iEl++) {
		mat->m[iEl] *= scalar;
	}
}


/////////////////////////////////////////////////////////////////////
// BuildCrossMatrix: set up the cross-product matrix of the 
// passed vector
/////////////////////////////////////////////////////////////////////
void BuildCrossMat(VEC *vec, MAT *mat)
{
	mat->m[XX] = (REAL)0.0f;
	mat->m[XY] = -vec->v[Z];
	mat->m[XZ] = vec->v[Y];
	
	mat->m[YX] = vec->v[Z];
	mat->m[YY] = (REAL)0.0f;
	mat->m[YZ] = -vec->v[X];

	mat->m[ZX] = -vec->v[Y];
	mat->m[ZY] = vec->v[X];
	mat->m[ZZ] = (REAL)0.0f;
}

/////////////////////////////////////////////////////////////////////
// VecCrossMat: the vector crodd-product matrix multiplying a
// matrix
/////////////////////////////////////////////////////////////////////
void VecCrossMat(VEC *vecLeft, MAT *matRight, MAT *matOut)
{

	matOut->m[XX] = vecLeft->v[Y] * matRight->m[ZX] - vecLeft->v[Z] * matRight->m[YX];
	matOut->m[XY] = vecLeft->v[Y] * matRight->m[ZY] - vecLeft->v[Z] * matRight->m[YY];
	matOut->m[XZ] = vecLeft->v[Y] * matRight->m[ZZ] - vecLeft->v[Z] * matRight->m[YZ];

	matOut->m[YX] = vecLeft->v[Z] * matRight->m[XX] - vecLeft->v[X] * matRight->m[ZX];
	matOut->m[YY] = vecLeft->v[Z] * matRight->m[XY] - vecLeft->v[X] * matRight->m[ZY];
	matOut->m[YZ] = vecLeft->v[Z] * matRight->m[XZ] - vecLeft->v[X] * matRight->m[ZZ];

	matOut->m[ZX] = vecLeft->v[X] * matRight->m[YX] - vecLeft->v[Y] * matRight->m[XX];
	matOut->m[ZY] = vecLeft->v[X] * matRight->m[YY] - vecLeft->v[Y] * matRight->m[XY];
	matOut->m[ZZ] = vecLeft->v[X] * matRight->m[YZ] - vecLeft->v[Y] * matRight->m[XZ];

}

void MatCrossVec(MAT *matLeft, VEC *vecRight, MAT *matOut)
{

	matOut->m[XX] = - vecRight->v[Y] * matLeft->m[XZ] + vecRight->v[Z] * matLeft->m[XY];
	matOut->m[XY] = - vecRight->v[Z] * matLeft->m[XX] + vecRight->v[X] * matLeft->m[XZ];
	matOut->m[XZ] = - vecRight->v[X] * matLeft->m[XY] + vecRight->v[Y] * matLeft->m[XX];

	matOut->m[YX] = - vecRight->v[Y] * matLeft->m[YZ] + vecRight->v[Z] * matLeft->m[YY];
	matOut->m[YY] = - vecRight->v[Z] * matLeft->m[YX] + vecRight->v[X] * matLeft->m[YZ];
	matOut->m[YZ] = - vecRight->v[X] * matLeft->m[YY] + vecRight->v[Y] * matLeft->m[YX];

	matOut->m[ZX] = - vecRight->v[Y] * matLeft->m[ZZ] + vecRight->v[Z] * matLeft->m[ZY];
	matOut->m[ZY] = - vecRight->v[Z] * matLeft->m[ZX] + vecRight->v[X] * matLeft->m[ZZ];
	matOut->m[ZZ] = - vecRight->v[X] * matLeft->m[ZY] + vecRight->v[Y] * matLeft->m[ZX];

}

/////////////////////////////////////////////////////////////////////
// SwapVecs: swap two vectors
/////////////////////////////////////////////////////////////////////

void SwapVecs(VEC *a, VEC *b) 
{
	VEC store;
	store.v[X] = a->v[X];
	store.v[Y] = a->v[Y];
	store.v[Z] = a->v[Z];

	a->v[X] = b->v[X];
	a->v[Y] = b->v[Y];
	a->v[Z] = b->v[Z];

	b->v[X] = store.v[X];
	b->v[Y] = store.v[Y];
	b->v[Z] = store.v[Z];
}


/////////////////////////////////////////////////////////////////////
//
// TransMat: transpose forst matrix and stroe in second
//
/////////////////////////////////////////////////////////////////////

void TransMat(MAT *src, MAT *dest) 
{
	dest->m[XX] = src->m[XX];
	dest->m[XY] = src->m[YX];
	dest->m[XZ] = src->m[ZX];

	dest->m[YX] = src->m[XY];
	dest->m[YY] = src->m[YY];
	dest->m[YZ] = src->m[ZY];

	dest->m[ZX] = src->m[XZ];
	dest->m[ZY] = src->m[YZ];
	dest->m[ZZ] = src->m[ZZ];
}


/////////////////////////////////////////////////////////////////////
//
// InvertMat: invert a 3x3 matrix using gaussian elimination
// with partial pivoting (Graphics Gems IV pp. 554)
//
/////////////////////////////////////////////////////////////////////

void InvertMat(MAT *mat)
{

	// As workMat evolves towards identity
	// mat evolves towards inverse
	MAT	a;
	MAT	b;
	REAL	tmpReal;

	int i, j;
	int pivotCandidate;							// Row with largest pivot candidate

	CopyMat(mat, &a);
	SetMatUnit(&b);


	// Loop over columns from left to right, eliminating above and below diagonal
	for (j = 0; j < 3; ++j) {
		pivotCandidate = j;
		
		for (i = j + 1; i < 3; ++i) {
			if (abs(a.mv[i].v[j]) > abs(a.mv[pivotCandidate].v[j])) {
				pivotCandidate = i;
			}
		}

		// Swap rows `pivotCandidate' and `j' in workMat and mat to put pivot on diagonal
		SwapVecs(&a.mv[pivotCandidate], &a.mv[j]);
		SwapVecs(&b.mv[pivotCandidate], &b.mv[j]);

		// Scale row `j' to have a unit diagonal
		if (a.mv[j].v[j] == (REAL)0.0) {
			// Cannot get inverse of this matrix as rows are linearly dependent
			SetMatZero(mat);
			return;
		}
		tmpReal = a.mv[j].v[j];
		VecDivScalar(&b.mv[j], tmpReal);
		VecDivScalar(&a.mv[j], tmpReal);

		// Eliminate off-diagonal elements in column `j'
		for (i = 0; i < 3; ++i) {
			if (i != j) {
				tmpReal = a.mv[i].v[j];
				b.mv[i].v[X] -= tmpReal * b.mv[j].v[X];
				b.mv[i].v[Y] -= tmpReal * b.mv[j].v[Y];
				b.mv[i].v[Z] -= tmpReal * b.mv[j].v[Z];
				
				a.mv[i].v[X] -= tmpReal * a.mv[j].v[X];
				a.mv[i].v[Y] -= tmpReal * a.mv[j].v[Y];
				a.mv[i].v[Z] -= tmpReal * a.mv[j].v[Z];
			}
		}
	}

	CopyMat(&b, mat);
}

/////////////////////////////////////////////////////////////////////
//
// MatPlusMat:
//
/////////////////////////////////////////////////////////////////////

void MatPlusMat(MAT *matLeft, MAT *matRight, MAT *matOut)
{
	matOut->m[XX] = matLeft->m[XX] + matRight->m[XX];
	matOut->m[XY] = matLeft->m[XY] + matRight->m[XY];
	matOut->m[XZ] = matLeft->m[XZ] + matRight->m[XZ];

	matOut->m[YX] = matLeft->m[YX] + matRight->m[YX];
	matOut->m[YY] = matLeft->m[YY] + matRight->m[YY];
	matOut->m[YZ] = matLeft->m[YZ] + matRight->m[YZ];

	matOut->m[ZX] = matLeft->m[ZX] + matRight->m[ZX];
	matOut->m[ZY] = matLeft->m[ZY] + matRight->m[ZY];
	matOut->m[ZZ] = matLeft->m[ZZ] + matRight->m[ZZ];
}

void MatPlusEqScalarMat(MAT *matLeft, REAL scalar, MAT *matRight)
{
	matLeft->m[XX] += scalar * matRight->m[XX];
	matLeft->m[XY] += scalar * matRight->m[XY];
	matLeft->m[XZ] += scalar * matRight->m[XZ];

	matLeft->m[YX] += scalar * matRight->m[YX];
	matLeft->m[YY] += scalar * matRight->m[YY];
	matLeft->m[YZ] += scalar * matRight->m[YZ];

	matLeft->m[ZX] += scalar * matRight->m[ZX];
	matLeft->m[ZY] += scalar * matRight->m[ZY];
	matLeft->m[ZZ] += scalar * matRight->m[ZZ];
}

/////////////////////////////////////////////////////////////////////
//
// BuildMatFromVec: given a vector, build a valid orientation matrix
//
/////////////////////////////////////////////////////////////////////

void BuildMatFromVec(VEC *vec, MAT *mat)
{
	mat->m[UX] = vec->v[X];
	mat->m[UY] = vec->v[Y];
	mat->m[UZ] = vec->v[Z];
	
	// Choose a right vector perpendicular to the up vector
	if (fabs(mat->m[UX]) < fabs(mat->m[UY])) {
		if (fabs(mat->m[UX]) < fabs(mat->m[UZ])) {	// either X or Z is smallest
			mat->m[RX] = 0.0f;					// up[X] is smallest
			mat->m[RY] = mat->m[UZ];
			mat->m[RZ] = -mat->m[UY];
		} else {
			mat->m[RX] = -mat->m[UY];		// up[Z] is smallest
			mat->m[RY] = mat->m[UX];
			mat->m[RZ] = 0.0f;
		}
	} else {										// either Y or Z is smallest
		if (fabs(mat->m[UY]) < fabs(mat->m[UZ])) {
			mat->m[RX] = mat->m[UZ];		// Y is smallest
			mat->m[RY] = 0.0f;
			mat->m[RZ] = -mat->m[UX];
		} else {
			mat->m[RX] = mat->m[UY];		// Z is smallest
			mat->m[RY] = -mat->m[UX];
			mat->m[RZ] = 0.0f;
		}
	}
	NormalizeVec(&mat->mv[R])

	// Calculate a right vector from the up and look vectors
	CrossProduct(&mat->mv[R], &mat->mv[U], &mat->mv[L]);
	NormalizeVec(&mat->mv[L]);

}


/////////////////////////////////////////////////////////////////////
//
// MovePlane: move a plane by the specified vector.
// Assumes plane normalised
//
/////////////////////////////////////////////////////////////////////

void MovePlane(PLANE *plane, VEC *dR)
{
	plane->v[D] -= VecDotVec(dR, PlaneNormal(plane));
}


/////////////////////////////////////////////////////////////////////
//
// PlaneIntersect3: calculate intersection point of three planes.
// Return true if planes intersect at same point, false otherwise.
//
/////////////////////////////////////////////////////////////////////

static BIGMAT	GEO_Coef;
static BIGVEC	GEO_Res;
static BIGVEC	GEO_Soln;
static BIGVEC	GEO_Work;
static int		GEO_OrigCol[BIG_NMAX];
static int		GEO_OrigRow[BIG_NMAX];

bool PlaneIntersect3(PLANE *p1, PLANE *p2, PLANE *p3, VEC *r) 
{
	int	nSolved;

	// Build the plane equations for the solver
	SetBigMatSize(&GEO_Coef, 3, 3);
	SetBigVecSize(&GEO_Res, 3);
	SetBigVecSize(&GEO_Soln, 3);

	GEO_Coef.m[0][A] = p1->v[A];
	GEO_Coef.m[0][B] = p1->v[B];
	GEO_Coef.m[0][C] = p1->v[C];
	GEO_Res.v[0] = -p1->v[D];

	GEO_Coef.m[1][A] = p2->v[A];
	GEO_Coef.m[1][B] = p2->v[B];
	GEO_Coef.m[1][C] = p2->v[C];
	GEO_Res.v[1] = -p2->v[D];

	GEO_Coef.m[2][A] = p3->v[A];
	GEO_Coef.m[2][B] = p3->v[B];
	GEO_Coef.m[2][C] = p3->v[C];
	GEO_Res.v[2] = -p3->v[D];

	nSolved = SolveLinearEquations(&GEO_Coef, &GEO_Res, ZERO, Real(0.0001), GEO_OrigRow, GEO_OrigCol, &GEO_Work, &GEO_Soln);
	if (nSolved < 3) {
		return FALSE;
	} else {
		SetVec(r, GEO_Soln.v[0], GEO_Soln.v[1], GEO_Soln.v[2]);
		return TRUE;
	}

	/*REAL	numerator;
	VEC	denominator;

	numerator = p1->v[D] - p2->v[D] - p3->v[D];
	VecMinusVec(PlaneNormal(p3), PlaneNormal(p1), &denominator);
	VecPlusEqVec(&denominator, PlaneNormal(p2));

	if (denominator.v[X] < SMALL_REAL) {
		SetVecZero(r);
		return FALSE;
	}
	r->v[X] = numerator / denominator.v[X];

	if (denominator.v[Y] < SMALL_REAL) {
		SetVecZero(r);
		return FALSE;
	}
	r->v[Y] = numerator / denominator.v[Y];
	
	if (denominator.v[Z] < SMALL_REAL) {
		SetVecZero(r);
		return FALSE;
	}
	r->v[Z] = numerator / denominator.v[Z];

	return TRUE;*/
}


/////////////////////////////////////////////////////////////////////
//
// RotTransPlane: rotate then translate the passed plane
//
/////////////////////////////////////////////////////////////////////

void RotTransPlane(PLANE *plane, MAT *rotMat, VEC *dR, PLANE *pOut)
{
	VecMulMat(PlaneNormal(plane), rotMat, PlaneNormal(pOut));

	pOut->v[D] = plane->v[D] - VecDotVec(dR, PlaneNormal(pOut));
}

/////////////////////////////////
// build a plane from 3 points //
/////////////////////////////////

void BuildPlane(VEC *a, VEC *b, VEC *c, PLANE *p)
{
	VEC vec1, vec2;

// build plane normal

	SubVector(b, a, &vec1)
	SubVector(c, a, &vec2)
	CrossProduct(&vec1, &vec2, (VEC*)p);
	NormalizeVector((VEC*)p);

// build plane W

	AddVector(a, b, &vec1);
	AddVector(&vec1, c, &vec1);
	p->v[D] = -DotProduct((VEC*)p, &vec1) / 3.0f;
}

/////////////////////////////////////////////////////////////////////
//
// BuildPlane: build plane equation from normal and point on plane
//
/////////////////////////////////////////////////////////////////////

void BuildPlane2(VEC *normal, VEC *pt, PLANE *plane)
{
	CopyVec(normal, PlaneNormal(plane));
	plane->v[D] = -VecDotVec(pt, normal);
}


/////////////////////////////////////////////////////////////////////
//
// QuatToMat: convert quaternion into rotation matrix
//
/////////////////////////////////////////////////////////////////////

void QuatToMat(QUATERNION *quat, MAT *mat) 
{
	REAL tReal1, tReal2;

	mat->m[XX] = ONE - 2 * (quat->v[VY] * quat->v[VY] + quat->v[VZ] * quat->v[VZ]);
	mat->m[YY] = ONE - 2 * (quat->v[VX] * quat->v[VX] + quat->v[VZ] * quat->v[VZ]);
	mat->m[ZZ] = ONE - 2 * (quat->v[VX] * quat->v[VX] + quat->v[VY] * quat->v[VY]);

	tReal1 = quat->v[VX] * quat->v[VY];
	tReal2 = quat->v[S] * quat->v[VZ];
	mat->m[YX] = 2 * (tReal1 - tReal2);
	mat->m[XY] = 2 * (tReal1 + tReal2);

	tReal1 = quat->v[VX] * quat->v[VZ];
	tReal2 = quat->v[S] * quat->v[VY];
	mat->m[ZX] = 2 * (tReal1 + tReal2);
	mat->m[XZ] = 2 * (tReal1 - tReal2);

	tReal1 = quat->v[VY] * quat->v[VZ];
	tReal2 = quat->v[S] * quat->v[VX];
	mat->m[ZY] = 2 * (tReal1 - tReal2);
	mat->m[YZ] = 2 * (tReal1 + tReal2);

}


/////////////////////////////////////////////////////////////////////
//
// MatToQuat: convert matrix to quaternion
//
/////////////////////////////////////////////////////////////////////

void MatToQuat(MAT *mat, QUATERNION *quat)
{
	REAL	tr, s;
	int		i = XX;

	tr = mat->m[XX] + mat->m[YY] + mat->m[ZZ];

	if (tr >= 0) {

		s = (REAL)sqrt(tr + ONE);
		quat->v[S]  = HALF * s;
		s = HALF / s;
		quat->v[VX] = (mat->m[YZ] - mat->m[ZY]) * s;
		quat->v[VY] = (mat->m[ZX] - mat->m[XZ]) * s;
		quat->v[VZ] = (mat->m[XY] - mat->m[YX]) * s;

	} else {
		if (mat->m[YY] > mat->m[XX]) {
			i = YY;
		}
		if (mat->m[ZZ] > mat->m[i]) {
			i = ZZ;
		}

		switch(i) {

		case XX:
			
			s = (REAL)sqrt((mat->m[XX] - (mat->m[YY] + mat->m[ZZ])) + 1);
			quat->v[VX] = HALF * s;
			s = HALF / s;
			quat->v[VY] = (mat->m[YX] + mat->m[XY]) * s;
			quat->v[VZ] = (mat->m[XZ] + mat->m[ZX]) * s;
			quat->v[S] = (mat->m[YZ] - mat->m[ZY]) * s;
			break;

		case YY:

			s = (REAL)sqrt((mat->m[YY] - (mat->m[ZZ] + mat->m[XX])) + 1);
			quat->v[VY] = HALF * s;
			s = HALF / s;
			quat->v[VZ] = (mat->m[ZY] + mat->m[YZ]) * s;
			quat->v[VX] = (mat->m[YX] + mat->m[XY]) * s;
			quat->v[S] = (mat->m[ZX] - mat->m[XZ]) * s;
			break;

		case ZZ:

			s = (REAL)sqrt((mat->m[ZZ] - (mat->m[XX] + mat->m[YY])) + 1);
			quat->v[VZ] = HALF * s;
			s = HALF / s;
			quat->v[VX] = (mat->m[XZ] + mat->m[ZX]) * s;
			quat->v[VY] = (mat->m[ZY] + mat->m[YZ]) * s;
			quat->v[S] = (mat->m[XY] - mat->m[YX]) * s;
			break;

		}
	}
}


/////////////////////////////////////////////////////////////////////
//
// QuatRotVec: rotate a vector by the given unit quaternion
//
/////////////////////////////////////////////////////////////////////

void QuatRotVec(QUATERNION *quat, VEC *vIn, VEC *vOut)
{
	REAL	sQ2;
	REAL	dot;
	VEC	cross;

	sQ2 = quat->v[S] * quat->v[S];
	dot = VecDotVec(vIn, VecOfQuat(quat));
	VecCrossVec(vIn, VecOfQuat(quat), &cross);

	VecCrossVec(&cross, VecOfQuat(quat), vOut)
	VecPlusEqScalarVec(vOut, sQ2, vIn);
	VecPlusEqScalarVec(vOut, -2 * quat->v[S], &cross);
	VecPlusEqScalarVec(vOut, dot, VecOfQuat(quat));

}


/////////////////////////////////////////////////////////////////////
//
// LerpQuat: linear interpolation between two quaternions
//
/////////////////////////////////////////////////////////////////////

void LerpQuat(QUATERNION *q0, QUATERNION *q1, REAL t, QUATERNION *qt)
{
	qt->v[VX] = (ONE - t) * q0->v[VX] + t * q1->v[VX];
	qt->v[VY] = (ONE - t) * q0->v[VY] + t * q1->v[VY];
	qt->v[VZ] = (ONE - t) * q0->v[VZ] + t * q1->v[VZ];
	qt->v[S] = (ONE - t) * q0->v[S] + t * q1->v[S];
}


/////////////////////////////////////////////////////////////////////
//
// SLerpQuat: spherical linear interpolation between two quaternions
//
/////////////////////////////////////////////////////////////////////

#ifndef _N64
void SLerpQuat(QUATERNION *q0, QUATERNION *q1, REAL t, QUATERNION *qt)
{
	REAL theta, sinT, sin1mtT, sintT;

	theta = (REAL)acosf(QuatDotQuat(q0, q1));

	// for small angles, spherical interpolation same as linear
	if (theta < SLERP_SMALL_ANGLE) {
		sinT = ZERO;
		sintT = t;
		sin1mtT = (ONE - t);
	} else {
		sinT = (REAL)sin(theta);
		sintT = (REAL)sin(t * theta);
		sin1mtT = (REAL)sin((ONE - t) * theta);
		sin1mtT /= sinT;
		sintT /= sinT;
	}

	qt->v[VX] = sin1mtT * q0->v[VX] + sintT * q1->v[VX];
	qt->v[VY] = sin1mtT * q0->v[VY] + sintT * q1->v[VY];
	qt->v[VZ] = sin1mtT * q0->v[VZ] + sintT * q1->v[VZ];
	qt->v[S] = sin1mtT * q0->v[S] + sintT * q1->v[S];
}
#endif


/////////////////////////////////////////////////////////////////////
//
// LinePoint: find the point(s) on a line which are a given distance
// from the passed point.
//
/////////////////////////////////////////////////////////////////////

bool LinePoint(VEC *p, REAL d, VEC *r0, VEC *r1, REAL *t1, REAL *t2)
{
	VEC	rP, dR;
	REAL	rPrP, dRdR, rPdR, dd, t;

	VecMinusVec(r1, r0, &dR);
	VecMinusVec(r0, p, &rP);
	
	dd = d * d;
	rPrP = VecDotVec(&rP, &rP);
	dRdR = VecDotVec(&dR, &dR);
	rPdR = VecDotVec(&rP, &dR);

	t = Real(4) * (rPdR * rPdR - dRdR * (rPrP - dd));

	if (t < ZERO) return FALSE;
	t = (REAL)sqrt(t);

	*t1 = (-rPdR + HALF * t) / dRdR;
	*t2 = (-rPdR - HALF * t) / dRdR;

	return TRUE;
}

void TestLinePoint()
{
	VEC	r0, r1, p;
	REAL	t1, t2;


	SetVec(&r0, ZERO, ZERO, ZERO);
	SetVec(&r1, ONE, ZERO, ZERO);

	SetVec(&p, HALF, ONE, ZERO);

	LinePoint(&p, ONE, &r0, &r1, &t1, &t2);

	LinePoint(&p, 1.1f, &r0, &r1, &t1, &t2);

	LinePoint(&p, 2.0f, &r0, &r1, &t1, &t2);

	SetVec(&p, ONE, ONE, ZERO);

	LinePoint(&p, ONE, &r0, &r1, &t1, &t2);

	LinePoint(&p, 1.1f, &r0, &r1, &t1, &t2);

	LinePoint(&p, 2.0f, &r0, &r1, &t1, &t2);

}


////////////////////////////////////////////////////////////////
// find intersection of a plane from two points and distances //
////////////////////////////////////////////////////////////////

void FindIntersection(VEC *point1, REAL dist1, VEC *point2, REAL dist2, VEC *out)
{
	REAL mul;
	VEC diff;

// get diff vector, mul

	SubVector(point2, point1, &diff);
	mul = -dist1 / (dist2 - dist1);

// get intersection

	out->v[X] = point1->v[X] + diff.v[X] * mul;
	out->v[Y] = point1->v[Y] + diff.v[Y] * mul;
	out->v[Z] = point1->v[Z] + diff.v[Z] * mul;
}

