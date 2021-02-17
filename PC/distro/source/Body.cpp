
#include "ReVolt.h"

#include "TypeDefs.h"
#include "Geom.h"
#include "Particle.h"
#include "NewColl.h"
#include "Body.h"
#ifndef _PSX
#include "Main.h"
#endif
#include "Gaussian.h"
#ifndef _PSX
#include "spark.h"
#endif

int NThisBodySparks = 0;
int NThisBodySmoke = 0;

NEWBODY	BDY_MassiveBody;

#if USE_DEBUG_ROUTINES
REAL DEBUG_MaxImpulseMag = ZERO;
REAL DEBUG_MaxAngImpulseMag = ZERO;
#endif

/////////////////////////////////////////////////////////////////////
//
// STATIC PROTOTYPES
//
static int BuildThisBodyCollList(NEWBODY *body);


/////////////////////////////////////////////////////////////////////
//
// GLOBAL PROTOTYPES
//
void SetBodyPos(NEWBODY *body, VEC *pos, MAT *mat);
void InitBodyDefault(NEWBODY *body);
void PostProcessBodyColls(NEWBODY *body);
void RemoveBodyColl(NEWBODY *body, COLLINFO_BODY *collInfo);
COLLINFO_BODY *AddBodyColl(NEWBODY *body, COLLINFO_BODY *newHead);


int DetectHullPolyColls(NEWBODY *body1, NEWBODY *body2);
int DetectConvexHullPolyColls(NEWBODY *body, NEWCOLLPOLY *collPoly);
void BuildCollisionEquations3(NEWBODY *body, BIGMAT *eqns, BIGVEC *residual);


/////////////////////////////////////////////////////////////////////
//
// InitBodyDefault: set physical parameters of body to safe defaults
//
/////////////////////////////////////////////////////////////////////

void InitBodyDefault(NEWBODY *body)
{
	body->Centre.Mass = ZERO;
	body->Centre.InvMass = ZERO;

	body->Centre.Hardness = ZERO;
	body->Centre.Resistance = ZERO;
	body->Centre.Grip = ZERO;
	body->Centre.StaticFriction = ZERO;
	body->Centre.KineticFriction =ZERO;
	body->Centre.Gravity = Real(2000);

	SetMatUnit(&body->BodyInertia);
	SetMatUnit(&body->BodyInvInertia);

	SetBodyConvex(body);
	body->CollSkin.Convex = NULL;
	body->CollSkin.WorldConvex = NULL;
	body->CollSkin.OldWorldConvex = NULL;
	body->CollSkin.Sphere = NULL;
	body->CollSkin.WorldSphere = NULL;
	body->CollSkin.OldWorldSphere = NULL;
	body->CollSkin.CollPoly = NULL;
	body->CollSkin.NConvex = 0;
	body->CollSkin.NSpheres = 0;
	body->CollSkin.NCollPolys = 0;
	body->CollSkin.AllowWorldColls = TRUE;
	body->CollSkin.AllowObjColls = TRUE;
	SetBBox(&body->CollSkin.TightBBox, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO);

	body->DefaultAngRes = ZERO;
	body->AngResMod = ONE;	
	body->AngResistance = ZERO;

	body->AllowSparks = FALSE;
	body->ScrapeMaterial = MATERIAL_NONE;

	body->Banged = FALSE;
	body->BangMag = ZERO;
	SetPlane(&body->BangPlane, ZERO, -ONE, ZERO, ZERO);

}


/////////////////////////////////////////////////////////////////////
//
// SetBodyPos
//
/////////////////////////////////////////////////////////////////////

void SetBodyPos(NEWBODY *body, VEC *pos, MAT *mat) 
{

	// Set orientation and position
	CopyVec(pos, &body->Centre.Pos);
	CopyVec(&body->Centre.Pos, &body->Centre.OldPos);
	CopyMatrix(mat, &body->Centre.WMatrix);
	CopyMat(&body->Centre.WMatrix, &body->Centre.OldWMatrix);
	MatToQuat(&body->Centre.WMatrix, &body->Centre.Quat);
	CopyQuat(&body->Centre.Quat, &body->Centre.OldQuat);

	// Reset Velocities etc
	SetVecZero(&body->Centre.Vel);
	SetVecZero(&body->Centre.Impulse);
	SetVecZero(&body->Centre.LastVel);
	SetVecZero(&body->AngVel);
	SetVecZero(&body->LastAngVel);
	SetVecZero(&body->AngImpulse);
	SetVecZero(&body->Centre.Shift);

	// Set the inverse inertia in the world frame
	GetFrameInertia(&body->BodyInvInertia, &body->Centre.WMatrix, &body->WorldInvInertia);
	BuildWorldSkin(&body->CollSkin, &body->Centre.Pos, &body->Centre.WMatrix);

	// Jitter stuff
	body->IsJittering = FALSE;
	body->JitterCount = 0;
	body->JitterFrames = 0;

	// Misc
	body->NoContactTime = ZERO;
	body->Centre.Boost = ZERO;
	body->LastScrapeTime = 0;
	body->ScrapeMaterial = MATERIAL_NONE;
}

/////////////////////////////////////////////////////////////////////
//
// ApplyBodyImpulse: apply an impulse to the NEWBODY at the given
// posisiotn relative to the centre of mass.
//
/////////////////////////////////////////////////////////////////////

void ApplyBodyImpulse(NEWBODY *body, VEC *impulse, VEC *impPos)
{
	VEC angImpulse;

	// Apply the linear impulse to the centre of mass
	ApplyParticleImpulse(&body->Centre, impulse);

	// Calculate the angular impulse and add it to the body
	CalcAngImpulse(impulse, impPos, &angImpulse);
	ApplyBodyAngImpulse(body, &angImpulse);
}


/////////////////////////////////////////////////////////////////////
//
// UpdateBody: update the state of the NEWBODY according to the impulses
// acting on it
//
/////////////////////////////////////////////////////////////////////

