#include <assert.h>
#include <time.h>

#include "fileio.h"
#include "compile.h"
#include "editor.h"
#include "states.h"

extern TRACKTHEME				Theme;
extern TRACKDESC				TrackData;
extern CURSORDESC				TrackCursor;

POPUP_MENU_STATES		PopupState			= PMS_INACTIVE;
POPUP_MENU_ACTIONS		PopupAction			= PMA_DISMISSED;
U16						PopupMenuPosition	= 160;
REAL					PopupMenuTime		= 0.0f;
U16						PopupCursorFrame	= 0;
U16						PopupCursorPosition	= 0;
REAL					PopupCursorTime		= 0.0f;

COMPILE_STATES			CompileState		= TCS_VALIDATING;
U8						ClockFrame			= 0;

NEW_TRACK_STATES		NewTrackState		= NTS_TOY_WORLD;
LOAD_TRACK_STATES		LoadTrackState		= LTS_CHOOSING;
SAVE_TRACK_STATES		SaveTrackState		= STS_ENTERING;

//***********************************************************

static EDIT_SCREEN_STATES	ScreenState	= ESS_PLACING_MODULE;
static bool					ErrorState = false;
static char					ErrorText[MAX_ERROR_LENGTH];

//***********************************************************

//**********************************************************************************************************************
// ErrorState Handlers
//**********************************************************************************************************************

//***********************************************************
// ErrorExists
//
// This simply returns true or false to indicate whether an
// error has been flagged. This is done in preference to
// making the ErrorState variable global.
//***********************************************************
bool ErrorExists(void)
{
	return ErrorState;
}

//***********************************************************
// FlagError
//
// Call this function to trigger the display of an error
// message. Supply a pointer the text that you wish to
// display. The text can contain embedded cr/lf chars
// and they will be handled appropriately
//
//***********************************************************

void FlagError(const char* message)
{
	assert(ErrorState == false); //user may have made excessive calls to this function

	strncpy(ErrorText, message, sizeof(ErrorText));
	ErrorState = true;
}

//***********************************************************
// CancelError
//
// Simply sets ErrorState to false.
// Again - used in preference to alowing global access.
//***********************************************************

void CancelError(void)
{
	assert(ErrorState == true); //user may have made excessive calls to this function

	ErrorState = false;
}

//***********************************************************
// GetErrorText
//
// Returns a const pointer to the current error message text
// This is preferential to giving global access to the text
//***********************************************************

const char* GetErrorText(void)
{
	return ErrorText;
}

//**********************************************************************************************************************
// ScreenState Handlers
//**********************************************************************************************************************

//***********************************************************
// EnteringState
//
// This function is called JUST AFTER ScreenState changes
// value and is the point where any one-off initialization
// can be done for the particular state.
//***********************************************************
void EnteringState(void)
{
	switch(GetScreenState())
	{
		case ESS_LOADING_TRACK:
			MakeFileList();
			LoadTrackState = LTS_CHOOSING;
		break;

		case ESS_ADJUSTING_TRACK:
			MakeTrackFromModules(&TrackData);
		break;
	}
}

//***********************************************************
// ExitingState
//
// This function is called JUST BEFORE ScreenState changes
// value and is the point where data needed only for the
// duration of a particular state can be deleted.
//***********************************************************
void ExitingState(void)
{
	switch(GetScreenState())
	{
		case ESS_LOADING_TRACK:
			EraseFileList();
		break;
	}
}

void SetScreenState(EDIT_SCREEN_STATES newstate)
{
	ExitingState();
	ScreenState = newstate;
	EnteringState();
}

EDIT_SCREEN_STATES GetScreenState(void)
{
	return ScreenState;
}

//**********************************************************************************************************************
// 
//**********************************************************************************************************************

void AnimatePopup(REAL elapsedtime)
{
	if(PopupState != PMS_INACTIVE)
	{
		if((PopupState == PMS_MOVING_UP) || (PopupState == PMS_MOVING_DOWN))
		{
			if(PopupCursorTime > elapsedtime)
			{
				PopupCursorTime -= elapsedtime;
			}
			else
			{
				PopupCursorTime = 0.0f;
				PopupState = PMS_ACTIVE;
			}
			PopupCursorFrame = 7 - (((U16)(PopupCursorTime * 20)) % 8);
		}

		if(PopupState == PMS_APPEARING)
		{
			if(PopupMenuTime > elapsedtime)
			{
				PopupMenuTime -= elapsedtime;
			}
			else
			{
				PopupMenuTime = 0.0f;
				PopupState = PMS_ACTIVE;
			}
			float portion = PopupMenuTime / 0.5f;
			PopupMenuPosition = (U16)(PopupSlideDistance * portion);
		}

		if(PopupState == PMS_DISAPPEARING)
		{
			if(PopupMenuTime > elapsedtime)
			{
				PopupMenuTime -= elapsedtime;
			}
			else
			{
				PopupMenuTime = 0.0f;
				PopupState = PMS_INACTIVE;
				if(PopupAction == PMA_SELECTED)
				{
					SetScreenState(static_cast<EDIT_SCREEN_STATES>(PopupCursorPosition));
					PopupAction = PMA_DISMISSED;
				}
			}
			float portion = PopupMenuTime / 0.5f;
			PopupMenuPosition = (U16)(PopupSlideDistance - (PopupSlideDistance * portion));
		}
	}
}

void AnimateCursor(REAL elapsedtime)
{
    REAL time = ((REAL)clock())/CLOCKS_PER_SEC;
	REAL interval = Real(0.5f);
	REAL timescale = ONE / interval;

	int progress = (int)(time * timescale);
	if(progress & 0x01)
	{
		UpdateTrackAndCursor(&TrackData);
	}
	else
	{
		MakeTrackFromModules(&TrackData);
	}
}

//**********************************************************************************************************************
// 
//**********************************************************************************************************************

void HeartBeat(REAL elapsedtime)
{
	switch(GetScreenState())
	{
		case ESS_EXPORTING_TRACK:
			CompileTrack(&TrackData, &Theme, &TrackCursor);
		break;

		case ESS_PLACING_MODULE:
			AnimateCursor(elapsedtime);
		break;
	}
	
	AnimatePopup(elapsedtime);

	switch(GetScreenState())
	{
		default:
		break;
	}
}
