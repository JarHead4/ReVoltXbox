#ifndef _EDITOR_H
#define _EDITOR_H

#include <bobtypes.h>
#include "TrackEditTypes.h"

void	DecreaseElevation(void);
void	IncreaseElevation(void);
void	RotateModule(void);
void	PlaceModuleNumber(TRACKDESC* track, INDEX moduleid, U16 xpos, U16 ypos);
void	PlaceModuleNumber(TRACKDESC* track, INDEX moduleid);
void	PlaceModule(TRACKDESC* track);
void	EraseModulesUnitsFromTrack(TRACKDESC* track, U16 xpos, U16 ypos);
void	DeleteModule(TRACKDESC* track);
void	SetTrackCursorSize(void);
void	MakeTrackFromModules(TRACKDESC* track);
void	UpdateTrackAndCursor(TRACKDESC* track);
void	CorrectCursor(CURSORDESC* cursor);
bool	CreateTrack(TRACKDESC* track, U32 width, U32 height, const char* name);
bool	CreateTrackAndCursor(TRACKDESC* track, U32 width, U32 height, const char* name);
bool	CloneTrack(TRACKDESC* desttrack, const TRACKDESC* srctrack);
void	CopyTrackRegion(TRACKDESC* desttrack, const TRACKDESC* srctrack, U16 destx, U16 desty, U16 srcx, U16 srcy, U16 srcwidth, U16 srcheight);
void	NewTrack(TRACKDESC* track);
void	SaveTrack(TRACKDESC* track);
void	SelectModuleUnderCursor(TRACKDESC* track);
void	DecreaseTrackWidth(TRACKDESC* track);
void	IncreaseTrackWidth(TRACKDESC* track);
void	DecreaseTrackHeight(TRACKDESC* track);
void	IncreaseTrackHeight(TRACKDESC* track);
void	SlideTrackRight(TRACKDESC* track);
void	SlideTrackLeft(TRACKDESC* track);
void	SlideTrackUp(TRACKDESC* track);
void	SlideTrackDown(TRACKDESC* track);
void	NextVariant(void);
void	PreviousVariant(void);

const	U16 DEFAULT_TRACK_WIDTH  = 16;
const	U16 DEFAULT_TRACK_HEIGHT = 16;

const	U16 MAX_TRACK_WIDTH  = 20;
const	U16 MAX_TRACK_HEIGHT = 20;

const	U16 MIN_TRACK_WIDTH  = 1;
const	U16 MIN_TRACK_HEIGHT = 1;

#endif
