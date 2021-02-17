
#include "revolt.h"
#include "sound.h"

static unsigned char Sample[4048*1024];

char NoSound = FALSE;
char SoundDevice = 0;
char SoundDeviceCount;
volatile char SoundDeviceFlag;
IDirectSound *DSObj; 
IDirectSoundBuffer *SBufferPrim;
IDirectSoundBuffer *SBufferSec[DUPLICATESAMPLES][MAXSAMPLES];
char SampleLoaded[MAXSAMPLES];
char SampleUsed[DUPLICATESAMPLES][MAXSAMPLES];
DWORD SampleLen[DUPLICATESAMPLES][MAXSAMPLES];

// **********************************************************************************************************************/
// ** FindSoundDeviceCallback  ***********************************************************************************/
// **********************************************************************************************************************/


BOOL CALLBACK FindSoundDeviceCallback(LPGUID lpGUID, LPSTR szName, LPSTR szDevice, LPVOID lParam)
{
	HRESULT r;

	if (SoundDevice == SoundDeviceCount++)
	{
		r = DirectSoundCreate(lpGUID, &DSObj, NULL);

		if (r == DD_OK)
		
			SoundDeviceFlag = 1;
		else
			SoundDeviceFlag = 2;

		return DDENUMRET_CANCEL;
	}

	return DDENUMRET_OK;
}

// **********************************************************************************************************************/

char SetupSound( HWND hwnd )
{

	if (NoSound)
		return TRUE;

	SoundDeviceFlag = SoundDeviceCount = 0;
	DirectSoundEnumerate( (LPDSENUMCALLBACK)FindSoundDeviceCallback, NULL);
	
	while (!SoundDeviceFlag);


	for( short h = 0; h < MAXSAMPLES; h++ )
		SampleLoaded[h] = 0;

	for( short i = 0; i < DUPLICATESAMPLES; i ++ )
		for( short j = 0; j < MAXSAMPLES; j++ )
			SampleUsed[i][j] = 0;

	// Setup Sound card

	DSObj->SetCooperativeLevel( hwnd, DSSCL_NORMAL );
	
	DSCAPS dscaps; 
	dscaps.dwSize = sizeof(DSCAPS); 
	DSObj->GetCaps(&dscaps); 
    DSObj->SetSpeakerConfig( DSSPEAKER_STEREO );


	// Create Primary Buffer

	DSBUFFERDESC DSBD;	
	ZeroMemory(&DSBD, sizeof(DSBUFFERDESC));
	DSBD.dwSize = sizeof(DSBUFFERDESC); 
	DSBD.dwFlags = DSBCAPS_PRIMARYBUFFER ;
	DSBD.dwBufferBytes = 0;
	DSBD.lpwfxFormat = NULL;

	DSObj->CreateSoundBuffer( &DSBD,&SBufferPrim,NULL );

	LoadWAV( "wavs\\moto.wav", 0 );


	return TRUE;
}


// **********************************************************************************************************************/

char LoadWAV( char *FileName, short Num )
{
	FILE *ap;

	unsigned char TempBuffer[64];

	if (NoSound)
		return TRUE;

	ap = fopen( FileName, "rb" );
	if (!ap)
		return FALSE;
	
	fread( &TempBuffer, 1, 4+8, ap  );
	
	fread( &TempBuffer, 1, 24, ap  );
	
	
	DSBUFFERDESC DSBD2;	
    PCMWAVEFORMAT pcmwf; 
	
    ZeroMemory(&pcmwf, sizeof(PCMWAVEFORMAT)); 
    pcmwf.wf.wFormatTag = WAVE_FORMAT_PCM; 
    pcmwf.wf.nChannels = *(unsigned short*) &TempBuffer[10]; 
    pcmwf.wf.nSamplesPerSec = *(unsigned short*) &TempBuffer[12]; 
    pcmwf.wf.nBlockAlign = pcmwf.wf.nChannels; 
    pcmwf.wf.nAvgBytesPerSec = pcmwf.wf.nSamplesPerSec * pcmwf.wf.nBlockAlign; 
    pcmwf.wBitsPerSample = 8; 

	fread( &TempBuffer, 1, 8, ap  );

    ZeroMemory(&DSBD2, sizeof(DSBUFFERDESC)); 
    DSBD2.dwSize = sizeof(DSBUFFERDESC); 
    DSBD2.dwFlags = DSBCAPS_CTRLALL | DSBCAPS_LOCSOFTWARE ; 
    DSBD2.dwBufferBytes = *(unsigned long*) &TempBuffer[4];
    DSBD2.lpwfxFormat = (LPWAVEFORMATEX)&pcmwf; 

    // Create secondary buffers 

	fread( &Sample[0], 1, DSBD2.dwBufferBytes, ap  );


	LPVOID SP, SP2;
	DWORD B, B2;

    for( short i = 0; i < DUPLICATESAMPLES; i++ )
	{

	DSObj->CreateSoundBuffer( &DSBD2, &SBufferSec[i][Num], NULL); 
    
	SBufferSec[i][Num]->SetVolume( DSBVOLUME_MAX );

	SampleLen[i][Num] = DSBD2.dwBufferBytes;
	SBufferSec[i][Num]->Lock(0, DSBD2.dwBufferBytes, &SP,  &B, &SP2, &B2, 0 );	
	CopyMemory( SP,&Sample[0], B );
	SBufferSec[i][Num]->Unlock( SP, B,  SP2, B2 );
	}
	
	SampleLoaded[Num] = 1;

	fclose (ap);

	return TRUE;
}


