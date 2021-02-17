//-----------------------------------------------------------------------------
// File: SoundEffectEngine.h
//
// Desc: Definition of Sound Effect Engine interface.  The sound effect engine
//       is the interface through which the game code triggers, updates, and
//       manipulates various sound effects.
//
// Hist: 1.06.02 - Created
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef SOUNDEFFECTENGINE_H
#define SOUNDEFFECTENGINE_H

#include <xtl.h>
#include <dsound.h>
//TODO (JarHead): Figure out where these header files are generated and do it manually

// These header files are generated when building the Sound Effect Description
// (*.sdf) files.  
#include "sounds_common.h"
#include "sounds_garden1.h"
#include "sounds_market1.h"
#include "sounds_muse1.h"
#include "sounds_nhood1.h"
#include "sounds_ship1.h"
#include "sounds_toylite.h"
#include "sounds_wild_west1.h"


#include "SoundBank.h"
// TODO (JHarding): These header files were necessary in order to include 
// object.h, which is not good.
#include "revolt.h"
#include "car.h"
#include "ctrlread.h"
#include "object.h"

static const DWORD MAX_ASSETS = 4;
static const DWORD MAX_VARIATIONS = 4;
static const DWORD MAX_EFFECTS = 60;
static const DWORD MAX_EFFECT_INSTANCES = 202;

static const DWORD VERSION_NUMBER = 0;

// Sound effect volume range of 60dB
static const DWORD SOUNDEFFECT_VOLUME_RANGE = 6000;

class CSoundEffectEngine;
extern DWORD g_dwLevelSoundsOffset;

// TODO (JHarding):
// Right now, this setup is pretty wasteful of space.  Need to optimize the
// Effect-Variation-Asset memory layout once we have a better idea of how
// we're using them.
class CSoundEffectAsset
{
public:
    DWORD           m_dwSoundBankEntry;       // Asset entry in sound bank
    LONG            m_lVolume;                // Relative volume of asset
#if 0
    LONG            m_lPitch;                 // Relative pitch of asset
    DSENVELOPEDESC  m_AmplitudeEnvelope;      // Amplitude envelope params
    DSENVELOPEDESC  m_MultiEnvelope;          // Multi-function envelope params
    DSLFODESC       m_PitchLFO;               // Pitch LFO params
    DSLFODESC       m_MultiLFO;               // Multi-function LFO params
    DSFILTERDESC    m_FilterBlock;            // Filter settings
#endif
};

class CSoundEffectVariation
{
public:
    DWORD               m_dwNumAssets;
    SHORT               m_aAssets[MAX_ASSETS];
};

class CSoundEffectDefinition
{
public:
    FLOAT                   m_fRolloffFactor;
    DWORD                   m_dwNumVariations;
    CSoundEffectVariation   m_aVariations[MAX_VARIATIONS];
};

struct SOUNDBUFFER
{
    LPDIRECTSOUNDBUFFER pDSBuffer;
    BOOL                bPaused;
    BOOL                bLooping;
};

class CSoundEffectInstance
{
public:
    friend CSoundEffectEngine;

    BOOL    IsActive() { return m_bInUse; }
    HRESULT SetVolume( LONG lVolume );
    HRESULT SetPitch( LONG lPitch ) { for( int i = 0; i < MAX_ASSETS; i++ ) if( m_apBuffers[i].pDSBuffer ) m_apBuffers[i].pDSBuffer->SetPitch( lPitch ); return S_OK; }

    SOUNDBUFFER         m_apBuffers[MAX_ASSETS];

    DWORD               m_dwEffect;
private:
    DWORD               m_dwVariation;
    LONG                m_lVolume;
    OBJECT*              m_pObject;

    LONG                m_bInUse:1;
    LONG                m_bReserved:1;
    LONG                m_bSource:1;
};

class CSoundEffectEngine
{
public:
    friend CSoundEffectInstance;

    CSoundEffectEngine();
    ~CSoundEffectEngine();

    HRESULT Initialize();
    HRESULT LoadSounds( CHAR* strSoundFile, DWORD* pdwIndex = NULL );
    HRESULT Unload();

    HRESULT Play3DSound( DWORD dwIndex, 
                         BOOL bLooping, 
                         OBJECT* obj, 
                         CSoundEffectInstance** ppInstance = NULL );
    HRESULT Play3DSound( DWORD dwIndex, 
                         BOOL bLooping, 
                         FLOAT fX, 
                         FLOAT fY, 
                         FLOAT fZ, 
                         CSoundEffectInstance** ppInstance = NULL );
    HRESULT Play2DSound( DWORD dwIndex, 
                         BOOL bLooping, 
                         CSoundEffectInstance** ppInstance = NULL );
    HRESULT PlaySubmixedSound( DWORD dwIndex,
                               BOOL bLooping,
                               CSoundEffectInstance* pMixin,
                               CSoundEffectInstance** ppInstance = NULL );
    HRESULT UpdateAll();

    HRESULT ReturnInstance( CSoundEffectInstance* pInstance );
    HRESULT CreateSoundSource( OBJECT* obj, CSoundEffectInstance** ppInstance );

    HRESULT StopAll();
    HRESULT PauseAll();
    HRESULT ResumeAll();

    HRESULT SetVolume( LONG lPercent );

private:
    CSoundEffectInstance* GetFreeInstance();

    CSoundEffectInstance m_aInstances[ MAX_EFFECT_INSTANCES ];
    DWORD m_dwLastReturnedInstance;

    DWORD                  m_dwNumEffects;
    CSoundEffectDefinition m_aEffectDefinitions[ MAX_EFFECTS ];
    DWORD                  m_dwNumAssets;
    CSoundEffectAsset*     m_pAssets;
    LONG                   m_lDSVolume;
};

extern CSoundEffectEngine g_SoundEngine;

#endif // SOUNDEFFECTENGINE_H