void UpdateBody(NEWBODY *body, REAL dt)
{
	REAL		scale;
	CONVEX		*tmpConvex;
	SPHERE		*tmpSphere;
	QUATERNION	tmpQuat;

	// DEBUGGING
#if USE_DEBUG_ROUTINES
	
	REAL impMag;
	impMag = VecLen(&body->Centre.Impulse);
	if (impMag > DEBUG_MaxImpulseMag) DEBUG_MaxImpulseMag = impMag;
	impMag = VecLen(&body->AngImpulse);
	if (impMag > DEBUG_MaxAngImpulseMag) DEBUG_MaxAngImpulseMag = impMag;

	CopyVec(&body->Centre.Impulse, &DEBUG_Impulse);
	CopyVec(&body->AngImpulse, &DEBUG_AngImpulse);

#endif

	// Store last frames angular velocity
	CopyVec(&body->AngVel, &body->LastAngVel);

	// If object is jittering, reduce angular impulse
#if REMOVE_JITTER
	if (body->IsJittering) {
		VecMulScalar(&body->AngImpulse, HALF);
	}
#endif

	// Update the linear position, velocity etc.
	UpdateParticle(&body->Centre, dt);

	// Calculate the angular acceleration from the impulse (actually acceleration * dt)
	MatMulVec(&body->WorldInvInertia, &body->AngImpulse, &body->AngAcc);

	// Update the body angular velocity
	VecPlusEqVec(&body->AngVel, &body->AngAcc);

	// Damp the angular velocity from air resistance
	scale = MulScalar(body->AngResistance, MulScalar(dt, FRICTION_TIME_SCALE));
	VecMulScalar(&body->AngVel, ONE - scale);

	// Calculate change in quaternion this frame
	VecMulQuat(&body->AngVel, &body->Centre.Quat, &tmpQuat);
	scale = MulScalar(HALF, dt);
	QuatPlusEqScalarQuat(&body->Centre.Quat, scale, &tmpQuat);
	NormalizeQuat(&body->Centre.Quat);
	QuatToMat(&body->Centre.Quat, &body->Centre.WMatrix);

	// Set the inverse inertia in the world frame
	GetFrameInertia(&body->BodyInvInertia, &body->Centre.WMatrix, &body->WorldInvInertia);

	// Set up the world collision skins
	tmpConvex = body->CollSkin.WorldConvex;
	body->CollSkin.WorldConvex = body->CollSkin.OldWorldConvex;
	body->CollSkin.OldWorldConvex = tmpConvex;
	tmpSphere = body->CollSkin.WorldSphere;
	body->CollSkin.WorldSphere = body->CollSkin.OldWorldSphere;
	body->CollSkin.OldWorldSphere = tmpSphere;
	BuildWorldSkin(&body->CollSkin, &body->Centre.Pos, &body->Centre.WMatrix);

	// Stuff for removing jitter
#if REMOVE_JITTER
	body->JitterFrames++;
	if (VecDotVec(&body->AngVel, &body->LastAngVel) < ZERO) {
		if (body->JitterFrames < body->JitterFramesMax) {
			body->JitterCount++;
		} else {
			body->JitterCount = 0;
			body->IsJittering = FALSE;
		}
		body->JitterFrames = 0;

		if (body->JitterCount > body->JitterCountMax) {
			body->IsJittering = TRUE;
		}
	} else if (body->IsJittering) {
		if (body->JitterFrames > body->JitterFramesMax) {
			body->IsJittering = FALSE;
		}
	}
#endif

	// Count time that body has been out of contact with the world
	if (body->NWorldContacts == 0) {
		body->NoContactTime += dt;
	} else {
		body->NoContactTime = ZERO;
	}

	// Zero the impulse for next time
	SetVecZero(&body->AngImpulse);
}


/////////////////////////////////////////////////////////////////////
//
// BodyPointVel: calculate the absolute velocity of a point on a body
//
/////////////////////////////////////////////////////////////////////

void BodyPointVel(NEWBODY *body, VEC *dR, VEC *vel)
{
	VecCrossVec(&body->AngVel, dR, vel);
	VecPlusEqVec(vel, &body->Centre.Vel);
}


/////////////////////////////////////////////////////////////////////
//
// SetupMassiveBody: set up the body structure for an object of
// infinite mass
//
/////////////////////////////////////////////////////////////////////

void SetupMassiveBody()
{
	SetVecZero(&BDY_MassiveBody.Centre.Pos);
	SetVecZero(&BDY_MassiveBody.Centre.OldPos);
	SetVecZero(&BDY_MassiveBody.Centre.Vel);
	SetVecZero(&BDY_MassiveBody.Centre.OldPos);
	SetVecZero(&BDY_MassiveBody.Centre.Acc);
	SetVecZero(&BDY_MassiveBody.Centre.Impulse);
	SetQuatUnit(&BDY_MassiveBody.Centre.Quat);
	SetQuatUnit(&BDY_MassiveBody.Centre.OldQuat);
	SetMatUnit(&BDY_MassiveBody.Centre.WMatrix);
	SetMatUnit(&BDY_MassiveBody.Centre.OldWMatrix);
	SetVecZero(&BDY_MassiveBody.Centre.Shift);

	BDY_MassiveBody.Centre.Mass = ZERO;
	BDY_MassiveBody.Centre.InvMass = ZERO;
	SetMatZero(&BDY_MassiveBody.BodyInertia);
	SetMatZero(&BDY_MassiveBody.BodyInvInertia);
	SetMatZero(&BDY_MassiveBody.WorldInvInertia);

	BDY_MassiveBody.Centre.StaticFriction = ONE;
	BDY_MassiveBody.Centre.KineticFriction = ONE;
	BDY_MassiveBody.Centre.Hardness = ZERO;
	BDY_MassiveBody.Centre.Resistance = ZERO;
	BDY_MassiveBody.Centre.Gravity = ZERO;

	SetVecZero(&BDY_MassiveBody.AngVel);
	SetVecZero(&BDY_MassiveBody.AngAcc);
	SetVecZero(&BDY_MassiveBody.AngImpulse);
	
	BDY_MassiveBody.CollSkin.Convex = NULL;
	BDY_MassiveBody.CollSkin.WorldConvex = NULL;
	BDY_MassiveBody.CollSkin.OldWorldConvex = NULL;
	BDY_MassiveBody.CollSkin.Sphere = NULL;
	BDY_MassiveBody.CollSkin.WorldSphere = NULL;
	BDY_MassiveBody.CollSkin.OldWorldSphere = NULL;
	BDY_MassiveBody.CollSkin.NConvex = 0;
	BDY_MassiveBody.CollSkin.NSpheres = 0;
}


/////////////////////////////////////////////////////////////////////
//
// SetBodyInertia: set up the (diagonal) inertia and inverse 
// inertia matrices
//
/////////////////////////////////////////////////////////////////////

void SetBodyInertia(NEWBODY *body, MAT *inertia)
{
	CopyMat(inertia, &body->BodyInertia);
	CopyMat(inertia, &body->BodyInvInertia);
	InvertMat(&body->BodyInvInertia);
}

/////////////////////////////////////////////////////////////////////
//
// GetFrameInvInertia: convert inertia matrix from body frame to
// world frame
//
/////////////////////////////////////////////////////////////////////

void GetFrameInertia(MAT *bodyInvInertia, MAT *transform, MAT *worldInvInertia)
{
	MAT tmpMat;

	TransMatMulMat(transform, bodyInvInertia, &tmpMat);
	MatMulMat(&tmpMat, transform, worldInvInertia);

}

/////////////////////////////////////////////////////////////////////
//
// BuildOneBodyColMat: build the collision matrix for a collision
// between a dynamic body and an immovable object.
//
// Inputs:
//		body			- the dynamic body of the collision
//		colPos			- the position on the body
//		colPos2			- the point where the collision is/was applied
// Outputs:
//		colMat			- the collision matrix;
//
/////////////////////////////////////////////////////////////////////

void BuildOneBodyColMat(NEWBODY *body, VEC *colPos, VEC *colPos2, MAT *colMat)
{

	MAT workMat;

	VecCrossMat(colPos, &body->WorldInvInertia, &workMat);
	MatCrossVec(&workMat, colPos2, colMat);

	colMat->m[XX] = body->Centre.InvMass - colMat->m[XX];
	colMat->m[XY] = -colMat->m[XY];
	colMat->m[XZ] = -colMat->m[XZ];

	colMat->m[YX] = -colMat->m[YX];
	colMat->m[YY] = body->Centre.InvMass - colMat->m[YY];
	colMat->m[YZ] = -colMat->m[YZ];

	colMat->m[ZX] = -colMat->m[ZX];
	colMat->m[ZY] = -colMat->m[ZY];
	colMat->m[ZZ] = body->Centre.InvMass - colMat->m[ZZ];

}

/////////////////////////////////////////////////////////////////////
//
// BuildTwoBodyColMat: build the collision matrix for a collision
// between two rigid bodies
//
// Inputs:
//		body			- the dynamic body of the collision
//		colPos			- the position on the body where the collision occurred
// Outputs:
//		colMat			- the collision matrix;
//		colMatInv		- the inverse collision matrix
//
/////////////////////////////////////////////////////////////////////

