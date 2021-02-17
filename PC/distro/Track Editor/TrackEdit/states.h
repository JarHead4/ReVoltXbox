#ifndef _STATES_H
#define _STATES_H

#include <typedefs.h>
#include "UnitInfo.h"
#include "TrackEditTypes.h"

const U16 MAX_ERROR_LENGTH = 128;

bool ErrorExists(void);
void FlagError(const char* message);
void CancelError(void);
const char* GetErrorText(void);

void SetScreenState(EDIT_SCREEN_STATES newstate);
EDIT_SCREEN_STATES GetScreenState(void);
void HeartBeat(REAL elapsedtime);

#endif
