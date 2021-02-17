/////////////////////////////////////////////////////////////////////
//
// Body.h: describe the physics of a solid 3D object
//
/////////////////////////////////////////////////////////////////////

#ifndef __NEWBODY_H__
#define __NEWBODY_H__

#define SMALL_ANGVEL TO_ANGVEL(Real(0.000001))

#define REMOVE_JITTER TRUE

#define CLOSE_BODY_COLL		TO_LENGTH(Real(300.0f))
#define CLOSE_BODY_COLL_DOT	(0.1f)

#define BODY_COLL_CONVEX	0x0
#define BODY_COLL_SPHERE	0x1
#define BODY_COLL_POLY		0x2

#define MAX_SPARKS_PER_BODY 10
#define MAX_SMOKES_PER_BODY 2

#define MAX_SCRAPE_TIME		Real(0.05f)

#define MIN_KNOCK_VEL		TO_VEL(Real(300))

struct BodyCollInfoStruct;

typedef struct NewBodyStruct {
	PARTICLE Centre;			// centre of mass properties
	
	MAT		BodyInertia;		// Mass Matrix					(note: could be made into a vector
	MAT		BodyInvInertia;		// Inverse Mass Matrix						since it is diagonal)

	MAT		WorldInvInertia;	// Inverse inertia matrix in world frame

	VEC		AngVel;				// Angular velocity
	VEC		AngAcc;				// Angular acceleration
	VEC		AngImpulse;			// Angular Impulse

	REAL	DefaultAngRes;
	REAL	AngResMod;	
	REAL	AngResistance;

	COLLSKIN	CollSkin;		// Collision skin: array of convex polytopes

	// Stuff to remove jitter
	VEC		LastAngVel;		// Angular Impulse from last frame
	bool	IsJittering;		// Whether body is jittering
	int		JitterCount;		// Number of times the object has "jittered"
	int		JitterCountMax;		// Max number of jitters allowed
	int		JitterFrames;		// Number of frames between last and next jitter
	int		JitterFramesMax;	// Max number of frames between jitters for it to count as a jitter

	// Info on the number of cantact points this frame
	COLLINFO_BODY	*BodyCollHead;				// Contact point list head
	int				NBodyColls;					// Number of body contact points

	int		NWorldContacts;
	int		NOtherContacts;
	REAL	NoContactTime;

	bool	AllowSparks;		// Whether this body sparks against hard materials or not
	long	ScrapeMaterial;		// The type of material the car is scraping against (or MATERIAL_NONE)
	REAL	LastScrapeTime;		// Time between scrapes

	bool	Banged;
	REAL	BangMag;
	PLANE	BangPlane;

} NEWBODY;


/////////////////////////////////////////////////////////////////////
//
// Defined functions
//

//void ApplyBodyAngImpulse(NEWBODY *body, VEC *angImpulse)
#define ApplyBodyAngImpulse(body, angImpulse) \
{ \
	VecPlusEqVec(&(body)->AngImpulse, (angImpulse)); \
}

//void CalcAngImpulse(VEC *impulse, VEC *pos, VEC *angImp)
#define CalcAngImpulse(impulse, pos, angImp) \
{ \
	VecCrossVec((pos), (impulse), (angImp)); \
}

#define SetBodyConvex(body)		((body)->CollSkin.CollType = BODY_COLL_CONVEX)
#define IsBodyConvex(body)		((body)->CollSkin.CollType == BODY_COLL_CONVEX)
#define SetBodySphere(body)		((body)->CollSkin.CollType = BODY_COLL_SPHERE)
#define IsBodySphere(body)		((body)->CollSkin.CollType == BODY_COLL_SPHERE)
#define SetBodyPoly(body)		((body)->CollSkin.CollType = BODY_COLL_POLY)
#define IsBodyPoly(body)		((body)->CollSkin.CollType == BODY_COLL_POLY)


#define BodyAllowsSparks(body)	((body)->AllowSparks)

/////////////////////////////////////////////////////////////////////
//
// Function prototypes
//

extern void ApplyBodyImpulse(NEWBODY *body, VEC *impulse, VEC *impulsePos);
extern void UpdateBody(NEWBODY *body, REAL dt);
extern void BodyPointVel(NEWBODY *body, VEC *dR, VEC *vel);

extern void InitBodyDefault(NEWBODY *body);
extern void SetBodyPos(NEWBODY *body, VEC *pos, MAT *mat);
extern void SetBodyInertia(NEWBODY *body, MAT *inertia);
extern void BuildCuboidInertia(REAL mass, REAL xDim, REAL yDim, REAL zDim, VEC *mat);
extern void MoveInertiaAxis(MAT *inIn, REAL mass, REAL dx, REAL dy, REAL dz, MAT *inOut);
extern void GetFrameInertia(MAT *bodyInvInertia, MAT *transform, MAT *worldInvInertia);
extern REAL BodyKE(NEWBODY *body);

extern void BuildOneBodyColMat(NEWBODY *body, VEC *colPos, VEC *colPos2, MAT *colMat);
extern void BuildTwoBodyColMat(NEWBODY *body1, NEWBODY *body2, VEC *colPos1, VEC *relPos1, VEC *colPos2, VEC *relPos2, MAT *colMat);

extern REAL OneBodyZeroFrictionImpulse(NEWBODY *body, 
								VEC *pos, 
								VEC *normal,
								REAL deltaVel);
extern void TwoBodyZeroFrictionImpulse(NEWBODY *body1, NEWBODY *body2,
								VEC *pos1, VEC *pos2, 
								VEC *normal,
								REAL deltaVel, 
								VEC *impulse);

extern void SetBodyBBoxes(NEWBODY *body);

extern void DetectBodyWorldColls(NEWBODY *body);
extern int DetectBodyBodyColls(NEWBODY *body1, NEWBODY *body2);
extern void DetectBodyPolyColls(NEWBODY *body, NEWCOLLPOLY *collPoly);
extern int DetectConvexHullPolyColls(NEWBODY *body, NEWCOLLPOLY *collPoly);
extern int DetectHullHullColls(NEWBODY *body1, NEWBODY *body2);
extern void DetectSpherePolyColls(NEWBODY *body, NEWCOLLPOLY *collPoly);
extern int DetectSphereSphereColls(NEWBODY *body1, NEWBODY *body2);
extern int DetectSphereHullColls(NEWBODY *body1, NEWBODY *body2);
extern void PreProcessBodyColls(NEWBODY *body);
extern void ProcessBodyColls3(NEWBODY *body);
extern void PostProcessBodyColls(NEWBODY *body);
extern void AddBodyFriction(NEWBODY *body, VEC *impulse, COLLINFO_BODY *collInfo);
extern void BodyTurboBoost(NEWBODY *body);

extern void SetupMassiveBody();

extern void RemoveBodyColl(NEWBODY *body, COLLINFO_BODY *collInfo);
extern COLLINFO_BODY *AddBodyColl(NEWBODY *body, COLLINFO_BODY *newHead);


/////////////////////////////////////////////////////////////////////
//
// Externed globals
//

extern REAL BDY_Tolerance;
extern NEWBODY	BDY_MassiveBody;

#endif
