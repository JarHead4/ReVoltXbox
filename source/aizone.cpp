
#include "revolt.h"
#include "car.h"
#include "ctrlread.h"
#include "object.h"
#include "player.h"
#include "main.h"
#include "editzone.h"
#include "geom.h"
#include "aizone.h"
#include "ainode.h"
#include "trigger.h"
#ifdef _N64
 #include "ffs_code.h"
 #include "ffs_list.h"
 #include "utils.h"
#endif

// globals

long AiZoneNum, AiZoneNumID;
AIZONE *AiZones;
AIZONE_HEADER *AiZoneHeaders;

///////////////////
// load ai zones //
///////////////////
#ifdef _PC
void LoadAiZones(char *file)
{
	long i, j, k;
	FILE *fp;
	FILE_ZONE taz;
	AIZONE zone;

// zero mem ptrs

	AiZones = NULL;
	AiZoneHeaders = NULL;

// open zone file

	fp = fopen(file, "rb");
	if (!fp)
	{
		return;
	}

// read zone num

	fread(&AiZoneNum, sizeof(AiZoneNum), 1, fp);
	if (!AiZoneNum)
	{
		fclose(fp);
		return;
	}

// alloc ram for zones

	AiZones = (AIZONE*)malloc(sizeof(AIZONE) * AiZoneNum);
	if (!AiZones)
	{
		fclose(fp);
		Box(NULL, "Can't alloc memory for AI zones!", MB_OK);
		QuitGame = TRUE;
		return;
	}

// load and convert each file zone

	for (i = 0 ; i < AiZoneNum ; i++)
	{

// read file zone

		fread(&taz, sizeof(taz), 1, fp);

// set ID

		AiZones[i].ID = taz.ID;

// set XYZ size

		AiZones[i].Size[0] = taz.Size[0];
		AiZones[i].Size[1] = taz.Size[1];
		AiZones[i].Size[2] = taz.Size[2];

// save Ccentre pos

		AiZones[i].Pos = taz.Pos;

// build 3 planes

		AiZones[i].Plane[0].v[A] = taz.Matrix.m[RX];
		AiZones[i].Plane[0].v[B] = taz.Matrix.m[RY];
		AiZones[i].Plane[0].v[C] = taz.Matrix.m[RZ];
		AiZones[i].Plane[0].v[D] = -DotProduct(&taz.Matrix.mv[R], &taz.Pos);

		AiZones[i].Plane[1].v[A] = taz.Matrix.m[UX];
		AiZones[i].Plane[1].v[B] = taz.Matrix.m[UY];
		AiZones[i].Plane[1].v[C] = taz.Matrix.m[UZ];
		AiZones[i].Plane[1].v[D] = -DotProduct(&taz.Matrix.mv[U], &taz.Pos);

		AiZones[i].Plane[2].v[A] = taz.Matrix.m[LX];
		AiZones[i].Plane[2].v[B] = taz.Matrix.m[LY];
		AiZones[i].Plane[2].v[C] = taz.Matrix.m[LZ];
		AiZones[i].Plane[2].v[D] = -DotProduct(&taz.Matrix.mv[L], &taz.Pos);
	}

// sort zones

	for (i = AiZoneNum - 1 ; i ; i--) for (j = 0 ; j < i ; j++) if (AiZones[j].ID > AiZones[j + 1].ID)
	{
		zone = AiZones[j];
		AiZones[j] = AiZones[j + 1];
		AiZones[j + 1] = zone;
	}

// set zone ID num

	AiZoneNumID = AiZones[AiZoneNum - 1].ID + 1;

// alloc memory for zone headers

	AiZoneHeaders = (AIZONE_HEADER*)malloc(sizeof(AIZONE_HEADER) * AiZoneNumID);
	if (!AiZoneHeaders)
	{
		fclose(fp);
		Box(NULL, "Can't alloc memory for AI zone headers!", MB_OK);
		QuitGame = TRUE;
		return;
	}

// setup zone headers

	for (i = 0 ; i < AiZoneNumID ; i++)
	{
		j = 0;
		while (AiZones[j].ID != i && j < AiZoneNum) j++;
		if (j == AiZoneNum)
		{
			AiZoneHeaders[i].Zones = AiZones;
			AiZoneHeaders[i].Count = 0;
		}
		else
		{	
			AiZoneHeaders[i].Zones = &AiZones[j];
			k = 0;
			while (AiZones[j].ID == i && j < AiZoneNum) j++, k++;
			AiZoneHeaders[i].Count = k;
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
void LoadAiZones(void)
{
	long		i, j, k;
	FIL			*fp;
	FILE_ZONE	taz;
	AIZONE zone;

// zero mem ptrs

	AiZones = NULL;
	AiZoneHeaders = NULL;

// open zone file

	printf("Loading AI zones.\n");
	fp = FFS_Open(FFS_TYPE_TRACK | TRK_AIZONES);
	if (!fp)
	{
		printf("...<!> Failed to open AI zone file.\n");
		return;
	}

// Read zone num
	FFS_Read(&AiZoneNum, sizeof(AiZoneNum), fp);
	if (!AiZoneNum)
	{
		printf("...<!> Number of AI Zones is zero, skipping.\n");
		FFS_Close(fp);
		return;
	}
	AiZoneNum = EndConvLong(AiZoneNum);

// Alloc ram for zones
	AiZones = (AIZONE*)malloc(sizeof(AIZONE) * AiZoneNum);
	if (!AiZones)
	{
		ERROR("AIZ", "LoadAiZones", "Failed to alloc memory for AI zones", 1);
	}

// Load and convert each file zone
	for (i = 0 ; i < AiZoneNum ; i++)
	{

// Read file zone
		FFS_Read(&taz, sizeof(taz), fp);
		taz.ID = EndConvLong(taz.ID);
		taz.Pos.v[0] = EndConvReal(taz.Pos.v[0]);
		taz.Pos.v[1] = EndConvReal(taz.Pos.v[1]);
		taz.Pos.v[2] = EndConvReal(taz.Pos.v[2]);
		for (j = 0; j < 9; j++)
		{
			taz.Matrix.m[j] = EndConvReal(taz.Matrix.m[j]);
		}
		taz.Size[0] = EndConvReal(taz.Size[0]);
		taz.Size[1] = EndConvReal(taz.Size[1]);
		taz.Size[2] = EndConvReal(taz.Size[2]);
	
// Set ID
		AiZones[i].ID = taz.ID;

// Set XYZ size
		AiZones[i].Size[0] = taz.Size[0];
		AiZones[i].Size[1] = taz.Size[1];
		AiZones[i].Size[2] = taz.Size[2];

// save centre pos
		AiZones[i].Pos.v[X] = taz.Pos.v[X];
		AiZones[i].Pos.v[Y] = taz.Pos.v[Y];
		AiZones[i].Pos.v[Z] = taz.Pos.v[Z];

// Build 3 planes
		AiZones[i].Plane[0].v[A] = taz.Matrix.m[RX];
		AiZones[i].Plane[0].v[B] = taz.Matrix.m[RY];
		AiZones[i].Plane[0].v[C] = taz.Matrix.m[RZ];
		AiZones[i].Plane[0].v[D] = -DotProduct(&taz.Matrix.mv[R], &taz.Pos);

		AiZones[i].Plane[1].v[A] = taz.Matrix.m[UX];
		AiZones[i].Plane[1].v[B] = taz.Matrix.m[UY];
		AiZones[i].Plane[1].v[C] = taz.Matrix.m[UZ];
		AiZones[i].Plane[1].v[D] = -DotProduct(&taz.Matrix.mv[U], &taz.Pos);

		AiZones[i].Plane[2].v[A] = taz.Matrix.m[LX];
		AiZones[i].Plane[2].v[B] = taz.Matrix.m[LY];
		AiZones[i].Plane[2].v[C] = taz.Matrix.m[LZ];
		AiZones[i].Plane[2].v[D] = -DotProduct(&taz.Matrix.mv[L], &taz.Pos);
	}

// sort zones
	for (i = AiZoneNum - 1 ; i ; i--) for (j = 0 ; j < i ; j++) if (AiZones[j].ID > AiZones[j + 1].ID)
	{
		zone = AiZones[j];
		AiZones[j] = AiZones[j + 1];
		AiZones[j + 1] = zone;
	}

// Set zone ID num
	AiZoneNumID = AiZones[AiZoneNum - 1].ID + 1;

// Alloc memory for zone headers
	AiZoneHeaders = (AIZONE_HEADER*)malloc(sizeof(AIZONE_HEADER) * AiZoneNumID);
	if (!AiZoneHeaders)
	{
		ERROR("AIZ", "LoadAiZones", "Unable to alloc memory for AI zone headers", 1);
	}

// Setup zone headers
	for (i = 0 ; i < AiZoneNumID ; i++)
	{
		j = 0;
		while (AiZones[j].ID != i && j < AiZoneNum) j++;
		if (j == AiZoneNum)
		{
			AiZoneHeaders[i].Zones = AiZones;
			AiZoneHeaders[i].Count = 0;
		}
		else
		{	
			AiZoneHeaders[i].Zones = &AiZones[j];
			k = 0;
			while (AiZones[j].ID == i && j < AiZoneNum) j++, k++;
			AiZoneHeaders[i].Count = k;
		}
	}

// Close file
	FFS_Close(fp);
}
#endif

///////////////////
// free ai zones //
///////////////////

void FreeAiZones(void)
{
	free(AiZones);
	free(AiZoneHeaders);
}

////////////////////////////////////////////////////
// update car zone - return TRUE if completed lap //
////////////////////////////////////////////////////

char UpdateCarAiZone(PLAYER *Player)
{
	long i, j, flag, nextzone;
	float dist;
	AIZONE *zone;
	CAR *car = &Player->car;

// quit if no zones

	if (!AiZones)
		return FALSE;

// loop thru all zones with next ID

	nextzone = (Player->CarAI.ZoneID + 1) % AiZoneNumID;
	zone = AiZoneHeaders[nextzone].Zones;

	for (i = 0 ; i < AiZoneHeaders[nextzone].Count ; i++, zone++)
	{

// test car against next zones

		flag = FALSE;
		for (j = 0 ; j < 3 ; j++)
		{
			dist = PlaneDist(&zone->Plane[j], &car->Body->Centre.Pos);
			if (dist < -zone->Size[j] || dist > zone->Size[j])
			{
				flag = TRUE;
				break;
			}
		}

// entered next zone?

		if (!flag)
		{

// yep, completed lap?

			if (!(Player->CarAI.ZoneID = nextzone))
			{
				if (car == &PLR_LocalPlayer->car)
					ResetTriggerFlags(TRIGGER_TRACK_DIR);

				return TRUE;
			}

// nope!

			return FALSE;
		}
	}

//	return false

	return FALSE;
}


