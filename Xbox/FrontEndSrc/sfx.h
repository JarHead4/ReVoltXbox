typedef DWORD HSAMPLE;  //$ADDITION: porting aid
//$NOTE: not directly using this file, but it has useful info like
/// sfx list, redbook track names, etc.  So we'll keep it here for now.
//$TODO: remove this file when we don't need it any more.

#ifndef SFX_H
#define SFX_H

#include "mss.h"

// macros

#define SFX_NUM_CHANNELS 2
#define SFX_SAMPLE_RATE 22050
#define SFX_BITS_PER_SAMPLE 16

#define SFX_MAX_LOAD 256
#define SFX_MAX_SAMPLES 64
#define SFX_MAX_SAMPLES_3D 64

#define SFX_MIN_VOL         0
#define SFX_DEFAULT_VOL     90
#define MUSIC_DEFAULT_VOL   70
#define SFX_MAX_VOL         127
#define SFX_LEFT_PAN        0
#define SFX_CENTRE_PAN      64
#define SFX_RIGHT_PAN       127

#define SFX_3D_PAN_MUL 64
#define SFX_3D_MIN_DIST 600
#define SFX_3D_SUB_DIST (8.0f / (float)SFX_MAX_VOL)
#define SFX_3D_SOS 1024

// redbook track list

typedef enum {
    REDBOOK_TRACK_JAMJAR,

    REDBOOK_TRACK_TOYSFORTHEBOYZ,
    REDBOOK_TRACK_LIVEWIRES,
    REDBOOK_TRACK_CANDYJUMPING,

    REDBOOK_TRACK_FLIPPED,
    REDBOOK_TRACK_RECHARGE,
    REDBOOK_TRACK_WHEELZOFSTEEL,
    REDBOOK_TRACK_LITTLETOYCARZ,
    REDBOOK_TRACK_OVERDRIVER,
    REDBOOK_TRACK_OUTOFCONTROL,

    REDBOOK_TRACK_WESTVOLT,

    REDBOOK_TRACK_WINCUP,

    REDBOOK_TRACK_LOSECUP,

    REDBOOK_TRACK_CREDITS,

    REDBOOK_TRACK_NUM

} REDBOOK_TRACKS;

// structs

typedef enum {
    REDBOOK_FADE_NONE,
    REDBOOK_FADE_UP,
    REDBOOK_FADE_DOWN,
} REDBOOK_FADE;

typedef struct {
    long Priority;
    long SfxNum;
    HSAMPLE Handle;
    unsigned long TimeStamp;
} SAMPLE_SFX;

typedef struct {
    void *Pos;
    long Size;
} SFX_LOAD;

typedef struct {
    long Alive, Num, Vol, Freq, Loop, Priority;
    //REAL LastDist, 
    REAL RangeMul;
    SAMPLE_SFX *Sample;
    VEC Pos;
    VEC OldPos;
    VEC Vel;
} SAMPLE_3D;

typedef struct {
    char *Name;
    char **Files;
} LEVEL_SFX;

typedef struct {
    unsigned long Start;
    unsigned long End;
} REDBOOK_TRACK;

// prototypes

extern long InitSound(long channels);
extern void ReleaseSound(void);
extern void SetSoundChannels(long num);
extern long LoadSfx(char *levelname);
extern void FreeSfx(void);
extern SAMPLE_SFX *PlaySfx(long num, long vol, long pan, long freq, long pri);
extern void StopSfx(SAMPLE_SFX *sample);
extern void StopAllSfx();
extern void PauseAllSfx();
extern void ResumeAllSfx();
extern SAMPLE_SFX *PlaySfx3D(long num, long vol, long freq, VEC *pos, long pri);
extern SAMPLE_3D *CreateSfx3D(long num, long vol, long freq, long loop, VEC *pos, long pri);
extern void FreeSfx3D(SAMPLE_3D *sample3d);
extern void GetSfxSettings3D(long *vol, long *freq, long *pan, VEC *pos, float vel, float rangemul);
extern void ChangeSfxSample3D(SAMPLE_3D *sample3d, long sfx);
extern SAMPLE_SFX *GetOldestSfx(long num);
extern void MaintainAllSfx(void);
extern SAMPLE_SFX *FindFreeSample(long pri);
extern void PlayMP3(char *file);
extern void StopMP3();
extern void PlayRedbookTrack(long track1, long track2, long loop);
extern void PlayRedbookTrackRandom(long track1, long track2, long loop);
extern void StopRedbook(void);
extern long IsRedbookPlaying(void);
extern void SetRedbookFade(REDBOOK_FADE fade);
extern void UpdateSfxVol(long vol);
extern void UpdateMusicVol(long vol);

// globals

extern long SoundOff, ChannelsFree, ChannelsUsed, ChannelsUsed3D;
extern long SfxSampleNum, SfxLoadNum;
extern long SfxMasterVol;
extern long MusicMasterVol;
extern char RedbookDeviceLetter;

// generic sfx list

