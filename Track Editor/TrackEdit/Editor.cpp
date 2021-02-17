
#include <assert.h>

#include "Editor.h"

#include "TrackEditTypes.h"
#include "UnitInfo.h"
#include "fileio.h"
#include <d3dutil.h>
#include <d3dmath.h>

//**********************************************
//	Declaration of external variables
//**********************************************
extern TRACKTHEME					Theme;
extern CURSORDESC					TrackCursor;
extern CURSORDESC					ModuleCursor;
extern INDEX						CurrentModule;
extern DIRECTION					ModuleDirection;
extern S16							ModuleElevation;
extern TRACKDESC					TrackData;

//**********************************************
// Tables used to change X,Y coords depending
// on the direction that we are facing.
// The arrays have 4 entries, one corresponding
// to each of the 4 major compass directions.
// These appear in the order defined by the 
// enumerated type DIRECTION (N,E,S,W).
// In order to transform an xy coordinate pair
// (which has positive X pointing east and
// positive y pointing north) into an xy pair
// with y pointing in a specified direction
// (and by implication x pointing 90 degrees
// clockwise from y) use the following
// S16 newx = (oldx * xxmultiplier[direction]) + (oldy * xymultiplier[direction]);
// S16 newy = (oldx * yxmultiplier[direction]) + (oldy * yymultiplier[direction]);
//**********************************************
static const S16 xxmultiplier[4] = {1, 0, -1, 0};
static const S16 xymultiplier[4] = {0, 1, 0, -1};
static const S16 yxmultiplier[4] = {0, -1, 0, 1};
static const S16 yymultiplier[4] = {1, 0, -1, 0};

//**********************************************
// EraseModule
//
// Removes the module which overlaps the
// specified coordinates.
// Note - the 
//**********************************************
void EraseModule(TRACKDESC* track, U16 xpos, U16 ypos)
{
	assert(track != NULL);

	//work out which element of module array we are dealing with
	U32 g = (ypos * track->Width) + xpos;

	//find the offset to the modules master position
	xpos -= track->Modules[g].XOffset;
	ypos -= track->Modules[g].YOffset;
	
	//calculate the index for this element
	g = (ypos * track->Width) + xpos;

	INDEX moduleid = track->Modules[g].ModuleID;
	if(moduleid != MAX_INDEX)
	{
		DIRECTION direction = track->Modules[g].Direction;

		TRACKMODULE* module = Theme.Modules[moduleid];
		for(U32 i = 0; i < module->InstanceCount; i++)
		{
			RevoltTrackUnitInstance* instance = module->Instances[i];

			//transform x & y offsets to reflect orientation of modules
			S16 xoffset = (instance->XPos * xxmultiplier[direction]) + (instance->YPos * xymultiplier[direction]);
			S16 yoffset = (instance->XPos * yxmultiplier[direction]) + (instance->YPos * yymultiplier[direction]);
			S16 x = xpos + xoffset;
			S16 y = ypos + yoffset;
			
			U16 n = x + (y * track->Width);
			track->Modules[n].ModuleID = MAX_INDEX ;
			track->Modules[n].XOffset = 0;
			track->Modules[n].YOffset = 0;
		}
	}
}

//**********************************************
//
//**********************************************
void PlaceModuleNumber(TRACKDESC* track, INDEX moduleid, U16 xpos, U16 ypos)
{
	TRACKMODULE* module = Theme.Modules[moduleid];
	for(U32 i = 0; i < module->InstanceCount; i++)
	{
		RevoltTrackUnitInstance* instance = module->Instances[i];

		//transform x & y offsets to reflect orientation of modules
		U16 d = (U16)ModuleDirection;
		S16 xoffset = (instance->XPos * xxmultiplier[d]) + (instance->YPos * xymultiplier[d]);
		S16 yoffset = (instance->XPos * yxmultiplier[d]) + (instance->YPos * yymultiplier[d]);
		S16 x = xpos + xoffset;
		S16 y = ypos + yoffset;
		
		EraseModule(track, x, y);
	}

	U32 g = (ypos * track->Width) + xpos;

	track->Modules[g].ModuleID = moduleid;
	track->Modules[g].Direction = ModuleDirection;
	track->Modules[g].Elevation = ModuleElevation;
	track->Modules[g].XOffset = 0;
	track->Modules[g].YOffset = 0;
	for(i = 0; i < module->InstanceCount; i++)
	{
		RevoltTrackUnitInstance* instance = module->Instances[i];

		//if at origin of module - don't wipe it out
		if((instance->XPos != 0) || (instance->YPos != 0))
		{
			//transform x & y offsets to reflect orientation of modules
			U16 d = (U16)ModuleDirection;
			S16 xoffset = (instance->XPos * xxmultiplier[d]) + (instance->YPos * xymultiplier[d]);
			S16 yoffset = (instance->XPos * yxmultiplier[d]) + (instance->YPos * yymultiplier[d]);
			S16 x = xpos + xoffset;
			S16 y = ypos + yoffset;
			
			U16 n = x + (y * track->Width);
			track->Modules[n].ModuleID = MAX_INDEX ;
			track->Modules[n].XOffset = xoffset;
			track->Modules[n].YOffset = yoffset;
		}
	}
}

