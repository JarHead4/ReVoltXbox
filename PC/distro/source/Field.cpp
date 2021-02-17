
#include "ReVolt.h"
#include "NewColl.h"
#include "Particle.h"
#include "Body.h"
#include "Geom.h"
#ifndef _PSX
#include "Main.h"
#include "Level.h"
#endif
#include "Field.h"
#ifdef _PC
 #include "EdField.h"
#endif

#if MSCOMPILER_FUDGE_OPTIMISATIONS
#pragma optimize("", off)
#endif

/////////////////////////////////////////////////////////////////////
// GLOBALS
//
FORCE_FIELD	*FLD_GravityField = NULL;
REAL		FLD_Gravity = TO_ACC(Real(2200));
BBOX		FLD_GlobalBBox = {-LARGEDIST, LARGEDIST, -LARGEDIST, LARGEDIST, -LARGEDIST, LARGEDIST};
VEC		FLD_GlobalSize = {LARGEDIST, LARGEDIST, LARGEDIST};


/////////////////////////////////////////////////////////////////////
// LOCALS
//
FORCE_FIELD FLD_ForceField[MAX_FIELDS];
int FLD_NForceFields = 0;

FORCE_FIELD *FLD_FieldHead;
FORCE_FIELD *FLD_EmptyFieldHead;


/////////////////////////////////////////////////////////////////////
// prototypes
//
static FORCE_FIELD *AddField(void);


void FreeForceFields();
void InitFields(void);
void RemoveField(FORCE_FIELD *field);
FORCE_FIELD *FirstField(void);
FORCE_FIELD *NextField(FORCE_FIELD *field);
void AllFieldImpulses(FIELD_DATA *data, VEC *imp, VEC *angImp);

FORCE_FIELD *AddLinearField(long parentID, long priority, VEC *posPtr, MAT *matPtr, BBOX *bBox, VEC *size, VEC *dir, REAL mag, REAL damping);
bool LinearFieldCollTest(FORCE_FIELD *field, VEC *pos);
void LinearFieldImpulse(FORCE_FIELD *field, FIELD_DATA *data, VEC *imp, VEC *angImp);

FORCE_FIELD *AddLinearTwistField(long parentID, long priority, VEC *posPtr, MAT *matPtr, BBOX *bBox, VEC *size, VEC *dir, REAL mag, VEC *torque, REAL damping);
bool LinearTwistFieldCollTest(FORCE_FIELD *field, VEC *pos);
void LinearTwistFieldImpulse(FORCE_FIELD *field, FIELD_DATA *data, VEC *imp, VEC *angImp);

FORCE_FIELD *AddLocalField(long parentID, long priority, VEC *posPtr, MAT *matPtr, BBOX *bBox, VEC *size, VEC *dir, REAL mag, REAL damping);
bool LocalFieldCollTest(FORCE_FIELD *field, VEC *pos);
void LocalFieldImpulse(FORCE_FIELD *field, FIELD_DATA *data, VEC *imp, VEC *angImp);

FORCE_FIELD *AddVelocityField(long parentID, long priority, VEC *posPtr, MAT *matPtr, BBOX *bBox, VEC *size, VEC *dir, REAL mag);
bool VelocityFieldCollTest(FORCE_FIELD *field, VEC *pos);
void VelocityFieldImpulse(FORCE_FIELD *field, FIELD_DATA *data, VEC *imp, VEC *angImp);

FORCE_FIELD *AddSphericalField(long parentID, long priority, VEC *posPtr, REAL rStart, REAL rEnd, REAL gStart, REAL gEnd);
bool SphericalFieldCollTest(FORCE_FIELD *field, VEC *pos);
void SphericalFieldImpulse(FORCE_FIELD *field, FIELD_DATA *data, VEC *imp, VEC *angImp);

FORCE_FIELD *AddCylindricalField(long parentID, long priority, VEC *posPtr, VEC *dir, REAL rStart, REAL rEnd, REAL gStart, REAL gEnd);
bool CylindricalFieldCollTest(FORCE_FIELD *field, VEC *pos);
void CylindricalFieldImpulse(FORCE_FIELD *field, FIELD_DATA *data, VEC *imp, VEC *angImp);

