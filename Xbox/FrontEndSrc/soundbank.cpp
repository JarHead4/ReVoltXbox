//-----------------------------------------------------------------------------
// File: SoundBank.cpp
//
// Desc: Implementation file containing class, structure, and constant definitions
//       for the CSoundBank class
//
// Hist: 12.06.01 - Created
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "SoundBank.h"
#include "debug.h"

// TODO (JHarding): Need to think about how to handle swapping different DSP 
// images in/out
#include <dsstdfx.h>



// Global sound bank object
CSoundBank g_SoundBank;



// Dummy wave format: 16-bit 44kHz mono PCM
WAVEFORMATEX g_wfxDummy     = { WAVE_FORMAT_PCM,
                                1,
                                44100,
                                88200,
                                2,
                                16,
                                0 };

// Buffer description for standard 2D buffer
DSBUFFERDESC g_dsbd2D       = { 0, 
                                0,
                                0,
                                &g_wfxDummy,
                                0,
                                0 };

// Buffer description for standard 3D buffer
DSBUFFERDESC g_dsbd3D       = { 0, 
                                DSBCAPS_CTRL3D,
                                0,
                                &g_wfxDummy,
                                0,
                                0 };

// Buffer description for a mix-in 3D buffer
DSBUFFERDESC g_dsbd3DMixIN  = { 0, 
                                DSBCAPS_CTRL3D | DSBCAPS_MIXIN,
                                0,
                                &g_wfxDummy,
                                0,
                                0 };




//-----------------------------------------------------------------------------
//
//  SoundBankExpandFormat
//
//  Description:
//      Expands a compressed wave format to a standard format structure.
//
//  Arguments:
//      LPCWAVEBANKMINIWAVEFORMAT [in]: compressed format.
//      LPWAVEBANKUNIWAVEFORMAT [out]: standard format.
//
//  Returns:  
//      BOOL: TRUE on success.
//
//-----------------------------------------------------------------------------
BOOL 
SoundBankExpandFormat
(
    LPCWAVEBANKMINIWAVEFORMAT  pwfxCompressed, 
    LPWAVEBANKUNIWAVEFORMAT    pwfxExpanded
)
{
    if(WAVEBANKMINIFORMAT_TAG_ADPCM == pwfxCompressed->wFormatTag)
    {
        XAudioCreateAdpcmFormat((WORD)pwfxCompressed->nChannels, pwfxCompressed->nSamplesPerSec, &pwfxExpanded->AdpcmWaveFormat);
    }
    else
    {
        XAudioCreatePcmFormat((WORD)pwfxCompressed->nChannels, pwfxCompressed->nSamplesPerSec, (WAVEBANKMINIFORMAT_BITDEPTH_16 == pwfxCompressed->wBitsPerSample) ? 16 : 8, &pwfxExpanded->WaveFormatEx);
    }

    return TRUE;
}



//-----------------------------------------------------------------------------
// Name: CSoundBank (constructor)
// Desc: Initializes member variables
//-----------------------------------------------------------------------------
CSoundBank::CSoundBank()
{
    m_pDSound               = NULL;
    m_pSoundBankEntries     = NULL;
    m_dwNumSounds           = 0;
    m_pbSampleData          = NULL;
    m_dwSampleLength        = 0;

    m_pAllNodes             = NULL;
    m_pFree2DBufferList     = NULL;
    m_pFree3DBufferList     = NULL;
    m_pBusyBufferList       = NULL;
    m_pReservedBufferList   = NULL;
}




//-----------------------------------------------------------------------------
// Name: ~CSoundBank (destructor)
// Desc: Frees up all resources owned by the sound engine
//-----------------------------------------------------------------------------
CSoundBank::~CSoundBank()
{
    StopAll();
    FreeBufferPool();
    FreeSoundBank();
}