//**********************************************
//
//**********************************************
void PlaceModuleNumber(TRACKDESC* track, INDEX moduleid)
{
	PlaceModuleNumber(track, moduleid, TrackCursor.X, TrackCursor.Y);
}

//**********************************************
//
//**********************************************
void PlaceModule(TRACKDESC* track)
{
	SetTrackCursorSize();
	PlaceModuleNumber(track, CurrentModule);
}

//**********************************************
//
//**********************************************
void RotateModule(void)
{
	static DIRECTION nextdirection[4] = {EAST, SOUTH, WEST, NORTH};
	ModuleDirection = nextdirection[ModuleDirection];
	SetTrackCursorSize();
}

//**********************************************
//
//**********************************************
void DecreaseElevation()
{
	if(ModuleElevation > 0)
	{
		ModuleElevation--;
	}
}

//**********************************************
//
//**********************************************
void IncreaseElevation()
{
	if(ModuleElevation < MAX_ELEVATION)
	{
		ModuleElevation++;
	}
}

//**********************************************
//
//**********************************************
U16 ModuleWidth(TRACKMODULE* module)
{
	S16 xmin = MAX_S16;
	S16 xmax = MIN_S16;
	RevoltTrackUnitInstance*  instance;

	for(U32 i = 0; i < module->InstanceCount; i++)
	{
		instance = module->Instances[i];
		xmax = max(xmax, instance->XPos);
		xmin = min(xmin, instance->XPos);
	}
	
	return U16((xmax - xmin) + 1);
}

//**********************************************
//
//**********************************************
U16 ModuleHeight(TRACKMODULE* module)
{
	S16 ymin = MAX_S16;
	S16 ymax = MIN_S16;
	RevoltTrackUnitInstance*  instance;

	for(U32 i = 0; i < module->InstanceCount; i++)
	{
		instance = module->Instances[i];
		ymax = max(ymax, instance->YPos);
		ymin = min(ymin, instance->YPos);
	}
	
	return U16((ymax - ymin) + 1);
}

//**********************************************
//
//**********************************************
void SetTrackCursorSize()
{
	TRACKMODULE* module = Theme.Modules[CurrentModule];
	switch(ModuleDirection)
	{
		case NORTH:
			TrackCursor.XSize = ModuleWidth(module);
			TrackCursor.YSize = -ModuleHeight(module);
		break;
		case EAST:
			TrackCursor.XSize = -ModuleHeight(module);
			TrackCursor.YSize = -ModuleWidth(module);
		break;
		case SOUTH:
			TrackCursor.XSize = -ModuleWidth(module);
			TrackCursor.YSize = ModuleHeight(module);
		break;
		case WEST:
			TrackCursor.XSize = ModuleHeight(module);
			TrackCursor.YSize = ModuleWidth(module);
		break;
	}
	CorrectCursor(&TrackCursor);
}

