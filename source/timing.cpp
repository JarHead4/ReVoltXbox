
#include "revolt.h"
#include "timing.h"
#include "ctrlread.h"
#include "object.h"
#include "player.h"
#include "aizone.h"
#ifdef _PC
#include "registry.h"
#endif
#include "camera.h"
#ifdef _PC
#include "input.h"
#endif
#include "NewColl.h"
#include "Particle.h"
#include "Body.h"
#include "Wheel.h"
#include "Car.h"
#include "Geom.h"
#include "Main.h"
#include "Control.h"
#ifdef _PC
#include "Ghost.h"
#include "panel.h"
#endif
#ifdef _N64
#include "gfx.h"
#endif

// globals

unsigned long TimerLast, TimerDiff, TimerFreq, TotalRaceTime, TotalRaceStartTime, CountdownTime, CountdownEndTime;
#ifdef _PC
unsigned long TimerCurrent = CurrentTimer();
#else
unsigned long TimerCurrent = 0;
#endif
long TimeQueue, TimeLoopCount;
RECORD_ENTRY TrackRecords;

static long RecordLapFlag[MAX_RECORD_TIMES];
static long RecordRaceFlag[MAX_RECORD_TIMES];


///////////////////////
// update TimeFactor //
///////////////////////

#ifdef _N64
void UpdateTimeFactor(void)
{
// set last / current / diff timer

	TimerLast = TimerCurrent;
	TimerCurrent = CurrentTimer();
	TimerDiff = TimerCurrent - TimerLast;

// set TimeFactor
	TimeFactor = (REAL)(TimerDiff) / 1000 * 60;
	if (TimeFactor > 15) TimeFactor = 15;

	TimeStep = TimeFactor / PhysicalFramesPerSecond;
}
#endif

#ifdef _PC
void UpdateTimeFactor(void)
{

// set last / current / diff timer

#if RECORD_AVI
	TimerDiff = MS2TIME(1000 / 30);
	//TimerDiff = MS2TIME(1000 / 200);
	TimerLast = TimerCurrent;
	TimerCurrent += TimerDiff;
#else
	TimerLast = TimerCurrent;
	TimerCurrent = CurrentTimer();
	TimerDiff = TimerCurrent - TimerLast;
#endif

// set TimeLoopCount

	TimeQueue += TimerDiff;
	TimeLoopCount = TimeQueue / (TimerFreq / 72);
	TimeQueue %= (TimerFreq / 72);
	if (TimeLoopCount > 10) TimeLoopCount = 10;

// set TimeFactor

	TimeFactor = (float)(TimerDiff) / ((float)TimerFreq / 72.0f);
	if (TimeFactor > 10) TimeFactor = 10;

// set time step

#if FIXED_TIME_STEP
	//TimeStep = TimeFactor / 72.0f;
	TimeStep = (REAL)1.0f / 100.0f;
#else
	TimeStep = TimeFactor / 72.0f;
#endif
}
#endif


//////////////////////////
// return current timer //
//////////////////////////

#ifdef _PC
unsigned long CurrentTimer(void)
{
	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);
	return time.LowPart;
}
#endif

#ifdef _N64
unsigned long CurrentTimer(void)
{
	return(OS_CYCLES_TO_USEC(osGetTime()) / 1000);
}
#endif
////////////////////////
// update race timers //
////////////////////////

