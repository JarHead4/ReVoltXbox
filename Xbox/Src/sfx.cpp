//$NOTE: not directly using this file, but it has useful info like
/// sfx list, redbook track names, etc.  So we'll keep it here for now.
//$TODO: remove this file when we don't need it any more.

#include "revolt.h"
#include "sfx.h"
#include "main.h"
#include "geom.h"
#include "camera.h"
#include "timing.h"
#include "settings.h"

#include "SoundEffectEngine.h"  //$ADDITION
#include "MusicManager.h"       //$ADDITION

// globals

long SoundOff, ChannelsFree, ChannelsUsed, ChannelsUsed3D;
long SfxMasterVol = 0;//SFX_MAX_VOL;
long MusicMasterVol = 0;//SFX_MAX_VOL;
long SfxSampleNum, SfxLoadNum;
char RedbookDeviceLetter = 'c';

static WAVEFORMATEX WaveFormat;
static SFX_LOAD SfxLoad[SFX_MAX_LOAD];
static SAMPLE_SFX Sample[SFX_MAX_SAMPLES];
static SAMPLE_3D Sample3D[SFX_MAX_SAMPLES_3D];
//$REMOVEDstatic DIG_DRIVER *DigDriver = NULL;
static unsigned long SfxTimerLast, SfxTimerCurrent;
static float SfxTimeMul;
static void *Mp3Pos = NULL;
static long SfxPause = FALSE;
//$REMOVEDstatic HREDBOOK RedbookDevice = 0;
static long RedbookTrackNum;
static REDBOOK_TRACK RedbookTrack[REDBOOK_TRACK_NUM];
static unsigned long RedbookStartTrack, RedbookEndTrack, RedbookLoop, RedbookPlaying;
static REDBOOK_FADE RedbookFade = REDBOOK_FADE_NONE;
static float RedbookFadeVol;
static float RedbookStatusTimer = 0.0f;

#ifdef OLD_AUDIO
static void GetRedbookTOC(void);

// generic sfx

static char *SfxGeneric[] = {
//$MODIFIED
//    "wavs\\moto.wav",
//    "wavs\\petrol.wav",
//    "wavs\\clockwrk.wav",
//    "wavs\\ufo.wav",
//    "wavs\\honkgood.wav",
//    "wavs\\scrape.wav",
//    "wavs\\skid_normal.wav",
//    "wavs\\skid_normal.wav",
//    "wavs\\skid_rough.wav",
//    "wavs\\pickup.wav",
//    "wavs\\pickgen.wav",
//    "wavs\\shock.wav",
//    "wavs\\shockfire.wav",
//    "wavs\\electro.wav",
//    "wavs\\electrozap.wav",
//    "wavs\\firefire.wav",
//    "wavs\\firebang.wav",
//    "wavs\\balldrop.wav",
//    "wavs\\ball.wav",
//    "wavs\\hit1.wav",
//    "wavs\\hit2.wav",
//    "wavs\\wbomb.wav",
//    "wavs\\wbombfire.wav",
//    "wavs\\wbombhit.wav",
//    "wavs\\wbombbounce.wav",
//    "wavs\\puttbang.wav",
//    "wavs\\fuse.wav",
//    "wavs\\oildrop.wav",
//    "wavs\\countdown.wav",
//    "wavs\\turbo.wav",
//    "wavs\\servo.wav",
//    "wavs\\menuNext.wav",
//    "wavs\\menuPrev.wav",
//    "wavs\\menuUpDown.wav",
//    "wavs\\menuLeftRight.wav",
//    "wavs\\beachball.wav",
//    "wavs\\lightflk.wav",
//    "wavs\\bottle.wav",
//    "wavs\\boxslide.wav",
//    "wavs\\starfire.wav",
//    "wavs\\tvstatic.wav",
//    "wavs\\splash.wav",
//    "wavs\\honka.wav",
    "D:\\wavs\\moto.wav",
    "D:\\wavs\\petrol.wav",
    "D:\\wavs\\clockwrk.wav",
    "D:\\wavs\\ufo.wav",
    "D:\\wavs\\honkgood.wav",
    "D:\\wavs\\scrape.wav",
    "D:\\wavs\\skid_normal.wav",  //$NOTE(cprince): SFX_SKID_NONE has same wav as SFX_SKID_NORMAL
    "D:\\wavs\\skid_normal.wav",  //$NOTE(cprince): this corresponds to SFX_SKID_NORMAL
    "D:\\wavs\\skid_rough.wav",
    "D:\\wavs\\pickup.wav",
    "D:\\wavs\\pickgen.wav",
    "D:\\wavs\\shock.wav",
    "D:\\wavs\\shockfire.wav",
    "D:\\wavs\\electro.wav",
    "D:\\wavs\\electrozap.wav",
    "D:\\wavs\\firefire.wav",
    "D:\\wavs\\firebang.wav",
    "D:\\wavs\\balldrop.wav",
    "D:\\wavs\\ball.wav",
    "D:\\wavs\\hit1.wav",
    "D:\\wavs\\hit2.wav",
    "D:\\wavs\\wbomb.wav",
    "D:\\wavs\\wbombfire.wav",
    "D:\\wavs\\wbombhit.wav",
    "D:\\wavs\\wbombbounce.wav",
    "D:\\wavs\\puttbang.wav",
    "D:\\wavs\\fuse.wav",
    "D:\\wavs\\oildrop.wav",
    "D:\\wavs\\countdown.wav",
    "D:\\wavs\\turbo.wav",
    "D:\\wavs\\servo.wav",
    "D:\\wavs\\menuNext.wav",
    "D:\\wavs\\menuPrev.wav",
    "D:\\wavs\\menuUpDown.wav",
    "D:\\wavs\\menuLeftRight.wav",
    "D:\\wavs\\beachball.wav",
    "D:\\wavs\\lightflk.wav",
    "D:\\wavs\\bottle.wav",
    "D:\\wavs\\boxslide.wav",
    "D:\\wavs\\starfire.wav",
    "D:\\wavs\\tvstatic.wav",
    "D:\\wavs\\splash.wav",
    "D:\\wavs\\honka.wav",
//$END_MODIFICATIONS

    NULL
};

// toy sfx

static char *SfxToy[] = {
//$MODIFIED
//    "wavs\\toy\\piano.wav",
//    "wavs\\toy\\plane.wav",
//    "wavs\\toy\\copter.wav",
//    "wavs\\toy\\dragon.wav",
//    "wavs\\toy\\creak.wav",
//    "wavs\\toy\\train.wav",
//    "wavs\\toy\\whistle.wav",
//    "wavs\\toy\\arcade.wav",
//    "wavs\\toy\\toybrick.wav",
    "D:\\wavs\\toy\\piano.wav",
    "D:\\wavs\\toy\\plane.wav",
    "D:\\wavs\\toy\\copter.wav",
    "D:\\wavs\\toy\\dragon.wav",
    "D:\\wavs\\toy\\creak.wav",
    "D:\\wavs\\toy\\train.wav",
    "D:\\wavs\\toy\\whistle.wav",
    "D:\\wavs\\toy\\arcade.wav",
    "D:\\wavs\\toy\\toybrick.wav",
//$END_MODIFICATIONS

    NULL
};

