//-----------------------------------------------------------------------------
// File: VoiceCommunicator.cpp
//
// Desc: Implementation of Voice Communicator support
//
// Hist: 1.17.02 - Created
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#include "VoiceCommunicator.h"
// TODO (JHarding): Their include system is SO messed up.
#include "revolt.h"
#include "network.h"
#include <cassert>

CVoiceCommunicator g_VoiceCommunicators[4];


//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
CVoiceCommunicator::CVoiceCommunicator()
{
    m_pMicrophoneXMO    = NULL;
    m_pHeadphoneXMO     = NULL;
    m_pEncoderXMO       = NULL;
    m_pVoiceQueueXMO    = NULL;

    m_lSlot             = INACTIVE_SLOT;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
CVoiceCommunicator::~CVoiceCommunicator()
{
    assert( m_pMicrophoneXMO == NULL &&
            m_pHeadphoneXMO  == NULL &&
            m_pEncoderXMO    == NULL &&
            m_pVoiceQueueXMO == NULL );
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CVoiceCommunicator::OnInsertion( DWORD dwSlot )
{
    DebugPrint( "Voice Communicator inserted into slot %d\n", dwSlot );

    m_lSlot = LONG( dwSlot );

    WAVEFORMATEX wfx;
    wfx.cbSize = 0;
    wfx.nChannels = 1;
    wfx.nSamplesPerSec = 8000;
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = wfx.wBitsPerSample / 8 * wfx.nChannels;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    wfx.wFormatTag = WAVE_FORMAT_PCM;

    XVoiceCreateMediaObject( XDEVICE_TYPE_VOICE_MICROPHONE, 
                             m_lSlot,
                             NUM_VOICE_PACKETS,
                             &wfx,
                             &m_pMicrophoneXMO );

    XVoiceCreateMediaObject( XDEVICE_TYPE_VOICE_HEADPHONE, 
                             m_lSlot,
                             NUM_VOICE_PACKETS,
                             &wfx,
                             &m_pHeadphoneXMO );

    // Set up the XMEDIAPACKET queues.  Note, I'm assuming that
    // the XMOs must complete packets in order, otherwise I'll
    // need to keep track of order of submission myself.
    m_pbMicrophoneBuffer = new BYTE[ NUM_VOICE_PACKETS * VOICE_PACKET_SIZE ];
    m_dwNextMicrophonePacket = 0;
    for( DWORD i = 0; i < NUM_VOICE_PACKETS; i++ )
    {
        // TODO (JHarding): The XMO should be setting status to pending
        // as soon as it receives the packet, but it's not.
        m_adwMicrophonePacketStatus[ i ] = XMEDIAPACKET_STATUS_PENDING;

        XMEDIAPACKET xmp = {0};
        xmp.pvBuffer = m_pbMicrophoneBuffer + i * VOICE_PACKET_SIZE;
        xmp.dwMaxSize = VOICE_PACKET_SIZE;
        xmp.pdwStatus = &m_adwMicrophonePacketStatus[ i ];

        m_pMicrophoneXMO->Process( NULL, &xmp );
    }

    // Insert a delay between mic and headphone
    // Sleep( 1000 );

    // Now headphone
    for( DWORD i = 0; i < NUM_VOICE_PACKETS; i++ )
        m_adwHeadphonePacketStatus[ i ] = XMEDIAPACKET_STATUS_SUCCESS;
    m_dwNextHeadphonePacket = 0;
    m_pbHeadphoneBuffer = new BYTE[ NUM_VOICE_PACKETS * VOICE_PACKET_SIZE ];

    XVoiceEncoderCreateMediaObject( FALSE,
                                    WAVE_FORMAT_VOXWARE_SC06,
                                    &wfx,
                                    20,
                                    &m_pEncoderXMO );

    QUEUE_XMO_CONFIG VoiceQueueCfg = {0};
    VoiceQueueCfg.cbSize                = sizeof( QUEUE_XMO_CONFIG );
    VoiceQueueCfg.wVoiceSamplingRate    = 8000;
    VoiceQueueCfg.dwCodecTag            = WAVE_FORMAT_VOXWARE_SC06;
    VoiceQueueCfg.wMsOfDataPerPacket    = 20;

    XVoiceQueueCreateMediaObject( &VoiceQueueCfg, &m_pVoiceQueueXMO );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CVoiceCommunicator::OnRemoval()
{
    DebugPrint( "Voice Communicator removed from slot %d\n", m_lSlot );

    m_lSlot = INACTIVE_SLOT;

    assert( m_pMicrophoneXMO != NULL &&
            m_pHeadphoneXMO  != NULL &&
            m_pEncoderXMO    != NULL &&
            m_pVoiceQueueXMO != NULL );

    m_pMicrophoneXMO->Release();
    m_pMicrophoneXMO = NULL;
    m_pHeadphoneXMO->Release();
    m_pHeadphoneXMO = NULL;
    m_pEncoderXMO->Release();
    m_pEncoderXMO = NULL;
    m_pVoiceQueueXMO->Release();
    m_pVoiceQueueXMO = NULL;

    assert( m_pbMicrophoneBuffer != NULL &&
            m_pbHeadphoneBuffer  != NULL );

    delete[] m_pbMicrophoneBuffer;
    m_pbMicrophoneBuffer = NULL;
    delete[] m_pbHeadphoneBuffer;
    m_pbHeadphoneBuffer = NULL;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ReceivePacket
// Desc:
//-----------------------------------------------------------------------------
HRESULT CVoiceCommunicator::ReceivePacket( VOID* pvData, INT nSize )
{
    if( m_lSlot != INACTIVE_SLOT && nSize > 0 )
    {
        // DebugPrint( "Received %d bytes at %lx\n", nSize, pvData );

        XMEDIAPACKET xmp = {0};

        // Send the packet to the queue
        xmp.pvBuffer          = pvData;
        xmp.dwMaxSize         = nSize;

        HRESULT hr = m_pVoiceQueueXMO->Process( &xmp, NULL );

        // TODO (JHarding): If the framerate drops too low for too long,
        // we could actually overflow the queue and this could fail.
        // assert( SUCCEEDED( hr ) );
    }


    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: BroadcastPacket
// Desc: Initializes voice commanders.
//-----------------------------------------------------------------------------
HRESULT CVoiceCommunicator::BroadcastPacket( VOID* pvData, INT nSize )
{
#ifndef XBOX_DISABLE_NETWORK
    if( soUDP != INVALID_SOCKET )
    {
        // DebugPrint( "Broadcasting %d bytes at %lx\n", nSize, pvData );

        MESSAGE_HEADER* header = (MESSAGE_HEADER *)SendMsgBuffer;
        VOID* pBuffer = (VOID *)( header + 1 );

        header->Type = MESSAGE_VOICE_PACKET;
        header->Contents = 0;

        memcpy( pBuffer, pvData, nSize );

        int cbSend = sizeof( MESSAGE_HEADER ) + nSize;
        sendto( soUDP, SendMsgBuffer, cbSend, 0, (sockaddr*)&SoAddrBroadcast, sizeof(SOCKADDR_IN) );
    }
#endif // ! XBOX_DISABLE_NETWORK

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: static InitCommunicators
// Desc: Initializes voice commanders.
//-----------------------------------------------------------------------------
HRESULT CVoiceCommunicator::InitCommunicators()
{
    // Get a mask of all currently available voice commanders. The mask
    // for headphones should be identical to microphones
    DWORD dwMicrophones = XGetDevices( XDEVICE_TYPE_VOICE_MICROPHONE );
    DWORD dwHeadphones = XGetDevices( XDEVICE_TYPE_VOICE_HEADPHONE );

    // This shouldn't be able to happen.
    if( dwMicrophones != dwHeadphones )
        return E_FAIL;

    for( DWORD i = 0; i < XGetPortCount(); i++ )
    {
        if( dwMicrophones & ( 1 << i ) )
            g_VoiceCommunicators[i].OnInsertion( i );
    }

    // Return OK for now. Could return FAIL, to detect no controllers are
    // inserted, or maybe return a DWORD of the # of inserted controllers
    return S_OK;
}


#define USE_QUEUE 1
//-----------------------------------------------------------------------------
// Name: static ProcessVoice
// Desc: Processes input from the input controller devices.
//-----------------------------------------------------------------------------
HRESULT CVoiceCommunicator::ProcessVoice()
{
    DWORD dwMicrophoneInsertions, dwMicrophoneRemovals;
    DWORD dwHeadphoneInsertions,  dwHeadphoneRemovals;

    XGetDeviceChanges( XDEVICE_TYPE_VOICE_MICROPHONE, 
                       &dwMicrophoneInsertions, 
                       &dwMicrophoneRemovals );
    XGetDeviceChanges( XDEVICE_TYPE_VOICE_HEADPHONE, 
                       &dwHeadphoneInsertions, 
                       &dwHeadphoneRemovals );

    // These should be equal
    if( dwMicrophoneInsertions != dwHeadphoneInsertions ||
        dwMicrophoneRemovals   != dwHeadphoneRemovals )
        return E_FAIL;

    for( DWORD i = 0; i < XGetPortCount(); i++ )
    {
        // Check for removals
        if( dwMicrophoneRemovals & ( 1 << i ) )
        {
            g_VoiceCommunicators[i].OnRemoval();
        }

        // Check for insertions
        if( dwMicrophoneInsertions & ( 1 << i ) )
        {
            g_VoiceCommunicators[i].OnInsertion( i );
        }

        // If we've got an active communicator, process the mic
        if( g_VoiceCommunicators[i].m_lSlot != INACTIVE_SLOT )
        {
            // Do whatever we need to update voice here.
            switch( g_VoiceCommunicators[i].m_adwMicrophonePacketStatus[ g_VoiceCommunicators[i].m_dwNextMicrophonePacket ] )
            {
            case XMEDIAPACKET_STATUS_SUCCESS:
                // Packet is done
                {
                    HRESULT hr;
                    WORD wCompressedBufferSize;
                    XVoiceGetCodecBufferSize( g_VoiceCommunicators[i].m_pEncoderXMO, VOICE_PACKET_SIZE, &wCompressedBufferSize );
                    BYTE* pCompressedBuffer = new BYTE[ wCompressedBufferSize ];
                    DWORD dwCompressedSize;
                    XMEDIAPACKET xmpSource = {0};
                    XMEDIAPACKET xmpDest = {0};

                    // Compress from microphone to temporary
                    xmpSource.pvBuffer          = g_VoiceCommunicators[i].m_pbMicrophoneBuffer + 
                                                  g_VoiceCommunicators[i].m_dwNextMicrophonePacket * VOICE_PACKET_SIZE;
                    xmpSource.dwMaxSize         = VOICE_PACKET_SIZE;

                    xmpDest.pvBuffer            = pCompressedBuffer;
                    xmpDest.dwMaxSize           = wCompressedBufferSize;
                    xmpDest.pdwCompletedSize    = &dwCompressedSize;

                    hr = g_VoiceCommunicators[i].m_pEncoderXMO->Process( &xmpSource, &xmpDest );
                    assert( SUCCEEDED( hr ) && dwCompressedSize <= wCompressedBufferSize );

                    // Nothing compressed, so nothing to decompress/playback
                    if( dwCompressedSize > 0 )
                    {
                        static BOOL bLoopback = FALSE;
                        if( bLoopback )
                        {
                            hr = g_VoiceCommunicators[i].ReceivePacket( pCompressedBuffer, dwCompressedSize );
                            assert( SUCCEEDED( hr ) );
                        }
                        else if( !bLoopback && dwCompressedSize > 0 )
                        {
                            hr = g_VoiceCommunicators[i].BroadcastPacket( xmpDest.pvBuffer, dwCompressedSize );
                            assert( SUCCEEDED( hr ) );
                        }
                    }

                    delete[] pCompressedBuffer;

                    // Re-submit microphone packet to microphone XMO
                    ZeroMemory( &xmpSource, sizeof( XMEDIAPACKET ) );
                    ZeroMemory( &xmpDest,   sizeof( XMEDIAPACKET ) );

                    xmpDest.pvBuffer  = g_VoiceCommunicators[i].m_pbMicrophoneBuffer +
                                          g_VoiceCommunicators[i].m_dwNextMicrophonePacket * VOICE_PACKET_SIZE;
                    xmpDest.dwMaxSize = VOICE_PACKET_SIZE;
                    xmpDest.pdwStatus = &( g_VoiceCommunicators[i].m_adwMicrophonePacketStatus[ g_VoiceCommunicators[i].m_dwNextMicrophonePacket ] );
                    *(xmpDest.pdwStatus) = XMEDIAPACKET_STATUS_PENDING;

                    g_VoiceCommunicators[i].m_pMicrophoneXMO->Process( NULL, &xmpDest );
                    g_VoiceCommunicators[i].m_dwNextMicrophonePacket = ( g_VoiceCommunicators[i].m_dwNextMicrophonePacket + 1 ) % NUM_VOICE_PACKETS;

                }
                break;
            case XMEDIAPACKET_STATUS_PENDING:
                for( DWORD j = 1; j < NUM_VOICE_PACKETS; j++ )
                {
                    if( !( g_VoiceCommunicators[i].m_adwMicrophonePacketStatus[ ( g_VoiceCommunicators[i].m_dwNextMicrophonePacket + j ) % NUM_VOICE_PACKETS ] == XMEDIAPACKET_STATUS_PENDING ) )
                    {
                        DebugPrint( "Packet %d is pending, but Packet %d has status %lx\n", g_VoiceCommunicators[i].m_dwNextMicrophonePacket, ( g_VoiceCommunicators[i].m_dwNextMicrophonePacket + j ) % NUM_VOICE_PACKETS, g_VoiceCommunicators[i].m_adwMicrophonePacketStatus[ ( g_VoiceCommunicators[i].m_dwNextMicrophonePacket + j ) % NUM_VOICE_PACKETS ] );
                        assert( false );
                    }
                }
                // This is fine
                break;
            default:
                // Bad!
                assert( false );
                g_VoiceCommunicators[i].m_adwMicrophonePacketStatus[ g_VoiceCommunicators[i].m_dwNextMicrophonePacket ] = XMEDIAPACKET_STATUS_SUCCESS;
                g_VoiceCommunicators[i].m_dwNextMicrophonePacket = ( g_VoiceCommunicators[i].m_dwNextMicrophonePacket + 1 ) % NUM_VOICE_PACKETS;
            }

            // If the headphone has room for data, check if there's anything ready.
            if( g_VoiceCommunicators[i].m_adwHeadphonePacketStatus[ g_VoiceCommunicators[i].m_dwNextHeadphonePacket ] != XMEDIAPACKET_STATUS_PENDING )
            {
                // Check the queue for data...
                HRESULT      hr;
                XMEDIAPACKET xmpFromQueue = {0};
                DWORD        dwFromQueue;
                xmpFromQueue.pvBuffer = g_VoiceCommunicators[i].m_pbHeadphoneBuffer + g_VoiceCommunicators[i].m_dwNextHeadphonePacket * VOICE_PACKET_SIZE;
                xmpFromQueue.dwMaxSize = VOICE_PACKET_SIZE;
                xmpFromQueue.pdwCompletedSize = &dwFromQueue;

                hr = g_VoiceCommunicators[i].m_pVoiceQueueXMO->Process( NULL, &xmpFromQueue );
                if( SUCCEEDED( hr ) )
                {
                    xmpFromQueue.dwMaxSize = dwFromQueue;
                    xmpFromQueue.pdwCompletedSize = NULL;
                    xmpFromQueue.pdwStatus = &( g_VoiceCommunicators[i].m_adwHeadphonePacketStatus[ g_VoiceCommunicators[i].m_dwNextHeadphonePacket ] );
                    (*xmpFromQueue.pdwStatus) = XMEDIAPACKET_STATUS_PENDING;

                    g_VoiceCommunicators[i].m_pHeadphoneXMO->Process( &xmpFromQueue, NULL );
                    g_VoiceCommunicators[i].m_dwNextHeadphonePacket = ( g_VoiceCommunicators[i].m_dwNextHeadphonePacket + 1 ) % NUM_VOICE_PACKETS;
                }
            }
        }
    }

    return S_OK;
}