void UpdateRaceTimers(void)
{
	unsigned long time, countdown;
	CAR *car;
	PLAYER *player;

// get current timer

	time = TimerCurrent;

// counting down?

	if (CountdownTime)
	{
		countdown = (CountdownEndTime - time);
		CountdownTime = TIME2MS(countdown);

		if (countdown & 0x80000000)
		{
			TotalRaceStartTime = time + countdown;
			CountdownTime = 0;

			for (player = PLR_PlayerHead ; player ; player = player->next)
			{
				player->car.CurrentLapStartTime = TotalRaceStartTime;
			}
		}
		return;
	}

// update race time

#if	RECORD_AVI
	TotalRaceTime += (1000 / 30);
#else
	if (GameSettings.Paws) TotalRaceStartTime += TimerDiff;
	TotalRaceTime = TIME2MS(time - TotalRaceStartTime);
#endif

// loop thru cars

	for (player = PLR_PlayerHead ; player ; player = player->next)
	{
		car = &player->car;

		switch (player->type)
		{

// LOCAL PLAYER

		case PLAYER_LOCAL:

// get lap timer

#if	RECORD_AVI
			car->CurrentLapTime += (1000 / 30);
#else
			if (GameSettings.Paws) car->CurrentLapStartTime += TimerDiff;
			car->CurrentLapTime = TIME2MS(time - car->CurrentLapStartTime);
#endif

// end of lap?

			if (UpdateCarAiZone(player))
			{

// yep, trigger end split time

				TriggerSplit(player, 0, -1, &LookVec);

// update lap times

				car->Laps++;
				car->NextSplit = 0;

				car->LastLapTime = car->CurrentLapTime;
				car->CurrentLapTime = 0;
				car->CurrentLapStartTime = time;

// Store times in ghosts data info
			
#ifdef _PC
				EndGhostData(PLR_LocalPlayer);
#endif

// best lap?

				if (car->LastLapTime < car->BestLapTime)
					car->BestLapTime = car->LastLapTime;

				if (car->AllowedBestTime) {
					CheckForBestLap(car);
				}

// reinitalise the ghost car
#ifdef _PC
				InitGhostData(PLR_LocalPlayer);
				InitBestGhostData();
#endif
// best race?

				if (car->Laps == 5)
				{
					car->LastRaceTime = TotalRaceTime;
					TotalRaceTime = 0;
					TotalRaceStartTime = time;

					if (car->LastRaceTime < car->BestRaceTime)
						car->BestRaceTime = car->LastRaceTime;

					if (car->AllowedBestTime) {
						CheckForBestRace(car);
					}
				}

			}

			break;

// GHOST PLAYER

#ifdef _PC
		case PLAYER_GHOST:

			if (Keys[DIK_6]) {
				if (Keys[DIK_LSHIFT]) {
					car->CurrentLapStartTime -= TimerDiff;
				} else {
					car->CurrentLapStartTime += TimerDiff;
				}
			}
			if (Keys[DIK_7]) {
					car->CurrentLapStartTime += TimerDiff * 2;
			}

#if RECORD_AVI
			car->CurrentLapTime += (1000 / 30);
#else
			if (GameSettings.Paws) car->CurrentLapStartTime += TimerDiff;
			car->CurrentLapTime = TIME2MS(time - car->CurrentLapStartTime);
#endif

			break;
#endif

// CPU PLAYER

		case PLAYER_CPU:
			break;

// DEFAULT PLAYER

		default:
			break;

		}
	}
}

/////////////////////////////
// check for best lap time //
/////////////////////////////

void CheckForBestLap(CAR *car)
{
	bool newGhostCar;
	unsigned long i, j;

// new ghost car?
#ifdef _PC
	if (car->LastLapTime < GHO_BestGhostInfo->Time[GHOST_LAP_TIME]) {
		newGhostCar = TRUE;
	} else {
		newGhostCar = FALSE;
	}
#endif
// loop thru best times

	for (i = 0 ; i < MAX_RECORD_TIMES ; i++)
	{

// check against car last

		if (car->LastLapTime < TrackRecords.RecordLap[i].Time)
		{

// beat it, copy lower ones down one

#ifdef _PC
			PlaySfx(SFX_RECORD, SFX_MAX_VOL, SFX_CENTRE_PAN, 22050);
#endif
			for (j = MAX_RECORD_TIMES - 1 ; j > i ; j--)
			{
				TrackRecords.RecordLap[j] = TrackRecords.RecordLap[j - 1];
				RecordLapFlag[j] = RecordLapFlag[j - 1];
			}

// set new record

			TrackRecords.RecordLap[i].Time = car->LastLapTime;
			memcpy(TrackRecords.RecordLap[i].Car, CarInfo[car->CarID].Name, CAR_NAMELEN);
#ifdef _PC
			memcpy(TrackRecords.RecordLap[i].Player, RegistrySettings.PlayerName, MAX_PLAYER_NAME);
#endif
			RecordLapFlag[i] = TRUE;

// best time?

			if (i == 0)
			{
				for (j = 0 ; j < MAX_SPLIT_TIMES ; j++)
					TrackRecords.SplitTime[j] = car->SplitTime[j];

				newGhostCar = TRUE;
			}

			break;
		}
	}

// store ghost car as best ghost car
#ifdef _PC
	if (newGhostCar || !GHO_GhostExists) {
		SwitchGhostDataStores();
	}
#endif
}

//////////////////////////////
// check for best race time //
//////////////////////////////

void CheckForBestRace(CAR *car)
{
	unsigned long i, j;

// loop thru best times

	for (i = 0 ; i < MAX_RECORD_TIMES ; i++)
	{

// check against car last

		if (car->LastRaceTime < TrackRecords.RecordRace[i].Time)
		{

// beat it, copy lower ones down one

			for (j = MAX_RECORD_TIMES - 1 ; j > i ; j--)
			{
				TrackRecords.RecordRace[j] = TrackRecords.RecordRace[j - 1];
				RecordRaceFlag[j] = RecordRaceFlag[j - 1];
			}

// set new record

			TrackRecords.RecordRace[i].Time = car->LastRaceTime;
			memcpy(TrackRecords.RecordRace[i].Car, CarInfo[car->CarID].Name, CAR_NAMELEN);
#ifdef _PC
			memcpy(TrackRecords.RecordRace[i].Player, RegistrySettings.PlayerName, MAX_PLAYER_NAME);
#endif
			RecordRaceFlag[i] = TRUE;

			break;
		}
	}
}