// neighbourhood sfx

static char *SfxNhood[] = {
//$MODIFIED
//    "wavs\\hood\\basketball.wav",
//    "wavs\\hood\\birds1.wav",
//    "wavs\\hood\\birds2.wav",
//    "wavs\\hood\\birds3.wav",
//    "wavs\\hood\\dogbark.wav",
//    "wavs\\hood\\kids.wav",
//    "wavs\\hood\\sprink.wav",
//    "wavs\\hood\\tv.wav",
//    "wavs\\hood\\lawnmower.wav",
//    "wavs\\hood\\digger.wav",
//    "wavs\\hood\\stream.wav",
//    "wavs\\hood\\cityamb2.wav",
//    "wavs\\hood\\roadcone.wav",
    "D:\\wavs\\hood\\basketball.wav",
    "D:\\wavs\\hood\\birds1.wav",
    "D:\\wavs\\hood\\birds2.wav",
    "D:\\wavs\\hood\\birds3.wav",
    "D:\\wavs\\hood\\dogbark.wav",
    "D:\\wavs\\hood\\kids.wav",
    "D:\\wavs\\hood\\sprink.wav",
    "D:\\wavs\\hood\\tv.wav",
    "D:\\wavs\\hood\\lawnmower.wav",
    "D:\\wavs\\hood\\digger.wav",
    "D:\\wavs\\hood\\stream.wav",
    "D:\\wavs\\hood\\cityamb2.wav",
    "D:\\wavs\\hood\\roadcone.wav",
//$END_MODIFICATIONS

    NULL,
};

// garden sfx

static char *SfxGarden[] = {
//$MODIFIED
//    "wavs\\garden\\tropics2.wav",
//    "wavs\\garden\\tropics3.wav",
//    "wavs\\garden\\tropics4.wav",
//    "wavs\\garden\\stream.wav",
//    "wavs\\garden\\animal1.wav",
//    "wavs\\garden\\animal2.wav",
//    "wavs\\garden\\animal3.wav",
//    "wavs\\garden\\animal4.wav",
    "D:\\wavs\\garden\\tropics2.wav",
    "D:\\wavs\\garden\\tropics3.wav",
    "D:\\wavs\\garden\\tropics4.wav",
    "D:\\wavs\\garden\\stream.wav",
    "D:\\wavs\\garden\\animal1.wav",
    "D:\\wavs\\garden\\animal2.wav",
    "D:\\wavs\\garden\\animal3.wav",
    "D:\\wavs\\garden\\animal4.wav",
//$END_MODIFICATIONS

    NULL,
};

// muse sfx

static char *SfxMuse[] = {
//$MODIFIED
//    "wavs\\muse\\museumam.wav",
//    "wavs\\muse\\laserhum.wav",
//    "wavs\\muse\\alarm2.wav",
//    "wavs\\muse\\escalate.wav",
//    "wavs\\muse\\rotating.wav",
//    "wavs\\muse\\largdoor.wav",
    "D:\\wavs\\muse\\museumam.wav",
    "D:\\wavs\\muse\\laserhum.wav",
    "D:\\wavs\\muse\\alarm2.wav",
    "D:\\wavs\\muse\\escalate.wav",
    "D:\\wavs\\muse\\rotating.wav",
    "D:\\wavs\\muse\\largdoor.wav",
//$END_MODIFICATIONS

    NULL,
};

// market sfx

static char *SfxMarket[] = {
//$MODIFIED
//    "wavs\\market\\aircond1.wav",
//    "wavs\\market\\cabnhum2.wav",
//    "wavs\\market\\carpark.wav",
//    "wavs\\market\\freezer1.wav",
//    "wavs\\market\\iceyarea.wav",
//    "wavs\\market\\sdrsopen.wav",
//    "wavs\\market\\sdrsclos.wav",
//    "wavs\\market\\carton.wav",
    "D:\\wavs\\market\\aircond1.wav",
    "D:\\wavs\\market\\cabnhum2.wav",
    "D:\\wavs\\market\\carpark.wav",
    "D:\\wavs\\market\\freezer1.wav",
    "D:\\wavs\\market\\iceyarea.wav",
    "D:\\wavs\\market\\sdrsopen.wav",
    "D:\\wavs\\market\\sdrsclos.wav",
    "D:\\wavs\\market\\carton.wav",
//$END_MODIFICATIONS

    NULL,
};

// ghost sfx

static char *SfxGhost[] = {
//$MODIFIED
//    "wavs\\ghost\\coyote1.wav",
//    "wavs\\ghost\\bats.wav",
//    "wavs\\ghost\\eagle1.wav",
//    "wavs\\ghost\\minedrip.wav",
//    "wavs\\ghost\\rattler.wav",
//    "wavs\\ghost\\townbell.wav",
//    "wavs\\ghost\\tumbweed.wav",
    "D:\\wavs\\ghost\\coyote1.wav",
    "D:\\wavs\\ghost\\bats.wav",
    "D:\\wavs\\ghost\\eagle1.wav",
    "D:\\wavs\\ghost\\minedrip.wav",
    "D:\\wavs\\ghost\\rattler.wav",
    "D:\\wavs\\ghost\\townbell.wav",
    "D:\\wavs\\ghost\\tumbweed.wav",
//$END_MODIFICATIONS

    NULL,
};

// ship sfx

static char *SfxShip[] = {
//$MODIFIED
//    "wavs\\ship\\intamb1.wav",
//    "wavs\\ship\\seagulls.wav",
//    "wavs\\ship\\shiphorn.wav",
//    "wavs\\ship\\strmrain.wav",
//    "wavs\\ship\\thunder1.wav",
//    "wavs\\ship\\wash.wav",
    "D:\\wavs\\ship\\intamb1.wav",
    "D:\\wavs\\ship\\seagulls.wav",
    "D:\\wavs\\ship\\shiphorn.wav",
    "D:\\wavs\\ship\\strmrain.wav",
    "D:\\wavs\\ship\\thunder1.wav",
    "D:\\wavs\\ship\\wash.wav",
//$END_MODIFICATIONS

    NULL,
};

// level sfx list ptrs