//**********************************************
//
//**********************************************
void SetUnitInfo(TRACKDESC* track, S16 x, S16 y, INDEX id, DIRECTION direction, S16 elevation)
{
		U16 n = x + (y * track->Width);
		track->Units[n].UnitID = id;
		track->Units[n].Direction = direction; 
		track->Units[n].XPos = x;
		track->Units[n].YPos = y;
		track->Units[n].Elevation = elevation;

		REAL h = elevation * -ElevationStep;
		D3DMATRIX TransMatrix;
		D3DMATRIX RotMatrix;

		D3DUtil_SetRotateYMatrix(RotMatrix, (REAL)((PI * HALF) * direction));

		D3DUtil_SetTranslateMatrix(TransMatrix, x * SMALL_CUBE_SIZE, h, y * SMALL_CUBE_SIZE);
		D3DMath_MatrixMultiply(track->Units[n].Matrix, TransMatrix, RotMatrix);
}

//**********************************************
//
//**********************************************
void WipeTrack(TRACKDESC* track)
{
	for(U16 x = 0; x < track->Width; x++)
	{
		for(U16 y = 0; y < track->Height; y++)
		{
			SetUnitInfo(track, x, y, UNIT_SPACER, NORTH, 0);
		}
	}
}

//**********************************************
//
//**********************************************
void WipeModules(TRACKDESC* track)
{
	U32 n = track->Width * track->Height;
	while(n--)
	{
		track->Modules[n].ModuleID = MAX_INDEX;		//flag as a non-existent module
		track->Modules[n].Direction = NORTH; 
		track->Modules[n].Elevation = 0;
		track->Modules[n].XOffset = 0;
		track->Modules[n].YOffset = 0;
	}
}

//**********************************************
//
//**********************************************
bool CreateTrack(TRACKDESC* track, U32 width, U32 height, const char* name)
{
	bool success = false;
	track->Units = new RevoltTrackUnitInstance[width * height];
	if(track->Units != NULL)
	{
		track->Modules = new RevoltTrackModuleInstance[width * height];
		if(track->Modules != NULL)
		{
			success = true;
			track->Width = width;
			track->Height = height;
			WipeTrack(track);
			WipeModules(track);
			strncpy(track->Name, name, MAX_DESCRIPTION_LENGTH);
		}
		else
		{
			delete[] track->Units;
			track->Units = NULL;
		}
	}
	return success;
}

//**********************************************
//
//**********************************************
bool CreateTrackAndCursor(TRACKDESC* track, U32 width, U32 height, const char* name)
{
	bool success = CreateTrack(track, width, height, name);
	if(success == true)
	{
		TrackCursor.XMax = track->Width;
		TrackCursor.YMax = track->Height;
		TrackCursor.X = TrackCursor.Y = 0;
		TrackCursor.AbsMax = track->Width * track->Height;
		TrackCursor.XSize = 1;
		TrackCursor.YSize = 1;
	}
	return success;
}

//**********************************************
//
//**********************************************
void CopyModuleInstance(RevoltTrackModuleInstance* destmodule, const RevoltTrackModuleInstance* srcmodule)
{
    destmodule->ModuleID = srcmodule->ModuleID;
    destmodule->Direction = srcmodule->Direction;
    destmodule->Elevation = srcmodule->Elevation;
    destmodule->XOffset = srcmodule->XOffset;
    destmodule->YOffset = srcmodule->YOffset;
}

//**********************************************
//
//**********************************************
void CopyTrackRegion(TRACKDESC* desttrack, const TRACKDESC* srctrack, U16 destx, U16 desty, U16 srcx, U16 srcy, U16 srcwidth, U16 srcheight)
{
	U16 copywidth = min(desttrack->Width - destx, srctrack->Width - srcx);
	copywidth = min(copywidth, srcwidth);
	U16 copyheight = min(desttrack->Height - desty, srctrack->Height - srcy);
	copyheight = min(copyheight, srcheight);

	for(U16 x = 0; x < copywidth; x++)
	{
		for(U16 y = 0; y < copyheight; y++)
		{
			U16 sm = ((y + srcy) * srctrack->Width) + x + srcx;
			U16 dm = ((y + desty) * desttrack->Width) + x + destx;
			RevoltTrackModuleInstance* srcmodule = &srctrack->Modules[sm];
			RevoltTrackModuleInstance* destmodule = &desttrack->Modules[dm];
			CopyModuleInstance(destmodule, srcmodule);
		}
	}
}