// **********************************************************************************************************************/

short PlaySample( short Num, LONG Vol, LONG Freq, LONG Pan, char Loop )
{
	// Num = Sample Number 
	// Vol = DSBVOLUME_MAX 
	// Freq =  50 - 44100
	// Pan = DSBPAN_CENTER DSBPAN_LEFT DSBPAN_RIGHT
	// Loop =  0 = No  1 = Yes

	// Returns Handle -1 if no buffers are free and sample was not played


	if (NoSound)
		return -1;

	short Handle = -1;

	if( SampleLoaded[Num] == 1 )
	{
       
		for( short i = 0; i < DUPLICATESAMPLES; i++ )
		if(	SampleUsed[i][Num] == 0 )
		{
			SampleUsed[i][Num] = 1;
			SBufferSec[i][Num]->SetVolume( Vol );
			SBufferSec[i][Num]->SetFrequency( Freq );
			SBufferSec[i][Num]->SetPan( Pan );
    
			if( Loop == 1)
				SBufferSec[i][Num]->Play( NULL, NULL, DSBPLAY_LOOPING );
			else
				SBufferSec[i][Num]->Play( NULL, NULL, NULL );
			
			Handle = i;
			i =  DUPLICATESAMPLES;
		}
	}

 
	return Handle;
}

// **********************************************************************************************************************/

char SoundManager()
{
	DWORD PlayStatus;

	if (NoSound)
		return TRUE;

	for( short i = 0; i < DUPLICATESAMPLES; i ++ )
		for( short j = 0; j < MAXSAMPLES; j++ )
			if( SampleUsed[i][j] == 1 )
			{
				SBufferSec[i][j]->GetStatus( &PlayStatus );
				if( (PlayStatus & DSBSTATUS_PLAYING)  == 0 )
					SampleUsed[i][j] = 0;

			}
			
	return TRUE;
}



// **********************************************************************************************************************/

char StopSample( short Num, short Handle )
{
	if (NoSound)
		return TRUE;

	if( SampleLoaded[Num] == 1 )
	{
			if( SampleUsed[Handle][Num] == 1 )
			{
				SBufferSec[Handle][Num]->Stop();
				SampleUsed[Handle][Num] = 0;
			}
	}

	return TRUE;
}

/***********************************************************************************************/

char ChangeSample( short Num, short Handle, LONG Vol, LONG Freq, LONG Pan )
{
	// Num = Sample Number 
	// Handle = as returned from PlaySample
	// Vol = DSBVOLUME_MAX 
	// Freq =  50 - 44100
	// Pan = DSBPAN_CENTER DSBPAN_LEFT DSBPAN_RIGHT
	
	
	if (NoSound)
		return TRUE;

	if( SampleLoaded[Num] == 1 )
	{
			SBufferSec[Handle][Num]->SetVolume( Vol );
			SBufferSec[Handle][Num]->SetFrequency( Freq );
			SBufferSec[Handle][Num]->SetPan( Pan );
	}

 
	return TRUE;
}

// **********************************************************************************************************************/

void KillSound( void )
{
	if (NoSound)
		return;

	RELEASE(DSObj);
}

// **********************************************************************************************************************/



