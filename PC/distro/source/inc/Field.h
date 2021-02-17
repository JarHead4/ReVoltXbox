
#ifndef __FIELD_H__
#define __FIELD_H__


/////////////////////////////////////////////////////////////////////
//
// Field.h: Force field header file
//
// Fields are allocated from a static linked list of available fields.
// When a field is created, a field from the empty list is transferred
// to the active field list. When a field is deleted, it is moved from
// the active list to the empty list.
//
// The fields apply an impulse to particles. The field impulses must be
// applied before any collision response is calculated.
//
/////////////////////////////////////////////////////////////////////

#ifndef _PSX
#define MAX_FIELDS	128
#else 
#define MAX_FIELDS	16
#endif

#define FIELD_PARENT_NONE -1

enum {
	FIELD_PRIORITY_MAX = 0,
	FIELD_PRIORITY_MIN
};


typedef struct LinearParamsStruct {
	VEC	Size;
	VEC	Dir;
	REAL	Mag;
	REAL	Damping;
} LINEAR_FIELD_PARAMS;

typedef struct LinearTwistParamsStruct {
	VEC	Size;
	VEC	Dir;
	REAL	Mag;
	REAL	Damping;
	VEC Torque;
} LINEARTWIST_FIELD_PARAMS;

typedef struct VelocityParamsStruct {
	VEC	Size;
	VEC	Dir;
	REAL	Mag;
} VELOCITY_FIELD_PARAMS;

typedef struct SphericalParamsStruct {
	REAL	RadStart, RadEnd;					// Start and end radii squared
	REAL	GradStart, GradEnd;					// Start and end gradient
} SPHERICAL_FIELD_PARAMS;

typedef struct CylindricalParamsStruct {
	VEC	Dir;
	REAL	RadStart, RadEnd;					// Start and end radii squared
	REAL	GradStart, GradEnd;					// Start and end gradient
} CYLINDRICAL_FIELD_PARAMS;

typedef struct OrientationParamsStruct {
	VEC	Size;
	VEC	Dir;
	REAL	Mag;
	REAL	Damping;
} ORIENTATION_FIELD_PARAMS;

typedef struct LocalParamsStruct {
	VEC	Size;
	VEC	*DirPtr;
	REAL	Mag;
	REAL	Damping;
} LOCAL_FIELD_PARAMS;

typedef struct VortexParamsStruct {
	VEC	Size;
	VEC	Dir;
	REAL	Mag;
	REAL	RMin;
	REAL	RMax;
} VORTEX_FIELD_PARAMS;

typedef struct FieldDataStruct {
	long ObjectID;
	long Priority;
	VEC *Pos;
	VEC *Vel;
	VEC *AngVel;
	MAT *Mat;
	QUATERNION *Quat;
	REAL Mass;
} FIELD_DATA;

typedef struct ForceFieldStruct {
	long	Status;
	long	Type;
	long	ParentID;
	long	Priority;

	VEC	*PosPtr;								// World position of centre of force field
	MAT	*MatPtr;								// World matrix of field
	BBOX	BBox;									// Local axis-aligned bounding box
	union ForceFieldParamsUnion {
		LINEAR_FIELD_PARAMS			LinearParams;
		LINEARTWIST_FIELD_PARAMS	LinearTwistParams;
		LOCAL_FIELD_PARAMS			LocalParams;
		VELOCITY_FIELD_PARAMS		VelocityParams;
		SPHERICAL_FIELD_PARAMS		SphericalParams;
		CYLINDRICAL_FIELD_PARAMS	CylindricalParams;
		ORIENTATION_FIELD_PARAMS	OrientationParams;
		VORTEX_FIELD_PARAMS			VortexParams;
	} Params;
	bool	(*FieldCollTest)(struct ForceFieldStruct *field, VEC *pos);	// Check if pos is in field area of effect	
	void	(*AddFieldImpulse)(struct ForceFieldStruct *field, FIELD_DATA *data, VEC *imp, VEC *angImp);	// Apply impulse to particle or body from field

	struct ForceFieldStruct *Prev;
	struct ForceFieldStruct *Next;

} FORCE_FIELD;



/*#define FIELD_OWNS_POS	1
#define FIELD_OWNS_MAT	2

#define SetFieldOwnsPos(field)	((field)->Status |= FIELD_OWNS_POS)
#define DoesFieldOwnPos(field)	((field)->Statuc & FIELD_OWNS_POS)
#define SetFieldOwnsMat(field)	((field)->Status |= FIELD_OWNS_MAT)
#define DoesFieldOwnMat(field)	((field)->Statuc & FIELD_OWNS_MAT)*/