void BuildTwoBodyColMat(NEWBODY *body1, NEWBODY *body2, VEC *colPos1, VEC *relPos1, VEC *colPos2, VEC *relPos2, MAT *colMat)
{
	MAT workMat1, workMat2;

	// Angular part
	VecCrossMat(relPos1, &body1->WorldInvInertia, &workMat1);
	MatCrossVec(&workMat1, colPos1, colMat);
	VecCrossMat(relPos2, &body2->WorldInvInertia, &workMat1);
	MatCrossVec(&workMat1, colPos2, &workMat2);
	MatPlusEqMat(colMat, &workMat2);

	// Linear part
	colMat->m[XX] = body1->Centre.InvMass + body2->Centre.InvMass - colMat->m[XX];
	colMat->m[XY] = -colMat->m[XY];
	colMat->m[XZ] = -colMat->m[XZ];

	colMat->m[YX] = -colMat->m[YX];
	colMat->m[YY] = body1->Centre.InvMass + body2->Centre.InvMass  - colMat->m[YY];
	colMat->m[YZ] = -colMat->m[YZ];

	colMat->m[ZX] = -colMat->m[ZX];
	colMat->m[ZY] = -colMat->m[ZY];
	colMat->m[ZZ] = body1->Centre.InvMass + body2->Centre.InvMass  - colMat->m[ZZ];
}


/////////////////////////////////////////////////////////////////////
//
// OneBodyZeroFrictionImpulse: calculate the zero-friction collision
// impulse on a body
//
/////////////////////////////////////////////////////////////////////

REAL OneBodyZeroFrictionImpulse(NEWBODY *body, 
								VEC *pos, 
								VEC *normal,
								REAL deltaVel)
{
	REAL	impMag;
	VEC	tmpVec1, tmpVec2;

	VecCrossVec(pos, normal, &tmpVec1);
	MatMulVec(&body->WorldInvInertia, &tmpVec1, &tmpVec2);
	VecCrossVec(&tmpVec2, pos, &tmpVec1);
	impMag = body->Centre.InvMass + VecDotVec(&tmpVec1, normal);

	return DivScalar(deltaVel, impMag);

}

void TwoBodyZeroFrictionImpulse(NEWBODY *body1, NEWBODY *body2,
								VEC *pos1, VEC *pos2, 
								VEC *normal,
								REAL deltaVel, 
								VEC *impulse)
{
	REAL	impMag;
	VEC	tmpVec1, tmpVec2;

	VecCrossVec(pos1, normal, &tmpVec1);
	MatMulVec(&body1->WorldInvInertia, &tmpVec1, &tmpVec2);
	VecCrossVec(&tmpVec2, pos1, &tmpVec1);
	impMag = body1->Centre.InvMass + VecDotVec(&tmpVec1, normal);

	VecCrossVec(pos2, normal, &tmpVec1);
	MatMulVec(&body2->WorldInvInertia, &tmpVec1, &tmpVec2);
	VecCrossVec(&tmpVec2, pos2, &tmpVec1);
	impMag += body2->Centre.InvMass + VecDotVec(&tmpVec1, normal);

	impMag = DivScalar(deltaVel, impMag);

	VecEqScalarVec(impulse, impMag, normal);

}


/////////////////////////////////////////////////////////////////////
//
// PreprocessBodyColls: remove or average any duplicates
// collisions
//
/////////////////////////////////////////////////////////////////////

void PreProcessBodyColls(NEWBODY *body)
{
	bool keepGoing;
	VEC dR, dRW;
	REAL	n1Dotn2, shiftMod;
	REAL	dRLenSq, dRWLenSq;
	VEC	*collPos1, *collPos2;
	REAL	collDepth1;
	COLLINFO_BODY	*collInfo1, *collInfo2;


	for (collInfo1 = body->BodyCollHead; collInfo1 != NULL; collInfo1 = collInfo1->Next) {

		collPos1 = &collInfo1->Pos1;
		collDepth1 = collInfo1->Depth;

		if (VecDotVec(&body->Centre.Shift, PlaneNormal(&collInfo1->Plane)) > -collDepth1) {
			RemoveBodyColl(body, collInfo1);
			continue;
		}
	}

	collInfo1 = body->BodyCollHead;
	while (collInfo1 != NULL) {

		collPos1 = &collInfo1->Pos1;
		collDepth1 = collInfo1->Depth;

		// Make sure the body is extracted from colliding object
		if (collInfo1->Depth < ZERO) {

			shiftMod = collInfo1->Body1->Centre.InvMass + collInfo1->Body2->Centre.InvMass;
			if (shiftMod > SMALL_REAL) {
				shiftMod = collInfo1->Body1->Centre.InvMass / shiftMod;
			} else {
				shiftMod = ZERO;
			}
			ModifyShift(&body->Centre.Shift, -shiftMod * collDepth1, PlaneNormal(&collInfo1->Plane));
		}

		// Merge close collisions
		collInfo2 = collInfo1->Next;
		keepGoing = TRUE;
		while ((collInfo2 != NULL) && keepGoing) {

			collPos2 = &collInfo2->Pos1;

			VecMinusVec(collPos1, collPos2, &dR);
			dRLenSq = VecDotVec(&dR, &dR);
			VecMinusVec(&collInfo1->WorldPos, &collInfo2->WorldPos, &dRW);
			dRWLenSq = VecDotVec(&dRW, &dRW);
			n1Dotn2 = VecDotVec(PlaneNormal(&collInfo1->Plane), PlaneNormal(&collInfo2->Plane));
			
			if (dRLenSq < CLOSE_BODY_COLL) {
				if (collInfo1->Depth < collInfo2->Depth) {
					RemoveBodyColl(body, collInfo2);
				} else {
					RemoveBodyColl(body, collInfo1);
					keepGoing = FALSE;
				}
				COL_NBodyDone++;
			}
			collInfo2 = collInfo2->Next;
		}
		collInfo1 = collInfo1->Next;
	}

	// Set up the collision pointer list for this body
	//COL_NThisBodyColls = BuildThisBodyCollList(body);
}


/////////////////////////////////////////////////////////////////////
//
// BuildThisBodyCollList: Fill the COL_ThisBodyColl array with
// pointers to the active collisions on the passed body.
//
/////////////////////////////////////////////////////////////////////

/*int BuildThisBodyCollList(NEWBODY *body)
{
	int iColl1, collCount;

	body->NWorldContacts = 0;
	body->NOtherContacts = 0;
	collCount = 0;

	for (iColl1 = 0; iColl1 < COL_NBodyColls; iColl1++) {
		if ((body != COL_BodyColl[iColl1].Body1)) continue;
		if (COL_BodyColl[iColl1].Ignore) continue;

		// Make sure there is enough space in the collision equation array
		if (collCount == MAX_COLLS_PER_BODY) return collCount;

		// Store pointer
		COL_ThisBodyColl[collCount++] = &COL_BodyColl[iColl1];

		if (COL_BodyColl[iColl1].Body2 == &BDY_MassiveBody) {
			body->NWorldContacts++;
		} else {
			body->NOtherContacts++;
		}

	}

	return collCount;
}*/