FORCE_FIELD *AddVortexField(long parentID, long priority, VEC *posPtr, MAT *matPtr, BBOX *bBox, VEC *size, VEC *dir, REAL mag, REAL rMin, REAL rMax);
bool VortexFieldCollTest(FORCE_FIELD *field, VEC *pos);
void VortexFieldImpulse(FORCE_FIELD *field, FIELD_DATA *data, VEC *imp, VEC *angImp);

FORCE_FIELD *AddOrientationField(long parentID, long priority, VEC *posPtr, MAT *matPtr, BBOX *bBox, VEC *size, VEC *dir, REAL mag, REAL damping);
bool OrientationFieldCollTest(FORCE_FIELD *field, VEC *pos);
void OrientationFieldImpulse(FORCE_FIELD *field, FIELD_DATA *data, VEC *imp, VEC *angImp);


/////////////////////////////////////////////////////////////////////
//
// InitFields: Reset the list of fields. 
//
/////////////////////////////////////////////////////////////////////

void InitFields(void)
{
	int iField;


	FLD_NForceFields = 0;

	FLD_ForceField[0].Prev = NULL;

	for (iField = 1; iField < MAX_FIELDS - 1; iField++) {

		FLD_ForceField[iField - 1].Next = &FLD_ForceField[iField];
		FLD_ForceField[iField].Prev = &FLD_ForceField[iField - 1];
		FLD_ForceField[iField].Next = &FLD_ForceField[iField + 1];

	}

	FLD_ForceField[MAX_FIELDS - 1].Prev = &FLD_ForceField[MAX_FIELDS - 2];
	FLD_ForceField[MAX_FIELDS - 1].Next = NULL;

	FLD_FieldHead = NULL;
	FLD_EmptyFieldHead = &FLD_ForceField[0];

}


/////////////////////////////////////////////////////////////////////
//
// AddField: return the address of an unused field and modify
// the used and unused field lists
//
/////////////////////////////////////////////////////////////////////

FORCE_FIELD *AddField(void)
{
	FORCE_FIELD *oldFirstField;
	FORCE_FIELD *oldNextEmpty;

	// Make sure there are spare fields
	if (FLD_EmptyFieldHead == NULL) {
		return NULL;
	}


	oldFirstField = FLD_FieldHead;
	oldNextEmpty = FLD_EmptyFieldHead->Next;
	
	// Add head of empty list 
	FLD_FieldHead = FLD_EmptyFieldHead;
	FLD_FieldHead->Next = oldFirstField;
	FLD_FieldHead->Prev = NULL;
	if (oldFirstField != NULL) {
		oldFirstField->Prev = FLD_FieldHead;
	}

	// Delete it from the empty list;
	FLD_EmptyFieldHead = oldNextEmpty;

	// Keep a count of the number of active fields
	FLD_NForceFields++;

	// Set some defaults
	FLD_FieldHead->PosPtr = NULL;
	FLD_FieldHead->MatPtr = NULL;

	return FLD_FieldHead;

}

void RemoveField(FORCE_FIELD *field)
{
	// Delete from active field list
	if (field->Prev != NULL) {
		(field->Prev)->Next = field->Next;
	} else {
		FLD_FieldHead = field->Next;
	}
	if (field->Next != NULL) {
		(field->Next)->Prev = field->Prev;
	}

	// Add to inactive field list
	field->Next = FLD_EmptyFieldHead;
	FLD_EmptyFieldHead = field;

	// Keep track of number of fields
	FLD_NForceFields--;

}


/////////////////////////////////////////////////////////////////////
//
// FirstField: return pointer to the first active field
//
/////////////////////////////////////////////////////////////////////

FORCE_FIELD *FirstField(void)
{
	return FLD_FieldHead;
}


/////////////////////////////////////////////////////////////////////
//
// NextField: return pointer to the next active field
//
/////////////////////////////////////////////////////////////////////

FORCE_FIELD *NextField(FORCE_FIELD *field)
{
	Assert(field != NULL);
	return field->Next;
}


