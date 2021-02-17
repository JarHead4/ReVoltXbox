//-----------------------------------------------------------------------------
// File: SoundEffectEngine.h
//
// Desc: Implementation of Sound Effect Engine.
//
// Hist: 1.06.02 - Created
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "SoundEffectEngine.h"
#include "camera.h"
#include "instance.h"

CSoundEffectEngine g_SoundEngine;


//-----------------------------------------------------------------------------
// Name: CSoundEffectEngine (ctor)
// Desc: Initializes member variables
//-----------------------------------------------------------------------------
CSoundEffectEngine::CSoundEffectEngine()
{
    ZeroMemory( m_aInstances, sizeof( m_aInstances ) );
    m_dwLastReturnedInstance = 0;
}




//-----------------------------------------------------------------------------
// Name: ~CSoundEffectEngine (dtor)
// Desc: Verifies that object was properly shut down
//-----------------------------------------------------------------------------
CSoundEffectEngine::~CSoundEffectEngine()
{
    for( DWORD i = 0; i < MAX_INSTANCES; i++ )
    {
        // all instances should really be shut down before the engine goes down
        assert( !m_aInstances[i].m_bInUse && 
                !m_aInstances[i].m_bReserved );
    }
}




//-----------------------------------------------------------------------------
// Name: Initialize
// Desc: Performs any one-time intialization needes
//-----------------------------------------------------------------------------
HRESULT CSoundEffectEngine::Initialize()
{
    g_SoundBank.Initialize();
    m_dwNumEffects = 0;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: LoadSounds
// Desc: Loads a sound effects file and its corresponding wave bank
//-----------------------------------------------------------------------------
HRESULT CSoundEffectEngine::LoadSounds( CHAR* strSoundFile, DWORD* pdwIndex )
{
    DWORD dwLen = strlen( strSoundFile );
    DWORD dwBankOffset;

    // Cook up the wave bank file name
    CHAR* strSoundBank = new CHAR[ dwLen + 1 ];
    strncpy( strSoundBank, strSoundFile, dwLen - 3 );
    strcpy( strSoundBank + dwLen - 3, "xwb" );
    
    // First, load the corresponding wave bank
    g_SoundBank.LoadSoundBank( strSoundBank, &dwBankOffset );

    // These are the maximum sizes of our pools given that:
    //     2 2D streams for Music
    // +   4 2D buffers for internal DSound use
    // +  24 2D streams for voice mixing
    // = 162 2D voices remaining
    //
    //    24 3D mixin buffers for cars
    // =  40 3D voices remaining
    g_SoundBank.InitializeBufferPool( 162, 40 );

    // Crack open the SFX file
    HANDLE hFile = CreateFile( strSoundFile, 
                               GENERIC_READ,
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL );
    if( hFile == INVALID_HANDLE_VALUE )
        return E_FAIL;

    // Read and verify the header
    DWORD dwRead;
    DWORD dwVersion;
    ReadFile( hFile, &dwVersion, sizeof( DWORD ), &dwRead, NULL );
    if( dwVersion != VERSION_NUMBER )
    {
        OutputDebugString( "Sound effect file is wrong version.\n" );
        return E_FAIL;
    }

    // Read in the effect list, appending to the end of the current list
    DWORD dwNumOldEffects = m_dwNumEffects;
    DWORD dwNumNewEffects;
    ReadFile( hFile, 
              &dwNumNewEffects, 
              sizeof( DWORD ), 
              &dwRead, 
              NULL );
    ReadFile( hFile, 
              &m_aEffectDefinitions[dwNumOldEffects], 
              dwNumNewEffects * sizeof( CSoundEffectDefinition ), 
              &dwRead, 
              NULL );
    m_dwNumEffects = dwNumOldEffects + dwNumNewEffects;

    // Read in the asset list, appending to the end of the current list
    DWORD dwNumOldAssets = m_dwNumAssets;
    DWORD dwNumNewAssets;
    ReadFile( hFile, &dwNumNewAssets, sizeof( DWORD ), &dwRead, NULL );

    // Re-allocate our asset buffer, if needed
    CSoundEffectAsset* pNewAssets = new CSoundEffectAsset[ dwNumOldAssets + dwNumNewAssets ];
    if( m_pAssets )
    {
        memcpy( pNewAssets, m_pAssets, dwNumOldAssets * sizeof( CSoundEffectAsset ) );
        delete[] m_pAssets;
    }
    m_pAssets = pNewAssets;

    ReadFile( hFile, &m_pAssets[dwNumOldAssets], sizeof( CSoundEffectAsset ) * dwNumNewAssets, &dwRead, NULL );
    m_dwNumAssets = dwNumOldAssets + dwNumNewAssets;

    // Done with the SFX file
    CloseHandle( hFile );

    // Fix up sound entries to point to appropriate asset entries
    for( DWORD i = dwNumOldEffects; i < m_dwNumEffects; i++ )
    {
        for( DWORD j = 0; j < m_aEffectDefinitions[i].m_dwNumVariations; j++ )
        {
            for( DWORD k = 0; k < m_aEffectDefinitions[i].m_aVariations[j].m_dwNumAssets; k++ )
            {
                m_aEffectDefinitions[i].m_aVariations[j].m_aAssets[k] += (SHORT)dwNumOldAssets;
            }
        }
    }

    // Fix up relative wave bank entries
    for( DWORD i = dwNumOldAssets; i < m_dwNumAssets; i++ )
    {
        m_pAssets[i].m_dwSoundBankEntry += dwBankOffset;
    }

    if( pdwIndex )
    {
        *pdwIndex = dwNumOldAssets;
    }
    
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: Unload
// Desc: Releases all level resources
//-----------------------------------------------------------------------------
HRESULT CSoundEffectEngine::Unload()
{
    for( DWORD i = 0; i < MAX_EFFECT_INSTANCES; i++ )
    {
        // Anyone who had a handle to an instance should
        // have returned it by now.
        Assert( !m_aInstances[ i ].m_bReserved );
    }

    StopAll();
    UpdateAll();

    g_SoundBank.FreeBufferPool();
    g_SoundBank.FreeSoundBank();


    m_dwNumEffects = 0;
    delete[] m_pAssets;
    m_pAssets = NULL;
    m_dwNumAssets = 0;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: Play3DSound (object)
// Desc: Plays the specified sound effect as a 3d-positioned sound, tracking
//          the specified object
//-----------------------------------------------------------------------------
HRESULT CSoundEffectEngine::Play3DSound( DWORD dwIndex, 
                                         BOOL bLooping, 
                                         OBJECT* obj, 
                                         CSoundEffectInstance** ppInstance )
{
    assert( dwIndex < m_dwNumEffects );

    CSoundEffectInstance* pNewInstance = GetFreeInstance();

    // If looping, they really should be holding onto the instance
    Assert( !bLooping || ppInstance );

    pNewInstance->m_bInUse = 1;
    pNewInstance->m_dwEffect = dwIndex;

    DWORD dwVariation = rand() % m_aEffectDefinitions[ dwIndex ].m_dwNumVariations;
    pNewInstance->m_dwVariation = dwVariation;
    CSoundEffectVariation* pVariation = &( m_aEffectDefinitions[ dwIndex ].m_aVariations[ dwVariation ] );

    // Set up buffers for each asset in the variation
    for( DWORD i = 0; i < pVariation->m_dwNumAssets; i++ )
    {
        if( FAILED( g_SoundBank.GetBuffer( TRUE, &( pNewInstance->m_apBuffers[i].pDSBuffer ) ) ) )
        {
            ReturnInstance( pNewInstance );
            if( ppInstance )
                *ppInstance = NULL;
            return E_FAIL;
        }
        pNewInstance->m_apBuffers[i].pDSBuffer->SetPosition( obj->body.Centre.Pos.v[0], 
                                                   obj->body.Centre.Pos.v[1], 
                                                   obj->body.Centre.Pos.v[2], 
                                                   DS3D_DEFERRED );
        pNewInstance->m_apBuffers[i].pDSBuffer->SetRolloffFactor( m_aEffectDefinitions[ dwIndex ].m_fRolloffFactor, DS3D_IMMEDIATE );
    }

    // Set up instance parameters
    pNewInstance->m_pObject = obj;
    pNewInstance->SetVolume( 0 );

    // Play each buffer in rapid succession
    for( DWORD i = 0; i < pVariation->m_dwNumAssets; i++ )
    {
        g_SoundBank.PlaySound( m_pAssets[ pVariation->m_aAssets[i] ].m_dwSoundBankEntry, 
                               bLooping, 
                               TRUE,
                               pNewInstance->m_apBuffers[i].pDSBuffer );
    }

    if( ppInstance )
    {
        // Make sure we're not overwriting an old instance.
        assert( *ppInstance == NULL );
        *ppInstance = pNewInstance;
        pNewInstance->m_bReserved = TRUE;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: Play3DSound (position)
// Desc: Plays the specified sound effect as a 3d positioned sound at the
//          given position
//-----------------------------------------------------------------------------
HRESULT CSoundEffectEngine::Play3DSound( DWORD dwIndex, 
                                         BOOL bLooping, 
                                         FLOAT fX, 
                                         FLOAT fY, 
                                         FLOAT fZ,
                                         CSoundEffectInstance** ppInstance )
{
    assert( dwIndex < m_dwNumEffects );

    CSoundEffectInstance* pNewInstance = GetFreeInstance();

    // If looping, they really should be holding onto the instance
    Assert( !bLooping || ppInstance );

    pNewInstance->m_bInUse = 1;
    pNewInstance->m_dwEffect = dwIndex;

    DWORD dwVariation = rand() % m_aEffectDefinitions[ dwIndex ].m_dwNumVariations;
    pNewInstance->m_dwVariation = dwVariation;
    CSoundEffectVariation* pVariation = &( m_aEffectDefinitions[ dwIndex ].m_aVariations[ dwVariation ] );

    // Set up buffers for each asset in the variation
    for( DWORD i = 0; i < pVariation->m_dwNumAssets; i++ )
    {
        if( FAILED( g_SoundBank.GetBuffer( TRUE, &(pNewInstance->m_apBuffers[i].pDSBuffer) ) ) )
        {
            ReturnInstance( pNewInstance );
            if( ppInstance )
                *ppInstance = NULL;

            return E_FAIL;
        }

        if( _finite( fX ) &&
            _finite( fY ) &&
            _finite( fZ ) )
        {
            pNewInstance->m_apBuffers[i].pDSBuffer->SetPosition( fX, fY, fZ, DS3D_DEFERRED );
        }
        pNewInstance->m_apBuffers[i].pDSBuffer->SetRolloffFactor( m_aEffectDefinitions[ dwIndex ].m_fRolloffFactor, DS3D_IMMEDIATE );
    }

    // Set up instance parameters
    pNewInstance->SetVolume( 0 );

    // Play each buffer in rapid succession
    for( DWORD i = 0; i < pVariation->m_dwNumAssets; i++ )
    {
        g_SoundBank.PlaySound( m_pAssets[ pVariation->m_aAssets[i] ].m_dwSoundBankEntry, 
                               bLooping, 
                               TRUE,
                               pNewInstance->m_apBuffers[i].pDSBuffer );
    }

    if( ppInstance )
    {
        // Make sure we're not overwriting an old instance.
        assert( *ppInstance == NULL );
        *ppInstance = pNewInstance;
        pNewInstance->m_bReserved = TRUE;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: Play2DSound
// Desc: Plays the specified sound effect as non-positioned sound
//-----------------------------------------------------------------------------
HRESULT CSoundEffectEngine::Play2DSound( DWORD dwIndex, 
                                         BOOL bLooping,
                                         CSoundEffectInstance** ppInstance )
{
    //$HACK(Apr02_GameBash) - when accepting an invitiation, we end up 
    // playing menu sounds w/ sounds unloaded
    if( dwIndex >= m_dwNumEffects )
        return E_FAIL;

    assert( dwIndex < m_dwNumEffects );

    CSoundEffectInstance* pNewInstance = GetFreeInstance();

    // If looping, they really should be holding onto the instance
    Assert( !bLooping || ppInstance );

    pNewInstance->m_bInUse = 1;
    pNewInstance->m_dwEffect = dwIndex;

    // Pick a variation
    DWORD dwVariation = rand() % m_aEffectDefinitions[ dwIndex ].m_dwNumVariations;
    pNewInstance->m_dwVariation = dwVariation;
    CSoundEffectVariation* pVariation =  &( m_aEffectDefinitions[ dwIndex ].m_aVariations[ dwVariation ] );

    // Set up buffers for each asset in the variation
    for( DWORD i = 0; i < pVariation->m_dwNumAssets; i++ )
    {
        if( FAILED( g_SoundBank.GetBuffer( FALSE, &(pNewInstance->m_apBuffers[i].pDSBuffer) ) ) )
        {
            ReturnInstance( pNewInstance );
            if( ppInstance )
                *ppInstance = NULL;

            return E_FAIL;
        }
    }

    // Set up instance parameters
    pNewInstance->SetVolume( 0 );

    // Play each buffer in rapid succession
    for( DWORD i = 0; i < pVariation->m_dwNumAssets; i++ )
    {
        g_SoundBank.PlaySound( m_pAssets[ pVariation->m_aAssets[i] ].m_dwSoundBankEntry, 
                               bLooping, 
                               FALSE,
                               pNewInstance->m_apBuffers[i].pDSBuffer );
    }

    if( ppInstance )
    {
        // Make sure we're not overwriting an old instance.
        assert( *ppInstance == NULL );
        *ppInstance = pNewInstance;
        pNewInstance->m_bReserved = TRUE;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: PlaySubmixedSound
// Desc: Plays the specified sound effect, mixed into the given mix destination
//-----------------------------------------------------------------------------
HRESULT CSoundEffectEngine::PlaySubmixedSound( DWORD dwIndex, 
                                               BOOL bLooping,
                                               CSoundEffectInstance* pMixin,
                                               CSoundEffectInstance** ppInstance )
{
    assert( dwIndex < m_dwNumEffects );

    CSoundEffectInstance* pNewInstance = GetFreeInstance();

    // If looping, they really should be holding onto the instance
    Assert( !bLooping || ppInstance );

    pNewInstance->m_bInUse = 1;
    pNewInstance->m_dwEffect = dwIndex;

    // Pick a variation
    DWORD dwVariation = rand() % m_aEffectDefinitions[ dwIndex ].m_dwNumVariations;
    pNewInstance->m_dwVariation = dwVariation;
    CSoundEffectVariation* pVariation =  &( m_aEffectDefinitions[ dwIndex ].m_aVariations[ dwVariation ] );

    // Set up buffers for each asset in the variation
    for( DWORD i = 0; i < pVariation->m_dwNumAssets; i++ )
    {
        if( FAILED( g_SoundBank.GetBuffer( FALSE, &(pNewInstance->m_apBuffers[i].pDSBuffer) ) ) )
        {
            ReturnInstance( pNewInstance );
            if( ppInstance )
                *ppInstance = NULL;

            return E_FAIL;
        }
        pNewInstance->m_apBuffers[i].pDSBuffer->SetOutputBuffer( pMixin->m_apBuffers[ 0 ].pDSBuffer );
    }

    // Set up instance parameters
    pNewInstance->SetVolume( 0 );

    // Play each buffer in rapid succession
    for( DWORD i = 0; i < pVariation->m_dwNumAssets; i++ )
    {
        g_SoundBank.PlaySound( m_pAssets[ pVariation->m_aAssets[i] ].m_dwSoundBankEntry, 
                               bLooping, 
                               FALSE,
                               pNewInstance->m_apBuffers[i].pDSBuffer,
                               pMixin->m_apBuffers[ 0 ].pDSBuffer );
    }

    if( ppInstance )
    {
        // Make sure we're not overwriting an old instance.
        assert( *ppInstance == NULL );
        *ppInstance = pNewInstance;
        pNewInstance->m_bReserved = TRUE;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: UpdateAll
// Desc: Run once per frame to handle maintenance of all sound effects:
//       1) Scrub the list of active sound effect instances to see if anyone
//          has stopped playing
//       2) Update position of all object-tracking sound effect instances
//       3) Update position and orientation of the listener
//       4) Commit 3d changes
//       5) Pump DirectSound's work queue
//-----------------------------------------------------------------------------
HRESULT CSoundEffectEngine::UpdateAll()
{
    g_SoundBank.ScrubBusyList();
#if _DEBUG
    DWORD dwInUse = 0;
#endif // _DEBUG

    // 1) Scrub the list for completed sounds
    for( DWORD i = 0; i < MAX_EFFECT_INSTANCES; i++ )
    {
        // Only check active, non submix instances
        if( m_aInstances[i].m_bInUse &&
            !m_aInstances[i].m_bSource )
        {
            // Assume we're not playing anymore
            m_aInstances[i].m_bInUse = FALSE;

            // Check the status of each buffer
            DWORD dwStatus;
            for( DWORD j = 0; j < MAX_ASSETS; j++ )
            {
                if( m_aInstances[i].m_apBuffers[j].pDSBuffer != NULL )
                {
                    if( m_aInstances[i].m_apBuffers[j].bPaused )
                    {
                        // Paused implies active
                        m_aInstances[i].m_bInUse = TRUE;
                    }
                    else
                    {
                        // Check the status of the buffer
                        m_aInstances[i].m_apBuffers[j].pDSBuffer->GetStatus( &dwStatus );

                        if( dwStatus & DSBSTATUS_PLAYING )
                        {
                            // If any of the buffers are playing, then
                            // the instance is still active
                            m_aInstances[i].m_bInUse = TRUE;
                        }
                        else // if( !m_aInstances[i].m_bReserved )
                        {
                            // We can return any unneeded buffers
                            m_aInstances[i].m_apBuffers[j].pDSBuffer->SetOutputBuffer( NULL );
                            g_SoundBank.ReturnBuffer( m_aInstances[i].m_apBuffers[j].pDSBuffer );
                            m_aInstances[i].m_apBuffers[j].pDSBuffer = NULL;
                        }
                    }
                }
            }

            // If it's no longer playing, we don't need to track the object
            // any more
            if( !m_aInstances[i].m_bInUse )
            {
                m_aInstances[i].m_pObject = NULL;
            }
        }

        // Update all Instances that are tracking objects
        if( m_aInstances[i].m_bInUse && m_aInstances[i].m_pObject )
        {
            DWORD dwNumBuffers;

            if( m_aInstances[i].m_bSource )
            {
                dwNumBuffers = 1;
            }
            else
            {
                CSoundEffectDefinition* pEffect = &(m_aEffectDefinitions[ m_aInstances[i].m_dwEffect ]);
                CSoundEffectVariation* pVariation = &(pEffect->m_aVariations[ m_aInstances[i].m_dwVariation ]);

                dwNumBuffers = pVariation->m_dwNumAssets;
            }

            for( DWORD j = 0; j < dwNumBuffers; j++ )
            {
                // BUGBUG (JHarding, 1/24/02): The physics engine is sometimes generating NaNs - 
                // rather than try to grok the entire physics model, we're going to protect against
                // using the NaNs here, since none of the rest of the game engine has a problem 
                // with them.
                if( _finite( m_aInstances[i].m_pObject->body.Centre.Pos.v[0] ) &&
                    _finite( m_aInstances[i].m_pObject->body.Centre.Pos.v[1] ) &&
                    _finite( m_aInstances[i].m_pObject->body.Centre.Pos.v[2] ) )
                {
                    m_aInstances[i].m_apBuffers[j].pDSBuffer->SetPosition( m_aInstances[i].m_pObject->body.Centre.Pos.v[0],
                                                                 m_aInstances[i].m_pObject->body.Centre.Pos.v[1],
                                                                 m_aInstances[i].m_pObject->body.Centre.Pos.v[2],
                                                                 DS3D_DEFERRED );
                }
            }
        }

#if _DEBUG
        if( m_aInstances[i].m_bInUse ||
            m_aInstances[i].m_bReserved )
        {
            dwInUse++;
        }
#endif // _DEBUG
    }

#if _DEBUG
    if( dwInUse > MAX_EFFECT_INSTANCES * 95 / 100 )
    {
        OutputDebugString( "SoundEngine: Warning! Over 95% of Effect Instances are in use!\n" );
    }
#endif // _DEBUG

    // BUGBUG (JHarding, 1/24/02): The physics engine is sometimes generating NaNs - 
    // rather than try to grok the entire physics model, we're going to protect against
    // using the NaNs here, since none of the rest of the game engine has a problem 
    // with them.
    if( _finite( CAM_MainCamera->WPos.v[0] ) &&
        _finite( CAM_MainCamera->WPos.v[1] ) &&
        _finite( CAM_MainCamera->WPos.v[2] ) )
    {
        g_SoundBank.GetDSound()->SetPosition( CAM_MainCamera->WPos.v[0],
                                              CAM_MainCamera->WPos.v[1],
                                              CAM_MainCamera->WPos.v[2],
                                              DS3D_DEFERRED );
    }

    VEC vLook = { 0.0f, 0.0f, 1.0f };
    VEC vLookResult;
    VEC vUp = { 0.0f, 1.0f, 0.0f };
    VEC vUpResult;
    RotVector( &CAM_MainCamera->WMatrix, &vLook, &vLookResult );
    RotVector( &CAM_MainCamera->WMatrix, &vUp, &vUpResult );
    // BUGBUG (cprince, 2002 Feb 19): Also protect against NAN here, until
    // we track down then root problem.
    if( _finite( vLookResult.v[0] ) &&
        _finite( vLookResult.v[1] ) &&
        _finite( vLookResult.v[2] ) &&
        _finite( vUpResult.v[0] )   &&
        _finite( vUpResult.v[1] )   &&
        _finite( vUpResult.v[2] )   )
    {
        g_SoundBank.GetDSound()->SetOrientation( vLookResult.v[0], 
                                                 vLookResult.v[1], 
                                                 vLookResult.v[2], 
                                                 vUpResult.v[0], 
                                                 vUpResult.v[1], 
                                                 vUpResult.v[2],
                                                 DS3D_DEFERRED );
    }

    g_SoundBank.GetDSound()->CommitDeferredSettings();

    DirectSoundDoWork();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: GetFreeInstance
// Desc: Returns a free sound effect instance from the pool
//-----------------------------------------------------------------------------
CSoundEffectInstance* CSoundEffectEngine::GetFreeInstance()
{
    DWORD dwTest = ( m_dwLastReturnedInstance + 1 ) % MAX_EFFECT_INSTANCES ;

    while( m_aInstances[ dwTest ].m_bInUse ||
           m_aInstances[ dwTest ].m_bReserved )
    {
        dwTest = ( dwTest + 1 ) % MAX_EFFECT_INSTANCES ;
        Assert( dwTest != m_dwLastReturnedInstance );
    }

    ZeroMemory( &m_aInstances[ dwTest ], sizeof( CSoundEffectInstance ) );
    return &m_aInstances[ dwTest ];
}




//-----------------------------------------------------------------------------
// Name: ReturnInstance
// Desc: Returns a sound effect instance to the pool.  Note that this
//          automatically stops all buffers, and that the instance won't
//          really be available until the voices have shut off
//-----------------------------------------------------------------------------
HRESULT CSoundEffectEngine::ReturnInstance( CSoundEffectInstance* pInstance )
{
    pInstance->m_bReserved = FALSE;

    if( pInstance->m_bSource )
    {
        Assert( pInstance->m_apBuffers[0].pDSBuffer != NULL &&
                pInstance->m_apBuffers[1].pDSBuffer == NULL &&
                pInstance->m_apBuffers[2].pDSBuffer == NULL &&
                pInstance->m_apBuffers[3].pDSBuffer == NULL );
        
        INT nRefs = pInstance->m_apBuffers[0].pDSBuffer->Release();
        assert( nRefs == 0 );
        pInstance->m_apBuffers[0].pDSBuffer = NULL;
        pInstance->m_bSource = FALSE;
    }
    else
    {
        for( DWORD i = 0; i < MAX_ASSETS; i++ )
        {
            if( pInstance->m_apBuffers[ i ].pDSBuffer )
            {
                pInstance->m_apBuffers[ i ].bPaused = FALSE;
                pInstance->m_apBuffers[ i ].pDSBuffer->SetOutputBuffer( NULL );
                pInstance->m_apBuffers[ i ].pDSBuffer->StopEx( 0, DSBSTOPEX_IMMEDIATE );
                if( !pInstance->m_bInUse )
                {
                    g_SoundBank.ReturnBuffer( pInstance->m_apBuffers[i].pDSBuffer );
                    pInstance->m_apBuffers[i].pDSBuffer = NULL;
                }
            }
        }
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CreateSoundSource
// Desc: Creates a mixin buffer to be used as a positionable sound source
//-----------------------------------------------------------------------------
HRESULT CSoundEffectEngine::CreateSoundSource( OBJECT* obj, CSoundEffectInstance** ppInstance )
{
    *ppInstance = GetFreeInstance();

    (*ppInstance)->m_bSource    = TRUE;
    (*ppInstance)->m_bInUse     = TRUE;
    (*ppInstance)->m_bReserved  = TRUE;
    (*ppInstance)->m_pObject    = obj;

    DSBUFFERDESC dsbd = {0};
    dsbd.dwSize = sizeof( DSBUFFERDESC );
    dsbd.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_MIXIN;

    HRESULT hr = DirectSoundCreateBuffer( &dsbd, &((*ppInstance)->m_apBuffers[0].pDSBuffer) );
    assert( SUCCEEDED( hr ) &&
            (*ppInstance)->m_apBuffers[0].pDSBuffer != NULL );

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CSoundEffectEngine::StopAll()
{
    for( DWORD i = 0; i < MAX_EFFECT_INSTANCES; i++ )
    {
        // That said, a fire-and-forget instance could still
        // be playing.  In that case, shut down all buffers,
        // and wait for it to really shut off
        if( m_aInstances[i].m_bInUse &&
            !m_aInstances[i].m_bSource )
        {
            for( DWORD j = 0; j < MAX_ASSETS; j++ )
            {
                if( m_aInstances[ i ].m_apBuffers[ j ].pDSBuffer )
                {
                    m_aInstances[ i ].m_apBuffers[ j ].bPaused = FALSE;
                    m_aInstances[ i ].m_apBuffers[ j ].pDSBuffer->SetOutputBuffer( NULL );
                    m_aInstances[ i ].m_apBuffers[ j ].pDSBuffer->StopEx( 0, DSBSTOPEX_IMMEDIATE );

                    DWORD dwStatus;
                    do
                    {
                        m_aInstances[ i ].m_apBuffers[ j ].pDSBuffer->GetStatus( &dwStatus );
                    } while( dwStatus & DSBSTATUS_PLAYING );
                }
            }
        }
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: PauseAll
// Desc: Pauses all currently playing sound effects, remembering who was
//          actually playing and if they were looping
//-----------------------------------------------------------------------------
HRESULT CSoundEffectEngine::PauseAll()
{
    for( DWORD i = 0; i < MAX_EFFECT_INSTANCES; i++ )
    {
        if( m_aInstances[i].m_bInUse &&
            !m_aInstances[i].m_bSource )
        {
            for( DWORD j = 0; j < MAX_ASSETS; j++ )
            {
                if( m_aInstances[ i ].m_apBuffers[ j ].pDSBuffer )
                {
                    DWORD dwStatus;
                    m_aInstances[ i ].m_apBuffers[ j ].pDSBuffer->GetStatus( &dwStatus );

                    if( dwStatus & DSBSTATUS_PLAYING )
                    {
                        m_aInstances[ i ].m_apBuffers[ j ].bPaused = TRUE;
                        m_aInstances[ i ].m_apBuffers[ j ].bLooping = dwStatus & DSBSTATUS_LOOPING;
                        m_aInstances[ i ].m_apBuffers[ j ].pDSBuffer->Stop();
                    }
                }   // Active buffer
            }   // Asset loop
        }   // Active instance
    }   // Instance loop

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ResumeAll
// Desc: Resumes all paused buffers.  Note that this can be expensive, because
//       resuming a buffer from where it was paused is a pretty slow operation
//       (DSound limitation)
//-----------------------------------------------------------------------------
HRESULT CSoundEffectEngine::ResumeAll()
{
    for( DWORD i = 0; i < MAX_EFFECT_INSTANCES; i++ )
    {
        if( m_aInstances[i].m_bInUse &&
            !m_aInstances[i].m_bSource )
        {
            for( DWORD j = 0; j < MAX_ASSETS; j++ )
            {
                if( m_aInstances[ i ].m_apBuffers[ j ].bPaused )
                {
                    assert( m_aInstances[ i ].m_apBuffers[ j ].pDSBuffer );
                    m_aInstances[ i ].m_apBuffers[ j ].pDSBuffer->Play( 0, 0, m_aInstances[ i ].m_apBuffers[ j ].bLooping ? DSBPLAY_LOOPING : 0 );
                    m_aInstances[ i ].m_apBuffers[ j ].bPaused = FALSE;
                }
            }
        }
    }

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: SetVolume
// Desc: Sets the global sound effect volume
//-----------------------------------------------------------------------------
HRESULT CSoundEffectEngine::SetVolume( LONG lPercent )
{
    if( lPercent ==  0 )
    {
        // 0% means REALLY off
        m_lDSVolume = DSBVOLUME_MIN;
    }
    else 
    {
        // Otherwise, convert the percentage to millibels
        m_lDSVolume = LONG( 2000 * log( lPercent / 100.0f ) );
    }

    for( DWORD i = 0; i < MAX_EFFECT_INSTANCES; i++ )
    {
        if( m_aInstances[i].m_bInUse &&
            !m_aInstances[i].m_bSource )
        {
            // Update instance volume
            m_aInstances[i].SetVolume( m_aInstances[i].m_lVolume );
        }
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: SetVolume
// Desc: Sets the volume of the sound effect instance.  This volume is a
//       combination of:
//       * Global sound effect volume
//       * Asset-specific volume
//       * lVolume passed in
//-----------------------------------------------------------------------------
HRESULT CSoundEffectInstance::SetVolume( LONG lVolume )
{ 
    m_lVolume = lVolume;

    for( int i = 0; i < MAX_ASSETS; i++ ) 
    {
        CSoundEffectDefinition* pEffect     = &g_SoundEngine.m_aEffectDefinitions[ m_dwEffect ];
        CSoundEffectVariation*  pVariation  = &pEffect->m_aVariations[ m_dwVariation ];
        if( m_apBuffers[i].pDSBuffer ) 
        {
            LONG lFinalVolume = g_SoundEngine.m_lDSVolume + 
                                m_lVolume + 
                                g_SoundEngine.m_pAssets[ pVariation->m_aAssets[ i ] ].m_lVolume;

            if( lFinalVolume < DSBVOLUME_MIN )
                lFinalVolume = DSBVOLUME_MIN;
            else if( lFinalVolume > DSBVOLUME_MAX )
                lFinalVolume = DSBVOLUME_MAX;

            m_apBuffers[i].pDSBuffer->SetVolume( lFinalVolume );
        }
    }

    return S_OK;
}



DWORD g_dwLevelSoundsOffset;