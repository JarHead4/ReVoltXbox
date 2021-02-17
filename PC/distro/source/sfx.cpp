
#include "revolt.h"
#include "sfx.h"
#include "main.h"
#include "geom.h"
#include "camera.h"
#include "timing.h"

// globals

long SoundOff;

static SFX_LOAD SfxLoad[SFX_MAX_LOAD];
static SAMPLE_SFX Sample[SFX_MAX_SAMPLES];
static SAMPLE_3D Sample3D[SFX_MAX_SAMPLES_3D];
static long SfxSampleNum, SfxLoadNum;
static DIG_DRIVER *DigDriver;
static unsigned long SfxTimerLast, SfxTimerCurrent;
static float SfxTimeMul;
static HSTREAM StreamMP3;

// generic sfx

static char *SfxGeneric[] = {
	"wavs\\moto.wav",
	"wavs\\honkgood.wav",
	"wavs\\scrape1.wav",
	"wavs\\screech.wav",
	"wavs\\woohoo.wav",
	"wavs\\pickup.wav",
	"wavs\\pickup2.wav",
	"wavs\\shock.wav",
	"wavs\\electro.wav",
	"wavs\\firework.wav",
	"wavs\\firebang.wav",
	"wavs\\ball.wav",
	"wavs\\ballhit.wav",
	"wavs\\wbomb.wav",
	"wavs\\wbombhit.wav",
	"wavs\\puttbang.wav",
	"wavs\\fuse.wav",

	NULL
};

// toy sfx

static char *SfxToy[] = {
	"wavs\\piano.wav",
	"wavs\\plane.wav",
	"wavs\\copter.wav",
	"wavs\\dragon.wav",
	"wavs\\creak.wav",
	"wavs\\train.wav",
	"wavs\\whistle.wav",

	NULL
};

// level sfx list ptrs

LEVEL_SFX SfxLevel[] = {
	"TOYLITE", SfxToy,
	"TOY2", SfxToy,

	NULL
};

///////////////////////
// init sound system //
///////////////////////

long InitSound(void)
{
	long r;
	WAVEFORMATEX format;

// quit if sound off

	if (SoundOff)
		return TRUE;

// startup

	r = AIL_startup();
	if (!r)
	{
		Box(NULL, "Failed to init sound system!", MB_OK);
		SoundOff = TRUE;
		return FALSE;
	}

// use direct sound

   AIL_set_preference(DIG_USE_WAVEOUT, NO);

// set format

   format.wFormatTag = WAVE_FORMAT_PCM;
   format.nChannels = SFX_NUM_CHANNELS;
   format.nSamplesPerSec = SFX_SAMPLE_RATE;
   format.nAvgBytesPerSec = SFX_SAMPLE_RATE * (SFX_BITS_PER_SAMPLE / 8) * SFX_NUM_CHANNELS;
   format.nBlockAlign = (SFX_BITS_PER_SAMPLE / 8) * SFX_NUM_CHANNELS;
   format.wBitsPerSample = SFX_BITS_PER_SAMPLE;

	r = AIL_waveOutOpen(&DigDriver, NULL, NULL, (WAVEFORMAT*)&format);
	if (r)
	{
		Box(NULL, "Failed to set sound format!", MB_OK);
		SoundOff = TRUE;
		return FALSE;
	}

/*HREDBOOK hr;
hr = AIL_redbook_open(0);
unsigned long start, end;

AIL_redbook_track_info(hr, 12, &start, &end);
AIL_redbook_play(hr, start - 30000, end);*/

// return OK

	return TRUE;
}

//////////////////////////
// release sound system //
//////////////////////////

void ReleaseSound(void)
{

// quit if sound off

	if (SoundOff)
		return;

// shutdown sound

	AIL_shutdown();
}

//////////////
// load sfx //
//////////////