LEVEL_SFX SfxLevel[] = {
    "ToyLite", SfxToy,
    "Toy2", SfxToy,
    "NHood1", SfxNhood,
    "NHood2", SfxNhood,
    "stunts", SfxNhood,
    "Nhood1_Battle", SfxNhood,
    "Garden1", SfxGarden,
    "Bot_Bat", SfxGarden,
    "Muse1", SfxMuse,
    "Muse2", SfxMuse,
    "Muse_Bat", SfxMuse,
    "Market1", SfxMarket,
    "Market2", SfxMarket,
    "MarkAr", SfxMarket,
    "Wild_West1", SfxGhost,
    "Wild_West2", SfxGhost,
    "Ship1", SfxShip,
    "Ship2", SfxShip,

    NULL
};
#endif // OLD_AUDIO


///////////////////////
// init sound system //
///////////////////////

#ifdef OLD_AUDIO
long InitSound(long channels)
{
// quit if sound off

    if (SoundOff)
        return TRUE;

//$MODIFIED
//    long r;
//
//// startup
//
//    r = AIL_startup();
//    if (!r)
//    {
//        DumpMessage(NULL, "Failed to init sound system!");
//        SoundOff = TRUE;
//        return FALSE;
//    }
//
//// use direct sound
//
//   AIL_set_preference(DIG_USE_WAVEOUT, NO);
//
//// set channel num
//
//   AIL_set_preference(DIG_MIXER_CHANNELS, channels);
//
//// set format
//
//    WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
//    WaveFormat.nChannels = SFX_NUM_CHANNELS;
//    WaveFormat.nSamplesPerSec = SFX_SAMPLE_RATE;
//    WaveFormat.nAvgBytesPerSec = SFX_SAMPLE_RATE * (SFX_BITS_PER_SAMPLE / 8) * SFX_NUM_CHANNELS;
//    WaveFormat.nBlockAlign = (SFX_BITS_PER_SAMPLE / 8) * SFX_NUM_CHANNELS;
//    WaveFormat.wBitsPerSample = SFX_BITS_PER_SAMPLE;
//
//    r = AIL_waveOutOpen(&DigDriver, NULL, NULL, (WAVEFORMAT*)&WaveFormat);
//    if (r)
//    {
//        AIL_shutdown();
//        DumpMessage(NULL, "Failed to set sound format!");
//        SoundOff = TRUE;
//        return FALSE;
//    }
//
///// open redbook device
//
//    RedbookDevice = AIL_redbook_open_drive(RedbookDeviceLetter);
//
//// clear redbook flags
//
//    RedbookLoop = FALSE;
//    RedbookPlaying = FALSE;
//    RedbookFade = REDBOOK_FADE_NONE;
//    RedbookFadeVol = 1.0f;
//
//// get redbook TOC
//
//    GetRedbookTOC();
//
//// return OK
//
//    return TRUE;


    g_SoundEngine.Initialize();
    g_MusicManager.Initialize();
    g_MusicManager.SetVolume(0);
    return TRUE;
//$END_MODIFICATIONS

}

//////////////////////////
// release sound system //
//////////////////////////

void ReleaseSound(void)
{
// quit if sound off

    if (SoundOff)
        return;

//$MODIFIED
//// shutdown sound
//
//    AIL_shutdown();
//
//// shutdown redbook device
//
//    if (RedbookDevice)
//        AIL_redbook_close(RedbookDevice);

    for( int i = 0 ; i < SFX_MAX_SAMPLES ; i++ )
    {
        if( Sample[i].Handle != NULL )
        {
            g_SoundEngine.ReturnInstance( (CSoundEffectInstance*)(Sample[i].Handle) );
            Sample[i].Handle = NULL;
        }
    }

    g_SoundEngine.Unload();  //$TODO(JHarding): What other shutdown?
//$END_MODIFICATIONS

}

////////////////////////
// set sound channels //
////////////////////////

void SetSoundChannels(long channels)
{
// quit if sound off

    if (SoundOff)
        return;

//$MODIFIED
//    long r;
//
//// close driver
//
//    AIL_waveOutClose(DigDriver);
//
//// set channel num
//
//   AIL_set_preference(DIG_MIXER_CHANNELS, channels);
//
//// open driver
//
//    r = AIL_waveOutOpen(&DigDriver, NULL, NULL, (WAVEFORMAT*)&WaveFormat);
//    if (r)
//    {
//        AIL_shutdown();
//        DumpMessage(NULL, "Failed to set sound format!");
//        SoundOff = TRUE;
//    }

    ; // nothing to do here
//$END_MODIFICATIONS

}

//////////////
// load sfx //
//////////////

long LoadSfx(char *levelname)
{
// quit if sound off

    if (SoundOff)
        return TRUE;

//$MODIFIED
//    long i;
//    char buf[128];
//    char **wavs;
//
//// load generic wavs
//
//    wavs = SfxGeneric;
//
//    for (i = 0 ; i < SFX_MAX_LOAD ; i++)
//    {
//
//// end of list?
//
//        if (!wavs[i]) break;
//
//// nope, load next wav
//
//        FILE *fp;
//        if ((fp = fopen(wavs[i], "rb"))) fclose(fp);
//
//        SfxLoad[i].Pos = AIL_file_read(wavs[i], NULL);
//        if (!SfxLoad[i].Pos)
//        {
//            sprintf(buf, "Can't load generic sfx '%s' into slot %d!", wavs[i], i);
//            DumpMessage(NULL, buf);
//        }
//        else
//        {
//            SfxLoad[i].Size = AIL_file_size(wavs[i]);
//        }
//    }
//
//    SfxLoadNum = i;
//
//// load level wavs
//
//    i = 0;
//    while (SfxLevel[i].Name && strcmp(SfxLevel[i].Name, levelname)) i++;
//
//    if (SfxLevel[i].Name)
//    {
//        wavs = SfxLevel[i].Files;
//
//        for (i = SfxLoadNum ; i < SFX_MAX_LOAD ; i++)
//        {
//
//// end of list?
//
//            if (!wavs[i - SfxLoadNum]) break;
//
//// nope, load next wav
//
//            SfxLoad[i].Pos = AIL_file_read(wavs[i - SfxLoadNum], NULL);
//            if (!SfxLoad[i].Pos)
//            {
//                sprintf(buf, "Can't load level sfx '%s' into slot %d!", wavs[i - SfxLoadNum], i);
//                DumpMessage(NULL, buf);
//            }
//            else
//            {
//                SfxLoad[i].Size = AIL_file_size(wavs[i - SfxLoadNum]);
//            }
//        }
//
//        SfxLoadNum = i;
//    }
//
//// allocate sample handles
//
//    for (i = 0 ; i < (long)RegistrySettings.SfxChannels ; i++)
//    {
//        Sample[i].Handle = AIL_allocate_sample_handle(DigDriver);
//        if (!Sample[i].Handle) break;
//        AIL_init_sample(Sample[i].Handle);
//    }
//
//    SfxSampleNum = i;
//
//// init 3D samples
//
//    for (i = 0 ; i < SFX_MAX_SAMPLES_3D ; i++)
//    {
//        Sample3D[i].Alive = FALSE;
//    }
//
//// clear pause flag
//
//    SfxPause = FALSE;
//
//// re-get redbook TOC
//
//    GetRedbookTOC();
//
//// clear redbook flags
//
//    RedbookLoop = FALSE;
//    RedbookPlaying = FALSE;
//    RedbookFade = REDBOOK_FADE_NONE;
//    RedbookFadeVol = 1.0f;
//
//// return OK
//
//    return TRUE;

    // load generic wavs
//    g_SoundEngine.Unload();  //$TODO: is this necessary?
//    g_SoundEngine.Initialize();  //$TODO: is this necessary?
    g_SoundEngine.LoadLevel( "COMMON" );
    //$REVISIT: D:\levels\COMMON\sounds.* might not be the best place for these common/generic
    /// sound files.  (Re-volt stored them in D:\wavs\.)  But this fit in with the current codebase.

    //$BUGBUG: in D:\levels\COMMON\, "sounds.xwb" was generated from Re-Volt assets, but "sounds.sfx" was grabbed from one of the level dirs!
    /// (It had the desired #entries -- 42 -- but I don't know if the values stored inside make sense.)

//$BUGBUG: not handling per-level sounds yet!

    //$NOTE: not touching Sample[] array, 'SfxSampleNum', 'Sample[]', or 'Sample3D[]'

    // clear pause flag
    SfxPause = FALSE;

    return TRUE;
//$END_MODIFICATIONS

}

