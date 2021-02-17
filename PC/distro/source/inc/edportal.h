
#ifndef EDPORTAL_H
#define EDPORTAL_H

// macros

#define MAX_EDIT_PORTALS 1024

enum {
	PORTAL_AXIS_XY,
	PORTAL_AXIS_XZ,
	PORTAL_AXIS_ZY,
	PORTAL_AXIS_X,
	PORTAL_AXIS_Y,
	PORTAL_AXIS_Z,
};

typedef struct {
	long Region;
	long ID1, ID2;
	VEC Pos;
	MAT Matrix;
	float Size[3];
	long Flag[4];
} EDIT_PORTAL;

// prototypes

extern void LoadEditPortals(char *file);
extern void KillEditPortals(void);
extern void SaveEditPortals(char *file);
extern EDIT_PORTAL *AllocPortal(void);
extern void FreeEditPortal(EDIT_PORTAL *portal);
extern void EditPortals(void);
extern void DrawPortals(void);
extern void DisplayPortalInfo(EDIT_PORTAL *portal);

// globals

extern EDIT_PORTAL *CurrentPortal;

#endif