/////////////////////////////////////////////////////////////////////
//
// BuildCollisionEquations: set up the simultaneous equations to
// get the impulses on the passed body
//
/////////////////////////////////////////////////////////////////////
#ifndef _PSX
void BuildCollisionEquations(NEWBODY *body, COLLINFO_BODY **collInfoList, int nColls, BIGMAT *eqns, BIGVEC *residual) 
{
	int		iColl, iColl2, iEqn, iImp, iImp2;
	int		nEqns;
	REAL	dVelNorm;
	VEC	tmpVec, tmpVec2;
	MAT	collAxes, collMat;
	COLLINFO_BODY	*collInfo, *collInfo2;

	// Number of simulataneous equations
	nEqns = 3 * nColls;

	// Initialise the equation matrix
	SetBigMatSize(eqns, nEqns, nEqns);
	SetBigVecSize(residual, nEqns);
	ClearBigMat(eqns);

	// Set up the three equations for each contact point (currently zero friction)
	iEqn = 0;
	for (iColl = 0; iColl < nColls; iColl++) {
		iImp = 3 * iColl;
		collInfo = collInfoList[iColl];

		// build collision axes (up = normal; look, right = tangents)
		CopyVec(PlaneNormal(&collInfo->Plane), &collAxes.mv[U]);
		BuildMatrixFromUp(&collAxes);

		// P_i.T_1i = 0
		eqns->m[iEqn][X + iImp] = collAxes.m[RX];
		eqns->m[iEqn][Y + iImp] = collAxes.m[RY];
		eqns->m[iEqn][Z + iImp] = collAxes.m[RZ];
		residual->v[iEqn] = ZERO;
		iEqn++;

		// P_i.T_2i = 0
		eqns->m[iEqn][X + iImp] = collAxes.m[LX];
		eqns->m[iEqn][Y + iImp] = collAxes.m[LY];
		eqns->m[iEqn][Z + iImp] = collAxes.m[LZ];
		residual->v[iEqn] = ZERO;
		iEqn++;

		// dV_i.n_i = -(1+e)V.n_i
		for (iColl2 = 0; iColl2 < nColls; iColl2++) {
			iImp2 = 3 * iColl2;
			collInfo2 = collInfoList[iColl2];
			BuildOneBodyColMat(body, &collInfo->Pos1, &collInfo2->Pos1, &collMat);

			eqns->m[iEqn][X + iImp2] = collMat.m[XX] * collAxes.m[UX] + collMat.m[YX] * collAxes.m[UY] + collMat.m[ZX] * collAxes.m[UZ];
			eqns->m[iEqn][Y + iImp2] = collMat.m[XY] * collAxes.m[UX] + collMat.m[YY] * collAxes.m[UY] + collMat.m[ZY] * collAxes.m[UZ];
			eqns->m[iEqn][Z + iImp2] = collMat.m[XZ] * collAxes.m[UX] + collMat.m[YZ] * collAxes.m[UY] + collMat.m[ZZ] * collAxes.m[UZ];
		}
		residual->v[iEqn] = -(ONE + collInfo->Restitution) * VecDotVec(&collInfo->Vel, &collAxes.mv[U]);
		// Make sure the existing impulses are taken into account
		VecCrossVec(&body->AngImpulse, &collInfo->Pos1, &tmpVec);
		MatMulVec(&body->WorldInvInertia, &tmpVec, &tmpVec2);
		VecPlusEqScalarVec(&tmpVec2, body->Centre.InvMass, &body->Centre.Impulse);
		dVelNorm = VecDotVec(PlaneNormal(&collInfo->Plane), &tmpVec2);
		if (dVelNorm < residual->v[iEqn]) {
			residual->v[iEqn] -= dVelNorm;
		} else {
			residual->v[iEqn] = ZERO;
		}
		iEqn++;
	}
}
#endif

#ifndef _PSX
void BuildCollisionEquations2(NEWBODY *body, COLLINFO_BODY **collInfoList, int nColls, BIGMAT *eqns, BIGVEC *residual) 
{
	int		iColl, iColl2, iEqn, iImp, iImp2;
	int		nEqns;
	MAT	collMat;
	COLLINFO_BODY	*collInfo, *collInfo2;

	// Number of simulataneous equations
	nEqns = 3 * nColls;

	// Initialise the equation matrix
	SetBigMatSize(eqns, nEqns, nEqns);
	SetBigVecSize(residual, nEqns);
	ClearBigMat(eqns);

	// Set up the three equations for each contact point (currently zero friction)
	iEqn = 0;
	for (iColl = 0; iColl < nColls; iColl++) {
		iImp = 3 * iColl;
		iEqn = 3 * iColl;
		collInfo = collInfoList[iColl];

		// dV_i = -(1+e)V
		for (iColl2 = 0; iColl2 < nColls; iColl2++) {
			iImp2 = 3 * iColl2;
			collInfo2 = collInfoList[iColl2];
			BuildOneBodyColMat(body, &collInfo->Pos1, &collInfo2->Pos1, &collMat);

			eqns->m[iEqn][0 + iImp2] = collMat.m[XX];
			eqns->m[iEqn][1 + iImp2] = collMat.m[XY];
			eqns->m[iEqn][2 + iImp2] = collMat.m[XZ];
			residual->v[iEqn] = -(ONE + collInfo->Restitution) * collInfo->Vel.v[X];

			eqns->m[iEqn + 1][0 + iImp2] = collMat.m[YX];
			eqns->m[iEqn + 1][1 + iImp2] = collMat.m[YY];
			eqns->m[iEqn + 1][2 + iImp2] = collMat.m[YZ];
			residual->v[iEqn + 1] = -(ONE + collInfo->Restitution) * collInfo->Vel.v[Y];

			eqns->m[iEqn + 2][0 + iImp2] = collMat.m[ZX];
			eqns->m[iEqn + 2][1 + iImp2] = collMat.m[ZY];
			eqns->m[iEqn + 2][2 + iImp2] = collMat.m[ZZ];
			residual->v[iEqn + 2] = -(ONE + collInfo->Restitution) * collInfo->Vel.v[Z];
		}
	}
}
#endif

void BuildCollisionEquations3(NEWBODY *body, BIGMAT *eqns, BIGVEC *residual) 
{
	int		iColl, iColl2, iEqn;
	REAL	dVelNorm, tReal;
	VEC	tmpVec, tmpVec2;
	MAT	collMat;
	COLLINFO_BODY	*collInfo, *collInfo2;
	int nColls = body->NBodyColls;

	// Initialise the equation matrix
	SetBigMatSize(eqns, nColls, nColls);
	SetBigVecSize(residual, nColls);
	ClearBigMat(eqns);

	iEqn = 0;
	collInfo = body->BodyCollHead;
	for (iColl = 0; iColl < nColls; iColl++) {
		Assert(collInfo != NULL);

		// dV_i.n_i = -(1+e)V.n_i
		collInfo2 = body->BodyCollHead;
		for (iColl2 = 0; iColl2 < nColls; iColl2++) {
			Assert(collInfo2 != NULL);

			if (iColl2 == iColl) {
				BuildTwoBodyColMat(collInfo->Body1, collInfo->Body2, &collInfo->Pos1, &collInfo2->Pos1, &collInfo->Pos2, &collInfo2->Pos2, &collMat);
			} else {
				if (body == collInfo->Body1) {
					BuildOneBodyColMat(collInfo->Body1, &collInfo->Pos1, &collInfo2->Pos1, &collMat);
				} else {
					BuildOneBodyColMat(collInfo->Body2, &collInfo->Pos2, &collInfo2->Pos2, &collMat);
				}
			}
			tReal = 
				MulScalar(collMat.m[XX], collInfo2->Plane.v[X]) + 
				MulScalar(collMat.m[XY], collInfo2->Plane.v[Y]) + 
				MulScalar(collMat.m[XZ], collInfo2->Plane.v[Z]);
			eqns->m[iEqn][iColl2] = MulScalar(collInfo->Plane.v[X], tReal);
			tReal = 
				MulScalar(collMat.m[YX], collInfo2->Plane.v[X]) + 
				MulScalar(collMat.m[YY], collInfo2->Plane.v[Y]) + 
				MulScalar(collMat.m[YZ], collInfo2->Plane.v[Z]);
			eqns->m[iEqn][iColl2] += MulScalar(collInfo->Plane.v[Y], tReal);
			tReal = 
				MulScalar(collMat.m[ZX], collInfo2->Plane.v[X]) + 
				MulScalar(collMat.m[ZY], collInfo2->Plane.v[Y]) + 
				MulScalar(collMat.m[ZZ], collInfo2->Plane.v[Z]);
			eqns->m[iEqn][iColl2] += MulScalar(collInfo->Plane.v[Z], tReal);

			collInfo2 = collInfo2->Next;
		}
		residual->v[iEqn] = -MulScalar((ONE + collInfo->Restitution), VecDotVec(&collInfo->Vel, PlaneNormal(&collInfo->Plane)));
		// Make sure the existing impulses are taken into account
		VecCrossVec(&body->AngImpulse, &collInfo->Pos1, &tmpVec);
		MatMulVec(&body->WorldInvInertia, &tmpVec, &tmpVec2);
		VecPlusEqScalarVec(&tmpVec2, body->Centre.InvMass, &body->Centre.Impulse);
		dVelNorm = VecDotVec(PlaneNormal(&collInfo->Plane), &tmpVec2);
		if (dVelNorm < residual->v[iEqn]) {
			residual->v[iEqn] -= dVelNorm;
		} else {
			residual->v[iEqn] = ZERO;
		}
		iEqn++;

		collInfo = collInfo->Next;
	}
}