//////////////
// free sfx //
//////////////

void FreeSfx(void)
{
// quit if sound off

    if (SoundOff)
        return;

//$MODIFIED
//    long i;
//
//// release samples
//
//    for (i = 0 ; i < SfxSampleNum ; i++)
//    {
//        StopSfx(&Sample[i]);
//        AIL_release_sample_handle(Sample[i].Handle);
//    }
//
//    SfxSampleNum = 0;
//
//// free all mem
//
//    for (i = 0 ; i < SfxLoadNum ; i++)
//    {
//        if (SfxLoad[i].Pos)
//            AIL_mem_free_lock(SfxLoad[i].Pos);
//    }


    //$NOTE: FreeSfx() is the corollary to LoadSfx()
    int i;

    for( i = 0 ; i < SFX_MAX_SAMPLES ; i++ )
    {
        if( Sample[i].Handle != NULL )
        {
            g_SoundEngine.ReturnInstance( (CSoundEffectInstance*)(Sample[i].Handle) );
            Sample[i].Handle = NULL;
        }
    }

    g_SoundEngine.Unload();
    g_SoundEngine.Initialize();  //$TODO: is this necessary?
//$END_MODIFICATIONS

}

//////////////
// play sfx //
//////////////

SAMPLE_SFX *PlaySfx(long num, long vol, long pan, long freq, long pri)
{
// quit if sound off

    if (SoundOff)
        return NULL;

//$MODIFIED
//    SAMPLE_SFX *sample;
//
//// quit if vol zero
//
//    if (!vol)
//        return NULL;
//
//// quit if NULL sound
//
//    if (!SfxLoad[num].Pos)
//        return NULL;
//
//// find a free sample
//
//    sample = FindFreeSample(pri);
//    if (sample)
//    {
//        sample->Priority = pri;
//        sample->SfxNum = num;
//        sample->TimeStamp = TimerCurrent;
//
//        AIL_init_sample(sample->Handle);
//        AIL_set_sample_file(sample->Handle, SfxLoad[num].Pos, -1);
//
//        if (vol != -1) AIL_set_sample_volume(sample->Handle, vol * SfxMasterVol / SFX_MAX_VOL);
//        if (pan != -1) AIL_set_sample_pan(sample->Handle, pan);
//        if (freq != -1) AIL_set_sample_playback_rate(sample->Handle, freq);
//
//        AIL_start_sample(sample->Handle);
//
//        return sample;
//    }
//
//// none!
//
//    return NULL;

    if( num >= SFX_GENERIC_NUM ) return NULL;  //$BUGBUG: we only support the non-level-specific sounds right now

    // Bookkeeping: find a free slot in Sample[]
    SAMPLE_SFX* pSample = FindFreeSample(0);
    if( NULL == pSample )
    {
        DumpMessage( "Warning", "No empty sample slots" );
        return NULL;
    }

    // Play the sound
    CSoundEffectInstance* pInstance;
    g_SoundEngine.Play2DSound( num, FALSE, &pInstance );

    // Bookkeeping: store pointer
    pSample->Handle = pInstance;

    return pSample;
//$END_MODIFICATIONS

}

////////////////
// stop a sfx //
////////////////

void StopSfx(SAMPLE_SFX *sample)
{
// quit if sound off

    if (SoundOff)
        return;

//$MODIFIED
//// stop sfx
//
//    AIL_end_sample(sample->Handle);

    ; //$TODO: not yet implemented
//$END_MODIFICATIONS

}

////////////////////////////////////////////////////////////////
// Stop all sfx
////////////////////////////////////////////////////////////////

void StopAllSfx()
{
// quit if sound off

    if (SoundOff)
        return;

//$MODIFIED
//    int i;
//
//// stop all sfx
//
//    if (!Mp3Pos)
//    {
//        StopSfx(&Sample[0]);
//    }
//    else
//    {
//        StopMP3();
//    }
//
//    for (i = 1 ; i <SfxSampleNum ; i++)
//    {
//        StopSfx(&Sample[i]);
//    }

    ; //$TODO: not yet implemented

//$END_MODIFICATIONS

}

///////////////////
// pause all sfx //
///////////////////

void PauseAllSfx()
{
// quit if sound off

    if (SoundOff)
        return;

//$MODIFIED
//    long i;
//
//// pause all
//
//    for (i = 0 ; i < SfxSampleNum ; i++)
//    {
//        if (AIL_sample_status(Sample[i].Handle) == SMP_PLAYING)
//        {
//            AIL_stop_sample(Sample[i].Handle);
//        }
//    }
//
//// pause redbook
//
//    if (RedbookDevice && AIL_redbook_status(RedbookDevice) == REDBOOK_PLAYING)
//    {
//        AIL_redbook_pause(RedbookDevice);
//    }
//
//// set pause flag
//
//    SfxPause = TRUE;
    
    ; //$TODO: not yet implemented
//$END_MODIFICATIONS

}

////////////////////
// resume all sfx //
////////////////////