//**********************************************
//
//**********************************************
void DestroyTrack(TRACKDESC* track)
{
	if(track->Units != NULL)
	{
		delete[] track->Units;
		track->Units = NULL;
	}
	if(track->Modules != NULL)
	{
		delete[] track->Modules;
		track->Modules = NULL;
	}
	track->Width = 0;
	track->Height = 0;
}

//**********************************************
//
//**********************************************
bool CloneTrack(TRACKDESC* desttrack, const TRACKDESC* srctrack)
{
	DestroyTrack(desttrack);
	bool success = CreateTrack(desttrack, srctrack->Width, srctrack->Height, srctrack->Name);
	if(success == true)
	{
		CopyTrackRegion(desttrack, srctrack, 0, 0, 0, 0, srctrack->Width, srctrack->Height);
	}
	return success;
}

//**********************************************
//
//**********************************************
void NewTrack(TRACKDESC* track)
{
	char name[MAX_DESCRIPTION_LENGTH];
	NextFreeDescription(name);
	CurrentModule = MODULE_STARTGRID;
	CreateTrackAndCursor(track, DEFAULT_TRACK_WIDTH, DEFAULT_TRACK_HEIGHT, name);
}

//**********************************************
//
//**********************************************
void GetTrackExtents(TRACKDESC* track, U16* left, U16* right, U16 *top, U16* bottom)
{
	assert(left != NULL);
	assert(right != NULL);
	assert(top != NULL);
	assert(bottom != NULL);
	assert(track != NULL);

	MakeTrackFromModules(track);
	*right = 0;
	*left = track->Width-1;
	*top = 0;
	*bottom = track->Height-1;

	for(U16 x = 0; x < track->Width; x++)
	{
		for(U16 y = 0; y < track->Height; y++)
		{
			U16 m = (y * track->Width) + x;
			if(track->Units[m].UnitID != UNIT_SPACER)
			{
				*left = min(*left, x);
				*right = max(*right, x);
				*bottom = min(*bottom, y);
				*top = max(*top, y);
			}
		}
	}
}

//**********************************************
//
//**********************************************
U16 GetMaxXExtent(TRACKDESC* track)
{
	U16 left;
	U16 right;
	U16 top;
	U16 bottom;
	GetTrackExtents(track, &left, &right, &top, &bottom);
	return right;
}

//**********************************************
//
//**********************************************
U16 GetMaxYExtent(TRACKDESC* track)
{
	U16 left;
	U16 right;
	U16 top;
	U16 bottom;
	GetTrackExtents(track, &left, &right, &top, &bottom);
	return top;
}

//**********************************************
//
//**********************************************
void PlotModuleOnTrack(TRACKDESC* track, TRACKMODULE* module, U16 xpos, U16 ypos, DIRECTION direction, S16 elevation)
{
	for(U32 i = 0; i < module->InstanceCount; i++)
	{
		RevoltTrackUnitInstance* instance = module->Instances[i];

		//transform x & y offsets to reflect orientation of modules
		U16 d = (U16)direction;
		S16 x = xpos + (instance->XPos * xxmultiplier[d]) + (instance->YPos * xymultiplier[d]);
		S16 y = ypos + (instance->XPos * yxmultiplier[d]) + (instance->YPos * yymultiplier[d]);
		
		//fix overflows caused by changes to number of modules in the theme
		x = max(x,0);
		y = max(y,0);
		x = min(x, track->Width - 1);
		y = min(y, track->Height - 1);

		SetUnitInfo(track, x, y, instance->UnitID, direction, elevation);
	}
}

//**********************************************
//
//**********************************************
void EraseModulesUnitsFromTrack(TRACKDESC* track, U16 xpos, U16 ypos)
{
	RevoltTrackModuleInstance* instance = &track->Modules[(ypos * track->Width) + xpos];
	xpos -= instance->XOffset;
	ypos -= instance->YOffset;
	instance = &track->Modules[(ypos * track->Width) + xpos];

	if(instance->ModuleID != MAX_INDEX)
	{
		TRACKMODULE* module;
		DIRECTION direction = instance->Direction;
		module = Theme.Modules[instance->ModuleID];
		for(U32 i = 0; i < module->InstanceCount; i++)
		{
			RevoltTrackUnitInstance* instance = module->Instances[i];

			//transform x & y offsets to reflect orientation of modules
			U16 d = (U16)direction;
			S16 x = xpos + (instance->XPos * xxmultiplier[d]) + (instance->YPos * xymultiplier[d]);
			S16 y = ypos + (instance->XPos * yxmultiplier[d]) + (instance->YPos * yymultiplier[d]);
			
			//fix overflows caused by changes to number of modules in the theme
			x = max(x,0);
			y = max(y,0);
			x = min(x, track->Width - 1);
			y = min(y, track->Height - 1);

			SetUnitInfo(track, x, y, UNIT_SPACER, direction, 0);
		}
	}
}

