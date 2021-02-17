
#ifndef CAMERA_H
#define CAMERA_H

#ifdef _PC
 #include "car.h"
 #include "object.h"
#endif
#ifdef _N64
 #include "geom.h"
 #include "model.h"
 #include "particle.h"
 #include "aerial.h"
 #include "NewColl.h"
 #include "Body.h"
 #include "car.h"
 #include "visibox.h"
 #include "ctrlread.h"
 #include "object.h"
 #include "Control.h"
 #include "level.h"
 #include "player.h"
#endif

// macros

#define MAX_CAMERAS 8

#ifdef _PC
#define CAMERA_LEFT (Keys[DIK_A])
#define CAMERA_RIGHT (Keys[DIK_D])
#define CAMERA_UP (Keys[DIK_W])
#define CAMERA_DOWN (Keys[DIK_S])
#define CAMERA_BACKWARDS (Keys[DIK_Q])
#define CAMERA_FORWARDS (Keys[DIK_E])

#define LENS_DIST_MOD	0.25f
#define MIN_LENS		(-HALF * BaseGeomPers)
#endif

#ifdef _N64
#define CAMERA_LEFT 		(Buttons & BUT_C_LEFT)
#define CAMERA_RIGHT 		(Buttons & BUT_C_RIGHT)
#define CAMERA_UP 			(Buttons & BUT_C_UP)
#define CAMERA_DOWN 		(Buttons & BUT_C_DOWN)
#define CAMERA_BACKWARDS 	(Buttons & BUT_DOWN)
#define CAMERA_FORWARDS 	(Buttons & BUT_UP)

#define LENS_DIST_MOD	0.025f
#define MIN_LENS		(-HALF * BaseGeomPers)
#endif


enum {
	CAM_FOLLOW,
	CAM_ATTACHED,
	CAM_RAIL,
	CAM_FREEDOM,
	CAM_NEWFOLLOW,
	CAM_EDIT,

	CAM_NTYPES
};
typedef long CAMTYPE;

enum {
	CAMERA_FLAG_FREE,
	CAMERA_FLAG_PRIMARY,
	CAMERA_FLAG_SECONDARY,
};

typedef struct CameraStruct {

	// Camera type
	long Type;
	long SubType;

	// Position and matrix function pointers
	void (*CalcCamPos)(struct CameraStruct *camera);
	void (*CalcCamLook)(struct CameraStruct *camera);

	// Camera Orientation 
	MAT	WMatrix;
	QUATERNION Quat;

	// Camera's world position (this and last frame)
	VEC	WPos;
	VEC	OldWPos;
	VEC	Vel;

	// Camera's object-relative offset
	VEC	DestOffset;						// desired pre-rotated offset
	VEC	PosOffset;						// current pre-rotated offset
	VEC	WorldPosOffset;					// current post-rotated offset

	// Camera's object-relative look-at position
	VEC	LookOffset;
	VEC OldLookOffset;

	// The object which the camera is looking at, or NULL for static cameras
	OBJECT *Object;

	// Field of view and clipping info
	REAL Lens;
	REAL NearClip;
	REAL ZoomMod;

	// Misc flags
	bool Collide;
	bool Zoom;
	bool Change;

	REAL Timer;
	REAL Shake;

	// View screen collision
	NEWCOLLPOLY CollPoly;

	VEC	CollPos;
	VEC	OldCollPos;

	REAL	X, Y, Xsize, Ysize;
	long	Flag;
#ifdef _N64
	PLAYER	*Player;
#endif

} CAMERA;


/////////////////////////////////////////////////////////////////////
//
// Camera Collision info
//
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// Camera node stuff
//
enum CamNodeTypeEnum {
	CAMNODE_MONORAIL,
	CAMNODE_STATIC,

	CAMNODE_NTYPES
};

typedef struct CamNodeStruct {
	long Type;
	long ID;
	VEC Pos;
	REAL ZoomMod;
	long Link;
	VISIMASK VisiMask;
} CAMNODE;