void ResumeAllSfx()
{
// quit if sound off

    if (SoundOff)
        return;

//$MODIFIED
//    long i;
//
//// resume all
//
//    for (i = 0 ; i < SfxSampleNum ; i++)
//    {
//        if (AIL_sample_status(Sample[i].Handle) == SMP_STOPPED)
//        {
//            AIL_resume_sample(Sample[i].Handle);
//        }
//    }
//
//// resume redbook
//
//    if (RedbookDevice && AIL_redbook_status(RedbookDevice) == REDBOOK_PAUSED)
//    {
//        AIL_redbook_resume(RedbookDevice);
//    }
//
//// clear pause flag
//
//    SfxPause = FALSE;

    ; //$TODO: not yet implemented
//$END_MODIFICATIONS

}

/////////////////
// play sfx 3D //
/////////////////

SAMPLE_SFX *PlaySfx3D(long num, long vol, long freq, VEC *pos, long pri)
{
// quit if sound off

    if (SoundOff)
        return NULL;

//$MODIFIED
//    long pan;
//
//// get 3D settings
//
//    GetSfxSettings3D(&vol, &freq, &pan, pos, 0, 1.0f);
//
//// play sfx
//
//    return PlaySfx(num, vol, pan, freq, pri);

    if( num >= SFX_GENERIC_NUM ) return NULL;  //$BUGBUG: we only support the non-level-specific sounds right now

    // Bookkeeping: find a free slot in Sample[]
    SAMPLE_SFX* pSample = FindFreeSample(0);
    if( NULL == pSample )
    {
        DumpMessage( "Warning", "No empty sample slots" );
        return NULL;
    }

    // Play the sound
    CSoundEffectInstance* pInstance;
    g_SoundEngine.Play3DSound( num, FALSE, pos->v[0], pos->v[1], pos->v[2], &pInstance );

    // Bookkeeping: store pointer
    pSample->Handle = pInstance;

    return pSample;
//$END_MODIFICATIONS

}

///////////////////
// create sfx 3D //
///////////////////

SAMPLE_3D *CreateSfx3D(long num, long vol, long freq, long loop, VEC *pos, long pri)
{
// quit if sound off

    if (SoundOff)
        return NULL;

//$MODIFIED
//    long i;
//    SAMPLE_SFX *sample;
//
//// find free slot
//
//    for (i = 0 ; i < SFX_MAX_SAMPLES_3D ; i++) if (!Sample3D[i].Alive)
//    {
//
//// got one, set misc
//
//        Sample3D[i].Num = num;
//        Sample3D[i].Vol = vol;
//        Sample3D[i].Freq = freq;
//        Sample3D[i].Loop = loop;
////      Sample3D[i].LastDist = 0;
//        Sample3D[i].Pos = *pos;
//        Sample3D[i].OldPos = *pos;
//        SetVecZero(&Sample3D[i].Vel);
//        Sample3D[i].Priority = pri;
//        Sample3D[i].RangeMul = 1.0f;
//
//// return now if looping
//
//        if (loop)
//        {
//            Sample3D[i].Sample = NULL;
//            Sample3D[i].Alive = TRUE;
//            return &Sample3D[i];
//        }
//
//// quit if NULL sound
//
//        if (!SfxLoad[num].Pos)
//            return NULL;
//
//// else start sample now
//
//        sample = FindFreeSample(pri);
//        if (sample)
//        {
//            sample->Priority = pri;
//            sample->SfxNum = num;
//            sample->TimeStamp = TimerCurrent;
//
//            AIL_init_sample(sample->Handle);
//            AIL_set_sample_file(sample->Handle, SfxLoad[Sample3D[i].Num].Pos, -1);
//            AIL_set_sample_loop_count(sample->Handle, 1);
//            AIL_set_sample_volume(sample->Handle, SFX_MIN_VOL);
//            AIL_start_sample(sample->Handle);
//
//            Sample3D[i].Sample = sample;
//            Sample3D[i].Alive = TRUE;
//            return &Sample3D[i];
//        }
//
//// couldn't start, cancel sound
//
//        Sample3D[i].Alive = FALSE;
//        return NULL;
//    }
//
//// no slots
//
//    return NULL;

    if( num >= SFX_GENERIC_NUM ) return NULL;  //$BUGBUG: we only support the non-level-specific sounds right now
    
    // Bookkeeping: find a non-used entry in Sample3D[]
    for( int i3D=0 ; i3D < SFX_MAX_SAMPLES_3D ; i3D++ )
    {
        if( Sample3D[i3D].Sample == NULL || Sample3D[i3D].Sample->Handle == NULL )
        {
            break;
        }
    }
    if( i3D == SFX_MAX_SAMPLES_3D )
    {
        DumpMessage( "Warning", "No available 3D sample slots!" );
        return NULL;
    }

    // Bookkeeping: find a non-used entry in Sample[]
    SAMPLE_SFX* pSample = FindFreeSample(0);
    if( NULL == pSample )
    {
        DumpMessage( "Warning", "No available sample slots!" );
        return NULL;
    }
    Sample3D[i3D].Sample = pSample;

    // sound is already created; just play it
    CSoundEffectInstance* pInstance;
    g_SoundEngine.Play3DSound( num, loop, &Sample3D[i3D], &pInstance );

    // Bookkeeping: store poiner
    pSample->Handle = pInstance;

    return &Sample3D[i3D];
//$END_MODIFICATIONS

}

///////////////////
// create sfx 3D //
///////////////////

void FreeSfx3D(SAMPLE_3D *sample3d)
{
// quit if sound off

    if (SoundOff)
        return;

//$MODIFIED
//// kill playing sample
//
//    if (sample3d->Sample)
//    {
//        StopSfx(sample3d->Sample);
//    }
//
//// kill 3d sample
//
//    sample3d->Alive = FALSE;

    if( NULL != sample3d->Sample->Handle )
    {
        g_SoundEngine.ReturnInstance( (CSoundEffectInstance*)(sample3d->Sample->Handle) );
        sample3d->Sample->Handle = NULL;
    }
//$END_MODIFICATIONS

}

/////////////////////////
// get sfx 3D settings //
/////////////////////////