#ifdef _PC
//////////////////////
// load track times //
//////////////////////

void LoadTrackTimes(LEVELINFO *lev)
{
	long i;
	FILE *fp;

// set time defaults

	for (i = 0 ; i < MAX_SPLIT_TIMES ; i++)
	{
		TrackRecords.SplitTime[i] = MAKE_TIME(60, 0, 0);
	}

	for (i = 0 ; i < MAX_RECORD_TIMES ; i++)
	{
		TrackRecords.RecordLap[i].Time = MAKE_TIME(5, 0, 0);
		wsprintf(TrackRecords.RecordLap[i].Player, "Player");
		wsprintf(TrackRecords.RecordLap[i].Car, "Car");

		TrackRecords.RecordRace[i].Time = MAKE_TIME(60, 0, 0);
		wsprintf(TrackRecords.RecordRace[i].Player, "Player");
		wsprintf(TrackRecords.RecordRace[i].Car, "Car");

		RecordLapFlag[i] = FALSE;
		RecordRaceFlag[i] = FALSE;
	}

// read in record file

	if (GameSettings.Mirrored)
		fp = fopen(GetLevelFilename(RECORDS_FILENAME_MIRRORED, FILENAME_GAME_SETTINGS), "rb");
	else
		fp = fopen(GetLevelFilename(RECORDS_FILENAME, FILENAME_GAME_SETTINGS), "rb");
	if (!fp) return;

	fread(&TrackRecords, sizeof(TrackRecords), 1, fp);
	fclose(fp);
}

//////////////////////
// save track times //
//////////////////////

void SaveTrackTimes(LEVELINFO *lev)
{
	long j, k, l, update, updateGhost;
	FILE *fp;
	char buf[128];
	RECORD_ENTRY record;

// open records file

	if (GameSettings.Mirrored)
		fp = fopen(GetLevelFilename(RECORDS_FILENAME_MIRRORED, FILENAME_GAME_SETTINGS), "rb+");
	else
		fp = fopen(GetLevelFilename(RECORDS_FILENAME, FILENAME_GAME_SETTINGS), "rb+");

// if none, create new

	if (!fp)
	{
		if (GameSettings.Mirrored)
			fp = fopen(GetLevelFilename(RECORDS_FILENAME_MIRRORED, FILENAME_GAME_SETTINGS), "wb");
		else
			fp = fopen(GetLevelFilename(RECORDS_FILENAME, FILENAME_GAME_SETTINGS), "wb");


		if (!fp)
		{
			Box(buf, "Failed to create record file", MB_OK);
			return;
		}

		fwrite(&TrackRecords, sizeof(TrackRecords), 1, fp);
		fclose(fp);
		SaveGhostData(lev);
		return;
	}

// else munge times with file

	updateGhost = FALSE;
	update = FALSE;
	fread(&record, sizeof(record), 1, fp);

// check lap times

	for (j = 0 ; j < MAX_RECORD_TIMES ; j++)
	{
		if (RecordLapFlag[j])
		{
			RecordLapFlag[j] = FALSE;

			for (k = 0 ; k < MAX_RECORD_TIMES ; k++)
			{
				if (TrackRecords.RecordLap[j].Time < record.RecordLap[k].Time)
				{
					for (l = MAX_RECORD_TIMES - 1 ; l > k ; l--) record.RecordLap[l] = record.RecordLap[l - 1];
					record.RecordLap[k] = TrackRecords.RecordLap[j];
					update = TRUE;

// best time?

					if (k == 0)
					{
						for (l = 0 ; l < MAX_SPLIT_TIMES ; l++)
							record.SplitTime[l] = TrackRecords.SplitTime[l];

						updateGhost = TRUE;
					}
					break;
				}
			}
		}
	}

// check race times

	for (j = 0 ; j < MAX_RECORD_TIMES ; j++)
	{
		if (RecordRaceFlag[j])
		{
			RecordRaceFlag[j] = FALSE;

			for (k = 0 ; k < MAX_RECORD_TIMES ; k++)
			{
				if (TrackRecords.RecordRace[j].Time < record.RecordRace[k].Time)
				{
					for (l = MAX_RECORD_TIMES - 1 ; l > k ; l--) record.RecordRace[l] = record.RecordRace[l - 1];
					record.RecordRace[k] = TrackRecords.RecordRace[j];
					update = TRUE;
					break;
				}
			}
		}
	}

// update record?

	if (update)
	{
		fseek(fp, 0, SEEK_SET);
		fwrite(&record, sizeof(record), 1, fp);
	}

// close file

	fclose(fp);

// update the ghost if required

	if (updateGhost) 
	{
		SaveGhostData(&LevelInf[GameSettings.Level]);
	} else {
		LoadGhostData(&LevelInf[GameSettings.Level]);
	}
}

#endif