//-----------------------------------------------------------------------------
// File: VoiceManager.cpp
//
// Desc: Implementation of Voice Communicator support
//
// Hist: 1.17.02 - Created
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#include "VoiceManager.h"
#include <cassert>
#include <stdio.h>
#include "XBFont.h"

// Global instance of the voice manager
CVoiceManager g_VoiceManager;

// Ramp headphone up/down over some range of headroom
const DWORD MAX_HEADROOM =  2;
// Ramp headphone up/down over a period of time
const DWORD MAX_RAMP_TIME = 340;

//-----------------------------------------------------------------------------
// Name: CVoiceManager (ctor)
// Desc: Initializes member variables
//-----------------------------------------------------------------------------
CVoiceManager::CVoiceManager()
{
    m_dwMicrophoneState         = 0;
    m_dwHeadphoneState          = 0;
    m_dwConnectedCommunicators  = 0;
    m_dwLoopback                = 0;
    m_dwEnabled                 = 0x0000000F;
    m_bIsInChatSession          = FALSE;
    m_pChatters                 = NULL;
    m_pbTempEncodedPacket       = NULL;
    m_bFlushQueuesOnNextProcess = FALSE;

    m_pfnCommunicatorCallback   = NULL;
    m_pfnVoiceDataCallback      = NULL;

#if _DEBUG
    m_dwCurrentLogEntry         = 0;
    for( DWORD i = 0; i < s_dwNumLogEntries; i++ )
        m_strDebugLog[i][0] = L'\0';
#endif _DEBUG
}




//-----------------------------------------------------------------------------
// Name: ~CVoiceManager (dtor)
// Desc: Verifies that object was shut down properly
//-----------------------------------------------------------------------------
CVoiceManager::~CVoiceManager()
{
    assert( m_pChatters == NULL             &&
            m_pbTempEncodedPacket == NULL );

    assert( m_dwConnectedCommunicators == 0 );
}