void GetSfxSettings3D(long *vol, long *freq, long *pan, VEC *pos, float vel, float rangemul)
{
// quit if sound off

    if (SoundOff)
        return;

//$MODIFIED
//    long per;
//    float x, f, len;
//    VEC vec1, vec2;
//
//// global ambience if rangemul zero
//
//    if (!rangemul)
//    {
//        *pan = SFX_CENTRE_PAN;
//        return;
//    }
//
//// get camera space vector
//
//    SubVector(pos, &CAM_MainCamera->WPos, &vec1);
//    TransposeRotVector(&CAM_MainCamera->WMatrix, &vec1, &vec2);
//    len = Length(&vec2);
//
//// calc vol
//
//    f = (SFX_3D_MIN_DIST * rangemul) / len - SFX_3D_SUB_DIST;
//    if (f < 0) f = 0;
//    else if (f > 1) f = 1;
//
//    FTOL(f * 256, per);
//    *vol = (*vol * per / 256);
//    
//// calc pan
//
//    f = len / 100.0f;
//    if (f > 1.0f) f = 1.0f;
//
//    x = vec2.v[X] * RenderSettings.GeomPers / abs(vec2.v[Z]) * f;
//    x = (x * SFX_3D_PAN_MUL / REAL_SCREEN_XSIZE) + SFX_CENTRE_PAN;
//    if (x < SFX_LEFT_PAN + 1) x = SFX_LEFT_PAN + 1;
//    else if (x > SFX_RIGHT_PAN - 1) x = SFX_RIGHT_PAN - 1;
//
//    FTOL(x, *pan);
//
//// calc freq
//
//    f = SFX_3D_SOS / (vel + SFX_3D_SOS);
//
//    FTOL(f * 256, per);
//    *freq = *freq * per / 256;

    ; //$TODO: not yet implemented
//$END_MODIFICATIONS

}

//////////////////////////
// change 3D sfx sample //
//////////////////////////

void ChangeSfxSample3D(SAMPLE_3D *sample3d, long sfx)
{
// quit if sound off

    if (SoundOff)
        return;

//$MODIFIED
//// change sfx
//
//    if (sample3d->Sample && sample3d->Sample->SfxNum == sample3d->Num)
//    {
//        StopSfx(sample3d->Sample);
//    }
//
//    sample3d->Sample = NULL;
//    sample3d->Num = sfx;

    //$TODO: is this right thing to do for ChangeSfxSample3d() ??
    //$TODO: I think we need to handle looping samples (perhaps start new sound here, with new 'sfx' param?)
    if( NULL != sample3d->Sample->Handle )
    {
        g_SoundEngine.ReturnInstance( (CSoundEffectInstance*)(sample3d->Sample->Handle) );
        sample3d->Sample->Handle = NULL;
    }
//$END_MODIFICATIONS

}

//////////////////////////////////////
// get oldest sfx of particlar type //
//////////////////////////////////////

SAMPLE_SFX *GetOldestSfx(long num)
{
// quit if sound off

    if (SoundOff)
        return NULL;

//$MODIFIED
//    long i;
//    unsigned long time, otime;
//    SAMPLE_SFX *sample;
//
//// loop thru all samples
//
//    sample = NULL;
//    otime = 0;
//    for (i = 0 ; i < SfxSampleNum ; i++)
//    {
//
//// correct num and still playing?
//
//        if (Sample[i].SfxNum == num && AIL_sample_status(Sample[i].Handle) != SMP_DONE)
//        {
//
//// oldest?
//
//            time = TimerCurrent - Sample[i].TimeStamp;
//            if (time > otime)
//            {
//                otime = time;
//                sample = &Sample[i];
//            }
//        }
//    }
//
//// return what we got
//
//    return sample;

    return NULL;  //$TODO: not yet implemented
//$END_MODIFICATIONS

}

/////////////////////
// maintain sounds //
/////////////////////