//**********************************************
//
//**********************************************
void MakeTrackFromModules(TRACKDESC* track)
{
	WipeTrack(track);

	for(U32 x = 0; x < track->Width; x++)
	{
		for(U32 y = 0; y < track->Height; y++)
		{
			U32 g = (y * track->Width) + x;
			if(track->Modules[g].ModuleID != MAX_INDEX)
			{
				if(!((track->Modules[g].ModuleID == 0) && (track->Units[g].UnitID != 0)))
				{
					RevoltTrackModuleInstance* module = &track->Modules[g];
					PlotModuleOnTrack(track, Theme.Modules[module->ModuleID], x, y, module->Direction, module->Elevation);
				}
			}
		}
	}
}

//**********************************************
//
//**********************************************
void WipeUnitsForModulesUnderCursor(TRACKDESC* track) 
{
	TRACKMODULE* module = Theme.Modules[CurrentModule];
	for(U32 i = 0; i < module->InstanceCount; i++)
	{
		RevoltTrackUnitInstance* instance = module->Instances[i];

		U16 d = (U16)ModuleDirection;
		S16 xoffset = (instance->XPos * xxmultiplier[d]) + (instance->YPos * xymultiplier[d]);
		S16 yoffset = (instance->XPos * yxmultiplier[d]) + (instance->YPos * yymultiplier[d]);
		S16 x = TrackCursor.X + xoffset;
		S16 y = TrackCursor.Y + yoffset;
		
		EraseModulesUnitsFromTrack(track, x, y);
	}
}

//**********************************************
//
//**********************************************
void UpdateTrackAndCursor(TRACKDESC* track)
{
	MakeTrackFromModules(track);
	
	WipeUnitsForModulesUnderCursor(track);

	if(CurrentModule < Theme.ModuleCount)
	{
		TRACKMODULE* module = Theme.Modules[CurrentModule];
		PlotModuleOnTrack(track, module, TrackCursor.X, TrackCursor.Y, ModuleDirection, ModuleElevation);
	}
}

//**********************************************
//
//**********************************************
void CorrectCursor(CURSORDESC* cursor)
{
	if((cursor->X + cursor->XSize) < -1)
	{
		cursor->X = -1 - cursor->XSize;
	}
	if((cursor->X + cursor->XSize) > cursor->XMax)
	{
		cursor->X = cursor->XMax - cursor->XSize;
	}
	if((cursor->Y + cursor->YSize) < -1)
	{
		cursor->Y = -1 - cursor->YSize;
	}
	if((cursor->Y + cursor->YSize) > cursor->YMax)
	{
		cursor->Y = cursor->YMax - cursor->YSize;
	}
}

//**********************************************
//
//**********************************************
void SaveTrack(TRACKDESC* track)
{
	if(DescribedFileExists(track->Name))
	{
		char filename[MAX_PATH];
		GetDescribedFilename(track->Name, filename);
		WriteTDF(filename, track);
	}
	else
	{
		char filename[MAX_PATH];
		if(NextFreeFile(filename))
		{
			WriteTDF(filename, track);
		}
	}
}

//**********************************************
//
//**********************************************
void SelectModuleUnderCursor(TRACKDESC* track)
{
	U32 g = (TrackCursor.Y * track->Width) + TrackCursor.X;
	S16 xpos = TrackCursor.X - track->Modules[g].XOffset; 
	S16 ypos = TrackCursor.Y - track->Modules[g].YOffset; 
	g = (ypos * track->Width) + xpos;
	INDEX moduleid = track->Modules[g].ModuleID;
	if(moduleid != MAX_INDEX)
	{
		CurrentModule	= moduleid;
		ModuleDirection = track->Modules[g].Direction;
		ModuleElevation = track->Modules[g].Elevation;
		TrackCursor.X = xpos;
		TrackCursor.Y = ypos;
	}
}