#define FieldPriorityTest(fieldPriority, objectPriority) ((objectPriority) >= (fieldPriority))

#define FIELD_LINEAR	0
#define FIELD_SPHERICAL	1
#define FIELD_VELOCITY	2
#define FIELD_ORIENTATION 4
#define FIELD_VORTEX	8
#define FIELD_LOCAL		16
#define FIELD_LINEARTWIST 32

/////////////////////////////////////////////////////////////////////
//
// GLOBAL FUNCTIONS
//
/////////////////////////////////////////////////////////////////////

extern void FreeForceFields();
extern void InitFields(void);
extern void RemoveField(FORCE_FIELD *field);
extern FORCE_FIELD *FirstField(void);
extern FORCE_FIELD *NextField(FORCE_FIELD *field);
extern void AllFieldImpulses(FIELD_DATA *data, VEC *imp, VEC *angImp);

extern FORCE_FIELD *AddLinearField(long parentID, long priority, VEC *posPtr, MAT *matPtr, BBOX *bBox, VEC *size, VEC *dir, REAL mag, REAL damping);
extern bool LinearFieldCollTest(FORCE_FIELD *field, VEC *pos);
extern void LinearFieldImpulse(FORCE_FIELD *field, FIELD_DATA *data, VEC *imp, VEC *angImp);

extern FORCE_FIELD *AddLinearTwistField(long parentID, long priority, VEC *posPtr, MAT *matPtr, BBOX *bBox, VEC *size, VEC *dir, REAL mag, VEC *torque, REAL damping);
extern bool LinearTwistFieldCollTest(FORCE_FIELD *field, VEC *pos);
extern void LinearTwistFieldImpulse(FORCE_FIELD *field, FIELD_DATA *data, VEC *imp, VEC *angImp);

extern FORCE_FIELD *AddLocalField(long parentID, long priority, VEC *posPtr, MAT *matPtr, BBOX *bBox, VEC *size, VEC *dir, REAL mag, REAL damping);
extern bool LocalFieldCollTest(FORCE_FIELD *field, VEC *pos);
extern void LocalFieldImpulse(FORCE_FIELD *field, FIELD_DATA *data, VEC *imp, VEC *angImp);

extern FORCE_FIELD *AddVelocityField(long parentID, long priority, VEC *posPtr, MAT *matPtr, BBOX *bBox, VEC *size, VEC *dir, REAL mag);
extern bool VelocityFieldCollTest(FORCE_FIELD *field, VEC *pos);
extern void VelocityFieldImpulse(FORCE_FIELD *field, FIELD_DATA *data, VEC *imp, VEC *angImp);

extern FORCE_FIELD *AddSphericalField(long parentID, long priority, VEC *posPtr, REAL rStart, REAL rEnd, REAL gStart, REAL gEnd);
extern bool SphericalFieldCollTest(FORCE_FIELD *field, VEC *pos);
extern void SphericalFieldImpulse(FORCE_FIELD *field, FIELD_DATA *data, VEC *imp, VEC *angImp);

extern FORCE_FIELD *AddCylindricalField(long parentID, long priority, VEC *posPtr, VEC *dir, REAL rStart, REAL rEnd, REAL gStart, REAL gEnd);
extern bool CylindricalFieldCollTest(FORCE_FIELD *field, VEC *pos);
extern void CylindricalFieldImpulse(FORCE_FIELD *field, FIELD_DATA *data, VEC *imp, VEC *angImp);

extern FORCE_FIELD *AddVortexField(long parentID, long priority, VEC *posPtr, MAT *matPtr, BBOX *bBox, VEC *size, VEC *dir, REAL mag, REAL rMin, REAL rMax);
extern bool VortexFieldCollTest(FORCE_FIELD *field, VEC *pos);
extern void VortexFieldImpulse(FORCE_FIELD *field, FIELD_DATA *data, VEC *imp, VEC *angImp);

extern FORCE_FIELD *AddOrientationField(long parentID, long priority, VEC *posPtr, MAT *matPtr, BBOX *bBox, VEC *size, VEC *dir, REAL mag, REAL damping);
extern bool OrientationFieldCollTest(FORCE_FIELD *field, VEC *pos);
extern void OrientationFieldImpulse(FORCE_FIELD *field, FIELD_DATA *data, VEC *imp, VEC *angImp);


/////////////////////////////////////////////////////////////////////
//
// GLOBALS
//
/////////////////////////////////////////////////////////////////////

extern int FLD_NForceFields;
extern REAL FLD_Gravity;
extern BBOX FLD_GlobalBBox;
extern VEC FLD_GlobalSize;
extern FORCE_FIELD	*FLD_GravityField;
#endif