void MaintainAllSfx(void)
{
// quit if sound off

    if (SoundOff)
        return;

//$MODIFIED
//    long i, vol, freq, pan, status;
//    float dist, vel;
//    VEC vec, vRel;
//    SAMPLE_SFX *sample;
//
//// quit if paused
//
//    if (SfxPause)
//        return;
//
//// fade redbook vol?
//
//    if (RedbookDevice && RedbookFade == REDBOOK_FADE_UP)
//    {
//        RedbookFadeVol += TimeStep * 4.0f;
//        if (RedbookFadeVol > 1.0f)
//        {
//            RedbookFadeVol = 1.0f;
//            RedbookFade = REDBOOK_FADE_NONE;
//        }
//        UpdateMusicVol(MusicMasterVol);
//    }
//
//    if (RedbookDevice && RedbookFade == REDBOOK_FADE_DOWN)
//    {
//        RedbookFadeVol -= TimeStep * 4.0f;
//        if (RedbookFadeVol < 0.0f)
//        {
//            RedbookFadeVol = 0.0f;
//            RedbookFade = REDBOOK_FADE_NONE;
//        }
//        UpdateMusicVol(MusicMasterVol);
//    }
//
//// check redbook status
//
//    if (RedbookDevice && RedbookPlaying)
//    {
//        RedbookStatusTimer -= TimeStep;
//        if (RedbookStatusTimer < 0.0f)
//        {
//            RedbookStatusTimer = 2.0f;
//            status = AIL_redbook_status(RedbookDevice);
//
//            if (RedbookLoop && status == REDBOOK_STOPPED)
//            {
//                AIL_redbook_play(RedbookDevice, RedbookTrack[RedbookStartTrack].Start, RedbookTrack[RedbookEndTrack].End);
//            }
//
//            if (status == REDBOOK_ERROR)
//            {
//                StopRedbook();
//            }
//        }
//    }
//
//// count free sound slots
//
//    ChannelsUsed = 0;
//
//    for (i = 0 ; i < SfxSampleNum ; i++)
//        if (AIL_sample_status(Sample[i].Handle) != SMP_DONE)
//            ChannelsUsed++;
//
//// get sfx time mul
//
//    SfxTimerLast = SfxTimerCurrent;
//    SfxTimerCurrent = CurrentTimer();
//    SfxTimeMul = ((float)TimerFreq / 100) / (float)(SfxTimerCurrent - SfxTimerLast);
//
//// maintain all 3D sfx
//
//    ChannelsUsed3D = 0;
//    for (i = 0 ; i < SFX_MAX_SAMPLES_3D ; i++) if (Sample3D[i].Alive)
//    {
//        ChannelsUsed3D++;
//
//// has sample been stolen from me?
//
//        if (Sample3D[i].Sample)
//        {
//            if (Sample3D[i].Sample->SfxNum != Sample3D[i].Num)
//            {
//                Sample3D[i].Sample = NULL;
//            }   
//        }
//
//// non-looping sfx finished?
//
//        if (!Sample3D[i].Loop)
//        {
//            if (Sample3D[i].Sample)
//            {
//                 if (AIL_sample_status(Sample3D[i].Sample->Handle) == SMP_DONE)
//                    Sample3D[i].Sample = NULL;
//            }
//            if (!Sample3D[i].Sample)
//                continue;
//        }
//
//// get 3d sound settings
//
////      SubVector(&Sample3D[i].Pos, &CAM_MainCamera->WPos, &vec);
////      dist = Length(&vec);
////      vel = (dist - Sample3D[i].LastDist) * SfxTimeMul;
////      Sample3D[i].LastDist = dist;
//
//// get relative velocity between camera and sound
//
//        if (TimeStep > SMALL_REAL) 
//        {
//            VecMinusVec(&Sample3D[i].Pos, &Sample3D[i].OldPos, &Sample3D[i].Vel);
//            VecDivScalar(&Sample3D[i].Vel, TimeStep);
//        } 
//        else 
//        {
//            SetVecZero(&Sample3D[i].Vel);
//        }
//        CopyVec(&Sample3D[i].Pos, &Sample3D[i].OldPos);
//        VecMinusVec(&Sample3D[i].Vel, &CAM_MainCamera->Vel, &vRel);
//
//        VecMinusVec(&Sample3D[i].Pos, &CAM_MainCamera->WPos, &vec);
//        dist = VecLen(&vec);
//
//        if (dist > SMALL_REAL) {
//            VecDivScalar(&vec, dist);
//            vel = VecDotVec(&vRel, &vec) * 0.005f;
//        } else {
//            vel = ZERO;
//        }
//
//        vol = Sample3D[i].Vol;
//        freq = Sample3D[i].Freq;
//
//        GetSfxSettings3D(&vol, &freq, &pan, &Sample3D[i].Pos, vel, Sample3D[i].RangeMul);
//
//// stop sample if outside range?
//
//        if (Sample3D[i].Loop && !vol)
//        {
//            if (Sample3D[i].Sample)
//            {
//                StopSfx(Sample3D[i].Sample);
//                Sample3D[i].Sample = NULL;
//            }
//
//            continue;
//        }
//
//// start sample if in range?
//
//        if (Sample3D[i].Loop && !Sample3D[i].Sample && SfxLoad[Sample3D[i].Num].Pos)
//        {
//            sample = FindFreeSample(Sample3D[i].Priority);
//            if (sample)
//            {
//                sample->Priority = Sample3D[i].Priority;
//                sample->SfxNum = Sample3D[i].Num;
//
//                AIL_init_sample(sample->Handle);
//                AIL_set_sample_file(sample->Handle, SfxLoad[Sample3D[i].Num].Pos, -1);
//                AIL_set_sample_loop_count(sample->Handle, 0);
//                AIL_set_sample_volume(sample->Handle, SFX_MIN_VOL);
//                AIL_start_sample(sample->Handle);
//
//                Sample3D[i].Sample = sample;
//                break;
//            }
//        }
//
//// skip if still no sample
//
//        if (!Sample3D[i].Sample)
//            continue;
//
//// set vol / pan / freq
//
//        AIL_set_sample_volume(Sample3D[i].Sample->Handle, vol * SfxMasterVol / SFX_MAX_VOL);
//        AIL_set_sample_pan(Sample3D[i].Sample->Handle, pan);
//        AIL_set_sample_playback_rate(Sample3D[i].Sample->Handle, freq);
//    }

    int i;

    g_SoundEngine.UpdateAll();

    for( i = 0 ; i < SFX_MAX_SAMPLES ; i++ )
    {
        if( Sample[i].Handle != NULL )
        {
            CSoundEffectInstance* pSoundInstance = (CSoundEffectInstance*)(Sample[i].Handle);
            if( !pSoundInstance->IsActive() )
            {
                g_SoundEngine.ReturnInstance( pSoundInstance );
                Sample[i].Handle = NULL;
            }
        }
    }

/*
    for( i = 0 ; i < SFX_MAX_SAMPLES_3D ; i++ )
    {
        if( Sample3D[i].Sample != NULL
            &&  Sample3D[i].Sample->Handle != NULL )
        {
            CSoundEffectInstance* pSoundInstance = (CSoundEffectInstance*)(Sample3D[i].Sample->Handle);
            pSoundInstance->m_apBuffers[0]->SetPosition( Sample3D[i].Pos.v[0],
                                                         Sample3D[i].Pos.v[1],
                                                         Sample3D[i].Pos.v[2],
                                                         DS3D_IMMEDIATE );
            pSoundInstance->m_apBuffers[0]->SetFrequency( Sample3D[i].Freq );
//            pSoundInstance->m_apBuffers[0]->Volume( Sample3D[i].Vol );
        }
    }
*/        
//$END_MODIFICATIONS

}

////////////////////////
// find a free sample //
////////////////////////

SAMPLE_SFX *FindFreeSample(long pri)
{
// quit if sound off

    if (SoundOff)
        return NULL;

//$MODIFIED
//    long i, lslot, lpri;
//
//// find empty
//
//    for (i = 0 ; i < SfxSampleNum ; i++)
//    {
//        if (AIL_sample_status(Sample[i].Handle) == SMP_DONE)
//        {
//            return &Sample[i];
//        }
//    }
//
//// find lowest priority
//
//    lpri = 0x7fffffff;
//
//    for (i = 0 ; i < SfxSampleNum ; i++)
//    {
//        if (Sample[i].Priority < lpri)
//        {
//            lpri = Sample[i].Priority;
//            lslot = i;
//        }
//    }
//
//// if lower than ours, steal!
//
//    if (lpri < pri)
//    {
//        StopSfx(&Sample[lslot]);
//        return &Sample[lslot];
//    }
//
//// none free
//
//    return NULL;

    for( int i=0 ; i < SFX_MAX_SAMPLES ; i++ )
    {
        if( Sample[i].Handle == NULL )
        {
            return &Sample[i];
        }
    }

    return NULL;
//$END_MODIFICATIONS

}

//////////////////
// play an .MP3 //
//////////////////

void PlayMP3(char *file)
{
// quit if sound off

    if (SoundOff)
        return;

//$MODIFIED
//// not if already playing
//
//    if (Mp3Pos)
//        return;
//
//// read mp3
//
//    FILE *fp;
//    if ((fp = fopen(file, "rb"))) fclose(fp);
//
//    Mp3Pos = AIL_file_read(file, NULL);
//    if (!Mp3Pos)
//        return;
//
//// play
//
//    long size = AIL_file_size(file);
//
//    AIL_init_sample(Sample[0].Handle);
//    AIL_set_named_sample_file(Sample[0].Handle, ".mp3", Mp3Pos, size, -1);
//    AIL_set_sample_volume(Sample[0].Handle, MusicMasterVol);
//    AIL_set_sample_playback_rate(Sample[0].Handle, 44100);
//    AIL_set_sample_loop_count(Sample[0].Handle, 0);
//    AIL_start_sample(Sample[0].Handle);
//
//// set priority
//
//    Sample[0].Priority = 0x7fffffff;

    ; //$TODO: not yet implemented (and maybe never will be)
//$END_MODIFICATIONS

}

//////////////////
// stop an .MP3 //
//////////////////

