#define STRICT
#define D3D_OVERLOADS
#include <d3dframe.h>

#include "UnitInfo.h"
#include "Inputs.h"
#include "Editor.h"
#include "fileio.h"
#include "compile.h"
#include "states.h"

extern POPUP_MENU_STATES		PopupState;
extern POPUP_MENU_ACTIONS		PopupAction;
extern U16						PopupMenuPosition;
extern REAL						PopupMenuTime;
extern U16						PopupCursorFrame;
extern U16						PopupCursorPosition;
extern REAL						PopupCursorTime;

extern NEW_TRACK_STATES			NewTrackState;
extern LOAD_TRACK_STATES		LoadTrackState;
extern SAVE_TRACK_STATES		SaveTrackState;

extern PLACEMENT_CAMERA_DIR		PlacementViewpoint;
extern CURSORDESC				UnitCursor;
extern CURSORDESC				ModuleCursor;
extern CURSORDESC				TrackCursor;
extern INDEX					CurrentModule;
extern TRACKDESC				TrackData;
extern TRACKTHEME				Theme;
extern U16						FileWindowStart;
extern U16						FileWindowOffset;
extern char**					FileList;
extern U16						FileCount;

void HandleKeyDown(HWND hWnd, int vKeyCode)
{
	if(ErrorExists())
	{
		if(vKeyCode == VK_RETURN)
		{
			CancelError();
		}
	}
	else
	{
		switch(GetScreenState())
		{
			case ESS_PLACING_MODULE:
				Handle_Module_Placement(hWnd, vKeyCode);
			break;

			case ESS_CHOOSING_MODULE:
				Handle_Module_Selection(hWnd, vKeyCode);
			break;

			case ESS_LOADING_TRACK:	//user is selecting a pre-created track
				Handle_Load_Menu_Input(hWnd, vKeyCode);
			break;

			case ESS_SAVING_TRACK:	//user is saving a track
				Handle_Save_Track_Input(hWnd, vKeyCode);
			break;

			case ESS_CREATING_TRACK:	//user is creating a new track
				Handle_New_Track_Input(hWnd, vKeyCode);
			break;

			case ESS_ADJUSTING_TRACK:	//user is resizing/shifting the track
				Handle_Adjust_Track_Input(hWnd, vKeyCode);
			break;

			default:
				switch(vKeyCode)
				{
					case VK_ESCAPE:
						SetScreenState(ESS_PLACING_MODULE);
					break;
				}
			break;
		}
	}
}

int ViewpointAdjustedKey(int vKeyCode)
{
	int adjustedkeys[] = {VK_UP, VK_LEFT, VK_DOWN, VK_RIGHT, VK_UP, VK_LEFT, VK_DOWN};

	int offset = PlacementViewpoint;
	switch(vKeyCode)
	{
		case VK_UP:
		break;

		case VK_RIGHT:
			offset += 3;
		break;

		case VK_DOWN:
			offset += 2;
		break;

		case VK_LEFT:
			offset += 1;
		break;

		default:
			return vKeyCode;
		break;
	}
	return adjustedkeys[offset];
}

void Process_Placement_Keys(HWND hWnd, int vKeyCode)
{
	static PLACEMENT_CAMERA_DIR nextviewpoint[4] = {PCD_EAST, PCD_SOUTH, PCD_WEST, PCD_NORTH};
	static PLACEMENT_CAMERA_DIR prevviewpoint[4] = {PCD_WEST, PCD_NORTH, PCD_EAST, PCD_SOUTH};

	switch(vKeyCode)
	{
		case VK_ADD:
			PlacementViewpoint = nextviewpoint[PlacementViewpoint];
		break;

		case VK_SUBTRACT:
			PlacementViewpoint = prevviewpoint[PlacementViewpoint];
		break;

		case VK_PRIOR:
			IncreaseElevation();
		break;

		case VK_NEXT:
			DecreaseElevation();
		break;

		case VK_SPACE:
			RotateModule();
		break;

		case VK_RETURN:
			PlaceModule(&TrackData);	//place the currently selected module onto the track
		break;

		case VK_DELETE:
			DeleteModule(&TrackData);
		break;

		case 'P':
			SelectModuleUnderCursor(&TrackData);
		break;

		case VK_LEFT:
		case VK_RIGHT:
		case VK_DOWN:
		case VK_UP:
			MoveCursor(&TrackCursor, ViewpointAdjustedKey(vKeyCode));
		break;
		
		case VK_TAB:
			PopupState = PMS_APPEARING;
			PopupMenuPosition = PopupSlideDistance;
			PopupMenuTime = 0.5f;
			PopupCursorPosition = 0;
			PopupCursorFrame = 0;
		break;

		case 'X':
			NextVariant();
		break;

		case 'Z':
			PreviousVariant();
		break;
	}
}