/////////////////////////////////////////////////////////////////////
//
// ProcessOneBodyColls3: process collisions simultaneously
//
/////////////////////////////////////////////////////////////////////

static BIGVEC	Soln;
static BIGMAT	Coefficients;
static BIGVEC	Residual;

#if DEBUG_SOLVER
static BIGMAT	OrigCoefs;
static BIGVEC	OrigRes, NewRes;
#endif

#if USE_DEBUG_ROUTINES
int DEBUG_NIts = 0;
bool DEBUG_Converged = TRUE;
REAL DEBUG_Res = ZERO;
#endif



void ProcessBodyColls3(NEWBODY *body) 
{
	int iColl, nIts, maxIts;
	REAL res, tol, knock;
	VEC	tmpVec, collImpulse;
	VEC	totImpulse = {ZERO, ZERO, ZERO};
	VEC	totAngImpulse = {ZERO, ZERO, ZERO};
	COLLINFO_BODY *collInfo;
#if DEBUG_SOLVER && FALSE
	char buf[256];
#endif

	// Set up the set of simulatneous equations
	BuildCollisionEquations3(body, &Coefficients, &Residual);

	// Set up the solution vector
	SetBigVecSize(&Soln, body->NBodyColls);

#if DEBUG_SOLVER
	// Save original equations for debugging
	CopyBigMat(&Coefficients, &OrigCoefs);
	CopyBigVec(&Residual, &OrigRes);
	SetBigVecSize(&NewRes, body->NBodyColls);
#endif

	// Solve the collision equations
	maxIts = 5;
	tol = Real(0.001);
	if (body->NBodyColls > 1) {
		ConjGrad(&Coefficients, &Residual, tol, maxIts, &Soln, &res, &nIts);
	} else {
		Soln.v[0] = DivScalar(Residual.v[0], Coefficients.m[0][0]);
		res = ZERO;
	}

#if USE_DEBUG_ROUTINES
	DEBUG_NIts = nIts;
	DEBUG_Converged = TRUE;
#endif

	// If not converged, do not apply impulses
	if (abs(res) > 1000 * tol) {

#if DEBUG_SOLVER && defined(wsprintf) && (FALSE)
	// Check that the residual is less than the tolerance
		int ii, jj;
		wsprintf(buf, "\nCollision equations not converged\n");
		WriteLogEntry(buf);
		wsprintf(buf, "res = %d; tol = %d; its = %d\n\n", (int)(1.0e6 * res), (int)(1.0e6 * tol), nIts);
		WriteLogEntry(buf);
		for (ii = 0; ii < NRows(&OrigCoefs); ii++) {
			WriteLogEntry("| ");
			for (jj = 0; jj < NRows(&OrigCoefs); jj++) {
				wsprintf(buf, "%8d ", (int)(1.0e6 * OrigCoefs.m[ii][jj]));
				WriteLogEntry(buf);
			}
			wsprintf(buf, "|   | %8d | %s | %8d |\n",
				(int)(1.0e6 * Soln.v[ii]), 
				(ii == NRows(&OrigCoefs) / 2)? " = ": "   ",
				(int)(1.0e6 * OrigRes.v[ii]));
			WriteLogEntry(buf);
		}
		//TellChris = TRUE;
#endif
#if USE_DEBUG_ROUTINES
	// A few checks
		DEBUG_Converged = FALSE;
		DEBUG_Res = res;
#endif
		return;

	} else {

#if USE_DEBUG_ROUTINES
		DEBUG_Converged = TRUE;
#endif

	}

	// Add up the linear and angular components of the impulse
	NThisBodySparks = 0;
	NThisBodySmoke = 0;
	body->Banged = TRUE;
	
	collInfo = body->BodyCollHead;
	for (iColl = 0; iColl < body->NBodyColls; iColl++) {
		Assert(collInfo != NULL);

		// Frictionless impulse
		if (body == collInfo->Body1) {
			VecEqScalarVec(&collImpulse, Soln.v[iColl], PlaneNormal(&collInfo->Plane));
		} else {
			VecEqScalarVec(&collImpulse, -Soln.v[iColl], PlaneNormal(&collInfo->Plane));
		}

		// Add friction
		AddBodyFriction(body, &collImpulse, collInfo);

		// Store
		VecPlusEqVec(&totImpulse, &collImpulse);
		CalcAngImpulse(&collImpulse, &collInfo->Pos1, &tmpVec);
		VecPlusEqVec(&totAngImpulse, &tmpVec);

		// Check for hard knocks
		knock = MulScalar(abs(Soln.v[iColl]), body->Centre.InvMass);
		if (knock > body->BangMag) {
			body->BangMag = knock;
			CopyPlane(&collInfo->Plane, &body->BangPlane);
		}

		collInfo = collInfo->Next;
	}

	// Apply the impulses to the body
	ApplyBodyAngImpulse(body, &totAngImpulse);
	ApplyParticleImpulse(&body->Centre, &totImpulse);

}


/////////////////////////////////////////////////////////////////////
//
// AddBodyFriction: add friction to a previously calculated normal 
// impulse
//
/////////////////////////////////////////////////////////////////////

