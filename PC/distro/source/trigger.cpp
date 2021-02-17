
#include "revolt.h"
#include "trigger.h"
#include "edittrig.h"
#include "main.h"
#include "geom.h"
#include "car.h"
#include "ctrlread.h"
#include "object.h"
#include "control.h"
#include "player.h"
#include "piano.h"
#include "panel.h"
#ifdef _PC
#include "camera.h"
#include "ai_car.h"
#endif
#ifdef _N64
#include "ffs_code.h"
#include "ffs_list.h"
#include "utils.h"
#endif

// globals

long TriggerNum;
TRIGGER *Triggers;

// trigger info - handler, local only

static TRIGGER_INFO TriggerInfo[] = {
	TriggerPiano, FALSE,
	TriggerSplit, TRUE,
	TriggerTrackDir, TRUE,
#ifdef _PC
	TriggerCamera, FALSE,
	CAI_TriggerAiHome, TRUE,
#endif
};

///////////////////
// load triggers //
///////////////////
#ifdef _PC
void LoadTriggers(char *file)
{
	long i;
	REAL time;
	FILE *fp;
	FILE_TRIGGER ftri;
	VEC vec;

// zero misc

	TriggerNum = 0;
	Triggers = NULL;

// open trigger file

	fp = fopen(file, "rb");
	if (!fp)
	{
		return;
	}

// read trigger num

	fread(&TriggerNum, sizeof(TriggerNum), 1, fp);
	if (!TriggerNum)
	{
		fclose(fp);
		return;
	}

// alloc ram for triggers

	Triggers = (TRIGGER*)malloc(sizeof(TRIGGER) * TriggerNum);
	if (!Triggers)
	{
		fclose(fp);
		Box(NULL, "Can't alloc memory for triggers!", MB_OK);
		QuitGame = TRUE;
		return;
	}

// load and convert each trigger

	for (i = 0 ; i < TriggerNum ; i++)
	{

// read file zone

		fread(&ftri, sizeof(ftri), 1, fp);

// set misc

		Triggers[i].ID = ftri.ID;
		Triggers[i].Flag = ftri.Flag;
		Triggers[i].GlobalFirst = TRUE;
		Triggers[i].Function = TriggerInfo[ftri.ID].Func;
		Triggers[i].LocalPlayerOnly = TriggerInfo[ftri.ID].LocalPlayerOnly;

// set XYZ size

		Triggers[i].Size[0] = ftri.Size[0];
		Triggers[i].Size[1] = ftri.Size[1];
		Triggers[i].Size[2] = ftri.Size[2];

// build 3 planes

		Triggers[i].Plane[0].v[A] = ftri.Matrix.m[RX];
		Triggers[i].Plane[0].v[B] = ftri.Matrix.m[RY];
		Triggers[i].Plane[0].v[C] = ftri.Matrix.m[RZ];
		Triggers[i].Plane[0].v[D] = -DotProduct(&ftri.Matrix.mv[R], &ftri.Pos);

		Triggers[i].Plane[1].v[A] = ftri.Matrix.m[UX];
		Triggers[i].Plane[1].v[B] = ftri.Matrix.m[UY];
		Triggers[i].Plane[1].v[C] = ftri.Matrix.m[UZ];
		Triggers[i].Plane[1].v[D] = -DotProduct(&ftri.Matrix.mv[U], &ftri.Pos);

		Triggers[i].Plane[2].v[A] = ftri.Matrix.m[LX];
		Triggers[i].Plane[2].v[B] = ftri.Matrix.m[LY];
		Triggers[i].Plane[2].v[C] = ftri.Matrix.m[LZ];
		Triggers[i].Plane[2].v[D] = -DotProduct(&ftri.Matrix.mv[L], &ftri.Pos);

// set vec?

		vec.v[Y] = 0;

		if (Triggers[i].ID == TRIGGER_AIHOME)
		{
			if (Triggers[i].Flag < 8)
			{
				time = (float)Triggers[i].Flag / 8.0f;
				vec.v[X] = -Triggers[i].Size[X];
				vec.v[Z] = -Triggers[i].Size[Z] + Triggers[i].Size[Z] * time * 2;
			}
			else if (Triggers[i].Flag < 16)
			{
				time = (float)(Triggers[i].Flag - 8) / 8.0f;
				vec.v[X] = -Triggers[i].Size[X] + Triggers[i].Size[X] * time * 2;
				vec.v[Z] = Triggers[i].Size[Z];
			}
			else if (Triggers[i].Flag < 24)
			{
				time = (float)(Triggers[i].Flag - 16) / 8.0f;
				vec.v[X] = Triggers[i].Size[X];
				vec.v[Z] = Triggers[i].Size[Z] - Triggers[i].Size[Z] * time * 2;
			}
			else if (Triggers[i].Flag < 32)
			{
				time = (float)(Triggers[i].Flag - 24) / 8.0f;
				vec.v[X] = Triggers[i].Size[X] - Triggers[i].Size[X] * time * 2;
				vec.v[Z] = -Triggers[i].Size[Z];
			}

			RotTransVector(&ftri.Matrix, &ftri.Pos, &vec, &Triggers[i].Vector);
		}
	}

// close file

	fclose(fp);
}
#endif