/////////////////////////////////////////////////////////////////////
//
// LinearField: Constant acceleration
//
/////////////////////////////////////////////////////////////////////

FORCE_FIELD *AddLinearField(long parentID, long priority, VEC *pos, MAT *mat, BBOX *bBox, VEC *size, VEC *dir, REAL mag, REAL damping)
{
	FORCE_FIELD *field;
	LINEAR_FIELD_PARAMS *params;
	
	field = AddField();
	params = &field->Params.LinearParams;

	field->Type = FIELD_LINEAR;
	field->ParentID = parentID;
	field->Priority = priority;
	field->PosPtr = pos;
	field->MatPtr = mat;
	field->FieldCollTest = LinearFieldCollTest;
	field->AddFieldImpulse = LinearFieldImpulse;

	CopyBBox(bBox, &field->BBox);
	CopyVec(size, &params->Size);
	CopyVec(dir, &params->Dir);
	params->Mag = mag;
	params->Damping = damping;

	return field;
}

bool LinearFieldCollTest(FORCE_FIELD *field, VEC *pos)
{
	int iFace;
	VEC dR;
	LINEAR_FIELD_PARAMS	*params = &field->Params.LinearParams;

	VecMinusVec(pos, field->PosPtr, &dR);

	// Bounding box test
	if (!PointInBBox(&dR, &field->BBox)) {
		return FALSE;
	}

	// Proper test
	for (iFace = 0; iFace < 3; iFace++) {
		if (abs(VecDotVec(&dR, &field->MatPtr->mv[iFace])) > params->Size.v[iFace]) {
			return FALSE;
		}
	}

	return TRUE;

}

void LinearFieldImpulse(FORCE_FIELD *field, FIELD_DATA *data, VEC *imp, VEC *angImp)
{
	REAL	scale;
	LINEAR_FIELD_PARAMS *params = &field->Params.LinearParams;

	scale = params->Mag - MulScalar(params->Damping, VecDotVec(&params->Dir, data->Vel));
	scale = MulScalar(scale, MulScalar(TimeStep, data->Mass));
	//scale = particle->Mass * TimeStep * (params->Mag - params->Damping * VecDotVec(&params->Dir, &particle->Vel));

	VecPlusEqScalarVec(imp, scale, &params->Dir);

}

/////////////////////////////////////////////////////////////////////
//
// LinearTwistField: Constant acceleration with turning effect
//
/////////////////////////////////////////////////////////////////////

FORCE_FIELD *AddLinearTwistField(long parentID, long priority, VEC *pos, MAT *mat, BBOX *bBox, VEC *size, VEC *dir, REAL mag, VEC * twist, REAL damping)
{
	FORCE_FIELD *field;
	LINEARTWIST_FIELD_PARAMS *params;
	
	field = AddField();
	params = &field->Params.LinearTwistParams;

	field->Type = FIELD_LINEARTWIST;
	field->ParentID = parentID;
	field->Priority = priority;
	field->PosPtr = pos;
	field->MatPtr = mat;
	field->FieldCollTest = LinearTwistFieldCollTest;
	field->AddFieldImpulse = LinearTwistFieldImpulse;

	CopyBBox(bBox, &field->BBox);
	CopyVec(size, &params->Size);
	CopyVec(dir, &params->Dir);
	params->Mag = mag;
	params->Damping = damping;
	CopyVec(twist, &params->Torque);

	return field;
}

bool LinearTwistFieldCollTest(FORCE_FIELD *field, VEC *pos)
{
	int iFace;
	VEC dR;
	LINEARTWIST_FIELD_PARAMS	*params = &field->Params.LinearTwistParams;

	VecMinusVec(pos, field->PosPtr, &dR);

	// Bounding box test
	if (!PointInBBox(&dR, &field->BBox)) {
		return FALSE;
	}

	// Proper test
	for (iFace = 0; iFace < 3; iFace++) {
		if (abs(VecDotVec(&dR, &field->MatPtr->mv[iFace])) > params->Size.v[iFace]) {
			return FALSE;
		}
	}

	return TRUE;

}