void AddBodyFriction(NEWBODY *body, VEC *impulse, COLLINFO_BODY *collInfo)
{
	REAL	impDotNorm, velDotNorm, velTanLen, impTanLen, impTanMax, impTanMod;
	VEC	impTan, velTan, sparkVel;
	
	impDotNorm = VecDotVec(impulse, PlaneNormal(&collInfo->Plane));
	if (impDotNorm < ZERO) {
		return;
	}
	velDotNorm = VecDotVec(&collInfo->Vel, PlaneNormal(&collInfo->Plane));
	VecPlusScalarVec(&collInfo->Vel, -velDotNorm, PlaneNormal(&collInfo->Plane), &velTan);

	// Calculate sliding velocity
	velTanLen = Length(&velTan);

	// Create a spark
#ifndef _PSX
	if ((collInfo->Material != NULL) && 
		(velTanLen > MIN_SPARK_VEL) &&
		BodyAllowsSparks(body)) 
	{
		body->ScrapeMaterial = collInfo->Material - COL_MaterialInfo;	// Material number
		body->LastScrapeTime = ZERO;

		if (MaterialAllowsSparks(collInfo->Material) && 
			(NThisBodySparks < MAX_SPARKS_PER_BODY) && 
			(frand(ONE) < SparkProbability(velTanLen)))
		{
			NThisBodySparks++;
#ifdef _PC
			VecEqScalarVec(&sparkVel, -HALF, &velTan);
#endif
#ifdef _N64
			VecEqScalarVec(&sparkVel, -0.1, &velTan);
#endif
			CreateSpark(SPARK_SPARK, &collInfo->WorldPos, &sparkVel, HALF * velTanLen, 0);
		}
	}
#endif

	// Turn sliding velocity into tangential impulse
	//impTanLen = -wheel->Grip * impDotNorm;
	impTanLen = MulScalar(body->Centre.Grip, impDotNorm);
	CopyVec(&velTan, &impTan);
	VecMulScalar(&impTan, -impTanLen);
	impTanLen = MulScalar(impTanLen, velTanLen);

	// Scale sliding to friction cone
	impTanMax = MulScalar(MulScalar(HALF, TimeStep), MulScalar(body->Centre.Mass, body->Centre.Gravity));
	if (impTanLen > impTanMax) {
		impTanMod = DivScalar(impTanMax, impTanLen);
		//VecDivScalar(&impTan, impTanLen / (TimeStep * collInfo->KineticFriction * body->Centre.Mass * body->Centre.Gravity));
		VecMulScalar(&impTan, impTanMod);
		//impTanLen = Length(&impTan);
		impTanLen = impTanMax;

	}
	impTanMax = MulScalar(collInfo->StaticFriction, impDotNorm);
	if (impTanLen > impTanMax) {
		impTanMod = DivScalar(MulScalar(collInfo->KineticFriction, impDotNorm), impTanLen);
		//VecDivScalar(&impTan, impTanLen / (collInfo->KineticFriction * impDotNorm))
		VecMulScalar(&impTan, impTanMod);
	}

	VecPlusEqVec(impulse, &impTan);
}



/////////////////////////////////////////////////////////////////////
//
// BodyWorldColl: detect all collisions between passed body and
// world polys
//
/////////////////////////////////////////////////////////////////////

void DetectBodyWorldColls(NEWBODY *body)
{
	long	iPoly;
	long	nCollPolys;
	COLLGRID *collGrid;

#ifndef _PSX
	NEWCOLLPOLY **collPolyPtr;
#endif
	NEWCOLLPOLY *collPoly;

	// Calculate the grid position and which polys to check against
	collGrid = PosToCollGrid(&body->Centre.Pos);
	if (collGrid == NULL) return;

#ifndef _PSX
	collPolyPtr = collGrid->CollPolyPtr;
#endif
	nCollPolys = collGrid->NCollPolys;

	for (iPoly = 0; iPoly < nCollPolys; iPoly++) {

#ifndef _PSX
		collPoly = collPolyPtr[iPoly];
#else
		collPoly = &COL_WorldCollPoly[collGrid->CollPolyIndices[iPoly]];
#endif

		if (PolyCameraOnly(collPoly)) continue;

		// BODY - WORLD
		DetectBodyPolyColls(body, collPoly);

	}
}

/////////////////////////////////////////////////////////////////////
//
// DetectBodyPolyColls: detect collision between a rigid body and
// the passed polygon of a collision mesh
//
/////////////////////////////////////////////////////////////////////

void DetectBodyPolyColls(NEWBODY *body, NEWCOLLPOLY *collPoly)
{
	DetectConvexHullPolyColls(body, collPoly);
}

#ifdef _PSX
extern int NPassed;
#endif
int DetectConvexHullPolyColls(NEWBODY *body, NEWCOLLPOLY *collPoly)
{
	int		iSphere;
	VERTEX	*oldPos, *newPos;
	COLLINFO_BODY *collInfo;
	int nColls = 0;

	// Quick Bounding-box check
	if (!BBTestYXZ(&collPoly->BBox, &body->CollSkin.BBox)) return 0;

#ifdef _PSX
	NPassed++;
#endif

	// Loop over collision spheres
	for (iSphere = 0; iSphere < body->CollSkin.NSpheres; iSphere++) {

		// allocate the collision info storage
		if ((collInfo = NextBodyCollInfo()) == NULL) return nColls;

		oldPos = &body->CollSkin.OldWorldSphere[iSphere].Pos;
		newPos = &body->CollSkin.WorldSphere[iSphere].Pos;

		// Actual collision test against poly
		if (!SphereCollPoly(oldPos, newPos, body->CollSkin.WorldSphere[iSphere].Radius, collPoly, &collInfo->Plane, &collInfo->Pos1, &collInfo->WorldPos, &collInfo->Depth, &collInfo->Time)) {
			continue;
		}
		VecPlusEqVec(&collInfo->Pos1, newPos);
		VecMinusEqVec(&collInfo->Pos1, &body->Centre.Pos);

		// Check velocity is not directed away from face
		VecCrossVec(&body->AngVel, &collInfo->Pos1, &collInfo->Vel);
		VecPlusEqVec(&collInfo->Vel, &body->Centre.Vel);
		if (VecDotVec(&collInfo->Vel, PlaneNormal(&collPoly->Plane)) >= ZERO) {
			continue;
		}

		// COLLISION OCCURRED

		collInfo->Body1 = body;
		collInfo->Body2 = &BDY_MassiveBody;
		SetVecZero(&collInfo->Pos2);
		CopyPlane(&collPoly->Plane, &collInfo->Plane);
		collInfo->Grip = MulScalar(body->Centre.Grip, COL_MaterialInfo[collPoly->Material].Gripiness);
		collInfo->StaticFriction = MulScalar(body->Centre.StaticFriction, COL_MaterialInfo[collPoly->Material].Roughness);
		collInfo->KineticFriction = MulScalar(body->Centre.KineticFriction, COL_MaterialInfo[collPoly->Material].Roughness);
		collInfo->Restitution = MulScalar(body->Centre.Hardness, COL_MaterialInfo[collPoly->Material].Hardness);
		collInfo->Material = &COL_MaterialInfo[collPoly->Material];
		if (abs(VecDotVec(PlaneNormal(&collInfo->Plane), &DownVec)) < 0.15f) {
			collInfo->StaticFriction = MulScalar(Real(0.1), collInfo->StaticFriction);
			collInfo->KineticFriction = MulScalar(Real(0.1), collInfo->KineticFriction);
			collInfo->Restitution = collInfo->Restitution + 0.1f;
		}

		// Adjust collision info according to the material
		AdjustBodyColl(collInfo, collInfo->Material);

		AddBodyColl(body, collInfo);
		nColls++;

	}

	return nColls;
}



/////////////////////////////////////////////////////////////////////
//
// DetectBodyBodyColls: Detect collisions between two rigid bodies
//
/////////////////////////////////////////////////////////////////////

int DetectBodyBodyColls(NEWBODY *body1, NEWBODY *body2)
{

	if (IsBodyConvex(body1)) {
		if (IsBodyPoly(body2)) {

			return DetectHullPolyColls(body1, body2);

		} else if (IsBodySphere(body2)) {

			return DetectHullHullColls(body2, body1);

		} else if (IsBodyConvex(body2)) {

			return DetectHullHullColls(body1, body2);

		}
	}
	else if (IsBodySphere(body1)) {
		if (IsBodyPoly(body2)) {

			return DetectHullPolyColls(body1, body2);

		} else if (IsBodySphere(body2)) {

			return DetectSphereSphereColls(body1, body2);

		} else if (IsBodyConvex(body2)) {

			return DetectHullHullColls(body1, body2);

		}
	}
	else if (IsBodyPoly(body1)) {
		if (IsBodyPoly(body2)) {

			return 0;
			
		} else if (IsBodySphere(body2)) {

			return DetectHullPolyColls(body2, body1);

		} else if (IsBodyConvex(body2)) {

			return DetectHullPolyColls(body2, body1);

		}
	}

	return 0;

}