long LoadSfx(char *levelname)
{
	long i;
	char buf[128];
	char **wavs;

// quit if sound off

	if (SoundOff)
		return TRUE;

// load generic wavs

	wavs = SfxGeneric;

	for (i = 0 ; i < SFX_MAX_LOAD ; i++)
	{

// end of list?

		if (!wavs[i]) break;

// nope, load next wav

		SfxLoad[i].Pos = AIL_file_read(wavs[i], NULL);
		if (!SfxLoad[i].Pos)
		{
			wsprintf(buf, "Can't load '%s' into slot %d!", wavs[i], i);
			Box(NULL, buf, MB_OK);
		}
		else
		{
			SfxLoad[i].Size = AIL_file_size(wavs[i]);
		}
	}

	SfxLoadNum = i;

// load level wavs

	i = 0;
	while (SfxLevel[i].Name && strcmp(SfxLevel[i].Name, levelname)) i++;

	if (SfxLevel[i].Name)
	{
		wavs = SfxLevel[i].Files;

		for (i = SfxLoadNum ; i < SFX_MAX_LOAD ; i++)
		{

// end of list?

			if (!wavs[i - SfxLoadNum]) break;

// nope, load next wav

			SfxLoad[i].Pos = AIL_file_read(wavs[i - SfxLoadNum], NULL);
			if (!SfxLoad[i].Pos)
			{
				wsprintf(buf, "Can't load '%s' into slot %d!", wavs[i - SfxLoadNum], i);
				Box(NULL, buf, MB_OK);
			}
			else
			{
				SfxLoad[i].Size = AIL_file_size(wavs[i - SfxLoadNum]);
			}
		}

		SfxLoadNum = i;
	}

// allocate sample handles

	for (i = 0 ; i < SFX_MAX_SAMPLES ; i++)
	{
		Sample[i].Handle = AIL_allocate_sample_handle(DigDriver);
		if (!Sample[i].Handle) break;
		AIL_init_sample(Sample[i].Handle);
	}

	SfxSampleNum = i;

// init 3D samples

	for (i = 0 ; i < SFX_MAX_SAMPLES_3D ; i++)
	{
		Sample3D[i].Alive = FALSE;
	}

// return OK

	return TRUE;
}

//////////////
// free sfx //
//////////////

void FreeSfx(void)
{
	long i;

// quit if sound off

	if (SoundOff)
		return;

// free all mem

	for (i = 0 ; i < SfxLoadNum ; i++)
	{
		if (SfxLoad[i].Pos)
			AIL_mem_free_lock(SfxLoad[i].Pos);
	}

// release samples

	for (i = 0 ; i <SfxSampleNum ; i++)
	{
		AIL_release_sample_handle(Sample[i].Handle);
	}
}

//////////////
// play sfx //
//////////////

void PlaySfx(long num, long vol, long pan, long freq)
{
	long i;

// quit if sound off

	if (SoundOff)
		return;

// quit if NULL sound

	if (!SfxLoad[num].Pos)
		return;

// find a free sample

	for (i = 0 ; i < SfxSampleNum ; i++)
	{
		if (AIL_sample_status(Sample[i].Handle) == SMP_DONE)
		{

// set up and play

			AIL_init_sample(Sample[i].Handle);
			AIL_set_sample_file(Sample[i].Handle, SfxLoad[num].Pos, -1);

			if (vol != -1) AIL_set_sample_volume(Sample[i].Handle, vol);
			if (pan != -1) AIL_set_sample_pan(Sample[i].Handle, pan);
			if (freq != -1) AIL_set_sample_playback_rate(Sample[i].Handle, freq);

			AIL_start_sample(Sample[i].Handle);
			break;
		}
	}
}

////////////////
// stop a sfx //
////////////////

void StopSfx(SAMPLE_SFX *sample)
{
// quit if sound off

	if (SoundOff)
		return;

// stop sfx

	AIL_end_sample(sample->Handle);
}

///////////////////
// pause all sfx //
///////////////////

void PauseAllSfx()
{
	long i;

	for (i = 0 ; i < SfxSampleNum ; i++)
	{
		if (AIL_sample_status(Sample[i].Handle) == SMP_PLAYING)
		{
			AIL_stop_sample(Sample[i].Handle);
		}
	}
}

