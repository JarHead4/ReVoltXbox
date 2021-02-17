/*-----------------------------------------------------------------------------------------------------------
	rv_ObjAnimControlView.h

	Converted to C++ by Harvey Gilpin (Rv), March 97.
	Original C code by Dave Collins.

	Copyright Probe Entertainment Ltd,
	All rights reserved.
-----------------------------------------------------------------------------------------------------------*/
#ifndef _RV_MATRIX4X4_H_
#define _RV_MATRIX4X4_H_
//-----------------------------------------------------------------------------------------------------------
#define	D2R(x)		(( x ) * ( 1.0F / 57.2957F))		// Degrees to radians
//-----------------------------------------------------------------------------------------------------------
class JCCLASS_DLL rv_Matrix4x4
{
public:
	float	_11, _21, _31, _41;
	float	_12, _22, _32, _42;
	float	_13, _23, _33, _43;
	float	_14, _24, _34, _44;

	rv_Matrix4x4();
	~rv_Matrix4x4();

	void MultiplyBy(rv_Matrix4x4 *m);
	void Multiply(rv_Matrix4x4 *m0, rv_Matrix4x4 *m1);
	void ApplyTo(D3DVECTOR * v);
	void Transpose();
	void Copy(rv_Matrix4x4 *m);
	void BuildFromAxisAndAngle(float angle, D3DVECTOR * axis);
	void BuildFromAxisVectors(D3DVECTOR *vx, D3DVECTOR *vy, D3DVECTOR *vz);
};
//-----------------------------------------------------------------------------------------------------------
#endif // bouncer