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

#include "SoundBank.h"
#ifdef OLD_AUDIO
  #include "sfx.h"
#else
  // TODO (JHarding): These header files were necessary in order to include 
  // object.h, which is not good.
  #include "revolt.h"
  #include "car.h"
  #include "ctrlread.h"
  #include "object.h"
#endif

static const DWORD MAX_ASSETS = 4;
static const DWORD MAX_VARIATIONS = 4;
static const DWORD MAX_EFFECTS = 60;
static const DWORD MAX_EFFECT_INSTANCES = 96;

static const DWORD VERSION_NUMBER = 0;

class CSoundEffectEngine;

// TODO (JHarding):
// Right now, this setup is pretty wasteful of space.  Need to optimize the
// Effect-Variation-Asset memory layout once we have a better idea of how
// we're using them.
class CSoundEffectAsset
{
public:
    DWORD           m_dwSoundBankEntry;       // Asset entry in sound bank
#if 0
    LONG            m_lVolume;                // Relative volume of asset
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
    DWORD                   m_dwNumVariations;
    CSoundEffectVariation   m_aVariations[MAX_VARIATIONS];
};

class CSoundEffectInstance
{
public:
    friend CSoundEffectEngine;

    BOOL    IsActive() { return m_bInUse; }
    HRESULT SetVolume( LONG lVolume ) { for( int i = 0; i < MAX_ASSETS; i++ ) if( m_apBuffers[i] ) m_apBuffers[i]->SetVolume( lVolume ); return S_OK; }
    HRESULT SetPitch( LONG lPitch ) { for( int i = 0; i < MAX_ASSETS; i++ ) if( m_apBuffers[i] ) m_apBuffers[i]->SetPitch( lPitch ); return S_OK; }

    LPDIRECTSOUNDBUFFER m_apBuffers[MAX_ASSETS];

private:
    DWORD               m_dwEffect;
    DWORD               m_dwVariation;
 #ifdef OLD_AUDIO
    SAMPLE_3D*          m_pSample3D;
 #else
    OBJECT*             m_pObject;
 #endif

    LONG                m_bInUse:1;
    LONG                m_bReserved:1;
    LONG                m_bSource:1;
};

class CSoundEffectEngine
{
public:
    CSoundEffectEngine();
    ~CSoundEffectEngine();

    HRESULT Initialize();
    HRESULT LoadLevel( CHAR* strLevel );
    HRESULT Unload();

    HRESULT Play3DSound( DWORD dwIndex, 
                         BOOL bLooping, 
 #ifdef OLD_AUDIO
                         SAMPLE_3D* pSample3D,
 #else
                         OBJECT* obj, 
 #endif
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
 #ifdef OLD_AUDIO
    HRESULT CreateSoundSource( SAMPLE_3D* pSample3D, CSoundEffectInstance** ppInstance );
 #else
    HRESULT CreateSoundSource( OBJECT* obj, CSoundEffectInstance** ppInstance );
 #endif

    // TODO (JHarding): Implement these
    HRESULT StopAll();
    HRESULT PauseAll();
    HRESULT ResumeAll();

private:
    CSoundEffectInstance* GetFreeInstance();

    CSoundEffectInstance m_aInstances[ MAX_EFFECT_INSTANCES ];
    DWORD m_dwLastReturnedInstance;

    DWORD                  m_dwNumEffects;
    CSoundEffectDefinition m_aEffectDefinitions[ MAX_EFFECTS ];
    CSoundEffectAsset*     m_pAssets;
};

extern CSoundEffectEngine g_SoundEngine;

#endif // SOUNDEFFECTENGINE_H
