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

static const DWORD MAX_ASSETS = 4;
static const DWORD MAX_VARIATIONS = 4;
static const DWORD MAX_EFFECTS = 60;
static const DWORD MAX_EFFECT_INSTANCES = 64;

static const DWORD MAX_TOTAL_ASSETS = MAX_EFFECTS * MAX_VARIATIONS * MAX_ASSETS;

class CSoundEffectEngine;

typedef struct _DSENVELOPEDESC
{
    DWORD           dwEG;                   // Envelope generator to set data on
    DWORD           dwMode;                 // Envelope mode
    DWORD           dwDelay;                // Count of 512-sample blocks to delay before attack
    DWORD           dwAttack;               // Attack segment length, in 512-sample blocks
    DWORD           dwHold;                 // Count of 512-sample blocks to hold after attack
    DWORD           dwDecay;                // Decay segment length, in 512-sample blocks
    DWORD           dwRelease;              // Release segment length, in 512-sample blocks
    DWORD           dwSustain;              // Sustain level
    LONG            lPitchScale;            // Pitch scale (multi-function envelope only)
    LONG            lFilterCutOff;          // Filter cut-off (multi-function envelope only)
} DSENVELOPEDESC, *LPDSENVELOPEDESC;

typedef struct _DSLFODESC
{
    DWORD           dwLFO;                  // LFO to set data on
    DWORD           dwDelay;                // Initial delay before LFO is applied, in 32-sample blocks
    DWORD           dwDelta;                // Delta added to LFO each frame
    LONG            lPitchModulation;       // Pitch modulation
    LONG            lFilterCutOffRange;     // Frequency cutoff range (multi-function LFO only)
    LONG            lAmplitudeModulation;   // Amplitude modulation (multi-function LFO only)
} DSLFODESC, *LPDSLFODESC;

typedef struct _DSFILTERDESC
{
    DWORD           dwMode;                 // Filter mode
    DWORD           dwQCoefficient;         // Q-coefficient (PEQ only)
    DWORD           adwCoefficients[4];     // Filter coefficients
} DSFILTERDESC, *LPDSFILTERDESC;

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
#endif // 0
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
    FLOAT                   m_fRolloffFactor;         // Rolloff curve
    DWORD                   m_dwNumVariations;
    CSoundEffectVariation   m_aVariations[MAX_VARIATIONS];
};

#endif // SOUNDEFFECTENGINE_H