//-----------------------------------------------------------------------------
// Name: Initialize
// Desc: Initializes the sound engine:
//       * Downloads default DSP image
//-----------------------------------------------------------------------------
HRESULT CSoundBank::Initialize()
{
    DirectSoundCreate( NULL, &m_pDSound, NULL );
    DirectSoundUseFullHRTF();
    m_pDSound->SetDistanceFactor( 0.0033f, DS3D_IMMEDIATE );
    m_pDSound->SetRolloffFactor( 0.0033f, DS3D_IMMEDIATE );

    if( !XLoadSection( "DSPImage" ) )
        return E_FAIL;

    LPDSEFFECTIMAGEDESC pDesc;
    DSEFFECTIMAGELOC dsImageLoc = { I3DL2_CHAIN_I3DL2_REVERB, I3DL2_CHAIN_XTALK };
    if( FAILED( XAudioDownloadEffectsImage( "DSPImage", 
                                            &dsImageLoc, 
                                            XAUDIO_DOWNLOADFX_XBESECTION,
                                            &pDesc ) ) )
        return E_FAIL;

    XFreeSection( "DSPImage" );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: LoadSoundBank
// Desc: Loads a sound bank from the specified file
//       Right now, this can only be called when there is no sound bank loaded
//-----------------------------------------------------------------------------
HRESULT CSoundBank::LoadSoundBank( LPCSTR strFilename )
{
    Assert( m_dwSampleLength == 0 &&
            m_pbSampleData == NULL );

    // First, open the soundbank file
    HANDLE hFile = CreateFile( strFilename,
                               GENERIC_READ,
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL );
    Assert( hFile != INVALID_HANDLE_VALUE );

    // Read the header info
    WAVEBANKHEADER header;
    DWORD           dwRead;
    ReadFile( hFile, &header, sizeof( WAVEBANKHEADER ), &dwRead, NULL );
    Assert( dwRead == sizeof( WAVEBANKHEADER ) );
    Assert( header.dwSignature == WAVEBANKHEADER_SIGNATURE &&
            header.dwVersion   == WAVEBANKHEADER_VERSION );


    // Allocate space for the sound bank entries
    m_dwNumSounds = header.dwEntryCount;
    m_pSoundBankEntries = new WAVEBANKENTRY[ m_dwNumSounds ];
    Assert( m_pSoundBankEntries != NULL );


    // Read the sound bank entries
    ReadFile( hFile, 
              m_pSoundBankEntries, 
              m_dwNumSounds * sizeof( WAVEBANKENTRY ),
              &dwRead,
              NULL );
    Assert( dwRead == m_dwNumSounds * sizeof( WAVEBANKENTRY ) );


    // Calculate data length
    DWORD dwFileSize = GetFileSize( hFile, NULL );
    m_dwSampleLength = dwFileSize - sizeof( WAVEBANKHEADER ) - m_dwNumSounds * sizeof( WAVEBANKENTRY );

    // Allocate Memory for the sound data
    m_pbSampleData = new BYTE[ m_dwSampleLength ];
    Assert( m_pbSampleData != NULL );

    // Read sample data from the file
    ReadFile( hFile, m_pbSampleData, m_dwSampleLength, &dwRead, NULL );
    Assert( dwRead == m_dwSampleLength );

    CloseHandle( hFile );

    // If we have an active buffer pool, we need to remap it
    for( DWORD i = 0; i < m_dwNumNodes; i++ )
    {
        m_pAllNodes[i].pBuffer->SetBufferData( m_pbSampleData, m_dwSampleLength );
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FreeSoundBank
// Desc: Frees the currently loaded sound bank
//-----------------------------------------------------------------------------
HRESULT CSoundBank::FreeSoundBank()
{
    // Can't do this while buffers are playing
    Assert( m_pBusyBufferList == NULL );

    // Unmap the buffers
    for( DWORD i = 0; i < m_dwNumNodes; i++ )
    {
        m_pAllNodes[i].pBuffer->SetBufferData( NULL, 0 );
    }

    // Free up sound bank entries
    delete[] m_pSoundBankEntries;
    m_pSoundBankEntries = NULL;
    m_dwNumSounds       = 0;

    // Free up sample data
    delete[] m_pbSampleData;
    m_pbSampleData      = NULL;
    m_dwSampleLength    = 0;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InitializeBufferPool
// Desc: Sets up the buffer pool with the specified number of 2d and 3d buffers
//-----------------------------------------------------------------------------
HRESULT CSoundBank::InitializeBufferPool( DWORD dw2DBuffers,
                                          DWORD dw3DBuffers )
{
    m_dwNumNodes = dw2DBuffers + dw3DBuffers;
    m_pAllNodes = new BufferListNode[ m_dwNumNodes ];
    Assert( m_pAllNodes != NULL );

    // 2D Buffer pool
    for( DWORD i = 0; i < dw2DBuffers; i++ )
    {
        DirectSoundCreateBuffer( &g_dsbd2D, &m_pAllNodes[i].pBuffer );
        Assert( m_pAllNodes[i].pBuffer != NULL );

        m_pAllNodes[i].pBuffer->SetBufferData( m_pbSampleData, m_dwSampleLength );
        m_pAllNodes[i].pNext = m_pFree2DBufferList;
        m_pAllNodes[i].dwFlags = BUFFERFLAGS_2D | BUFFERFLAGS_NOMIX;
        m_pFree2DBufferList = &m_pAllNodes[i];
    }

    for( DWORD i = dw2DBuffers; i < m_dwNumNodes; i++ )
    {
        DirectSoundCreateBuffer( &g_dsbd3D, &m_pAllNodes[i].pBuffer );
        Assert( m_pAllNodes[i].pBuffer != NULL );

        m_pAllNodes[i].pBuffer->SetBufferData( m_pbSampleData, m_dwSampleLength );
        m_pAllNodes[i].pNext = m_pFree3DBufferList;
        m_pAllNodes[i].dwFlags = BUFFERFLAGS_3D | BUFFERFLAGS_NOMIX;
        m_pFree3DBufferList = &m_pAllNodes[i];
    }

#if _DEBUG
    m_dwNum2DBuffersDbg = dw2DBuffers;
    m_dwNum3DBuffersDbg = dw3DBuffers;
#endif // _DEBUG
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FreeBufferPool
// Desc: Releases all resources associated with the buffer pool
//-----------------------------------------------------------------------------
HRESULT CSoundBank::FreeBufferPool()
{
    // All reserved buffers shold have been returned
    Assert( m_pReservedBufferList == NULL );

    while( m_pBusyBufferList != NULL )
    {
        // If you get into a loop here, it's because you didn't
        // shut down a looping sound effect
        ScrubBusyList();
    }
    // Can't do this while sounds are playing
    Assert( m_pBusyBufferList == NULL );

    for( DWORD i = 0; i < m_dwNumNodes; i++ )
    {
        m_pAllNodes[i].pBuffer->Release();
    }

    delete[] m_pAllNodes;
    m_pAllNodes     = NULL;
    m_dwNumNodes    = 0;

    m_pFree2DBufferList = NULL;
    m_pFree3DBufferList = NULL;
    m_pBusyBufferList   = NULL;

#if _DEBUG
    m_dwNum2DBuffersDbg = 0;
    m_dwNum3DBuffersDbg = 0;
#endif // _DEBUG

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: PlaySound
// Desc: Plays the given sound.  If a specific buffer is specified, plays the
//       sound on that buffer
//-----------------------------------------------------------------------------
HRESULT CSoundBank::PlaySound( DWORD               dwIndex,
                               BOOL                bLooping,
                               BOOL                b3D,
                               LPDIRECTSOUNDBUFFER pBuffer )
{
    Assert( dwIndex < m_dwNumSounds );

    LPDIRECTSOUNDBUFFER pBufferToPlay;

    if( pBuffer )
    {
        pBufferToPlay = pBuffer;
    }
    else
    {
        BufferListNode* pNode;

        pNode = GetNodeFromList( b3D ? &m_pFree3DBufferList : &m_pFree2DBufferList );
        if( !pNode )
            return E_FAIL;

        pBufferToPlay = pNode->pBuffer;
        pBufferToPlay->SetVolume( DSBVOLUME_MAX );

        AddNodeToList( &m_pBusyBufferList, pNode );
    }


    WAVEBANKENTRY* pSound = &m_pSoundBankEntries[ dwIndex ];

    WAVEBANKUNIWAVEFORMAT wfx;
    SoundBankExpandFormat( &pSound->Format, &wfx );
    pBufferToPlay->SetFormat( (WAVEFORMATEX *)&wfx );

    pBufferToPlay->SetPlayRegion( pSound->PlayRegion.dwStart, 
                                  pSound->PlayRegion.dwLength );
    if( bLooping )
    {
        pBufferToPlay->SetLoopRegion( pSound->LoopRegion.dwStart, 
                                      pSound->LoopRegion.dwLength );
    }

    pBufferToPlay->PlayEx( 0, DSBPLAY_FROMSTART | ( bLooping ? DSBPLAY_LOOPING : 0 ) );

    return S_OK;
}





//-----------------------------------------------------------------------------
// Name: GetBuffer
// Desc: Gets a free buffer of the appropriate type, so that parameters may
//       be adjusted before playing a sound
//-----------------------------------------------------------------------------
HRESULT CSoundBank::GetBuffer( BOOL                   b3D,
                               LPDIRECTSOUNDBUFFER*   ppBuffer )
{
    BufferListNode* pNode = GetNodeFromList( b3D ? &m_pFree3DBufferList : &m_pFree2DBufferList );
    if( !pNode )
        return E_FAIL;

    AddNodeToList( &m_pReservedBufferList, pNode );

    *ppBuffer = pNode->pBuffer;
    (*ppBuffer)->SetVolume( DSBVOLUME_MAX );
    (*ppBuffer)->SetOutputBuffer( NULL );
    (*ppBuffer)->SetMixBins( NULL );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ReturnBuffer
// Desc: Returns a reserved buffer back to the pool
// TODO (JHarding): consider reworking this mechanism
//-----------------------------------------------------------------------------
HRESULT CSoundBank::ReturnBuffer( LPDIRECTSOUNDBUFFER pBuffer )
{
    BufferListNode*  pNode;
    BufferListNode** ppRemove = &m_pReservedBufferList;

    while( (*ppRemove)->pBuffer != pBuffer )
    {
        ppRemove = &((*ppRemove)->pNext);
        Assert( *ppRemove != NULL );
    }
    pNode = *ppRemove;
    *ppRemove = (*ppRemove)->pNext;

    AddNodeToList( &m_pBusyBufferList, pNode );

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: PauseAll
// Desc: Pauses all currently playing buffers
//-----------------------------------------------------------------------------
HRESULT CSoundBank::PauseAll()
{
    BufferListNode* pNode = m_pBusyBufferList;

    while( pNode )
    {
        pNode->pBuffer->Stop();
        pNode = pNode->pNext;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ResumeAll
// Desc: Resumes all paused buffers
// TODO (JHarding): Need some extra state in order to resume buffers... this
//       kinda sucks
//-----------------------------------------------------------------------------
HRESULT CSoundBank::ResumeAll()
{
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: StopAll
// Desc: Stops all playing buffers
// TODO (JHarding): Think about sync vs. async - when do we need to use this?
// TODO (JHarding): Also thinking about different stop modes (envelopes, etc)
//-----------------------------------------------------------------------------
HRESULT CSoundBank::StopAll()
{
    BufferListNode* pNode = m_pBusyBufferList;

    while( pNode )
    {
        pNode->pBuffer->StopEx( 0, DSBSTOPEX_IMMEDIATE );
        pNode = pNode->pNext;
    }    

    return S_OK;
}





//-----------------------------------------------------------------------------
// Name: GetNodeFromList
// Desc: Retrieves a buffer from the given list
//-----------------------------------------------------------------------------
BufferListNode* CSoundBank::GetNodeFromList( BufferListNode **ppList )
{
    BufferListNode* pNode = *ppList;

    // If there is still a node in the list, remove the head
    if( pNode )
    {
        *ppList = pNode->pNext;
        pNode->pNext = NULL;
    }

    return pNode;
}




//-----------------------------------------------------------------------------
// Name: AddNodeToList
// Desc: Adds the given node to the given list
//-----------------------------------------------------------------------------
VOID CSoundBank::AddNodeToList( BufferListNode** ppList,
                                BufferListNode*  pNode )
{
    pNode->pNext = *ppList;
    *ppList = pNode;

    return;
}




//-----------------------------------------------------------------------------
// Name: ScrubBusyList
// Desc: Scrubs the busy list, looking for completed buffers to return
//-----------------------------------------------------------------------------
HRESULT CSoundBank::ScrubBusyList()
{
    BufferListNode** ppNode = &m_pBusyBufferList;

    while( *ppNode != NULL )
    {
        // Check the status of each buffer in our busy list
        DWORD dwStatus;
        (*ppNode)->pBuffer->GetStatus( &dwStatus );

        if( !(dwStatus & DSBSTATUS_PLAYING) )
        {
            // If it's no longer playing, remove it from the list
            BufferListNode* pNodeRemove = *ppNode;
            *ppNode = (*ppNode)->pNext;
            if( pNodeRemove->dwFlags & BUFFERFLAGS_2D )
            {
                AddNodeToList( &m_pFree2DBufferList, pNodeRemove );
            }
            else
            {
                AddNodeToList( &m_pFree3DBufferList, pNodeRemove );
            }
        }
        else
        {
            // Else, move on to the next one
            ppNode = &((*ppNode)->pNext);
        }
    }

#if _DEBUG
    if( ValidateStateDbg() != S_OK )
        _asm int 3;
#endif // _DEBUG

    return S_OK;
}




#if _DEBUG
//-----------------------------------------------------------------------------
// Name: ValidateStateDbg
// Desc: Validates internal state of the sound engine
//-----------------------------------------------------------------------------
HRESULT CSoundBank::ValidateStateDbg()
{
    BufferListNode* pNode;
    DWORD           dwFree2DBuffers = 0;
    DWORD           dwFree3DBuffers = 0;
    DWORD           dwBusy2DBuffers = 0;
    DWORD           dwBusy3DBuffers = 0;

    // Check the free 2D buffer list
    pNode = m_pFree2DBufferList;
    while( pNode )
    {
        Assert( pNode->dwFlags == ( BUFFERFLAGS_2D | BUFFERFLAGS_NOMIX ) );
        Assert( pNode->pBuffer != NULL );

        DWORD dwStatus;
        pNode->pBuffer->GetStatus( &dwStatus );
        Assert( !( dwStatus & DSBSTATUS_PLAYING ) );

        pNode = pNode->pNext;

        ++dwFree2DBuffers;
    }

    // Check the free 3D buffer list
    pNode = m_pFree3DBufferList;
    while( pNode )
    {
        Assert( pNode->dwFlags == ( BUFFERFLAGS_3D | BUFFERFLAGS_NOMIX ) );
        Assert( pNode->pBuffer != NULL );

        DWORD dwStatus;
        pNode->pBuffer->GetStatus( &dwStatus );
        Assert( !( dwStatus & DSBSTATUS_PLAYING ) );

        pNode = pNode->pNext;

        ++dwFree3DBuffers;
    }

    // Check the busy buffer list
    pNode = m_pBusyBufferList;
    while( pNode )
    {
        Assert( pNode->pBuffer != NULL );

        if( pNode->dwFlags & BUFFERFLAGS_2D )
        {
            Assert( pNode->dwFlags == ( BUFFERFLAGS_2D | BUFFERFLAGS_NOMIX ) );
            ++dwBusy2DBuffers;
        }
        else if( pNode->dwFlags & BUFFERFLAGS_3D )
        {
            Assert( pNode->dwFlags == ( BUFFERFLAGS_3D | BUFFERFLAGS_NOMIX ) );
            ++dwBusy3DBuffers;
        }
        else
        {
            // Has to be one or the other!
            Assert( FALSE );
            return E_FAIL;
        }

        pNode = pNode->pNext;
    }

    // Now check the reserved buffer list
    pNode = m_pReservedBufferList;
    while( pNode )
    {
        if( pNode->dwFlags & BUFFERFLAGS_2D )
        {
            Assert( pNode->dwFlags == ( BUFFERFLAGS_2D | BUFFERFLAGS_NOMIX ) );
            ++dwBusy2DBuffers;
        }
        else if( pNode->dwFlags & BUFFERFLAGS_3D )
        {
            Assert( pNode->dwFlags == ( BUFFERFLAGS_3D | BUFFERFLAGS_NOMIX ) );
            ++dwBusy3DBuffers;
        }
        else
        {
            // Has to be one or the other!
            Assert( FALSE );
            return E_FAIL;
        }

        pNode = pNode->pNext;
    }

    // Make sure all our numbers line up
    Assert( dwFree2DBuffers + dwBusy2DBuffers == m_dwNum2DBuffersDbg );
    Assert( dwFree3DBuffers + dwBusy3DBuffers == m_dwNum3DBuffersDbg );
    Assert( dwFree2DBuffers + dwBusy2DBuffers + dwFree3DBuffers + dwBusy3DBuffers == m_dwNumNodes );

    if( dwBusy2DBuffers > m_dwNum2DBuffersDbg * 95 / 100 )
    {
        OutputDebugString( "SoundBank: Warning! Over 95% of 2D Buffers are in use!\n" );
    }
    if( dwBusy3DBuffers > m_dwNum3DBuffersDbg * 95 / 100 )
    {
        OutputDebugString( "SoundBank: Warning! Over 95% of 3D Buffers are in use!\n" );
    }

    return S_OK;
}
#endif // _DEBUG