////////////////////
// resume all sfx //
////////////////////

void ResumeAllSfx()
{
	long i;

	for (i = 0 ; i < SfxSampleNum ; i++)
	{
		if (AIL_sample_status(Sample[i].Handle) == SMP_STOPPED)
		{
			AIL_resume_sample(Sample[i].Handle);
		}
	}
}

/////////////////
// play sfx 3D //
/////////////////

void PlaySfx3D(long num, long vol, long freq, VEC *pos)
{
	long pan;

// quit if sound off

	if (SoundOff)
		return;

// get 3D settings

	GetSfxSettings3D(&vol, &freq, &pan, pos, 0);

// play sfx

	PlaySfx(num, vol, pan, freq);
}

///////////////////
// create sfx 3D //
///////////////////

SAMPLE_3D *CreateSfx3D(long num, long vol, long freq, long loop, VEC *pos)
{
	long i, j;

// quit if sound off

	if (SoundOff)
		return NULL;

// find free slot

	for (i = 0 ; i < SFX_MAX_SAMPLES_3D ; i++) if (!Sample3D[i].Alive)
	{

// got one, set misc

		Sample3D[i].Num = num;
		Sample3D[i].Vol = vol;
		Sample3D[i].Freq = freq;
		Sample3D[i].Loop = loop;
		Sample3D[i].LastDist = 0;
		Sample3D[i].Pos = *pos;

// return now if looping

		if (loop)
		{
			Sample3D[i].Sample = NULL;
			Sample3D[i].Alive = TRUE;
			return &Sample3D[i];
		}

// quit if NULL sound

		if (!SfxLoad[num].Pos)
			return NULL;

// else start sample now

		for (j = 0 ; j < SfxSampleNum ; j++)
		{
			if (AIL_sample_status(Sample[j].Handle) == SMP_DONE)
			{
				AIL_init_sample(Sample[j].Handle);
				AIL_set_sample_file(Sample[j].Handle, SfxLoad[Sample3D[i].Num].Pos, -1);
				AIL_set_sample_loop_count(Sample[j].Handle, 1);
				AIL_set_sample_volume(Sample[j].Handle, SFX_MIN_VOL);
				AIL_start_sample(Sample[j].Handle);

				Sample3D[i].Sample = &Sample[j];
				Sample3D[i].Alive = TRUE;
				return &Sample3D[i];
			}
		}

// no free sample return NULL

		return NULL;
	}

// return no

	return NULL;
}

///////////////////
// create sfx 3D //
///////////////////

void FreeSfx3D(SAMPLE_3D *sample3d)
{

// quit if sound off

	if (SoundOff)
		return;

// kill playing sample

	if (sample3d->Sample)
	{
		StopSfx(sample3d->Sample);
	}

// kill 3d sample

	sample3d->Alive = FALSE;
}

/////////////////////////
// get sfx 3D settings //
/////////////////////////

void GetSfxSettings3D(long *vol, long *freq, long *pan, VEC *pos, float vel)
{
	long per;
	float x, f;
	VEC vec1, vec2;

// quit if sound off

	if (SoundOff)
		return;

// get camera space vector

	SubVector(pos, &CAM_MainCamera->WPos, &vec1);
	TransposeRotVector(&CAM_MainCamera->WMatrix, &vec1, &vec2);

// calc vol

	f = SFX_3D_MIN_DIST / Length(&vec2) - SFX_3D_SUB_DIST;
	if (f < 0) f = 0;
	else if (f > 1) f = 1;

	FTOL(f * 256, per);
	*vol = *vol * per / 256;
	
// calc pan

	x = vec2.v[X] * RenderSettings.GeomPers / abs(vec2.v[Z]);
	x = (x * SFX_3D_PAN_MUL / REAL_SCREEN_XSIZE) + SFX_CENTRE_PAN;
	if (x < SFX_LEFT_PAN + 1) x = SFX_LEFT_PAN + 1;
	else if (x > SFX_RIGHT_PAN - 1) x = SFX_RIGHT_PAN - 1;

	FTOL(x, *pan);

// calc freq

	f = SFX_3D_SOS / (vel + SFX_3D_SOS);

	FTOL(f * 256, per);
	*freq = *freq * per / 256;
}