//
// N64 version
//

#ifdef _N64
void LoadTriggers()
{
	long			i, j;
	REAL			time;
	FIL				*fp;
	FILE_TRIGGER 	ftri;
	VEC				vec;

// zero misc
	TriggerNum = 0;
	Triggers = NULL;

	printf("Loading trigger data...\n");
// open trigger file
	fp = FFS_Open(FFS_TYPE_TRACK | TRK_TRIGGERS);
	if (!fp)
	{
		printf("...unable to open trigger file.\n");
		return;
	}

// read trigger num

	FFS_Read(&TriggerNum, sizeof(TriggerNum), fp);
	if (!TriggerNum)
	{
		printf("...unable to read trigger file.\n");
		FFS_Close(fp);
		return;
	}
	TriggerNum = EndConvLong(TriggerNum);

// alloc ram for triggers
	Triggers = (TRIGGER *)malloc(sizeof(TRIGGER) * TriggerNum);
	if (!Triggers)
	{
		ERROR("TRG", "LoadTriggers", "Unable to alloc memory for triggers", 1);
	}

// load and convert each trigger
	for (i = 0 ; i < TriggerNum ; i++)
	{
// read file zone
		FFS_Read(&ftri, sizeof(ftri), fp);
		ftri.ID = EndConvLong(ftri.ID);
		ftri.Flag = EndConvLong(ftri.Flag);
		ftri.Pos.v[0] = EndConvReal(ftri.Pos.v[0]);
		ftri.Pos.v[1] = EndConvReal(ftri.Pos.v[1]);
		ftri.Pos.v[2] = EndConvReal(ftri.Pos.v[2]);
		for (j = 0; j < 9; j++)
		{
			ftri.Matrix.m[j] = EndConvReal(ftri.Matrix.m[j]);
		}
		ftri.Size[0] = EndConvReal(ftri.Size[0]);
		ftri.Size[1] = EndConvReal(ftri.Size[1]);
		ftri.Size[2] = EndConvReal(ftri.Size[2]);

// set misc
		Triggers[i].ID = ftri.ID;
		Triggers[i].Flag = ftri.Flag;
		Triggers[i].GlobalFirst = TRUE;
		Triggers[i].Function = TriggerInfo[ftri.ID].Func;
		Triggers[i].LocalPlayerOnly = TriggerInfo[ftri.ID].LocalPlayerOnly;

// set XYZ size
		Triggers[i].Size[0] = ftri.Size[0];
		Triggers[i].Size[1] = ftri.Size[1];
		Triggers[i].Size[2] = ftri.Size[2];

// build 3 planes
		Triggers[i].Plane[0].v[A] = ftri.Matrix.m[RX];
		Triggers[i].Plane[0].v[B] = ftri.Matrix.m[RY];
		Triggers[i].Plane[0].v[C] = ftri.Matrix.m[RZ];
		Triggers[i].Plane[0].v[D] = -DotProduct(&ftri.Matrix.mv[R], &ftri.Pos);

		Triggers[i].Plane[1].v[A] = ftri.Matrix.m[UX];
		Triggers[i].Plane[1].v[B] = ftri.Matrix.m[UY];
		Triggers[i].Plane[1].v[C] = ftri.Matrix.m[UZ];
		Triggers[i].Plane[1].v[D] = -DotProduct(&ftri.Matrix.mv[U], &ftri.Pos);

		Triggers[i].Plane[2].v[A] = ftri.Matrix.m[LX];
		Triggers[i].Plane[2].v[B] = ftri.Matrix.m[LY];
		Triggers[i].Plane[2].v[C] = ftri.Matrix.m[LZ];
		Triggers[i].Plane[2].v[D] = -DotProduct(&ftri.Matrix.mv[L], &ftri.Pos);

// set vec?
		vec.v[Y] = 0;

		if (Triggers[i].ID == TRIGGER_AIHOME)
		{
			if (Triggers[i].Flag < 8)
			{
				time = (float)Triggers[i].Flag / 8.0f;
				vec.v[X] = -Triggers[i].Size[X];
				vec.v[Z] = -Triggers[i].Size[Z] + Triggers[i].Size[Z] * time * 2;
			}
			else if (Triggers[i].Flag < 16)
			{
				time = (float)(Triggers[i].Flag - 8) / 8.0f;
				vec.v[X] = -Triggers[i].Size[X] + Triggers[i].Size[X] * time * 2;
				vec.v[Z] = Triggers[i].Size[Z];
			}
			else if (Triggers[i].Flag < 24)
			{
				time = (float)(Triggers[i].Flag - 16) / 8.0f;
				vec.v[X] = Triggers[i].Size[X];
				vec.v[Z] = Triggers[i].Size[Z] - Triggers[i].Size[Z] * time * 2;
			}
			else if (Triggers[i].Flag < 32)
			{
				time = (float)(Triggers[i].Flag - 24) / 8.0f;
				vec.v[X] = Triggers[i].Size[X] - Triggers[i].Size[X] * time * 2;
				vec.v[Z] = -Triggers[i].Size[Z];
			}
			RotTransVector(&ftri.Matrix, &ftri.Pos, &vec, &Triggers[i].Vector);
		}
	}

// close file
	FFS_Close(fp);
}
#endif