void LinearTwistFieldImpulse(FORCE_FIELD *field, FIELD_DATA *data, VEC *imp, VEC *angImp)
{
	REAL	scale;
	LINEARTWIST_FIELD_PARAMS *params = &field->Params.LinearTwistParams;

	scale = params->Mag - MulScalar(params->Damping, VecDotVec(&params->Dir, data->Vel));
	scale = MulScalar(scale, MulScalar(TimeStep, data->Mass));
	VecPlusEqScalarVec(imp, scale, &params->Dir);

	VecEqScalarVec(angImp, TimeStep, &params->Torque);

}

/////////////////////////////////////////////////////////////////////
//
// VelocityField: makes object travel at given velocity
//
/////////////////////////////////////////////////////////////////////

FORCE_FIELD *AddVelocityField(long parentID, long priority, VEC *pos, MAT *mat, BBOX *bBox, VEC *size, VEC *dir, REAL mag)
{
	FORCE_FIELD *field;
	VELOCITY_FIELD_PARAMS *params;
	
	field = AddField();
	params = &field->Params.VelocityParams;

	field->Type = FIELD_VELOCITY;
	field->ParentID = parentID;
	field->Priority = priority;
	field->PosPtr = pos;
	field->MatPtr = mat;
	field->FieldCollTest = VelocityFieldCollTest;
	field->AddFieldImpulse = VelocityFieldImpulse;

	CopyBBox(bBox, &field->BBox);
	CopyVec(size, &params->Size);
	CopyVec(dir, &params->Dir);
	params->Mag = mag;

	return field;
}

bool VelocityFieldCollTest(FORCE_FIELD *field, VEC *pos)
{
	int iFace;
	VEC dR;
	VELOCITY_FIELD_PARAMS	*params = &field->Params.VelocityParams;

	VecMinusVec(pos, field->PosPtr, &dR);

	// Bounding box test
	if (!PointInBBox(&dR, &field->BBox)) {
		return FALSE;
	}

	// Proper test
	for (iFace = 0; iFace < 3; iFace++) {
		if (abs(VecDotVec(&dR, &field->MatPtr->mv[iFace])) > params->Size.v[iFace]) {
			return FALSE;
		}
	}

	return TRUE;

}

void VelocityFieldImpulse(FORCE_FIELD *field, FIELD_DATA *data, VEC *imp, VEC *angImp)
{
	REAL mag;
	VELOCITY_FIELD_PARAMS *params = &field->Params.VelocityParams;

	mag = params->Mag - VecDotVec(&params->Dir, data->Vel);
	mag = MulScalar(mag, TimeStep);

	VecPlusEqScalarVec(imp, mag, &params->Dir);

}


/////////////////////////////////////////////////////////////////////
//
// SphericalField: repels/ attracts from centre
//
/////////////////////////////////////////////////////////////////////

FORCE_FIELD *AddSphericalField(long parentID, long priority, VEC *pos, REAL rStart, REAL rEnd, REAL gStart, REAL gEnd)
{
	
	FORCE_FIELD *field;
	SPHERICAL_FIELD_PARAMS *params;
	
	field = AddField();
	params = &field->Params.SphericalParams;

	field->Type = FIELD_SPHERICAL;
	field->ParentID = parentID;
	field->Priority = priority;
	field->PosPtr = pos;
	field->MatPtr = &Identity;
	field->FieldCollTest = SphericalFieldCollTest;
	field->AddFieldImpulse = SphericalFieldImpulse;
	SetBBox(&field->BBox, -rEnd, rEnd, -rEnd, rEnd, -rEnd, rEnd);

	params->RadStart = rStart;
	params->RadEnd = rEnd;
	params->GradStart = gStart;
	params->GradEnd = gEnd;

	return field;
}


bool SphericalFieldCollTest(FORCE_FIELD *field, VEC *pos)
{
	VEC	dR;
	REAL	dRLenSq;
	SPHERICAL_FIELD_PARAMS *params = &field->Params.SphericalParams;

	VecMinusVec(pos, field->PosPtr, &dR);
	dRLenSq = VecDotVec(&dR, &dR);

	if (dRLenSq < MulScalar(params->RadEnd, params->RadEnd)) {
		return TRUE;
	}

	return FALSE;
}

