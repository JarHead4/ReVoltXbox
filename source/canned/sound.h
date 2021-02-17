
#ifndef SOUND_H
#define SOUND_H

// macros

#define DUPLICATESAMPLES 4
#define MAXSAMPLES 24

// prototypes

extern char LoadWAV( char *FileName, short BuffNum );
extern char SetupSound( HWND hwnd );
extern short PlaySample( short Num, LONG Vol, LONG Freq, LONG Pan, char Loop );
extern char SoundManager();
extern char StopSample( short Num, short Handle );
extern char ChangeSample( short Num, short Handle, LONG Vol, LONG Freq, LONG Pan );
extern void KillSound( void );

// globals

extern char NoSound;
extern char SoundDevice;
extern char SoundDeviceCount;
extern volatile char SoundDeviceFlag;
extern IDirectSound *DSObj; 
extern IDirectSoundBuffer *SBufferPrim;
extern IDirectSoundBuffer *SBufferSec[][MAXSAMPLES];
extern char SampleLoaded[];
extern char SampleUsed[][MAXSAMPLES];
extern DWORD SampleLen[][MAXSAMPLES];

#endif