int DetectHullPolyColls(NEWBODY *body1, NEWBODY *body2)
{
	int		iSphere, iPoly;
	VERTEX	*oldPos, *newPos;
	COLLINFO_BODY *collInfo;
	NEWCOLLPOLY *collPoly;
	int nColls = 0;

	Assert(IsBodyPoly(body2) && !IsBodyPoly(body1));

	if (!BBTestYXZ(&body1->CollSkin.BBox, &body2->CollSkin.BBox)) return 0;

	for (iPoly = 0; iPoly < body2->CollSkin.NCollPolys; iPoly++) {
		collPoly = &body2->CollSkin.CollPoly[iPoly];
		
		// Quick Bounding-box check
		if (!BBTestYXZ(&collPoly->BBox, &body1->CollSkin.BBox)) continue;

		// Loop over collision spheres
		for (iSphere = 0; iSphere < body1->CollSkin.NSpheres; iSphere++) {

			if ((collInfo = NextBodyCollInfo()) == NULL) return nColls;

			oldPos = &body1->CollSkin.OldWorldSphere[iSphere].Pos;
			newPos = &body1->CollSkin.WorldSphere[iSphere].Pos;

			// Actual collision test against poly
			if (!SphereCollPoly(oldPos, newPos, body1->CollSkin.WorldSphere[iSphere].Radius, collPoly, &collInfo->Plane, &collInfo->Pos1, &collInfo->WorldPos, &collInfo->Depth, &collInfo->Time)) {
				continue;
			}
			VecPlusEqVec(&collInfo->Pos1, newPos);
			VecMinusEqVec(&collInfo->Pos1, &body1->Centre.Pos);

			// Check velocity is not directed away from face
			VecCrossVec(&body1->AngVel, &collInfo->Pos1, &collInfo->Vel);
			VecPlusEqVec(&collInfo->Vel, &body1->Centre.Vel);
			VecMinusEqVec(&collInfo->Vel, &body2->Centre.Vel);
			if (VecDotVec(&collInfo->Vel, PlaneNormal(&collPoly->Plane)) >= ZERO) {
				continue;
			}

			// COLLISION OCCURRED

			collInfo->Body1 = body1;
			collInfo->Body2 = &BDY_MassiveBody;
			SetVecZero(&collInfo->Pos2);
			CopyPlane(&collPoly->Plane, &collInfo->Plane);
			collInfo->Grip = MulScalar(body1->Centre.Grip, COL_MaterialInfo[collPoly->Material].Gripiness);
			collInfo->StaticFriction = MulScalar(body1->Centre.StaticFriction, COL_MaterialInfo[collPoly->Material].Roughness);
			collInfo->KineticFriction = MulScalar(body1->Centre.KineticFriction, COL_MaterialInfo[collPoly->Material].Roughness);
			collInfo->Restitution = MulScalar(body1->Centre.Hardness, COL_MaterialInfo[collPoly->Material].Hardness);
			collInfo->Material = &COL_MaterialInfo[collPoly->Material];
			if (abs(VecDotVec(PlaneNormal(&collInfo->Plane), &DownVec)) < 0.15f) {
				collInfo->StaticFriction = MulScalar(Real(0.1), collInfo->StaticFriction);
				collInfo->KineticFriction = MulScalar(Real(0.1), collInfo->KineticFriction);
			}

			// Adjust collision info according to the material
			AdjustBodyColl(collInfo, collInfo->Material);

			AddBodyColl(body1, collInfo);
			nColls++;

		}

	}

	return nColls;
}

int DetectSphereSphereColls(NEWBODY *body1, NEWBODY *body2)
{
	REAL	dRLen;
	VEC	dR;
	COLLINFO_BODY	*bodyColl1, *bodyColl2;
	SPHERE *sphere1, *sphere2;

	sphere1 = &body1->CollSkin.WorldSphere[0];
	sphere2 = &body2->CollSkin.WorldSphere[0];

	Assert(IsBodySphere(body1));
	Assert(IsBodySphere(body2));

	// Relative position and separation
	VecMinusVec(&sphere1->Pos, &sphere2->Pos, &dR);
	dRLen = VecLen(&dR);

	// Check for collision
	if (dRLen > sphere1->Radius + sphere2->Radius) return 0;

	if ((bodyColl1 = NextBodyCollInfo()) == NULL) return 0;
	AddBodyColl(body1, bodyColl1);
	if ((bodyColl2 = NextBodyCollInfo()) == NULL) {
		RemoveBodyColl(body1, bodyColl1);
		return 0;
	}
	AddBodyColl(body2, bodyColl2);

	bodyColl1->Body1 = body1;
	bodyColl1->Body2 = body2;
	bodyColl2->Body1 = body2;
	bodyColl2->Body2 = body1;

	bodyColl1->Depth = (dRLen - sphere1->Radius - sphere2->Radius) * HALF;
	bodyColl2->Depth = bodyColl1->Depth;

	// Collision plane
	if (dRLen > SMALL_REAL) {
		CopyVec(&dR, PlaneNormal(&bodyColl1->Plane));
		VecDivScalar(PlaneNormal(&bodyColl1->Plane), dRLen);
		bodyColl1->Time = ONE - DivScalar(bodyColl1->Depth, dRLen);
	} else {
		SetVec(PlaneNormal(&bodyColl1->Plane), ONE, ZERO, ZERO);
		bodyColl1->Time = ONE;
	}
	FlipPlane(&bodyColl1->Plane, &bodyColl2->Plane);
	bodyColl2->Time = bodyColl1->Time;

	// collision positions
	VecPlusScalarVec(&sphere1->Pos, sphere1->Radius, PlaneNormal(&bodyColl1->Plane), &bodyColl1->Pos1);
	VecPlusScalarVec(&sphere2->Pos, -sphere2->Radius, PlaneNormal(&bodyColl1->Plane), &bodyColl1->Pos2);
	VecMinusEqVec(&bodyColl1->Pos1, &body1->Centre.Pos);
	VecMinusEqVec(&bodyColl1->Pos2, &body2->Centre.Pos);
	VecPlusScalarVec(&sphere1->Pos, HALF, &dR, &bodyColl1->WorldPos);
	CopyVec(&bodyColl1->Pos2, &bodyColl2->Pos1);
	CopyVec(&bodyColl1->Pos1, &bodyColl2->Pos2);
	CopyVec(&bodyColl1->WorldPos, &bodyColl2->WorldPos);
	
	// collsion velocity
	VecCrossVec(&body1->AngVel, &bodyColl1->Pos1, &bodyColl1->Vel);
	VecPlusEqVec(&bodyColl1->Vel, &body1->Centre.Vel);
	VecCrossVec(&body2->AngVel, &bodyColl2->Pos1, &bodyColl2->Vel);
	VecPlusEqVec(&bodyColl2->Vel, &body2->Centre.Vel);
	VecMinusEqVec(&bodyColl1->Vel, &bodyColl2->Vel);
	CopyVec(&bodyColl1->Vel, &bodyColl2->Vel);
	NegateVec(&bodyColl2->Vel);


	// Other stuff
	bodyColl1->Grip = MulScalar(body1->Centre.Grip, body2->Centre.Grip);
	bodyColl1->StaticFriction = MulScalar(body1->Centre.StaticFriction, body2->Centre.StaticFriction);
	bodyColl1->KineticFriction = MulScalar(body1->Centre.KineticFriction, body2->Centre.KineticFriction);
	bodyColl1->Restitution = MulScalar(body1->Centre.Hardness, body2->Centre.Hardness);
	bodyColl1->Material = NULL;
	
	bodyColl2->Grip = bodyColl1->Grip;
	bodyColl2->StaticFriction = bodyColl1->StaticFriction;
	bodyColl2->KineticFriction = bodyColl1->KineticFriction;
	bodyColl2->Restitution = bodyColl1->Restitution;
	bodyColl2->Material = NULL;

	return 1;

}