void SphericalFieldImpulse(FORCE_FIELD *field, FIELD_DATA *data, VEC *imp, VEC *angImp)
{
	VEC	dR;
	REAL	dRLen, gradient;
	SPHERICAL_FIELD_PARAMS *params = &field->Params.SphericalParams;

	VecMinusVec(data->Pos, field->PosPtr, &dR);
	dRLen = (REAL)sqrt(VecDotVec(&dR, &dR));


	if (dRLen > params->RadStart && dRLen < params->RadEnd) {

		gradient = DivScalar((dRLen - params->RadStart), (params->RadEnd - params->RadStart));
		gradient = params->GradStart + MulScalar(gradient, (params->GradEnd - params->GradStart));
		gradient = DivScalar(MulScalar(gradient, TimeStep), dRLen);
		VecMulScalar(&dR, gradient);
		
		VecPlusEqScalarVec(imp, data->Mass, &dR);
	}

}



/////////////////////////////////////////////////////////////////////
//
// CylindricalField: repels/ attracts from centre
//
/////////////////////////////////////////////////////////////////////

FORCE_FIELD *AddCylindricalField(long parentID, long priority, VEC *pos, VEC *dir, REAL rStart, REAL rEnd, REAL gStart, REAL gEnd)
{
	
	FORCE_FIELD *field;
	CYLINDRICAL_FIELD_PARAMS *params;
	
	field = AddField();
	params = &field->Params.CylindricalParams;

	field->Type = FIELD_SPHERICAL;
	field->ParentID = parentID;
	field->Priority = priority;
	field->PosPtr = pos;
	field->MatPtr = &Identity;
	field->FieldCollTest = CylindricalFieldCollTest;
	field->AddFieldImpulse = CylindricalFieldImpulse;
	SetBBox(&field->BBox, -rEnd, rEnd, -rEnd, rEnd, -rEnd, rEnd);

	CopyVec(dir, &params->Dir);
	params->RadStart = rStart;
	params->RadEnd = rEnd;
	params->GradStart = gStart;
	params->GradEnd = gEnd;

	return field;
}


bool CylindricalFieldCollTest(FORCE_FIELD *field, VEC *pos)
{
	VEC	dR;
	REAL	dRLenSq;
	CYLINDRICAL_FIELD_PARAMS *params = &field->Params.CylindricalParams;

	VecMinusVec(pos, field->PosPtr, &dR);
	dRLenSq = VecDotVec(&dR, &dR);

	if (dRLenSq < MulScalar(params->RadEnd, params->RadEnd)) {
		return TRUE;
	}

	return FALSE;
}

void CylindricalFieldImpulse(FORCE_FIELD *field, FIELD_DATA *data, VEC *imp, VEC *angImp)
{
	VEC	dR;
	REAL	dRLen, gradient, dRDir;
	CYLINDRICAL_FIELD_PARAMS *params = &field->Params.CylindricalParams;

	VecMinusVec(data->Pos, field->PosPtr, &dR);
	dRDir = VecDotVec(&dR, &params->Dir);
	VecPlusEqScalarVec(&dR, -dRDir, &params->Dir);
	dRLen = VecLen(&dR);

	if (dRLen > params->RadStart && dRLen < params->RadEnd) {

		gradient = DivScalar((dRLen - params->RadStart), (params->RadEnd - params->RadStart));
		gradient = params->GradStart + MulScalar(gradient, (params->GradEnd - params->GradStart));
		//gradient = params->GradStart + ((dRLen - params->RadStart) / (params->RadEnd - params->RadStart)) * (params->GradEnd - params->GradStart);
		//gradient *= TimeStep / dRLen;
		VecMulScalar(&dR, gradient);
		
		VecPlusEqVec(imp, &dR);
	}

}



/////////////////////////////////////////////////////////////////////
//
// OrientationField: applies torque to get objects up vector along 
// given direction
//
/////////////////////////////////////////////////////////////////////

