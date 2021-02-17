//-----------------------------------------------------------------------------
// File: SoundEffectEngine.h
//
// Desc: Implementation of Sound Effect Engine.
//
// Hist: 1.06.02 - Created
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#define assert(w) {}

#include "SoundEffectEngine.h"
#include "camera.h"
#include "instance.h"

CSoundEffectEngine g_SoundEngine;


//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
CSoundEffectEngine::CSoundEffectEngine()
{
    ZeroMemory( m_aInstances, sizeof( m_aInstances ) );
    m_dwLastReturnedInstance = 0;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
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
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CSoundEffectEngine::Initialize()
{
    g_SoundBank.Initialize();
    m_dwNumEffects = 0;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: LoadLevel
// Desc: Sets up the sound engine for the specified game leve
//-----------------------------------------------------------------------------
HRESULT CSoundEffectEngine::LoadLevel( CHAR* strLevel )
{
    char strSoundBank[MAX_LEVEL_DIR_NAME+80];
    sprintf( strSoundBank, "d:\\levels\\%s\\sounds.xwb", strLevel );

    g_SoundBank.LoadSoundBank( strSoundBank );
    g_SoundBank.InitializeBufferPool( 128, 54 );
    //$TODO: determine correct buffer pool sizes

    char strSoundEffects[ MAX_LEVEL_DIR_NAME + 80 ];
    sprintf( strSoundEffects, "d:\\levels\\%s\\sounds.sfx", strLevel );

    HANDLE hFile = CreateFile( strSoundEffects, 
                               GENERIC_READ,
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL );

    DWORD dwRead;
    DWORD dwVersion;
    ReadFile( hFile, &dwVersion, sizeof( DWORD ), &dwRead, NULL );
    if( dwVersion != VERSION_NUMBER )
    {
        OutputDebugString( "Sound effect file is wrong version.\n" );
        return E_FAIL;
    }

    ReadFile( hFile, &m_dwNumEffects, sizeof( DWORD ), &dwRead, NULL );
    ReadFile( hFile, m_aEffectDefinitions, m_dwNumEffects * sizeof( CSoundEffectDefinition ), &dwRead, NULL );

    DWORD dwNumAssets;
    ReadFile( hFile, &dwNumAssets, sizeof( DWORD ), &dwRead, NULL );
    m_pAssets = new CSoundEffectAsset[ dwNumAssets ];
    ReadFile( hFile, m_pAssets, sizeof( CSoundEffectAsset ) * dwNumAssets, &dwRead, NULL );

    CloseHandle( hFile );
    
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

        // That said, a fire-and-forget instance could still
        // be playing.  In that case, shut down all buffers,
        // and wait for it to really shut off
        if( m_aInstances[ i ].m_bInUse )
        {
            for( DWORD j = 0; j < MAX_ASSETS; j++ )
            {
                if( m_aInstances[ i ].m_apBuffers[ j ] )
                {
                    DWORD dwStatus;

                    m_aInstances[ i ].m_apBuffers[ j ]->StopEx( 0, DSBSTOPEX_IMMEDIATE );
                    do
                    {
                        m_aInstances[ i ].m_apBuffers[ j ]->GetStatus( &dwStatus );
                    } while( dwStatus & DSBSTATUS_PLAYING );
                }
            }
        }
    }

    UpdateAll();
    g_SoundBank.FreeBufferPool();
    g_SoundBank.FreeSoundBank();


    m_dwNumEffects = 0;
    delete[] m_pAssets;
    m_pAssets = NULL;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CSoundEffectEngine::Play3DSound( DWORD dwIndex, 
                                         BOOL bLooping, 
#ifdef OLD_AUDIO
                                         SAMPLE_3D* pSample3D,
#else // !OLD_AUDIO
                                         OBJECT* obj, 
#endif // !OLD_AUDIO
                                         CSoundEffectInstance** ppInstance )
{
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
        if( FAILED( g_SoundBank.GetBuffer( TRUE, &(pNewInstance->m_apBuffers[i]) ) ) )
        {
            ReturnInstance( pNewInstance );
            if( ppInstance )
                *ppInstance = NULL;
            return E_FAIL;
        }
#ifdef OLD_AUDIO
        pNewInstance->m_apBuffers[i]->SetPosition( pSample3D->Pos.v[0], 
                                                   pSample3D->Pos.v[1], 
                                                   pSample3D->Pos.v[2], 
                                                   DS3D_IMMEDIATE );
        pNewInstance->m_apBuffers[i]->SetFrequency( pSample3D->Freq );
        //$BUGBUG: Not yet calling SetVolume here.  (Any other pSample3D vars we care about?)
        pNewInstance->m_pSample3D = pSample3D;
#else // !OLD_AUDIO
        pNewInstance->m_apBuffers[i]->SetPosition( obj->body.Centre.Pos.v[0], 
                                                   obj->body.Centre.Pos.v[1], 
                                                   obj->body.Centre.Pos.v[2], 
                                                   DS3D_IMMEDIATE );
        pNewInstance->m_pObject = obj;
#endif // !OLD_AUDIO
        g_SoundBank.PlaySound( m_pAssets[ pVariation->m_aAssets[i] ].m_dwSoundBankEntry, 
                               bLooping, 
                               TRUE,
                               pNewInstance->m_apBuffers[i] );
    }

    if( ppInstance )
    {
        // Make sure we're not overwriting an old instance.
#ifdef OLD_AUDIO
        // NOTE (JHarding): OLD_AUDIO codepath isn't taking care of this
#else
        assert( *ppInstance == NULL );
#endif
        *ppInstance = pNewInstance;
        pNewInstance->m_bReserved = TRUE;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CSoundEffectEngine::Play3DSound( DWORD dwIndex, 
                                         BOOL bLooping, 
                                         FLOAT fX, 
                                         FLOAT fY, 
                                         FLOAT fZ,
                                         CSoundEffectInstance** ppInstance )
{
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
        if( FAILED( g_SoundBank.GetBuffer( TRUE, &(pNewInstance->m_apBuffers[i]) ) ) )
        {
            ReturnInstance( pNewInstance );
            if( ppInstance )
                *ppInstance = NULL;

            return E_FAIL;
        }
        pNewInstance->m_apBuffers[i]->SetPosition( fX, fY, fZ, DS3D_IMMEDIATE );
        g_SoundBank.PlaySound( m_pAssets[ pVariation->m_aAssets[i] ].m_dwSoundBankEntry, 
                               bLooping, 
                               TRUE,
                               pNewInstance->m_apBuffers[i] );
    }

    if( ppInstance )
    {
        // Make sure we're not overwriting an old instance.
#ifdef OLD_AUDIO
        // NOTE (JHarding): OLD_AUDIO codepath isn't taking care of this
#else
        assert( *ppInstance == NULL );
#endif
        *ppInstance = pNewInstance;
        pNewInstance->m_bReserved = TRUE;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CSoundEffectEngine::Play2DSound( DWORD dwIndex, 
                                         BOOL bLooping,
                                         CSoundEffectInstance** ppInstance )
{
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
        if( FAILED( g_SoundBank.GetBuffer( FALSE, &(pNewInstance->m_apBuffers[i]) ) ) )
        {
            ReturnInstance( pNewInstance );
            if( ppInstance )
                *ppInstance = NULL;

            return E_FAIL;
        }
        g_SoundBank.PlaySound( m_pAssets[ pVariation->m_aAssets[i] ].m_dwSoundBankEntry, 
                               bLooping, 
                               FALSE,
                               pNewInstance->m_apBuffers[i] );
    }

    if( ppInstance )
    {
        // Make sure we're not overwriting an old instance.
#ifdef OLD_AUDIO
        // NOTE (JHarding): OLD_AUDIO codepath isn't taking care of this
#else
        assert( *ppInstance == NULL );
#endif
        *ppInstance = pNewInstance;
        pNewInstance->m_bReserved = TRUE;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CSoundEffectEngine::PlaySubmixedSound( DWORD dwIndex, 
                                               BOOL bLooping,
                                               CSoundEffectInstance* pMixin,
                                               CSoundEffectInstance** ppInstance )
{
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
        if( FAILED( g_SoundBank.GetBuffer( FALSE, &(pNewInstance->m_apBuffers[i]) ) ) )
        {
            ReturnInstance( pNewInstance );
            if( ppInstance )
                *ppInstance = NULL;

            return E_FAIL;
        }
        pNewInstance->m_apBuffers[i]->SetOutputBuffer( pMixin->m_apBuffers[ 0 ] );
        g_SoundBank.PlaySound( m_pAssets[ pVariation->m_aAssets[i] ].m_dwSoundBankEntry, 
                               bLooping, 
                               FALSE,
                               pNewInstance->m_apBuffers[i] );
    }

    if( ppInstance )
    {
        // Make sure we're not overwriting an old instance.
#ifdef OLD_AUDIO
        // NOTE (JHarding): OLD_AUDIO codepath isn't taking care of this
#else
        assert( *ppInstance == NULL );
#endif
        *ppInstance = pNewInstance;
        pNewInstance->m_bReserved = TRUE;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CSoundEffectEngine::UpdateAll()
{
    g_SoundBank.ScrubBusyList();
#if _DEBUG
    DWORD dwInUse = 0;
#endif // _DEBUG

    for( DWORD i = 0; i < MAX_EFFECT_INSTANCES; i++ )
    {
        // Check to see if the instance is still playing
        if( m_aInstances[i].m_bInUse &&
            !m_aInstances[i].m_bSource )
        {
            // Assume we're not
            m_aInstances[i].m_bInUse = FALSE;

            DWORD dwStatus;
            for( DWORD j = 0; j < MAX_ASSETS; j++ )
            {
                if( m_aInstances[i].m_apBuffers[j] != NULL )
                {
                    // Check the status of the buffer
                    m_aInstances[i].m_apBuffers[j]->GetStatus( &dwStatus );

                    if( dwStatus & DSBSTATUS_PLAYING )
                    {
                        // If anyone's playing, we're still active
                        m_aInstances[i].m_bInUse = TRUE;
                    }
                    else if( !m_aInstances[i].m_bReserved )
                    {
                        // We can return any unneeded buffers
                        g_SoundBank.ReturnBuffer( m_aInstances[i].m_apBuffers[j] );
                        m_aInstances[i].m_apBuffers[j] = NULL;
                    }
                }
            }

            if( !m_aInstances[i].m_bInUse )
            {
#ifdef OLD_AUDIO
                m_aInstances[i].m_pSample3D = NULL;
#else // !OLD_AUDIO
                m_aInstances[i].m_pObject = NULL;
#endif // !OLD_AUDIO
            }
        }

        // Update all Instances that are tracking objects
#ifdef OLD_AUDIO
        if( m_aInstances[i].m_bInUse && m_aInstances[i].m_pSample3D )
#else // !OLD_AUDIO
        if( m_aInstances[i].m_bInUse && m_aInstances[i].m_pObject )
#endif // !OLD_AUDIO
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
#ifdef OLD_AUDIO
                if( _finite( m_aInstances[i].m_pSample3D->Pos.v[0] ) &&
                    _finite( m_aInstances[i].m_pSample3D->Pos.v[1] ) &&
                    _finite( m_aInstances[i].m_pSample3D->Pos.v[2] ) )
                {
                    m_aInstances[i].m_apBuffers[j]->SetPosition( m_aInstances[i].m_pSample3D->Pos.v[0],
                                                                 m_aInstances[i].m_pSample3D->Pos.v[1],
                                                                 m_aInstances[i].m_pSample3D->Pos.v[2],
                                                                 DS3D_DEFERRED );
                    m_aInstances[i].m_apBuffers[j]->SetFrequency( m_aInstances[i].m_pSample3D->Freq );
                    //$BUGBUG: Not yet calling SetVolume here.  (Any other pSample3D vars we care about?)
                }
#else // !OLD_AUDIO
                if( _finite( m_aInstances[i].m_pObject->body.Centre.Pos.v[0] ) &&
                    _finite( m_aInstances[i].m_pObject->body.Centre.Pos.v[1] ) &&
                    _finite( m_aInstances[i].m_pObject->body.Centre.Pos.v[2] ) )
                {
                    m_aInstances[i].m_apBuffers[j]->SetPosition( m_aInstances[i].m_pObject->body.Centre.Pos.v[0],
                                                                 m_aInstances[i].m_pObject->body.Centre.Pos.v[1],
                                                                 m_aInstances[i].m_pObject->body.Centre.Pos.v[2],
                                                                 DS3D_DEFERRED );
                }
#endif // !OLD_AUDIO
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
// Name:
// Desc:
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

    return &m_aInstances[ dwTest ];
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CSoundEffectEngine::ReturnInstance( CSoundEffectInstance* pInstance )
{
    pInstance->m_bReserved = FALSE;

    if( pInstance->m_bSource )
    {
        Assert( pInstance->m_apBuffers[0] != NULL &&
                pInstance->m_apBuffers[1] == NULL &&
                pInstance->m_apBuffers[2] == NULL &&
                pInstance->m_apBuffers[3] == NULL );
                
        pInstance->m_apBuffers[0]->Release();
        pInstance->m_apBuffers[0] = NULL;
        pInstance->m_bSource = FALSE;
    }
    else
    {
        for( DWORD i = 0; i < MAX_ASSETS; i++ )
        {
            if( pInstance->m_apBuffers[ i ] )
            {
                pInstance->m_apBuffers[ i ]->StopEx( 0, DSBSTOPEX_IMMEDIATE );
                if( !pInstance->m_bInUse )
                {
                    g_SoundBank.ReturnBuffer( pInstance->m_apBuffers[i] );
                    pInstance->m_apBuffers[i] = NULL;
                }
            }
        }
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
#ifdef OLD_AUDIO
HRESULT CSoundEffectEngine::CreateSoundSource( SAMPLE_3D* pSample3D, CSoundEffectInstance** ppInstance )
#else // !OLD_AUDIO
HRESULT CSoundEffectEngine::CreateSoundSource( OBJECT* obj, CSoundEffectInstance** ppInstance )
#endif // !OLD_AUDIO
{
    *ppInstance = GetFreeInstance();

    (*ppInstance)->m_bSource    = TRUE;
    (*ppInstance)->m_bInUse     = TRUE;
    (*ppInstance)->m_bReserved  = TRUE;
#ifdef OLD_AUDIO
    (*ppInstance)->m_pSample3D  = pSample3D;
#else // !OLD_AUDIO
    (*ppInstance)->m_pObject    = obj;
#endif // !OLD_AUDIO

    DSBUFFERDESC dsbd = {0};
    dsbd.dwBufferBytes = 0;
    dsbd.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_MIXIN;

    HRESULT hr = g_SoundBank.GetDSound()->CreateSoundBuffer( &dsbd, &((*ppInstance)->m_apBuffers[0]), NULL );
    assert( SUCCEEDED( hr ) &&
            (*ppInstance)->m_apBuffers[0] != NULL );

    return S_OK;
}


// TODO (JHarding): Implement these
//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CSoundEffectEngine::StopAll()
{
    return E_NOTIMPL;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CSoundEffectEngine::PauseAll()
{
    return E_NOTIMPL;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CSoundEffectEngine::ResumeAll()
{
    return E_NOTIMPL;
}