int DetectHullHullColls(NEWBODY *body1, NEWBODY *body2) 
{
	int		iSkin, iSphere;
	int		nColls;
	REAL	radius;
	VEC	*sPos, *ePos, dR, vel1, vel2;
	COLLINFO_BODY	*bodyColl1, *bodyColl2;
	
	Assert((body1 != NULL) && (body2 != NULL));
	Assert(IsBodyConvex(body2));
	Assert(IsBodySphere(body1) || IsBodyConvex(body1));

	// Relative position
	VecMinusVec(&body1->Centre.Pos, &body2->Centre.Pos, &dR);

	// Bounding box test
	if (!BBTestXZY(&body2->CollSkin.BBox, &body1->CollSkin.BBox)) return 0;

	nColls = 0;
	// Check for points of body1 in body2
	for (iSphere = 0; iSphere < body1->CollSkin.NSpheres; iSphere++) {

		sPos = &body1->CollSkin.OldWorldSphere[iSphere].Pos;
		ePos = &body1->CollSkin.WorldSphere[iSphere].Pos;
		radius = body1->CollSkin.WorldSphere[iSphere].Radius;

		for (iSkin = 0; iSkin < body2->CollSkin.NConvex; iSkin++) {

			if ((bodyColl1 = NextBodyCollInfo()) == NULL) return nColls;

			// Collision?
			if (!SphereConvex(
					ePos, 
					radius, 
					&body2->CollSkin.WorldConvex[iSkin], 
					&bodyColl1->Pos1, &bodyColl1->Plane, 
					&bodyColl1->Depth)) 
			{
				continue;
			}
			nColls++;


			// Store collision info for sphere
			bodyColl1->Body1 = body1;
			bodyColl1->Body2 = body2;

			// Calculate the relative collision points for response
			VecMinusEqVec(&bodyColl1->Pos1, &body1->Centre.Pos);
			VecPlusScalarVec(&body1->Centre.Pos, -(radius + bodyColl1->Depth), PlaneNormal(&bodyColl1->Plane), &bodyColl1->WorldPos);
			VecMinusVec(&bodyColl1->WorldPos, &body2->Centre.Pos, &bodyColl1->Pos2);

			// Calculate velocity
			VecCrossVec(&body1->AngVel, &bodyColl1->Pos1, &vel1);
			VecPlusEqVec(&vel1, &body1->Centre.Vel);
			VecCrossVec(&body2->AngVel, &bodyColl1->Pos2, &vel2);
			VecPlusEqVec(&vel2, &body2->Centre.Vel);
			VecMinusVec(&vel1, &vel2, &bodyColl1->Vel);
			// Make sure that the sphere is not already travelling away from the surface
			if (VecDotVec(&bodyColl1->Vel, PlaneNormal(&bodyColl1->Plane)) > ZERO) continue;

			// Collision has occurred
			AddBodyColl(body1, bodyColl1);
			if ((bodyColl2 = NextBodyCollInfo()) == NULL) {
				RemoveBodyColl(body1, bodyColl1);
				return nColls;
			}
			AddBodyColl(body2, bodyColl2);

			bodyColl2->Body1 = body2;
			bodyColl2->Body2 = body1;
			CopyVec(&bodyColl1->Pos2, &bodyColl2->Pos1);
			CopyVec(&bodyColl1->Pos1, &bodyColl2->Pos2);
			CopyVec(&bodyColl1->Vel, &bodyColl2->Vel);
			NegateVec(&bodyColl2->Vel);

			FlipPlane(&bodyColl1->Plane, &bodyColl2->Plane);
			bodyColl1->Depth = MulScalar(HALF, bodyColl1->Depth);
			bodyColl2->Depth = bodyColl1->Depth;
			bodyColl1->Time = ZERO;		// DODGY...
			bodyColl2->Time = ZERO;		// DODGY...

			bodyColl1->Grip = MulScalar(body1->Centre.Grip, body2->Centre.Grip);
			bodyColl1->StaticFriction = MulScalar(body1->Centre.StaticFriction, body2->Centre.StaticFriction);
			bodyColl1->KineticFriction = MulScalar(body1->Centre.KineticFriction, body2->Centre.KineticFriction);
			bodyColl1->Restitution = MulScalar(body1->Centre.Hardness, body2->Centre.Hardness);
			bodyColl1->Material = NULL;
			bodyColl2->StaticFriction = bodyColl1->StaticFriction;
			bodyColl2->KineticFriction = bodyColl1->KineticFriction;
			bodyColl2->Restitution = bodyColl1->Restitution;
			bodyColl2->Grip = bodyColl1->Grip;
			bodyColl2->Material = NULL;

		}
	}

	return nColls;

}


/////////////////////////////////////////////////////////////////////
//
// BodyTurboBoost: boost the body in the direction it is facing
//
/////////////////////////////////////////////////////////////////////

void BodyTurboBoost(NEWBODY *body)
{
	REAL boostMag;
	// Make sure the body is being boosted
	if (body->Centre.Boost < SMALL_REAL) return;

	boostMag = MulScalar(MulScalar(TimeStep, body->Centre.Boost), body->Centre.Mass);
	VecPlusEqScalarVec(&body->Centre.Impulse, boostMag, &body->Centre.WMatrix.mv[L]);
}

/////////////////////////////////////////////////////////////////////
//
// PostProcessBodyColls:
//
/////////////////////////////////////////////////////////////////////

void PostProcessBodyColls(NEWBODY *body)
{

}


/////////////////////////////////////////////////////////////////////
//
// AddBodyColl:
//
/////////////////////////////////////////////////////////////////////

COLLINFO_BODY *AddBodyColl(NEWBODY *body, COLLINFO_BODY *newHead)
{
	COLLINFO_BODY *oldHead = body->BodyCollHead;

	body->BodyCollHead = newHead;
	newHead->Next = oldHead;
	newHead->Prev = NULL;

	if (oldHead != NULL) {
		oldHead->Prev = newHead;
	}

	newHead->Active = TRUE;

	body->NBodyColls++;
	COL_NBodyColls++;

	return newHead;
}

/////////////////////////////////////////////////////////////////////
//
// RemoveBodyColl:
//
/////////////////////////////////////////////////////////////////////

void RemoveBodyColl(NEWBODY *body, COLLINFO_BODY *collInfo)
{
	Assert(collInfo != NULL);
	Assert(collInfo->Active);

	if (collInfo->Next != NULL) {
		(collInfo->Next)->Prev = collInfo->Prev;
	}

	if (collInfo->Prev != NULL) {
		(collInfo->Prev)->Next = collInfo->Next;
	} else {
		body->BodyCollHead = collInfo->Next;
	}

	collInfo->Active = FALSE;

	body->NBodyColls--;

	Assert((body->NBodyColls == 0)? (body->BodyCollHead == NULL): (body->BodyCollHead != NULL));
}