void Process_Popup_Keys(HWND hWnd, int vKeyCode)
{
	switch(vKeyCode)
	{
		case VK_TAB:
			if(PopupState == PMS_ACTIVE)
			{
				PopupState = PMS_DISAPPEARING;
				PopupAction = PMA_DISMISSED;
				PopupMenuPosition = 0;
				PopupMenuTime = 0.5f;
			}
		break;

		case VK_DOWN:
			if(PopupCursorPosition < ESS_QUITTING)
			{
				PopupCursorPosition++;
				PopupState = PMS_MOVING_DOWN;
				PopupCursorTime = 0.4f;
			}
		break;

		case VK_UP:
			if(PopupCursorPosition > 0)
			{
				PopupCursorPosition--;
				PopupState = PMS_MOVING_UP;
				PopupCursorTime = 0.4f;
			}
		break;

		case VK_RETURN:
			if(PopupCursorPosition == ESS_QUITTING)
			{
				PostMessage(hWnd, WM_CLOSE, 0, 0);
			}
			else
			{
				PopupState = PMS_DISAPPEARING;
				PopupAction = PMA_SELECTED;
				PopupMenuPosition = 0;
				PopupMenuTime = 0.5f;
			}
		break;
	}
}

void Handle_Module_Placement(HWND hWnd, int vKeyCode)
{
	bool shiftpressed = false;
	if((GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0)
	{
		shiftpressed = true;
	}
	
	if(PopupState == PMS_INACTIVE)
	{
		Process_Placement_Keys(hWnd, vKeyCode);
	}
	else
	{
		if((PopupState != PMS_APPEARING) && (PopupState != PMS_DISAPPEARING))
		{
			Process_Popup_Keys(hWnd, vKeyCode);
		}
	}
}

void Handle_Module_Selection(HWND hWnd, int vKeyCode)
{
	switch(vKeyCode)
	{
		case VK_RETURN:
			CurrentModule = Theme.Lookup->Groups[ModuleCursor.Y].ModuleID;
			SetTrackCursorSize();
			SetScreenState(ESS_PLACING_MODULE);
		break;

		case VK_DOWN:
			MoveCursor(&ModuleCursor, VK_UP);
		break;
		case VK_UP:
			MoveCursor(&ModuleCursor, VK_DOWN);
		break;

		case VK_ESCAPE:
			SetScreenState(ESS_PLACING_MODULE);
		break;
	}
}

void Handle_Adjust_Track_Input(HWND hWnd, int vKeyCode)
{
	bool shiftpressed = false;
	if((GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0)
	{
		shiftpressed = true;
	}

	switch(vKeyCode)
	{
		case VK_LEFT:
			if(shiftpressed == false)
			{
				DecreaseTrackWidth(&TrackData);
			}
			else
			{
				SlideTrackLeft(&TrackData);
			}
		break;

		case VK_RIGHT:
			if(shiftpressed == false)
			{
				IncreaseTrackWidth(&TrackData);
			}
			else
			{
				SlideTrackRight(&TrackData);
			}
		break;

		case VK_DOWN:
			if(shiftpressed == false)
			{
				DecreaseTrackHeight(&TrackData);
			}
			else
			{
				SlideTrackDown(&TrackData);
			}
		break;

		case VK_UP:
			if(shiftpressed == false)
			{
				IncreaseTrackHeight(&TrackData);
			}
			else
			{
				SlideTrackUp(&TrackData);
			}
		break;

		case VK_ESCAPE:
			SetScreenState(ESS_PLACING_MODULE);
		break;
	}
}

void Handle_New_Track_Input(HWND hWnd, int vKeyCode)
{
	static NEW_TRACK_STATES next_entry[] =
	{
		NTS_RC_WORLD,
		NTS_CANCEL,
		NTS_TOY_WORLD
	};
	
	static NEW_TRACK_STATES prev_entry[] =
	{
		NTS_CANCEL,
		NTS_TOY_WORLD,
		NTS_RC_WORLD
	};
	
	switch(vKeyCode)
	{
		case VK_DOWN:
			NewTrackState = next_entry[NewTrackState];
		break;
			
		case VK_UP:
			NewTrackState = prev_entry[NewTrackState];
		break;

		case VK_RETURN:
			switch(NewTrackState)
			{
				case NTS_CANCEL:
					SetScreenState(ESS_PLACING_MODULE);
				break;

				case NTS_TOY_WORLD:
				case NTS_RC_WORLD:
					SetScreenState(ESS_PLACING_MODULE);
					NewTrack(&TrackData);
				break;
			}
		break;
	}
}

void Handle_Load_Menu_Input(HWND hWnd, int vKeyCode)
{
	if(LoadTrackState == LTS_CHOOSING)
	{
		switch(vKeyCode)
		{
			case VK_RIGHT:
				LoadTrackState = LTS_CANCEL;
			break;

			case VK_UP:
				if(FileWindowOffset > 0)
				{
					FileWindowOffset--;
				}
				else
				{
					if(FileWindowStart > 0)
					{
						FileWindowStart--;
					}
				}
			break;

			case VK_DOWN:
				if(FileWindowOffset < (MAX_FILES_IN_WINDOW - 1))
				{
					if((FileWindowOffset + FileWindowStart) < (FileCount - 1))
					{
						FileWindowOffset++;
					}
				}
				else
				{
					if(FileWindowStart < (FileCount - MAX_FILES_IN_WINDOW))
					{
						FileWindowStart++;
					}
				}
			break;

			case VK_RETURN:
				if(DescribedFileExists(FileList[FileWindowStart + FileWindowOffset]))
				{
					char filename[MAX_PATH];
					GetDescribedFilename(FileList[FileWindowStart + FileWindowOffset], filename);
					ReadTDF(filename, &TrackData);
					CurrentModule = MODULE_STARTGRID;
					SetScreenState(ESS_PLACING_MODULE);
				}
			break;
		}
	}
	else
	{
		switch(vKeyCode)
		{
			case VK_LEFT:
				LoadTrackState = LTS_CHOOSING;
			break;

			case VK_RETURN:
				SetScreenState(ESS_PLACING_MODULE);
			break;
		}
	}
}

void Handle_Save_Track_Input(HWND hWnd, int vKeyCode)
{
	static SAVE_TRACK_STATES next_state[] =
	{
		STS_SAVE,
		STS_CANCEL,
		STS_ENTERING
	};
	static SAVE_TRACK_STATES prev_state[] =
	{
		STS_CANCEL,
		STS_ENTERING,
		STS_SAVE
	};
	
	U16 textindex = strlen(TrackData.Name);
	int key = (int)MapVirtualKey(vKeyCode, 2);
	switch(SaveTrackState)
	{
		case STS_ENTERING:
			if(isalnum(key) || (key == ' '))
			{
				if((GetAsyncKeyState(VK_SHIFT) & 0x8000) == 0)
				{
					key = tolower(key);
				}
				if(textindex < (MAX_DESCRIPTION_LENGTH - 1))
				{
					TrackData.Name[textindex++] = key;
					TrackData.Name[textindex] = '\0';
				}
			}
			if(vKeyCode == VK_BACK)
			{
				if(textindex > 0)
				{
					TrackData.Name[--textindex] = '\0';
				}
			}
		break;

		case STS_SAVE:
			if(vKeyCode == VK_RETURN)
			{
				SaveTrack(&TrackData);
				SetScreenState(ESS_PLACING_MODULE);
			}
		break;

		case STS_CANCEL:
			if(vKeyCode == VK_RETURN)
			{
				SetScreenState(ESS_PLACING_MODULE);
			}
		break;
	}
	switch(vKeyCode)
	{
		case VK_UP:
			SaveTrackState = prev_state[SaveTrackState];
		break;

		case VK_DOWN:
			SaveTrackState = next_state[SaveTrackState];
		break;
	}
}

bool CursorPosIsValid(CURSORDESC* cursor, U16 testx, U16 testy)
{
	bool isvalid = true;
	if((testx >= cursor->XMax) && (testy >= cursor->YMax))
	{
		isvalid = false;
	}
	else
	{
		S16 var1, var2, xsize, ysize;

		var1 = max(cursor->XSize, 1);
		var2 = min(cursor->XSize, 1);
		var2 = abs(var2);
		xsize = var1 - var2;

		var1 = max(cursor->YSize, 1);
		var2 = min(cursor->YSize, 1);
		var2 = abs(var2);
		ysize = var1 - var2;

		S16 left = min(testx, testx + xsize);
		S16 right = max(testx, testx + xsize);
		S16 bottom = min(testy, testy + ysize);
		S16 top = max(testy, testy + ysize);

		if((left < 0) || (right >= cursor->XMax))
		{
			isvalid = false;
		}
		else
		{
			if((bottom < 0) || (top >= cursor->YMax))
			{
				isvalid = false;
			}
		}
	}
	return isvalid;
}

void MoveCursor(CURSORDESC* cursor, int vKeyCode)
{
	if(cursor != NULL)
	{
		S16 newx = cursor->X;
		S16 newy = cursor->Y;
		switch(vKeyCode)
		{
			case VK_LEFT:
				if(newx > 0)
				{
					newx--;
				}
			break;

			case VK_RIGHT:
				newx++;
			break;

			case VK_DOWN:
				if(newy > 0)
				{
					newy--;
				}
			break;

			case VK_UP:
				newy++;
			break;
		}
		if(CursorPosIsValid(cursor, newx, newy))
		{
			cursor->X = newx;
			cursor->Y = newy;
		}
	}
}