#define CAMERA_MAX_NODES	(1024)
#define MAX_CAMNODE_DIST	(OGU2GU_LENGTH * 10000.0f)


/////////////////////////////////////////////////////////////////////
// Camera offset data
//

typedef struct CamFollowDataStruct {
	bool Collide;
	VEC	PosOffset;
	VEC	LookOffset;
} CAMFOLLOWDATA;

typedef struct CamAttachedDataStruct {
	VEC PosOffset;
	bool Forward;
} CAMATTACHEDDATA;

enum {  // FollowType
	CAM_FOLLOW_BEHIND,
	CAM_FOLLOW_CLOSE,
	CAM_FOLLOW_LEFT,
	CAM_FOLLOW_RIGHT,
	CAM_FOLLOW_FRONT,
	CAM_FOLLOW_ROTATE,

	CAM_FOLLOW_NTYPES
};

enum {  // AttachedType
	CAM_ATTACHED_INCAR,
	CAM_ATTACHED_LEFT,
	CAM_ATTACHED_RIGHT,
	CAM_ATTACHED_REARVIEW,

	CAM_ATTACHED_NTYPES
};

enum { // Rail camera types
	CAM_RAIL_DYNAMIC_MONO,
	CAM_RAIL_STATIC_NEAREST,

	CAM_RAIL_NTYPES
};

// prototypes

extern void (*CameraFunction[])(void);
extern void SetCameraView(MAT *cammat, VEC *campos, REAL shake);
extern void SetViewport(REAL x, REAL y, REAL xsize, REAL ysize, REAL pers);
extern CAMERA *AddCamera(REAL x, REAL y, REAL xsize, REAL ysize, long flag);
extern void RemoveCamera(CAMERA *camera);
extern void InitCameras(void);
extern void SetProjMatrix(REAL n, REAL f, REAL fov);

extern CAMNODE *FindNearestCameraPath(VEC *pos, long *nodeNum, long *linkNum);

extern void UpdateCamera(CAMERA *camera);

extern void SetCameraFollow(CAMERA *camera, OBJECT *object, long type);
extern void SetCameraAttached(CAMERA *camera, OBJECT *object, long type);
extern void SetCameraFreedom(CAMERA *camera, OBJECT *object, long unUsed);
extern void SetCameraRail(CAMERA *camera, OBJECT *object, long type);
extern void SetCameraEdit(CAMERA *camera, OBJECT *object, long unUsed);
extern void SetCameraNewFollow(CAMERA *camera, OBJECT *object, long type);
extern void InitCamPos(CAMERA *camera);

extern void TriggerCamera(struct PlayerStruct *player, long flag, long n, VEC *vec);

#ifdef _PC
extern long LoadCameraNodes(FILE *fp);
#endif
#ifdef _N64
extern long LoadCameraNodes(void);
#endif


// globals

extern char CameraCount;
extern REAL CameraHomeHeight;
extern MAT ViewMatrixScaled, ViewMatrix, ViewCameraMatrix, ViewMatrixScaledMirrorY;
extern VEC ViewTransScaled, ViewTrans, ViewCameraPos;
#ifdef _N64
extern REAL ViewAngle;
#endif
extern VEC CameraHomePos;
extern REAL MouseXpos, MouseYpos, MouseXrel, MouseYrel;
extern REAL CameraEditXrel, CameraEditYrel, CameraEditZrel;
extern char MouseLeft, MouseRight, MouseLastLeft, MouseLastRight;
extern PLANE CameraPlaneLeft, CameraPlaneRight, CameraPlaneTop, CameraPlaneBottom;
extern CAMERA Camera[];
extern CAMERA *CAM_MainCamera;
extern CAMNODE CAM_CameraNode[CAMERA_MAX_NODES];
extern long CAM_NCameraNodes;
extern long CAM_ValidRailCamNode;
#ifdef _PC
extern D3DRECT ViewportRect;
#endif

#endif