//-----------------------------------------------------------------------------
// Name: Initialize
// Desc: Initializes the voice manager object
//-----------------------------------------------------------------------------
HRESULT CVoiceManager::Initialize( VOICE_MANAGER_CONFIG* pConfig )
{
    // Voice sampling rate should be 8kHz or 16kHz
    assert( pConfig->dwVoiceSamplingRate == 8000 ||
            pConfig->dwVoiceSamplingRate == 16000 );

    // Must have at least 1 remote player
    assert( pConfig->dwMaxRemotePlayers > 0 );

    // We need a minimum of 2 buffers to ping-pong between
    assert( pConfig->dwNumBuffers >= 2 );

    // Voice packets must be a multiple of 20ms
    assert( pConfig->dwVoicePacketTime % 20 == 0 );

    // Grab the config parameters
    m_dwSamplingRate            = pConfig->dwVoiceSamplingRate;
    m_dwPacketTime              = pConfig->dwVoicePacketTime;
    m_dwNumBuffers              = pConfig->dwNumBuffers;
    m_dwMaxChatters             = pConfig->dwMaxRemotePlayers;
    m_dwQueueResetThreshold     = pConfig->dwQueueResetThreshold;
    m_dwFirstSRCEffectIndex     = pConfig->dwFirstSRCEffectIndex;
    m_pDSPImageDesc             = pConfig->pEffectImageDesc;

    // Grab the callback params
    m_pCallbackContext          = pConfig->pCallbackContext;
    m_pfnCommunicatorCallback   = pConfig->pfnCommunicatorCallback;
    m_pfnVoiceDataCallback      = pConfig->pfnVoiceDataCallback;

    // Calculate other useful constants
    m_dwPacketSize      = ( m_dwPacketTime * m_dwSamplingRate / 1000 ) * 2;
    m_dwBufferSize      = m_dwPacketSize * m_dwNumBuffers;
    m_dwCompressedSize  = m_dwPacketTime * 8 / 20 + 2;  // The +2 comes from the encoder

    // Set up the wave format
    m_wfx.cbSize          = 0;
    m_wfx.nChannels       = 1;
    m_wfx.nSamplesPerSec  = m_dwSamplingRate;
    m_wfx.wBitsPerSample  = 16;
    m_wfx.nBlockAlign     = m_wfx.wBitsPerSample / 8 * m_wfx.nChannels;
    m_wfx.nAvgBytesPerSec = m_wfx.nSamplesPerSec * m_wfx.nBlockAlign;
    m_wfx.wFormatTag      = WAVE_FORMAT_PCM;

    // Allocate space for each of our remote chatters
    m_dwNumChatters = 0;
    m_pChatters = new REMOTE_CHATTER[ m_dwMaxChatters ];
    if( !m_pChatters )
        return E_OUTOFMEMORY;
    ZeroMemory( m_pChatters, m_dwMaxChatters * sizeof( REMOTE_CHATTER ) );

    // Set up structures we'll need for initializing chatters:

    // Voice Queue configuration
    XVOICE_QUEUE_XMO_CONFIG VoiceQueueCfg = {0};
    VoiceQueueCfg.dwMsOfDataPerPacket     = m_dwPacketTime;
    VoiceQueueCfg.dwCodecBufferSize       = m_dwCompressedSize;

    // Set Mixbin headrooms to 0
    pConfig->pDSound->SetMixBinHeadroom( DSMIXBIN_FXSEND_0, 0 );
    pConfig->pDSound->SetMixBinHeadroom( DSMIXBIN_FXSEND_1, 0 );
    pConfig->pDSound->SetMixBinHeadroom( DSMIXBIN_FXSEND_2, 0 );
    pConfig->pDSound->SetMixBinHeadroom( DSMIXBIN_FXSEND_3, 0 );

    // Stream mixbin configuration
    DSMIXBINVOLUMEPAIR dsmbvp[] = {
        { DSMIXBIN_FXSEND_0, 0 },
        { DSMIXBIN_FXSEND_1, 0 },
        { DSMIXBIN_FXSEND_2, 0 },
        { DSMIXBIN_FXSEND_3, 0 } };

    DWORD dwNumBins = 4;
    DSMIXBINS dsmb = { dwNumBins, dsmbvp };

    // Stream configuration
    DSSTREAMDESC dssd = {0};
    dssd.dwMaxAttachedPackets = m_dwNumBuffers;
    dssd.lpwfxFormat = &m_wfx;
    dssd.lpMixBins = &dsmb;
    
    // Initialize each chatter
    for( DWORD i = 0; i < m_dwMaxChatters; i++ )
    {
        if( FAILED( XVoiceQueueCreateMediaObject( &VoiceQueueCfg, &m_pChatters[i].pVoiceQueueXMO ) ) )
            return E_OUTOFMEMORY;

        if( FAILED( XVoiceCreateOneToOneDecoder( &m_pChatters[i].pDecoderXMO ) ) )
            return E_OUTOFMEMORY;

        if( FAILED( DirectSoundCreateStream( &dssd, &m_pChatters[i].pOutputStream ) ) )
            return E_OUTOFMEMORY;
        m_pChatters[i].pOutputStream->SetHeadroom( 0 );

        m_pChatters[i].pbStreamBuffer = new BYTE[ m_dwBufferSize ];
        if( !m_pChatters[i].pbStreamBuffer )
            return E_OUTOFMEMORY;
        
        m_pChatters[i].adwStatus = new DWORD[ m_dwNumBuffers ];
        if( !m_pChatters[i].adwStatus )
            return E_OUTOFMEMORY;
    }

    // Initialize communicators
    HRESULT hr = S_OK;
    for( DWORD i = 0; i < XGetPortCount(); i++ )
    {
        hr = m_aVoiceCommunicators[i].Initialize( this );
        if( FAILED( hr ) )
            return hr;
    }

    m_pbTempEncodedPacket = new BYTE[ m_dwCompressedSize ];
    if( !m_pbTempEncodedPacket )
        return E_OUTOFMEMORY;

    DWORD dwExpectedLatency = 0;
    dwExpectedLatency += m_dwPacketTime;    // One packet for microphone
    dwExpectedLatency += 1000 / 120;        // 1/2 frame to process
    dwExpectedLatency += 100;               // Assuming 100 ms transmit time
    dwExpectedLatency += 100;               // Guess at queue high water mark
    dwExpectedLatency += m_dwPacketTime * m_dwNumBuffers;   // Stream buffer
    dwExpectedLatency += m_dwPacketTime * m_dwNumBuffers;   // Headphone buffer
    VoiceLog( L"Initialized - Expected latency is %dms", dwExpectedLatency );

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: Shutdown
// Desc: Shuts down the voice manager
//-----------------------------------------------------------------------------
HRESULT CVoiceManager::Shutdown()
{
    LeaveChatSession();

    assert( m_pChatters || m_dwNumChatters == 0 );

    // Tear down each individual chatter
    for( DWORD i = 0; i < m_dwNumChatters; i++ )
    {
        if( m_pChatters[i].pVoiceQueueXMO )
            m_pChatters[i].pVoiceQueueXMO->Release();
        if( m_pChatters[i].pDecoderXMO )
            m_pChatters[i].pDecoderXMO->Release();
        if( m_pChatters[i].pOutputStream )
            m_pChatters[i].pOutputStream->Release();
        if( m_pChatters[i].pbStreamBuffer )
            delete[] m_pChatters[i].pbStreamBuffer;
        if( m_pChatters[i].adwStatus )
            delete[] m_pChatters[i].adwStatus;
    }

    // Delete the array
    if( m_pChatters )
    {
        delete[] m_pChatters;
        m_pChatters = NULL;
    }

    // Tear down each local communicator
    for( DWORD i = 0; i < XGetPortCount(); i++ )
        m_aVoiceCommunicators[i].Shutdown();

    if( m_pbTempEncodedPacket )
    {
        delete[] m_pbTempEncodedPacket;
        m_pbTempEncodedPacket = NULL;
    }

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: AddChatter
// Desc: Adds a chatter to the list of remote chatters
//-----------------------------------------------------------------------------
HRESULT CVoiceManager::AddChatter( XUID xuidPlayer )
{
    VoiceLog( L"Adding new chatter %I64x", xuidPlayer.qwUserID );

    assert( IsInChatSession() );
    assert( ChatterIndexFromXUID( xuidPlayer ) == m_dwNumChatters );
    assert( m_dwNumChatters < m_dwMaxChatters - 1 );

    // Grab the next open slot
    DWORD           dwNewChatterIndex = m_dwNumChatters++;
    REMOTE_CHATTER* pNewChatter = &m_pChatters[ dwNewChatterIndex ];

    // Initialize the new chatter
    pNewChatter->xuid = xuidPlayer;
    ResetChatter( dwNewChatterIndex );

    assert( ChatterIndexFromXUID( xuidPlayer ) < m_dwNumChatters );

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: RemoveChatter
// Desc: Removes a chatter from the list of remote chatters
//-----------------------------------------------------------------------------
HRESULT CVoiceManager::RemoveChatter( XUID xuidPlayer )
{
    VoiceLog( L"Removing chatter %I64x", xuidPlayer.qwUserID );

    assert( IsInChatSession() );
    assert( m_dwNumChatters > 0 );
    assert( ChatterIndexFromXUID( xuidPlayer ) < m_dwNumChatters );

    // If we've muted them, or they've muted us, pull them
    // out of the list.  Game code is responsible for
    // re-muting someone when they come back into the session
    for( WORD i = 0; i < XGetPortCount(); i++ )
    {
        if( IsPlayerMuted( xuidPlayer, i ) )
            UnMutePlayer( xuidPlayer, i );
        if( IsPlayerRemoteMuted( xuidPlayer, i ) )
            UnRemoteMutePlayer( xuidPlayer, i );
    }

    // Find the specified chatter
    DWORD chatterIndex = ChatterIndexFromXUID( xuidPlayer );
    m_pChatters[ chatterIndex ].pOutputStream->Discontinuity();
    
    // Swap with last chatter in list to remove
    REMOTE_CHATTER chatterTemp = m_pChatters[ chatterIndex ];
    m_pChatters[ chatterIndex ] = m_pChatters[ m_dwNumChatters - 1 ];
    m_pChatters[ m_dwNumChatters - 1 ] = chatterTemp;
    m_dwNumChatters--;

    assert( ChatterIndexFromXUID( xuidPlayer ) == m_dwNumChatters );

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: CheckDeviceChanges
// Desc: Processes device changes to look for insertions and removals of
//          communicators
//-----------------------------------------------------------------------------
HRESULT CVoiceManager::CheckDeviceChanges()
{
    DWORD dwMicrophoneInsertions;
    DWORD dwMicrophoneRemovals;
    DWORD dwHeadphoneInsertions;
    DWORD dwHeadphoneRemovals;

    // Must call XGetDevice changes to track possible removal and insertion
    // in one frame
    XGetDeviceChanges( XDEVICE_TYPE_VOICE_MICROPHONE,
                       &dwMicrophoneInsertions,
                       &dwMicrophoneRemovals );
    XGetDeviceChanges( XDEVICE_TYPE_VOICE_HEADPHONE,
                       &dwHeadphoneInsertions,
                       &dwHeadphoneRemovals );

    // Update state for removals
    m_dwMicrophoneState &= ~( dwMicrophoneRemovals );
    m_dwHeadphoneState  &= ~( dwHeadphoneRemovals );

    // Then update state for new insertions
    m_dwMicrophoneState |= ( dwMicrophoneInsertions );
    m_dwHeadphoneState  |= ( dwHeadphoneInsertions );

    for( WORD i = 0; i < XGetPortCount(); i++ )
    {
        // If either the microphone or the headphone was
        // removed since last call, remove the communicator
        if( m_dwConnectedCommunicators & ( 1 << i ) &&
            ( ( dwMicrophoneRemovals   & ( 1 << i ) ) ||
              ( dwHeadphoneRemovals    & ( 1 << i ) ) ) )
        {
            OnCommunicatorRemoved( i );
        }

        // If both microphone and headphone are present, and
        // we didn't have a communicator here last frame, and
        // the communicator is enabled, then  register the insertion
        if( ( m_dwMicrophoneState         & ( 1 << i ) ) &&
            ( m_dwHeadphoneState          & ( 1 << i ) ) &&
            !( m_dwConnectedCommunicators & ( 1 << i ) ) &&
            ( m_dwEnabled                 & ( 1 << i ) ) )
        {
            OnCommunicatorInserted( i );
        }
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: OnCommunicatorInserted
// Desc: Called when we detect that a communicator has physically been 
//          inserted into a controller
//-----------------------------------------------------------------------------
HRESULT CVoiceManager::OnCommunicatorInserted( DWORD dwControllerPort )
{
    // Tell the communicator to handle the insertion
    HRESULT hr = m_aVoiceCommunicators[ dwControllerPort ].OnInsertion( dwControllerPort );
    if( SUCCEEDED( hr ) )
    {
        assert( m_MuteList[ dwControllerPort ].empty() );
        assert( m_RemoteMuteList[ dwControllerPort ].empty() );

        m_dwConnectedCommunicators |= ( 1 << dwControllerPort );
        OnCommunicatorEvent( dwControllerPort, VOICE_COMMUNICATOR_INSERTED );

    }
    else
    {
        VoiceLog( L"Insertion on port %d failed", dwControllerPort );
        m_aVoiceCommunicators[ dwControllerPort ].OnRemoval();
    }

    return hr;
}



//-----------------------------------------------------------------------------
// Name: OnCommunicatorRemoved
// Desc: Called when we detect that a communicator has physically been
//          removed from a controller
//-----------------------------------------------------------------------------
HRESULT CVoiceManager::OnCommunicatorRemoved( DWORD dwControllerPort )
{
    m_dwConnectedCommunicators &= ~( 1 << dwControllerPort );
    m_aVoiceCommunicators[dwControllerPort].OnRemoval();

    OnCommunicatorEvent( dwControllerPort, VOICE_COMMUNICATOR_REMOVED );

    // Remove all muting info for this communicator
    for( MuteList::iterator it = m_MuteList[ dwControllerPort ].begin(); it < m_MuteList[ dwControllerPort ].end(); ++it )
    {
        UnMutePlayer( *it, dwControllerPort );
    }
    for( MuteList::iterator it = m_RemoteMuteList[ dwControllerPort ].begin(); it < m_RemoteMuteList[ dwControllerPort ].end(); ++it )
    {
        UnRemoteMutePlayer( *it, dwControllerPort );
    }

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: OnCommunicatorEvent
// Desc: Sends out status of whether or not we've got voice
//-----------------------------------------------------------------------------
HRESULT CVoiceManager::OnCommunicatorEvent( DWORD dwControllerPort, VOICE_COMMUNICATOR_EVENT event )
{
    m_pfnCommunicatorCallback( dwControllerPort, event, m_pCallbackContext );

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: EnterChatSession
// Desc: Brings the box into the chat session
//-----------------------------------------------------------------------------
VOID CVoiceManager::EnterChatSession()
{
    if( IsInChatSession() )
        return;

    m_bIsInChatSession = TRUE;

    for( WORD i = 0; i < XGetPortCount(); i++ )
    {
        assert( m_MuteList[i].empty() );
        assert( m_RemoteMuteList[i].empty() );
    }
}



//-----------------------------------------------------------------------------
// Name: LeaveChatSession
// Desc: Leaves the chat session
//-----------------------------------------------------------------------------
VOID CVoiceManager::LeaveChatSession()
{
    if( !IsInChatSession() )
        return;

    // Remove all our remote chatters
    while( m_dwNumChatters )
    {
        // RemoveChatter will swap a new guy into this position
        RemoveChatter( m_pChatters[0].xuid );
    }

    m_bIsInChatSession = FALSE;
}



//-----------------------------------------------------------------------------
// Name: ChatterIndexFromXUID
// Desc: Finds the index into m_pChatters for the given player XUID.
//          Returns m_dwNumChatters if not found
//-----------------------------------------------------------------------------
DWORD CVoiceManager::ChatterIndexFromXUID( XUID xuidPlayer )
{
    for( DWORD i = 0; i < m_dwNumChatters; i++ )
    {
        if( XOnlineAreUsersIdentical( &m_pChatters[i].xuid, &xuidPlayer ) )
            return i;
    }

    return m_dwNumChatters;
}



//-----------------------------------------------------------------------------
// Name: ToggleListenToChatter
// Desc: Modifies the chatter's stream's output mixbins so that the player 
//          on the specified port will or will not hear them
//-----------------------------------------------------------------------------
HRESULT CVoiceManager::ToggleListenToChatter( XUID xuidPlayer, BOOL bMute, DWORD dwPort )
{
    assert( IsInChatSession() );

    // Find the REMOTE_CHATTER struct
    DWORD chatterIndex = ChatterIndexFromXUID( xuidPlayer );
    assert( chatterIndex < m_dwNumChatters );

    // See if we're already OK (ie, already remote-muted, etc.)
    if( m_pChatters[ chatterIndex ].bMuted[ dwPort ] == bMute )
        return S_OK;

    // Update flag saying if this player is listening or not
    m_pChatters[ chatterIndex ].bMuted[ dwPort ] = bMute;

    // Update mix bin outputs
    DSMIXBINVOLUMEPAIR dsmbvp[XGetPortCount()];
    for( DWORD i = 0; i < XGetPortCount(); i++ )
    {
        dsmbvp[i].dwMixBin = DSMIXBIN_FXSEND_0 + i;
        dsmbvp[i].lVolume  = m_pChatters[ chatterIndex ].bMuted[ i ] ? DSBVOLUME_MIN : DSBVOLUME_MAX;
    }

    DSMIXBINS dsmb = { XGetPortCount(), dsmbvp };
    m_pChatters[ chatterIndex ].pOutputStream->SetMixBinVolumes( &dsmb );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: MutePlayer
// Desc: Handles muting a chatter
//-----------------------------------------------------------------------------
HRESULT CVoiceManager::MutePlayer( XUID xuidPlayer, DWORD dwControllerPort )
{
    VoiceLog( L"Muting player %I64x for port %d", xuidPlayer.qwUserID, dwControllerPort );

    assert( IsInChatSession() );
    assert( !IsPlayerMuted( xuidPlayer, dwControllerPort ) );

    // Add them to our mute list
    m_MuteList[ dwControllerPort ].push_back( xuidPlayer );

    // Make sure we don't hear the chatter anymore
    ToggleListenToChatter( xuidPlayer, TRUE, dwControllerPort );

    assert( IsPlayerMuted( xuidPlayer, dwControllerPort ) );

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: UnMutePlayer
// Desc: Handles un-muting a chatter
//-----------------------------------------------------------------------------
HRESULT CVoiceManager::UnMutePlayer( XUID xuidPlayer, DWORD dwControllerPort )
{
    VoiceLog( L"UnMuting player %I64x for port %d", xuidPlayer.qwUserID, dwControllerPort );

    assert( IsInChatSession() );
    assert( IsPlayerMuted( xuidPlayer, dwControllerPort ) );

    // Find the entry in our mute list and remove it
    for( MuteList::iterator it = m_MuteList[ dwControllerPort ].begin(); it < m_MuteList[ dwControllerPort ].end(); ++it )
    {
        if( XOnlineAreUsersIdentical( it, &xuidPlayer ) )
        {
            m_MuteList[ dwControllerPort ].erase( it );
            break;
        }
    }

    // If we're not remote-muted by that player, start listening to him
    if( !IsPlayerRemoteMuted( xuidPlayer, dwControllerPort ) )
        ToggleListenToChatter( xuidPlayer, FALSE, dwControllerPort );

    assert( !IsPlayerMuted( xuidPlayer, dwControllerPort ) );

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: IsPlayerMuted
// Desc: Returns TRUE if the player is in our mute list
//-----------------------------------------------------------------------------
BOOL CVoiceManager::IsPlayerMuted( XUID xuidPlayer, DWORD dwControllerPort )
{
    // Look for the player in our mute list
    for( MuteList::iterator it = m_MuteList[ dwControllerPort ].begin(); it < m_MuteList[ dwControllerPort ].end(); ++it )
    {
        if( XOnlineAreUsersIdentical( it, &xuidPlayer ) )
        {
            return TRUE;
        }
    }

    return FALSE;
}



//-----------------------------------------------------------------------------
// Name: RemoteMutePlayer
// Desc: Handles being muted by a different player
//-----------------------------------------------------------------------------
HRESULT CVoiceManager::RemoteMutePlayer( XUID xuidPlayer, DWORD dwControllerPort )
{
    VoiceLog( L"Remote muted port %d by player %I64x", dwControllerPort, xuidPlayer.qwUserID );

    assert( IsInChatSession() );
    assert( !IsPlayerRemoteMuted( xuidPlayer, dwControllerPort ) );

    // Add them to our remote mute list
    m_RemoteMuteList[ dwControllerPort ].push_back( xuidPlayer );

    // Make sure we don't hear him anymore
    ToggleListenToChatter( xuidPlayer, TRUE, dwControllerPort );

    assert( IsPlayerRemoteMuted( xuidPlayer, dwControllerPort ) );

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: UnRemoteMutePlayer
// Desc: Handles removing a remote-mute from another player
//-----------------------------------------------------------------------------
HRESULT CVoiceManager::UnRemoteMutePlayer( XUID xuidPlayer, DWORD dwControllerPort )
{
    VoiceLog( L"Remote UnMuted port %d by player %I64x", dwControllerPort, xuidPlayer.qwUserID );

    assert( IsInChatSession() );
    assert( IsPlayerRemoteMuted( xuidPlayer, dwControllerPort ) );

    // Remove them from our remote mute list
    for( MuteList::iterator it = m_RemoteMuteList[ dwControllerPort ].begin(); it < m_RemoteMuteList[ dwControllerPort ].end(); ++it )
    {
        if( XOnlineAreUsersIdentical( it, &xuidPlayer ) )
        {
            m_RemoteMuteList[ dwControllerPort ].erase( it );
            break;
        }
    }

    // If we don't have them muted, start listening to him
    if( !IsPlayerMuted( xuidPlayer, dwControllerPort ) )
        ToggleListenToChatter( xuidPlayer, FALSE, dwControllerPort );

    assert( !IsPlayerRemoteMuted( xuidPlayer, dwControllerPort ) );

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: IsPlayerRemoteMuted
// Desc: Returns TRUE if the player has been remotely muted
//-----------------------------------------------------------------------------
BOOL CVoiceManager::IsPlayerRemoteMuted( XUID xuidPlayer, DWORD dwControllerPort )
{
    for( MuteList::iterator it = m_RemoteMuteList[ dwControllerPort ].begin(); it < m_RemoteMuteList[ dwControllerPort ].end(); ++it )
    {
        if( XOnlineAreUsersIdentical( it, &xuidPlayer ) )
        {
            return TRUE;
        }
    }

    return FALSE;
}



//-----------------------------------------------------------------------------
// Name: DoesPlayerHaveVoice
// Desc: Returns TRUE if the player has a voice communicator connected
//-----------------------------------------------------------------------------
BOOL CVoiceManager::DoesPlayerHaveVoice( XUID xuidPlayer )
{
    DWORD chatterIndex = ChatterIndexFromXUID( xuidPlayer );

    return( chatterIndex < m_dwNumChatters );
}



//-----------------------------------------------------------------------------
// Name: IsPlayerTalking
// Desc: Returns TRUE if the player is currently talking
//-----------------------------------------------------------------------------
BOOL CVoiceManager::IsPlayerTalking( XUID xuidPlayer )
{
    DWORD chatterIndex = ChatterIndexFromXUID( xuidPlayer );
    assert( chatterIndex < m_dwNumChatters );

    return( m_pChatters[ chatterIndex ].bIsTalking );
}



//-----------------------------------------------------------------------------
// Name: BroadcastPacket
// Desc: Notify callback routine that a voice packet is ready
//-----------------------------------------------------------------------------
HRESULT CVoiceManager::BroadcastPacket( VOID* pvData, INT nSize, DWORD dwControllerPort )
{
    if( !IsInChatSession() )
        return S_OK;

    assert( nSize == m_dwCompressedSize );
    if( m_pfnVoiceDataCallback )
        m_pfnVoiceDataCallback( dwControllerPort, nSize, pvData, m_pCallbackContext );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ReceivePacket
// Desc: Handles receipt of a voice packet from the network
//-----------------------------------------------------------------------------
HRESULT CVoiceManager::ReceivePacket( XUID xuidFromPlayer, VOID* pvData, INT nSize )
{
    assert( IsInChatSession() );

    if( nSize == 0 )
        return S_OK;

    assert( nSize == m_dwCompressedSize );

    DWORD chatterIndex = ChatterIndexFromXUID( xuidFromPlayer );
    if( chatterIndex < m_dwNumChatters )
    {
        XMEDIAPACKET xmp = {0};

        // Send the packet to the queue
        xmp.pvBuffer  = pvData;
        xmp.dwMaxSize = nSize;

        // We keep track of how many packets are going in and coming out
        // of each voice queue, so that we can tell if it's not performing
        // well.
        REMOTE_CHATTER* pChatter = &m_pChatters[ chatterIndex ];
        pChatter->dwPacketsIn++;
        pChatter->dwPacketsWithoutOutput++;

        // If we have a long sequence of input with no output, or if we've
        // put in a lot more packets than we're getting out, then we should
        // reset the queue
        if( pChatter->dwPacketsWithoutOutput > m_dwQueueResetThreshold ||
            ( pChatter->dwPacketsIn > pChatter->dwPacketsOut + 20 &&
              pChatter->dwPacketsWithoutOutput > 2 ) )
        {
            VoiceLog( L"Queue %d has gone bad - resetting.", chatterIndex );
            ResetChatter( chatterIndex );
        }

        pChatter->pVoiceQueueXMO->Process( &xmp, NULL );
    }
    else
    {
        // This could happen if a remote player got our ADD_CHATTER message
        // and starts sending us voice, all before we get an ADD_CHATTER
        // message from them.  If it happens for a long period of time, it
        // probably means that an ADD_CHATTER message got lost somewhere.
        VoiceLog( L"Got packet from player %I64x, but no queue set up for them", xuidFromPlayer.qwUserID );
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: EnableCommunicator
// Desc: Enables or disabled voice on the specified controller.  If a 
//          controller has voice DISABLED, then no voice will be sent to
//          that peripheral, and no input will be captured FROM the peripheral
//          To the game code, it will look as if there is no communicator
//          plugged in.  If a game has a scenario where they want to know
//          that a communicator is plugged in, but still want it disabled,
//          then you'd want to change this so that the communicator was still
//          recognized, but just never processed.  Requires more work on the
//          game side to differentiate an inserted-but-banned communicator.
//-----------------------------------------------------------------------------
HRESULT CVoiceManager::EnableCommunicator( DWORD dwControllerPort, BOOL bEnabled )
{
    if( bEnabled )
    {
        // All we need to do is set the flag - if a communicator is currently
        // plugged in, it will be picked up in the next call to 
        // CheckDeviceChanges
        m_dwEnabled |= ( 1 << dwControllerPort );
    }
    else
    {
        m_dwEnabled &= ~( 1 << dwControllerPort );
        
        // Pretend the communicator was removed.  The enabled
        // flag will prevent it from being re-added in CheckDeviceChanges
        if( m_dwConnectedCommunicators & ( 1 << dwControllerPort ) )
        {
            OnCommunicatorRemoved( dwControllerPort );
        }
    }
        
    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: SetVoiceMask
// Desc: Controls voice masking for the specified controller.
//-----------------------------------------------------------------------------
HRESULT CVoiceManager::SetVoiceMask( DWORD dwControllerPort, XVOICE_MASK mask )
{
    assert( m_dwConnectedCommunicators & ( 1 << dwControllerPort ) );

    return m_aVoiceCommunicators[ dwControllerPort ].m_pEncoderXMO->SetVoiceMask( 0, &mask );
}



//-----------------------------------------------------------------------------
// Name: SetLoopback
// Desc: Sets a voice communicator to loop back on itself, rather than 
//          producing data.  Useful for voice mask testing
//-----------------------------------------------------------------------------
HRESULT CVoiceManager::SetLoopback( DWORD dwControllerPort, BOOL bLoopback )
{
    assert( m_dwConnectedCommunicators & ( 1 << dwControllerPort ) );

    if( bLoopback )
    {
        m_dwLoopback |= ( 1 << dwControllerPort );

        // Cut other players out of our headphone mix
        for( DWORD i = 0; i < m_dwNumChatters; i++ )
        {
            ToggleListenToChatter( m_pChatters[i].xuid, TRUE, dwControllerPort );
        }
        m_aVoiceCommunicators[ dwControllerPort ].ResetMicrophone();
        m_aVoiceCommunicators[ dwControllerPort ].ResetHeadphone();
    }
    else
    {
        m_dwLoopback &= ~( 1 << dwControllerPort );

        // Add other players back to our headphone mix
        for( DWORD i = 0; i < m_dwNumChatters; i++ )
        {
            if( !IsPlayerMuted( m_pChatters[i].xuid, dwControllerPort ) &&
                !IsPlayerRemoteMuted( m_pChatters[i].xuid, dwControllerPort ) )
            {
                ToggleListenToChatter( m_pChatters[i].xuid, FALSE, dwControllerPort );
            }
        }
        m_aVoiceCommunicators[ dwControllerPort ].ResetMicrophone();
        m_aVoiceCommunicators[ dwControllerPort ].ResetHeadphone();
    }

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: GetTemporaryPacket
// Desc: Fills out an XMEDIAPACKET pointing to a temporary (compressed) buffer
//-----------------------------------------------------------------------------
HRESULT CVoiceManager::GetTemporaryPacket( XMEDIAPACKET* pPacket )
{
    ZeroMemory( pPacket, sizeof( XMEDIAPACKET ) );

    pPacket->pvBuffer   = m_pbTempEncodedPacket;
    pPacket->dwMaxSize  = m_dwCompressedSize;

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: GetStreamPacket
// Desc: Fills out an XMEDIAPACKET pointing to the next stream packet for
//          the specified chatter
//-----------------------------------------------------------------------------
HRESULT CVoiceManager::GetStreamPacket( XMEDIAPACKET* pPacket, DWORD dwChatterIndex )
{
    REMOTE_CHATTER* pChatter = &m_pChatters[ dwChatterIndex ];

    ZeroMemory( pPacket, sizeof( XMEDIAPACKET ) );

    pPacket->pvBuffer   = pChatter->pbStreamBuffer + pChatter->dwCurrentPacket * m_dwPacketSize;
    pPacket->pdwStatus  = &pChatter->adwStatus[ pChatter->dwCurrentPacket ];
    pPacket->dwMaxSize  = m_dwPacketSize;

    ZeroMemory( pPacket->pvBuffer, pPacket->dwMaxSize );

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: SubmitStreamPacket
// Desc: Submits the XMEDIAPACKET for the specified chatter
//-----------------------------------------------------------------------------
HRESULT CVoiceManager::SubmitStreamPacket( XMEDIAPACKET* pPacket, DWORD dwChatterIndex )
{
    REMOTE_CHATTER* pChatter = &m_pChatters[ dwChatterIndex ];
    HRESULT hr;

    hr = pChatter->pOutputStream->Process( pPacket, NULL );
    pChatter->dwCurrentPacket = ( pChatter->dwCurrentPacket + 1 ) % m_dwNumBuffers;

    return hr;
}



//-----------------------------------------------------------------------------
// Name: ProcessMicrophones
// Desc: Processes input from the microphones
//-----------------------------------------------------------------------------
HRESULT CVoiceManager::ProcessMicrophones()
{
    for( WORD i = 0; i < XGetPortCount(); i++ )
    {
        // If we've got an active communicator, process the mic
        if( m_dwConnectedCommunicators & ( 1 << i ) )
        {
            // Check to see if we've starved the microphone - If we have, we 
            // should flush the encoder so that anyone receiving our packets 
            // will reset the corresponding queue.
            BOOL bStarved = TRUE;
            for( DWORD j = 0; j < m_dwNumBuffers; j++ )
            {
                if( m_aVoiceCommunicators[i].m_adwMicrophonePacketStatus[j] == XMEDIAPACKET_STATUS_PENDING )
                    bStarved = FALSE;
            }
            if( bStarved )
            {
                // Maybe we don't need to do anything here.  We're about to 
                // completely re-buffer the microphone, and hopefully anyone
                // listening to us will detect that their queue has gone bad
                VoiceLog( L"Microphone %d starved - resetting", i );
                m_aVoiceCommunicators[i].ResetMicrophone();
            }

            while( m_aVoiceCommunicators[i].MicrophonePacketStatus() != XMEDIAPACKET_STATUS_PENDING )
            {
                // Packet is done
                HRESULT hr;
                DWORD dwCompressedSize;
                XMEDIAPACKET xmpMicrophone;
                XMEDIAPACKET xmpCompressed;

                // Compress from microphone to temporary
                m_aVoiceCommunicators[i].GetMicrophonePacket( &xmpMicrophone );
                if( m_aVoiceCommunicators[i].MicrophonePacketStatus() == XMEDIAPACKET_STATUS_SUCCESS )
                {
                    GetTemporaryPacket( &xmpCompressed );
                    xmpCompressed.pdwCompletedSize    = &dwCompressedSize;

                    hr = m_aVoiceCommunicators[i].m_pEncoderXMO->ProcessMultiple( 1, &xmpMicrophone, 1, &xmpCompressed );
                    assert( SUCCEEDED( hr ) &&
                            ( dwCompressedSize == m_dwCompressedSize ||
                              dwCompressedSize == 0 ) );

                    // If this communicator is set to loopback, we'll encode,
                    // decode, and send straight to the headphone.  Otherwise,
                    // we notify the game code
                    if( m_dwLoopback & ( 1 << i ) )
                    {
                        XMEDIAPACKET xmpHeadphone;
                        m_aVoiceCommunicators[i].GetHeadphonePacket( &xmpHeadphone );
                        if( dwCompressedSize > 0 )
                        {
                            // If we got a compressed packet, decode it
                            m_aVoiceCommunicators[i].m_pDecoderXMO->ProcessMultiple( 1, &xmpCompressed, 1, &xmpHeadphone );
                        }
                        else
                        {
                            // No compressed data, so feed silence to the headphone
                            ZeroMemory( xmpHeadphone.pvBuffer, xmpHeadphone.dwMaxSize );
                        }
                        m_aVoiceCommunicators[i].SubmitHeadphonePacket( &xmpHeadphone );
                    }
                    else
                    {
                        // Not in loopback, so if we got a compressed packet, 
                        // notify the game code
                        if( dwCompressedSize > 0 )
                        {
                            // When a player starts speaking, we will ramp down
                            // the volume on their headphone
                            if( m_aVoiceCommunicators[i].m_dwRampTime < MAX_RAMP_TIME )
                                m_aVoiceCommunicators[i].m_dwRampTime += m_dwPacketTime;
                            
                            hr = BroadcastPacket( xmpCompressed.pvBuffer, dwCompressedSize, i );
                            assert( SUCCEEDED( hr ) );

                        } 
                        else
                        {
                            // When a player stops speaking, we will ramp the
                            // headphone volume back up
                            if( m_aVoiceCommunicators[i].m_dwRampTime > 0 )
                                m_aVoiceCommunicators[i].m_dwRampTime -= m_dwPacketTime;
                        }
                    }

                    // Re-submit microphone packet to microphone XMO
                    if( FAILED( m_aVoiceCommunicators[i].SubmitMicrophonePacket( &xmpMicrophone ) ) )
                        break;
                }
            }
        }
    }

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: ProcessQueues
// Desc: Processes output from the network queues
//-----------------------------------------------------------------------------
HRESULT CVoiceManager::ProcessQueues()
{
    for( DWORD i = 0; i < m_dwNumChatters; i++ )
    {
        DWORD dwQueueStatus;
        DWORD dwStreamStatus;

        // CONSIDER: Cap this so that if we glitched
        // a little we don't spend too much time catching up
        for( ; ; )
        {
            m_pChatters[i].pVoiceQueueXMO->GetStatus( &dwQueueStatus );
            m_pChatters[i].pOutputStream->GetStatus( &dwStreamStatus );

            // Check if the stream is starved
            if( dwStreamStatus & DSSTREAMSTATUS_STARVED )
            {
                VoiceLog( L"Stream %d starved", i );
                ResetChatter( i );
                break;
            }

            // See if we should pump data
            if( ( dwQueueStatus & XMO_STATUSF_ACCEPT_OUTPUT_DATA ) &&
                ( dwStreamStatus & XMO_STATUSF_ACCEPT_INPUT_DATA ) )
            {
                HRESULT         hr;
                XMEDIAPACKET    xmpCompressed;
                XMEDIAPACKET    xmpStream;
                DWORD           dwSizeFromQueue;

                // Fill out some XMEDIAPACKETS
                GetStreamPacket( &xmpStream, i );
                GetTemporaryPacket( &xmpCompressed );
                xmpCompressed.pdwCompletedSize = &dwSizeFromQueue;

                hr = m_pChatters[i].pVoiceQueueXMO->Process( NULL, &xmpCompressed );
                assert( SUCCEEDED( hr ) );
                assert( dwSizeFromQueue > 0 || m_pChatters[i].bHasOutput );

                m_pChatters[i].bHasOutput = TRUE;
                if( dwSizeFromQueue > 0 )
                {
                    assert( dwSizeFromQueue == m_dwCompressedSize );
                    m_pChatters[i].bIsTalking = TRUE;
                    m_pChatters[i].dwPacketsWithoutOutput = 0;
                    m_pChatters[i].dwPacketsOut++;

                    hr = m_pChatters[i].pDecoderXMO->ProcessMultiple( 1, &xmpCompressed, 1, &xmpStream );
                    assert( SUCCEEDED( hr ) );
                }
                else
                {
                    // Appopriate data was not available, so create a packet
                    // of silence
                    m_pChatters[i].bIsTalking = FALSE;
                    ZeroMemory( xmpStream.pvBuffer, m_dwPacketSize );
                }

                m_pChatters[i].pOutputStream->Pause( DSSTREAMPAUSE_RESUME );
                SubmitStreamPacket( &xmpStream, i );
            }
            else
            {
                break;
            }
        }
    }

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: GetSRCInfo
// Desc: Retrieves information about the state of the specified SRC DSP effect
//-----------------------------------------------------------------------------
HRESULT CVoiceManager::GetSRCInfo( DWORD dwControllerPort, 
                                   DWORD* pdwBufferSize,
                                   VOID** ppvBufferData, 
                                   DWORD* pdwWritePosition )
{
    // Get the state segment for the appropriate SRC effect
    DSEFFECTMAP* pEffectMap = &m_pDSPImageDesc->aEffectMaps[ m_dwFirstSRCEffectIndex + dwControllerPort ];
    LPCDSFX_SAMPLE_RATE_CONVERTER_PARAMS pSRCParams = LPCDSFX_SAMPLE_RATE_CONVERTER_PARAMS(pEffectMap->lpvStateSegment);

    if( pdwBufferSize )
        *pdwBufferSize = pEffectMap->dwScratchSize;

    if( ppvBufferData )
        *ppvBufferData = pEffectMap->lpvScratchSegment;

    if( pdwWritePosition )
        *pdwWritePosition = pSRCParams->dwScratchSampleOffset;

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: ProcessHeadphones
// Desc: Processes output to headphones
//-----------------------------------------------------------------------------
HRESULT CVoiceManager::ProcessHeadphones()
{
    // Ok, now try to feed headphones based off output from the SRC
    for( DWORD i = 0; i < XGetPortCount(); i++ )
    {
        // If we've got an active communicator, process the mic
        // If the communicator is in loopback mode, then we should
        // actually just bail out - the headphone is being directly fed
        // from the microphone.
        if( m_dwConnectedCommunicators & ( 1 << i ) &&
            !( m_dwLoopback            & ( 1 << i ) ) )
        {
            // Check to see if we've starved the headphone - If we have, we 
            // should reset it
            BOOL bStarved = TRUE;
            for( DWORD j = 0; j < m_dwNumBuffers; j++ )
            {
                if( m_aVoiceCommunicators[i].m_adwHeadphonePacketStatus[j] == XMEDIAPACKET_STATUS_PENDING )
                    bStarved = FALSE;
            }
            if( bStarved )
            {
                VoiceLog( L"Headphone %d starved - resetting", i );
                m_aVoiceCommunicators[i].ResetHeadphone();
            }

            // Grab packets of audio data from the output of the Sample 
            // Rate Converter DSP effect.  Right now, the effect is outputting
            // 32 bit samples instead of 16, so we have to downconvert them.
            // In the future, the SRC effect will be able to output 16
            // bit samples, and this will no longer be necessary
            DWORD dwSrcPacketSize = m_dwPacketSize * 2;

            DWORD dwCircularBufferSize;
            DWORD dwWritePosition;
            VOID* pvBufferData;
            GetSRCInfo( (WORD)i, &dwCircularBufferSize, &pvBufferData, &dwWritePosition );

            DWORD dwNumPackets = dwCircularBufferSize / dwSrcPacketSize;
            DWORD dwBytesAvailable = ( dwWritePosition + dwCircularBufferSize - m_aVoiceCommunicators[i].m_dwSRCReadPosition ) % dwCircularBufferSize;

            // If the headphone has space and there's a whole packet ready
            // (Note that these 2 should be in sync)
            while( m_aVoiceCommunicators[i].HeadphonePacketStatus() != XMEDIAPACKET_STATUS_PENDING &&
                   dwBytesAvailable >= dwSrcPacketSize )
            {
                // Downsample directly into the headphone buffer
                XMEDIAPACKET xmpHeadphone;
                m_aVoiceCommunicators[i].GetHeadphonePacket( &xmpHeadphone );

                // Convert from 32-bit to 16-bit samples as we copy into the 
                // headphone buffer
                DWORD  dwSamplesToStream = m_dwPacketSize / sizeof( WORD );
                PLONG  pSrcData = (PLONG)( (BYTE *)pvBufferData + m_aVoiceCommunicators[i].m_dwSRCReadPosition );
                PWORD  pDestData = (PWORD)xmpHeadphone.pvBuffer;

                // If the player is speaking into this particuar microphone,
                // we want to lower the volume level coming out their headphone
                // in order to minimize echo.  We ramp the amount of 
                // attenuation up and down, so it doesn't quickly shift between
                // volume levels.  
                // In a future XDK release, this will be done in the SRC effect
                // running on the DSP, and with finer granularity.
                DWORD dwAttenuation = m_aVoiceCommunicators[i].m_dwRampTime * MAX_HEADROOM / MAX_RAMP_TIME;
                DWORD dwShift = dwAttenuation + 16;

                // If we're wrapping around to the start of our circular
                // buffer, then process the last little bit here
                if( dwSrcPacketSize > dwCircularBufferSize - m_aVoiceCommunicators[i].m_dwSRCReadPosition )
                {
                    DWORD dwPartial = ( dwCircularBufferSize - m_aVoiceCommunicators[i].m_dwSRCReadPosition ) / sizeof( DWORD );
                    for( DWORD dwSample = 0; dwSample < dwPartial; dwSample++ )
                        *pDestData++ = WORD( *pSrcData++ >> dwShift );

                    dwSamplesToStream -= dwPartial;
                    assert( (PLONG)pSrcData == (PLONG)( (BYTE*)pvBufferData + dwCircularBufferSize ) );
                    pSrcData = (PLONG)pvBufferData;
                }

                for( DWORD dwSample = 0; dwSample < dwSamplesToStream; dwSample++ )
                    *pDestData++ = WORD( *pSrcData++ >> dwShift );

                m_aVoiceCommunicators[i].m_dwSRCReadPosition = ( m_aVoiceCommunicators[i].m_dwSRCReadPosition + dwSrcPacketSize ) % dwCircularBufferSize;
                dwBytesAvailable -= dwSrcPacketSize;

                if( FAILED( m_aVoiceCommunicators[i].SubmitHeadphonePacket( &xmpHeadphone ) ) )
                    break;
            }

            // Check to see if the headphone is falling behind - the communicator
            // and audio chip are on different clocks, so there may be some drift
            if( dwBytesAvailable > dwSrcPacketSize * ( dwNumPackets - 1 ) )
            {
                VoiceLog( L"Communicator %d is falling behind SRC", i );
                m_aVoiceCommunicators[i].m_dwSRCReadPosition = ( m_aVoiceCommunicators[i].m_dwSRCReadPosition + dwSrcPacketSize ) % dwCircularBufferSize;
                dwBytesAvailable -= dwSrcPacketSize;
            }

        }
    }

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: ProcessVoice
// Desc: Processes input from the input controller devices.
//-----------------------------------------------------------------------------
HRESULT CVoiceManager::ProcessVoice()
{
    CheckDeviceChanges();

    if( m_bFlushQueuesOnNextProcess )
    {
        m_bFlushQueuesOnNextProcess = FALSE;
        FlushQueuesInternal();
    }

    ProcessMicrophones();

    ProcessQueues();

    ProcessHeadphones();

    DirectSoundDoWork();

#if _DEBUG
    ValidateStateDbg();
#endif // _DEBUG
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ResetChatter
// Desc: Resets a remote chatter.  This should be called when we've detected
//          a glitch in processing the chatter, for exmple, if his stream
//          starved.
//-----------------------------------------------------------------------------
HRESULT CVoiceManager::ResetChatter( DWORD dwChatterIndex )
{
    REMOTE_CHATTER* pChatter = &m_pChatters[ dwChatterIndex ];

    if( !pChatter->pVoiceQueueXMO )
        return S_OK;
    
    VoiceLog( L"Flushing chatter index %d (player %I64x)", dwChatterIndex, pChatter->xuid.qwUserID );

    pChatter->bHasOutput = FALSE;
    pChatter->bIsTalking = FALSE;

    // Flush all XMOs associated with this chatter
    pChatter->pVoiceQueueXMO->Flush();
    pChatter->pOutputStream->Flush();
    pChatter->pDecoderXMO->Flush();

    // Pre-buffer the stream again
    pChatter->pOutputStream->Pause( DSSTREAMPAUSE_PAUSE );
    ZeroMemory( pChatter->pbStreamBuffer, m_dwBufferSize );
    ZeroMemory( pChatter->adwStatus, m_dwNumBuffers * sizeof( DWORD ) );
    pChatter->dwCurrentPacket = 0;

    XMEDIAPACKET xmp;
    for( DWORD i = 0; i < m_dwNumBuffers - 1; i++ )
    {
        GetStreamPacket( &xmp, dwChatterIndex );
        SubmitStreamPacket( &xmp, dwChatterIndex );
    }

    // Set up our various packet counts
    pChatter->dwPacketsWithoutOutput    = 0;
    pChatter->dwPacketsIn               = 0;
    pChatter->dwPacketsOut              = 0;

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: FlushQueuesInternal
// Desc: Iterates over each chatter, flushing their queue
//-----------------------------------------------------------------------------
HRESULT CVoiceManager::FlushQueuesInternal()
{
    VoiceLog( L"Flushing all voice queues and communicators." );

    // Flush and pre-buffer each communicator's headphone and mic
    for( DWORD i = 0; i < XGetPortCount(); i++ )
    {
        if( m_dwConnectedCommunicators & ( 1 << i ) )
        {
            m_aVoiceCommunicators[i].ResetMicrophone();
            m_aVoiceCommunicators[i].ResetHeadphone();
        }
    }

    for( DWORD j = 0; j < m_dwNumChatters; j++ )
    {
        ResetChatter( j );
    }

    return S_OK;
}



#if _DEBUG
//-----------------------------------------------------------------------------
// Name: VoiceLog
// Desc: Debug function for logging and displaying status messages
//-----------------------------------------------------------------------------
VOID CVoiceManager::VoiceLog( WCHAR* format, ... )
{
    va_list arglist;

    va_start( arglist, format );
    _vsnwprintf( m_strDebugLog[ m_dwCurrentLogEntry ], s_dwLogEntrySize, format, arglist );
    va_end( arglist );

    m_dwCurrentLogEntry = ( m_dwCurrentLogEntry + 1 ) % s_dwNumLogEntries;

}



//-----------------------------------------------------------------------------
// Name: DrawColoredQuad
// Desc: Draws a quad at the specified location in a single color
//-----------------------------------------------------------------------------
VOID DrawColoredQuad( FLOAT fX, FLOAT fY, FLOAT fWidth, FLOAT fHeight, DWORD dwColor )
{
    typedef struct { XGVECTOR4 p; D3DCOLOR c; } BUFFER_VERTEX;
    BUFFER_VERTEX vQuad[4];
    vQuad[0].p = XGVECTOR4( fX, fY + fHeight, 1.0f, 1.0f );
    vQuad[0].c = dwColor;
    vQuad[1].p = XGVECTOR4( fX, fY, 1.0f, 1.0f );
    vQuad[1].c = dwColor;
    vQuad[2].p = XGVECTOR4( fX + fWidth, fY, 1.0f, 1.0f );
    vQuad[2].c = dwColor;
    vQuad[3].p = XGVECTOR4( fX + fWidth, fY + fHeight, 1.0f, 1.0f );
    vQuad[3].c = dwColor;

    IDirect3DDevice8::SetVertexShader( D3DFVF_XYZRHW | D3DFVF_DIFFUSE );
    IDirect3DDevice8::SetRenderState( D3DRS_LIGHTING, FALSE );
    IDirect3DDevice8::SetRenderState( D3DRS_ZENABLE, FALSE );
    IDirect3DDevice8::SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
    IDirect3DDevice8::SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
    IDirect3DDevice8::SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
    IDirect3DDevice8::SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
    IDirect3DDevice8::DrawPrimitiveUP( D3DPT_QUADLIST, 1, vQuad, sizeof( BUFFER_VERTEX ) );
}



//-----------------------------------------------------------------------------
// Name: RenderBuffer
// Desc: Renders a visualization of a buffer with some filled, some empty
//          packets
//-----------------------------------------------------------------------------
VOID RenderBuffer( FLOAT fStartX, 
                   FLOAT fStartY, 
                   DWORD dwFilled, 
                   DWORD dwTotal, 
                   FLOAT fSize,
                   BOOL  bUp)
{
    FLOAT fX = fStartX;
    FLOAT fY = fStartY;

    DWORD dwEmptyColor = 0x80006000;
    DWORD dwFilledColor;
    if( dwFilled == 1 )
        dwFilledColor   = 0x80FF0000;
    else
        dwFilledColor   = 0x8000FF00;

    for( DWORD i = 0; i < dwTotal; i++ )
    {
        DrawColoredQuad( fX, fY, fSize, fSize, ( i < dwFilled ) ? dwFilledColor : dwEmptyColor );

        if( bUp )
            fY -= fSize;
        else
            fY += fSize;
    }
}


//-----------------------------------------------------------------------------
// Name: RenderDebugInfo
// Desc: Debug function for toggling debug display on/off
//-----------------------------------------------------------------------------
VOID CVoiceManager::RenderDebugInfo( CXBFont* pFont )
{
    // First render the local communicators
    FLOAT fStartX = 60.0f;
    FLOAT fSpacing = 140.0f;
    for( DWORD i = 0; i < XGetPortCount(); i++ )
    {
        if( m_dwConnectedCommunicators & ( 1 << i ) )
        {
            DrawColoredQuad( fStartX + fSpacing * i - 10, 360.0f, fSpacing / 2, 120.0f, 0xC0202020 );
            if( pFont )
                pFont->DrawText( fStartX + fSpacing * i, 440, 0xFFA0A0A0, L"Voice" );

            // Check the microphone
            DWORD dwMicrophoneFilled = 0;
            for( DWORD j = 0; j < m_dwNumBuffers; j++ )
            {
                if( m_aVoiceCommunicators[i].m_adwMicrophonePacketStatus[j] == XMEDIAPACKET_STATUS_PENDING )
                    ++dwMicrophoneFilled;
            }
            if( pFont )
                pFont->DrawText( fStartX + fSpacing * i, 425, 0xFFA0A0A0, L"M" );
            RenderBuffer( fStartX + fSpacing * i, 420, dwMicrophoneFilled, m_dwNumBuffers, 10, TRUE );

            // Check the SRC output buffer
            DWORD dwWritePosition;
            DWORD dwBufferSize;
            GetSRCInfo( (WORD)i, &dwBufferSize, NULL, &dwWritePosition );
            DWORD dwSRCFilled = ( ( dwWritePosition + dwBufferSize - m_aVoiceCommunicators[i].m_dwSRCReadPosition ) % dwBufferSize ) / ( m_dwPacketSize * 2 );
            if( pFont )
                pFont->DrawText( fStartX + fSpacing * i + 20, 425, 0xFFA0A0A0, L"S" );
            RenderBuffer( fStartX + fSpacing * i + 20, 420, dwSRCFilled, dwBufferSize / ( 2 * m_dwPacketSize ), 10, TRUE );

            // Check the headphone
            DWORD dwHeadphoneFilled = 0;
            for( DWORD j = 0; j < m_dwNumBuffers; j++ )
            {
                if( m_aVoiceCommunicators[i].m_adwHeadphonePacketStatus[j] == XMEDIAPACKET_STATUS_PENDING )
                    ++dwHeadphoneFilled;
            }
            if( pFont )
                pFont->DrawText( fStartX + fSpacing * i + 40, 425, 0xFFA0A0A0, L"H" );
            RenderBuffer( fStartX + fSpacing * i + 40, 420, dwHeadphoneFilled, m_dwNumBuffers, 10, TRUE );
        }
    }

    // Now render each remote chatter
    fStartX = 60.0f;
    fSpacing = 120.0f;
    for( DWORD i = 0; i < m_dwNumChatters; i++ )
    {
        DrawColoredQuad( fStartX + fSpacing * i - 10, 40, fSpacing, 100, 0xC0202020 );
        WCHAR str[100];
        swprintf( str, L"%I64x", m_pChatters[i].xuid.qwUserID );
        DWORD dwColor = m_pChatters[i].bIsTalking ? 0xFFFF6060 : 0xFFA0A0A0;
        if( pFont )
            pFont->DrawText( fStartX + fSpacing * i, 55, dwColor, str );

        // Count how many stream packets are filled
        DWORD dwStreamFilled = 0;
        for( DWORD j = 0; j < m_dwNumBuffers; j++ )
        {
            if( m_pChatters[i].adwStatus[j] == XMEDIAPACKET_STATUS_PENDING )
                ++dwStreamFilled;
        }

        RenderBuffer( fStartX + fSpacing * i, 75, dwStreamFilled, m_dwNumBuffers, 10, FALSE );
    }

    // Now render debug log text
    DrawColoredQuad( 70.0f, 160.0f, 450.0f, 30 * s_dwNumLogEntries + 20.0f, 0xC0202020 );
    for( DWORD i = 0; i < s_dwNumLogEntries; i++ )
    {
        if( pFont )
            pFont->DrawText( 80.0f, 170.0f + 30 * i, 0xFFA0A0A0, m_strDebugLog[ ( i + m_dwCurrentLogEntry ) % s_dwNumLogEntries ] );
    }
}



//-----------------------------------------------------------------------------
// Name: ValidateStateDbg
// Desc: Validates that the VoiceManager is in a consistent state
//-----------------------------------------------------------------------------
HRESULT CVoiceManager::ValidateStateDbg()
{
    // Only the bottom 4 bits should be used
    assert( !( m_dwConnectedCommunicators & 0xFFFFFFF0 ) );
    assert( !( m_dwMicrophoneState        & 0xFFFFFFF0 ) );
    assert( !( m_dwHeadphoneState         & 0xFFFFFFF0 ) );
    assert( !( m_dwLoopback               & 0xFFFFFFF0 ) );
    assert( !( m_dwEnabled                & 0xFFFFFFF0 ) );

    // Check the state of the chatters list
    for( DWORD i = 0; i < m_dwNumChatters; i++ )
    {
        // Make sure they're not duplicated in the chatters list
        for( DWORD j = i + 1; j < m_dwNumChatters; j++ )
        {
            if( XOnlineAreUsersIdentical( &m_pChatters[i].xuid, &m_pChatters[j].xuid ) )
            {
                assert( FALSE && "Duplicate player in chatters list!" );
            }
        }

        if( !m_pChatters[i].bHasOutput )
        {
            // If they haven't spoken, they can't be talking
            assert( !m_pChatters[i].bIsTalking );

            DWORD dwStreamStatus;
            m_pChatters[i].pOutputStream->GetStatus( &dwStreamStatus );
            assert( dwStreamStatus == ( DSSTREAMSTATUS_PAUSED | XMO_STATUSF_ACCEPT_INPUT_DATA ) );
        }
        else
        {
            DWORD dwStreamStatus;
            m_pChatters[i].pOutputStream->GetStatus( &dwStreamStatus );
            assert( dwStreamStatus & ( DSSTREAMSTATUS_PLAYING | DSSTREAMSTATUS_STARVED ) );
        }

        // Make sure that the mixing control is in sync with the mute
        // list
        for( WORD j = 0; j < XGetPortCount(); j++ )
        {
            if( m_pChatters[i].bMuted[j] )
            {
                assert( IsPlayerMuted( m_pChatters[i].xuid, j ) ||
                        IsPlayerRemoteMuted( m_pChatters[i].xuid, j ) ||
                        m_dwLoopback & ( 1 << j ) );
            }
            else
            {
                assert( !IsPlayerMuted( m_pChatters[i].xuid, j ) &&
                        !IsPlayerRemoteMuted( m_pChatters[i].xuid, j ) );
            }
        }
    }

    // Check the mute list and remote mute list - every user in them
    // should currently be in the chatters list
    for( WORD i = 0; i < XGetPortCount(); i ++ )
    {
        if( !( m_dwConnectedCommunicators & ( 1 << i ) ) )
        {
            assert( m_MuteList[i].empty() );
            assert( m_RemoteMuteList[i].empty() );
        }
        else
        {
            for( MuteList::iterator it = m_MuteList[i].begin(); it < m_MuteList[i].end(); ++it )
            {
                assert( ChatterIndexFromXUID( *it ) < m_dwNumChatters );
            }
            for( MuteList::iterator it = m_RemoteMuteList[i].begin(); it < m_RemoteMuteList[i].end(); ++it )
            {
                assert( ChatterIndexFromXUID( *it ) < m_dwNumChatters );
            }
        }
    }

    assert( IsInChatSession() || m_dwNumChatters == 0 );

    return S_OK;
}



#endif // DEBUG