//////////////////////////
// change 3D sfx sample //
//////////////////////////

void ChangeSfxSample3D(SAMPLE_3D *sample3d, long sfx)
{
	if (sample3d->Sample)
	{
		StopSfx(sample3d->Sample);
		sample3d->Sample = NULL;
	}

	sample3d->Num = sfx;
}

/////////////////////
// maintain sounds //
/////////////////////

void MaintainAllSfx(void)
{
	long i, j, vol, freq, pan;
	float dist, vel;
	VEC vec;

// quit if sound off

	if (SoundOff)
		return;

// get sfx time mul

	SfxTimerLast = SfxTimerCurrent;
	SfxTimerCurrent = CurrentTimer();
	SfxTimeMul = ((float)TimerFreq / 100) / (float)(SfxTimerCurrent - SfxTimerLast);

// maintain all 3D sfx

	for (i = 0 ; i < SFX_MAX_SAMPLES_3D ; i++) if (Sample3D[i].Alive)
	{

// non-looping sfx finished?

		if (!Sample3D[i].Loop)
		{
			if (Sample3D[i].Sample)
			{
				 if (AIL_sample_status(Sample3D[i].Sample->Handle) == SMP_DONE)
					Sample3D[i].Sample = NULL;
			}
			if (!Sample3D[i].Sample)
				continue;
		}

// get 3d sound settings

		SubVector(&Sample3D[i].Pos, &CAM_MainCamera->WPos, &vec);
		dist = Length(&vec);
		vel = (dist - Sample3D[i].LastDist) * SfxTimeMul;
		Sample3D[i].LastDist = dist;

		vol = Sample3D[i].Vol;
		freq = Sample3D[i].Freq;

		GetSfxSettings3D(&vol, &freq, &pan, &Sample3D[i].Pos, vel);

// stop sample if outside range?

		if (Sample3D[i].Loop && !vol)
		{
			if (Sample3D[i].Sample)
			{
				StopSfx(Sample3D[i].Sample);
				Sample3D[i].Sample = NULL;
			}

			continue;
		}

// start sample if in range?

		if (Sample3D[i].Loop && !Sample3D[i].Sample && SfxLoad[Sample3D[i].Num].Pos)
		{
			for (j = 0 ; j < SfxSampleNum ; j++)
			{
				if (AIL_sample_status(Sample[j].Handle) == SMP_DONE)
				{
					AIL_init_sample(Sample[j].Handle);
					AIL_set_sample_file(Sample[j].Handle, SfxLoad[Sample3D[i].Num].Pos, -1);
					AIL_set_sample_loop_count(Sample[j].Handle, 0);
					AIL_set_sample_volume(Sample[j].Handle, SFX_MIN_VOL);
					AIL_start_sample(Sample[j].Handle);

					Sample3D[i].Sample = &Sample[j];
					break;
				}
			}
		}

// skip if still no sample

		if (!Sample3D[i].Sample)
			continue;

// set vol / pan / freq

		AIL_set_sample_volume(Sample3D[i].Sample->Handle, vol);
		AIL_set_sample_pan(Sample3D[i].Sample->Handle, pan);
		AIL_set_sample_playback_rate(Sample3D[i].Sample->Handle, freq);
	}
}

//////////////////
// play an .MP3 //
//////////////////

void PlayMP3(char *file)
{
	StreamMP3 = AIL_open_stream(DigDriver, file, 0);
	AIL_set_stream_loop_count(StreamMP3, 0);
	AIL_set_stream_volume(StreamMP3, SFX_MAX_VOL);
	AIL_set_stream_playback_rate(StreamMP3, 44100);
	AIL_start_stream(StreamMP3);
}

//////////////////
// stop an .MP3 //
//////////////////

void StopMP3()
{
	AIL_close_stream(StreamMP3);
}