enum {
//$MODIFIED
//    SFX_ENGINE,
//    SFX_ENGINE_PETROL,
//    SFX_ENGINE_CLOCKWORK,
//    SFX_ENGINE_UFO,
//    SFX_HONK,
//    SFX_SCRAPE,
//    SFX_SKID_NONE,  //$NOTE(cprince): this name isn't used in the code
//    SFX_SKID_NORMAL,
//    SFX_SKID_ROUGH,
//    SFX_PICKUP,
//    SFX_PICKUP_REGEN,
//    SFX_SHOCKWAVE,
//    SFX_SHOCKWAVE_FIRE,
//    SFX_ELECTROPULSE,
//    SFX_ELECTROZAP,
//    SFX_FIREWORK,
//    SFX_FIREWORK_BANG,
//    SFX_CHROMEBALL_DROP,
//    SFX_CHROMEBALL,
//    SFX_HIT1,
//    SFX_HIT2,
//    SFX_WATERBOMB,
//    SFX_WATERBOMB_FIRE,
//    SFX_WATERBOMB_HIT,
//    SFX_WATERBOMB_BOUNCE,
//    SFX_PUTTYBOMB_BANG,
//    SFX_FUSE,
//    SFX_OILDROP,
//    SFX_COUNTDOWN,
//    SFX_TURBO,
//    SFX_SERVO,
//    SFX_MENU_BACK,
//    SFX_MENU_FORWARD,
//    SFX_MENU_UPDOWN,
//    SFX_MENU_LEFTRIGHT,
//    SFX_BEACHBALL,
//    SFX_LIGHT_FLICKER,
//    SFX_BOTTLE,
//    SFX_BOXENTRY,
//    SFX_GLOBAL,
//    SFX_TVSTATIC,
//    SFX_SPLASH,
//    SFX_HONKA,
    //$NOTE: different order because order of resources in our .xwb file
    /// doesn't match order of Re-Volt's .wav filename list.
    SFX_CHROMEBALL,
    SFX_CHROMEBALL_DROP,
    SFX_BEACHBALL,
    SFX_BOTTLE,
    SFX_BOXENTRY,
    SFX_ENGINE_CLOCKWORK,
    SFX_COUNTDOWN,
    SFX_ELECTROPULSE,
    SFX_ELECTROZAP,
    SFX_FIREWORK_BANG,
    SFX_FIREWORK,
    SFX_FUSE,
    SFX_HIT1,
    SFX_HIT2,
    SFX_HONKA,
    SFX_HONK,
    SFX_LIGHT_FLICKER,
    SFX_MENU_LEFTRIGHT,
    SFX_MENU_BACK,
    SFX_MENU_FORWARD,
    SFX_MENU_UPDOWN,
    SFX_ENGINE,
    SFX_OILDROP,
    SFX_ENGINE_PETROL,
    SFX_PICKUP_REGEN,
    SFX_PICKUP,
    SFX_PUTTYBOMB_BANG,
    SFX_SCRAPE,
    SFX_SERVO,
    SFX_SHOCKWAVE,
    SFX_SHOCKWAVE_FIRE,
    SFX_SKID_NORMAL,
    SFX_SKID_ROUGH,
    SFX_SPLASH,
    SFX_GLOBAL,
    SFX_TURBO,
    SFX_TVSTATIC,
    SFX_ENGINE_UFO,
    SFX_WATERBOMB,
    SFX_WATERBOMB_BOUNCE,
    SFX_WATERBOMB_FIRE,
    SFX_WATERBOMB_HIT,
//$END_MODIFICATIONS

    SFX_GENERIC_NUM
};

// toy sfx

enum {
    SFX_TOY_PIANO = SFX_GENERIC_NUM,
    SFX_TOY_PLANE,
    SFX_TOY_COPTER,
    SFX_TOY_DRAGON,
    SFX_TOY_CREAK,
    SFX_TOY_TRAIN,
    SFX_TOY_WHISTLE,
    SFX_TOY_ARCADE,
    SFX_TOY_BRICK,
};

// nhood sfx

enum {
    SFX_HOOD_BASKETBALL = SFX_GENERIC_NUM,
    SFX_HOOD_BIRDS1,
    SFX_HOOD_BIRDS2,
    SFX_HOOD_BIRDS3,
    SFX_HOOD_DOGBARK,
    SFX_HOOD_KIDS,
    SFX_HOOD_SPRINKLER,
    SFX_HOOD_TV,
    SFX_HOOD_LAWNMOWER,
    SFX_HOOD_DIGGER,
    SFX_HOOD_STREAM,
    SFX_HOOD_AMB,
    SFX_HOOD_CONE,
};

// garden sfx

enum {
    SFX_GARDEN_TROPICS2 = SFX_GENERIC_NUM,
    SFX_GARDEN_TROPICS3,
    SFX_GARDEN_TROPICS4,
    SFX_GARDEN_STREAM,
    SFX_GARDEN_ANIMAL1,
    SFX_GARDEN_ANIMAL2,
    SFX_GARDEN_ANIMAL3,
    SFX_GARDEN_ANIMAL4,
};

// muse sfx

enum {
    SFX_MUSE_AMB = SFX_GENERIC_NUM,
    SFX_MUSE_LASER,
    SFX_MUSE_ALARM,
    SFX_MUSE_ESCALATOR,
    SFX_MUSE_BARREL,
    SFX_MUSE_DOOR,
};

// market sfx

enum {
    SFX_MARKET_AIRCOND = SFX_GENERIC_NUM,
    SFX_MARKET_CABINET,
    SFX_MARKET_CARPARK,
    SFX_MARKET_FREEZER,
    SFX_MARKET_ICY,
    SFX_MARKET_DOOR_OPEN,
    SFX_MARKET_DOOR_CLOSE,
    SFX_MARKET_CARTON,
};

// ghost sfx

enum {
    SFX_GHOST_COYOTE = SFX_GENERIC_NUM,
    SFX_GHOST_BATS,
    SFX_GHOST_EAGLE,
    SFX_GHOST_DRIP,
    SFX_GHOST_RATTLER,
    SFX_GHOST_BELL,
    SFX_GHOST_TUMBLEWEED,
};

// ship sfx

enum {
    SFX_SHIP_INTAMB = SFX_GENERIC_NUM,
    SFX_SHIP_SEAGULLS,
    SFX_SHIP_FOGHORN,
    SFX_SHIP_STORM,
    SFX_SHIP_THUNDER,
    SFX_SHIP_CALM,
};

#endif // SFX_H