///////////////////
// free triggers //
///////////////////

void FreeTriggers(void)
{
	free(Triggers);
}

////////////////////
// check triggers //
////////////////////

void CheckTriggers(void)
{
	long i, k, skip, flag;
	float dist;
	TRIGGER *trigger;
	CAR *car;
	VEC *pos;
	PLAYER *player;

// loop thru all triggers

	trigger = Triggers;
	for (i = 0 ; i < TriggerNum ; i++, trigger++) if (trigger->ID < TRIGGER_NUM)
	{

// loop thru players

		for (player = PLR_PlayerHead ; player != NULL ; player = player->next)
		{
			car = &player->car;

			if (trigger->LocalPlayerOnly && car != &PLR_LocalPlayer->car)
				continue;

// inside trigger?

			pos = &car->Body->Centre.Pos;

			skip = FALSE;
			for (k = 0 ; k < 3 ; k++)
			{
				dist = PlaneDist(&trigger->Plane[k], pos);
				if (dist < -trigger->Size[k] || dist > trigger->Size[k])
				{
					skip = TRUE;
					break;
				}
			}

// yep

			if (!skip)
			{
				flag = 0;
				if (trigger->GlobalFirst) flag |= TRIGGER_GLOBAL_FIRST;
				if (trigger->FrameStamp != FrameCount) flag |= TRIGGER_FRAME_FIRST;

				if (trigger->Function) trigger->Function(player, flag, trigger->Flag, &trigger->Vector);

				trigger->FrameStamp = FrameCount;
				trigger->GlobalFirst = FALSE;
			}
		}
	}
}

/////////////////////////////////////////
// reset trigger flags of a given type //
/////////////////////////////////////////

void ResetTriggerFlags(long ID)
{
	long i;

	for (i = 0 ; i < TriggerNum ; i++)
	{
		if (Triggers[i].ID == ID)
		{
			Triggers[i].FrameStamp--;
			Triggers[i].GlobalFirst = TRUE;
		}
	}
}