void StopMP3()
{
// quit if sound off

    if (SoundOff)
        return;

//$MODIFIED
//// quit if no mp3 playing
//
//    if (!Mp3Pos)
//        return;
//
//// stop mp3
//
//    AIL_end_sample(Sample[0].Handle);
//    AIL_mem_free_lock(Mp3Pos);
//    Mp3Pos = NULL;

    ; //$TODO: not yet implemented (and maybe never will be)
//$END_MODIFICATIONS

}

/////////////////////////
// play redbook tracks //
/////////////////////////

void PlayRedbookTrack(long track1, long track2, long loop)
{
// quit if sound off

    if (SoundOff)
        return;

////$MODIFIED
//// do we have redbook
//
//    if (!RedbookDevice)
//        return;
//
//// valid track?
//
//    if ((track1 > RedbookTrackNum - 1) || (track2 > RedbookTrackNum - 1))
//        return;
//
//// save start + end track
//
//    RedbookStartTrack = track1;
//    RedbookEndTrack = track2;
//
//// play
//
//    AIL_redbook_play(RedbookDevice, RedbookTrack[RedbookStartTrack].Start, RedbookTrack[RedbookEndTrack].End);
//    RedbookLoop = loop;
//    RedbookPlaying = TRUE;

    g_MusicManager.RandomSong();
    if( RegistrySettings.MusicOn )
        g_MusicManager.Play();
    //$BUGBUG: this doesn't yet have the same behavior as the original function
//$END_MODIFICATIONS

}

/////////////////////////////////////////////////////////////////
// play redbook tracks, starting at random track between 1 & 2 //
/////////////////////////////////////////////////////////////////

void PlayRedbookTrackRandom(long track1, long track2, long loop)
{
// quit if sound off

    if (SoundOff)
        return;

//$MODIFIED
//    long track;
//
//// do we have redbook
//
//    if (!RedbookDevice)
//        return;
//
//// valid track?
//
//    if ((track1 > RedbookTrackNum - 1) || (track2 > RedbookTrackNum - 1))
//        return;
//
//// save start + end track
//
//    RedbookStartTrack = track1;
//    RedbookEndTrack = track2;
//
//// pick random 'tween track
//
//    if (track1 == track2)
//        track = track1;
//    else
//        track = track1 + (rand() % (track2 + 1 - track1));
//
//// play
//
//    AIL_redbook_play(RedbookDevice, RedbookTrack[track].Start, RedbookTrack[RedbookEndTrack].End);
//
//// set redbook flags
//
//    RedbookLoop = loop;
//    RedbookPlaying = TRUE;

    g_MusicManager.RandomSong();
    if( RegistrySettings.MusicOn )
        g_MusicManager.Play();
    //$BUGBUG: this doesn't yet have the same behavior as the original function
//$END_MODIFICATIONS

}

//////////////////
// stop redbook //
//////////////////

void StopRedbook(void)
{
// quit if sound off

    if (SoundOff)
        return;

//$MODIFIED
//// do we have redbook
//
//    if (!RedbookDevice)
//        return;
//
//// yep, stop
//
//    if (AIL_redbook_status(RedbookDevice) == REDBOOK_PLAYING || AIL_redbook_status(RedbookDevice) == REDBOOK_PAUSED)
//    {
//        AIL_redbook_stop(RedbookDevice);
//    }
//
//// clear redbook flags
//
//    RedbookLoop = FALSE;
//    RedbookPlaying = FALSE;

    g_MusicManager.Stop();
//$END_MODIFICATIONS

}

/////////////////////////
// is redbook playing? //
/////////////////////////

long IsRedbookPlaying(void)
{
// quit if sound off

    if (SoundOff)
        return FALSE;

//$MODIFIED
//// do we have redbook
//
//    if (!RedbookDevice)
//        return FALSE;
//
//// playing
//
//    return (AIL_sample_status(Sample[0].Handle) == SMP_PLAYING ? TRUE : FALSE);

    return (g_MusicManager.GetStatus() == MM_PLAYING);
//$END_MODIFICATIONS

}

////////////////////////
// start redbook fade //
////////////////////////

void SetRedbookFade(REDBOOK_FADE fade)
{
//$MODIFIED
//    RedbookFade = fade;
//
//    if (RedbookFade == REDBOOK_FADE_UP)
//        RedbookFadeVol = 0.0f;
//    else if (RedbookFade == REDBOOK_FADE_DOWN)
//        RedbookFadeVol = 1.0f;
//
//    UpdateMusicVol(MusicMasterVol);

    ; //$TODO: not yet implemented
      //$NOTE: maybe tie-in g_MusicManager here?
//$END_MODIFICATIONS

}

////////////////////////
// set sfx master vol //
////////////////////////

void UpdateSfxVol(long vol)
{
// quit if sound off

    if (SoundOff)
        return;

//$MODIFIED
//// update sfx vol
//
//    SfxMasterVol = vol / 2;

    ; //$TODO: not yet implemented
//$END_MODIFICATIONS

}

/////////////////////////
// update music volume //
/////////////////////////

void UpdateMusicVol(long vol)
{
// quit if sound off

    if (SoundOff)
        return;

//$MODIFIED
//    long status;
//
//// update music vol
//
//    MusicMasterVol = vol;
//
//// update mp3 vol?
//
//    if (Mp3Pos)
//    {
//        status = AIL_sample_status(Sample[0].Handle);
//        if ((status == SMP_PLAYING || status == SMP_STOPPED) && Mp3Pos)
//        {
//            AIL_set_sample_volume(Sample[0].Handle, MusicMasterVol);
//        }
//    }
//
//// update redbook vol?
//
//    if (RedbookDevice)
//    {
//        AIL_redbook_set_volume(RedbookDevice, (long)((float)MusicMasterVol * RedbookFadeVol));
//    }

    ; //$TODO: not yet implemented
//$END_MODIFICATIONS

}

///////////////////////////////////
// get redbook table of contents //
///////////////////////////////////

static void GetRedbookTOC(void)
{
// quit if sound off

    if (SoundOff)
        return;

//$MODIFIED
//    long i;
//
//// do we have redbook?
//
//    if (!RedbookDevice)
//        return;
//
//// get track num
//
//    RedbookTrackNum = AIL_redbook_tracks(RedbookDevice) - 1;
//    if (RedbookTrackNum > REDBOOK_TRACK_NUM)
//        RedbookTrackNum = REDBOOK_TRACK_NUM;
//
//// get start / end pos for each track
//
//    for (i = 0 ; i < RedbookTrackNum ; i++)
//    {
//        AIL_redbook_track_info(RedbookDevice, i + 2, &RedbookTrack[i].Start, &RedbookTrack[i].End);
//    }

    ; //$TODO: not yet implemented
      //$NOTE: maybe tie-in g_MusicManager here?
//$END_MODIFICATIONS

}
#endif //!OLD_AUDIO