//**********************************************
//
//**********************************************
void DecreaseTrackWidth(TRACKDESC* track)
{
	U16 xlimit = GetMaxXExtent(track) + 1;
	if(track->Width > MIN_TRACK_WIDTH)
	{
		if(xlimit < track->Width)
		{
			TRACKDESC temptrack;
			temptrack.Units = NULL;
			temptrack.Modules = NULL;
			CloneTrack(&temptrack, track);
			DestroyTrack(track);
			CreateTrack(track, temptrack.Width-1, temptrack.Height, temptrack.Name);
			CopyTrackRegion(track, &temptrack, 0, 0, 0, 0, track->Width, track->Height);
			TrackCursor.XMax = track->Width;
			TrackCursor.X = min(TrackCursor.X, track->Width-1);
			TrackCursor.AbsMax = track->Width * track->Height;
			MakeTrackFromModules(track);	//render the units from the modules
			DestroyTrack(&temptrack);
		}
	}
}

//**********************************************
//
//**********************************************
void IncreaseTrackWidth(TRACKDESC* track)
{
	if(track->Width < MAX_TRACK_WIDTH)
	{
		TRACKDESC temptrack;
		temptrack.Units = NULL;
		temptrack.Modules = NULL;
		CloneTrack(&temptrack, track);
		DestroyTrack(track);
		CreateTrack(track, temptrack.Width+1, temptrack.Height, temptrack.Name);
		CopyTrackRegion(track, &temptrack, 0, 0, 0, 0, track->Width, track->Height);
		TrackCursor.XMax = track->Width;
		TrackCursor.AbsMax = track->Width * track->Height;
		MakeTrackFromModules(track);
		DestroyTrack(&temptrack);
	}
}

//**********************************************
//
//**********************************************
void DecreaseTrackHeight(TRACKDESC* track)
{
	U16 ylimit = GetMaxYExtent(track) + 1;
	if(track->Height > MIN_TRACK_HEIGHT)
	{
		if(ylimit < track->Height)
		{
			TRACKDESC temptrack;
			temptrack.Units = NULL;
			temptrack.Modules = NULL;
			CloneTrack(&temptrack, track);
			DestroyTrack(track);
			CreateTrack(track, temptrack.Width, temptrack.Height-1, temptrack.Name);
			CopyTrackRegion(track, &temptrack, 0, 0, 0, 0, track->Width, track->Height);
			TrackCursor.YMax = track->Height;
			TrackCursor.Y = min(TrackCursor.Y, track->Height-1);
			TrackCursor.AbsMax = track->Width * track->Height;
			MakeTrackFromModules(track);
			DestroyTrack(&temptrack);
		}
	}
}

//**********************************************
//
//**********************************************
void IncreaseTrackHeight(TRACKDESC* track)
{
	if(track->Height < MAX_TRACK_HEIGHT)
	{
		TRACKDESC temptrack;
		temptrack.Units = NULL;
		temptrack.Modules = NULL;
		CloneTrack(&temptrack, track);
		DestroyTrack(track);
		CreateTrack(track, temptrack.Width, temptrack.Height+1, temptrack.Name);
		CopyTrackRegion(track, &temptrack, 0, 0, 0, 0, track->Width, track->Height);
		TrackCursor.YMax = track->Height;
		TrackCursor.AbsMax = track->Width * track->Height;
		MakeTrackFromModules(track);
		DestroyTrack(&temptrack);
	}
}

//**********************************************
//
//**********************************************
void SlideTrackRight(TRACKDESC* track)
{
	U16 minusedx;
	U16 minusedy;
	U16 maxusedx;
	U16 maxusedy;

	GetTrackExtents(track, &minusedx, &maxusedx, &maxusedy, &minusedy);
	if(maxusedx < (track->Width - 1))
	{
		U16 copywidth = (maxusedx - minusedx) + 1;
		U16 copyheight = (maxusedy - minusedy) + 1;
		TRACKDESC temptrack;
		temptrack.Units = NULL;
		temptrack.Modules = NULL;
		CloneTrack(&temptrack, track);
		WipeTrack(track);
		WipeModules(track);
		CopyTrackRegion(track, &temptrack, minusedx + 1, minusedy, minusedx, minusedy, copywidth, copyheight);
		if(TrackCursor.X < (track->Width - 1))
		{
			TrackCursor.X++;
		}
		MakeTrackFromModules(track);
		DestroyTrack(&temptrack);
	}
}