FORCE_FIELD *AddOrientationField(long parentID, long priority, VEC *pos, MAT *mat, BBOX *bBox, VEC *size, VEC *dir, REAL mag, REAL damping)
{
	FORCE_FIELD *field;
	ORIENTATION_FIELD_PARAMS *params;
	
	field = AddField();
	params = &field->Params.OrientationParams;

	field->Type = FIELD_ORIENTATION;
	field->ParentID = parentID;
	field->Priority = priority;
	field->PosPtr = pos;
	field->MatPtr = mat;
	field->FieldCollTest = OrientationFieldCollTest;
	field->AddFieldImpulse = NULL;

	CopyBBox(bBox, &field->BBox);
	CopyVec(size, &params->Size);
	CopyVec(dir, &params->Dir);
	params->Mag = mag;
	params->Damping = damping;

	return field;
}

bool OrientationFieldCollTest(FORCE_FIELD *field, VEC *pos)
{
	int iFace;
	VEC dR;
	ORIENTATION_FIELD_PARAMS	*params = &field->Params.OrientationParams;

	VecMinusVec(pos, field->PosPtr, &dR);

	// Bounding box test
	if (!PointInBBox(&dR, &field->BBox)) {
		return FALSE;
	}

	// Proper test
	for (iFace = 0; iFace < 3; iFace++) {
		if (abs(VecDotVec(&dR, &field->MatPtr->mv[iFace])) > params->Size.v[iFace]) {
			return FALSE;
		}
	}

	return TRUE;

}

void OrientationFieldImpulse(FORCE_FIELD *field, FIELD_DATA *data, VEC *imp, VEC *angImp)
{
	VEC	torque;
	REAL	scale;
	ORIENTATION_FIELD_PARAMS *params = &field->Params.OrientationParams;

	VecCrossVec(&params->Dir, &data->Mat->mv[U], &torque);
	scale = (ONE -  MulScalar(params->Damping, VecDotVec(&torque, data->AngVel)));
	scale = MulScalar(TimeStep, MulScalar(params->Mag, scale));
	VecMulScalar(&torque, scale);

	VecPlusEqVec(angImp, &torque);
}



/////////////////////////////////////////////////////////////////////
//
// VortexField: applies torque about an axis in given direction
// about given position to maintain given angular velocity about the axis
//
/////////////////////////////////////////////////////////////////////

FORCE_FIELD *AddVortexField(long parentID, long priority, VEC *pos, MAT *mat, BBOX *bBox, VEC *size, VEC *dir, REAL mag, REAL rMin, REAL rMax)
{
	FORCE_FIELD *field;
	VORTEX_FIELD_PARAMS *params;
	
	field = AddField();
	params = &field->Params.VortexParams;

	field->Type = FIELD_VORTEX;
	field->ParentID = parentID;
	field->Priority = priority;
	field->PosPtr = pos;
	field->MatPtr = mat;
	field->FieldCollTest = VortexFieldCollTest;
	field->AddFieldImpulse = VortexFieldImpulse;

	CopyBBox(bBox, &field->BBox);
	CopyVec(size, &params->Size);
	CopyVec(dir, &params->Dir);
	params->Mag = mag;
	params->RMin = rMin;
	params->RMax = rMax;

	return field;
}

bool VortexFieldCollTest(FORCE_FIELD *field, VEC *pos)
{
	int iFace;
	VEC dR;
	ORIENTATION_FIELD_PARAMS	*params = &field->Params.OrientationParams;

	VecMinusVec(pos, field->PosPtr, &dR);

	// Bounding box test
	if (!PointInBBox(&dR, &field->BBox)) {
		return FALSE;
	}

	// Proper test
	for (iFace = 0; iFace < 3; iFace++) {
		if (abs(VecDotVec(&dR, &field->MatPtr->mv[iFace])) > params->Size.v[iFace]) {
			return FALSE;
		}
	}

	return TRUE;

}

void VortexFieldImpulse(FORCE_FIELD *field, FIELD_DATA *data, VEC *imp, VEC *angImp)
{
	VEC	dR;
	VEC	impulse;
	REAL	scale, perpLen;
	VORTEX_FIELD_PARAMS *params = &field->Params.VortexParams;

	VecMinusVec(data->Pos, field->PosPtr, &dR);
	VecCrossVec(&dR, &params->Dir, &impulse);
	perpLen = VecLen(&impulse);

	if ((perpLen > params->RMin) && (perpLen < params->RMax)) {

		scale = params->Mag - DivScalar(VecDotVec(data->Vel, &impulse), (perpLen));
		scale = DivScalar(MulScalar(scale, TimeStep), perpLen);
		VecMulScalar(&impulse, scale);

		VecPlusEqVec(imp, &impulse);
	}
}



/////////////////////////////////////////////////////////////////////
//
// ApplyAllFieldImpulses: cycle through all active fields and add
// impulses to the passed particle
//
/////////////////////////////////////////////////////////////////////

void AllFieldImpulses(FIELD_DATA *data, VEC *imp, VEC *angImp)
{
	FORCE_FIELD *field;

	SetVecZero(imp);
	SetVecZero(angImp);

	for (field = FirstField(); field != NULL; field = NextField(field)) {

		// do not add impulses to the owner of the field
		if ((field->ParentID != FIELD_PARENT_NONE) && (field->ParentID == data->ObjectID)) continue;

		// ignore field if object priority says so
		if (!FieldPriorityTest(field->Priority, data->Priority)) continue;

		// Calculate the impulses
		if (field->AddFieldImpulse != NULL) {
			if ((*field->FieldCollTest)(field, data->Pos)) {
				(*field->AddFieldImpulse)(field, data, imp, angImp);
			}
		} 

	}

}


/////////////////////////////////////////////////////////////////////
//
// LinearField: Constant acceleration
//
/////////////////////////////////////////////////////////////////////

FORCE_FIELD *AddLocalField(long parentID, long priority, VEC *pos, MAT *mat, BBOX *bBox, VEC *size, VEC *dir, REAL mag, REAL damping)
{
	FORCE_FIELD *field;
	LOCAL_FIELD_PARAMS *params;
	
	field = AddField();
	params = &field->Params.LocalParams;

	field->Type = FIELD_LOCAL;
	field->ParentID = parentID;
	field->Priority = priority;
	field->PosPtr = pos;
	field->MatPtr = mat;
	field->FieldCollTest = LocalFieldCollTest;
	field->AddFieldImpulse = LocalFieldImpulse;

	CopyBBox(bBox, &field->BBox);
	CopyVec(size, &params->Size);
	params->DirPtr = dir;
	params->Mag = mag;
	params->Damping = damping;

	return field;
}

bool LocalFieldCollTest(FORCE_FIELD *field, VEC *pos)
{
	int iFace;
	VEC dR;
	LOCAL_FIELD_PARAMS	*params = &field->Params.LocalParams;

	VecMinusVec(pos, field->PosPtr, &dR);

	// Bounding box test
	if (!PointInBBox(&dR, &field->BBox)) {
		return FALSE;
	}

	// Proper test
	for (iFace = 0; iFace < 3; iFace++) {
		if (abs(VecDotVec(&dR, &field->MatPtr->mv[iFace])) > params->Size.v[iFace]) {
			return FALSE;
		}

	}

	return TRUE;

}

void LocalFieldImpulse(FORCE_FIELD *field, FIELD_DATA *data, VEC *imp, VEC *angImp)
{
	REAL	scale;
	LOCAL_FIELD_PARAMS *params = &field->Params.LocalParams;

	scale = (params->Mag - MulScalar(params->Damping, VecDotVec(params->DirPtr, data->Vel)));
	scale = MulScalar(MulScalar(scale, data->Mass), TimeStep);

	VecPlusEqScalarVec(imp, scale, params->DirPtr);

}



/////////////////////////////////////////////////////////////////////
//
// FreeFields: free any memory allocated by the fields
//
/////////////////////////////////////////////////////////////////////

void FreeForceFields()
{
#ifndef _PSX
	free(LEV_LevelFieldPos);
	free(LEV_LevelFieldMat);
	LEV_LevelFieldPos = NULL;
	LEV_LevelFieldMat = NULL;
#endif
}



#if MSCOMPILER_FUDGE_OPTIMISATIONS
#pragma optimize("", on)
#endif