//**********************************************
//
//**********************************************
void SlideTrackLeft(TRACKDESC* track)
{
	U16 minusedx;
	U16 minusedy;
	U16 maxusedx;
	U16 maxusedy;

	MakeTrackFromModules(track);
	GetTrackExtents(track, &minusedx, &maxusedx, &maxusedy, &minusedy);
	if(minusedx > 0)
	{
		U16 copywidth = (maxusedx - minusedx) + 1;
		U16 copyheight = (maxusedy - minusedy) + 1;
		TRACKDESC temptrack;
		temptrack.Units = NULL;
		temptrack.Modules = NULL;
		CloneTrack(&temptrack, track);
		WipeTrack(track);
		WipeModules(track);
		CopyTrackRegion(track, &temptrack, minusedx - 1, minusedy, minusedx, minusedy, copywidth, copyheight);
		if(TrackCursor.X > 0)
		{
			TrackCursor.X--;
		}
		MakeTrackFromModules(track);
		DestroyTrack(&temptrack);
	}
}

//**********************************************
//
//**********************************************
void SlideTrackUp(TRACKDESC* track)
{
	U16 minusedx;
	U16 minusedy;
	U16 maxusedx;
	U16 maxusedy;

	GetTrackExtents(track, &minusedx, &maxusedx, &maxusedy, &minusedy);
	if(maxusedy < (track->Height - 1))
	{
		U16 copywidth = (maxusedx - minusedx) + 1;
		U16 copyheight = (maxusedy - minusedy) + 1;
		TRACKDESC temptrack;
		temptrack.Units = NULL;
		temptrack.Modules = NULL;
		CloneTrack(&temptrack, track);
		WipeTrack(track);
		WipeModules(track);
		CopyTrackRegion(track, &temptrack, minusedx, minusedy + 1, minusedx, minusedy, copywidth, copyheight);
		if(TrackCursor.Y < (track->Height - 1))
		{
			TrackCursor.Y++;
		}
		MakeTrackFromModules(track);
		DestroyTrack(&temptrack);
	}
}

//**********************************************
//
//**********************************************
void SlideTrackDown(TRACKDESC* track)
{
	U16 minusedx;
	U16 minusedy;
	U16 maxusedx;
	U16 maxusedy;

	GetTrackExtents(track, &minusedx, &maxusedx, &maxusedy, &minusedy);
	if(minusedy > 0)
	{
		U16 copywidth = (maxusedx - minusedx) + 1;
		U16 copyheight = (maxusedy - minusedy) + 1;
		TRACKDESC temptrack;
		temptrack.Units = NULL;
		temptrack.Modules = NULL;
		CloneTrack(&temptrack, track);
		WipeTrack(track);
		WipeModules(track);
		CopyTrackRegion(track, &temptrack, minusedx, minusedy - 1, minusedx, minusedy, copywidth, copyheight);
		if(TrackCursor.Y > 0)
		{
			TrackCursor.Y--;
		}
		MakeTrackFromModules(track);
		DestroyTrack(&temptrack);
	}
}

//**********************************************
//
//**********************************************
void NextVariant(void)
{
	CurrentModule = Theme.Lookup->Changes[CurrentModule].NextVar;
	SetTrackCursorSize();
}

//**********************************************
//
//**********************************************
void PreviousVariant(void)
{
	CurrentModule = Theme.Lookup->Changes[CurrentModule].PreviousVar;
	SetTrackCursorSize();
}

//**********************************************
//
//**********************************************
void DeleteModule(TRACKDESC* track)
{
	//take copies of the elevation/direction settings
	S16 tempelevation = ModuleElevation;
	DIRECTION tempdirection = ModuleDirection;

	//set them to the values we want
	ModuleElevation = 0;
	ModuleDirection = NORTH;

	//drop a space module
	PlaceModuleNumber(track, MODULE_SPACER);

	//now restore the settings
	ModuleElevation = tempelevation;
	ModuleDirection = tempdirection;
}
