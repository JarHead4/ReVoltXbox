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
#include "VoiceManager.h"
#include "dspimage.h"
#include <cassert>
#include <stdio.h>


//-----------------------------------------------------------------------------
// Name: CVoiceCommunicator (ctor)
// Desc: Initializes member variables
//-----------------------------------------------------------------------------
CVoiceCommunicator::CVoiceCommunicator()
{
    m_pMicrophoneXMO    = NULL;
    m_pHeadphoneXMO     = NULL;
    m_pEncoderXMO       = NULL;
    m_pDecoderXMO       = NULL;
}




//-----------------------------------------------------------------------------
// Name: ~CVoiceCommunicator (dtor)
// Desc: Verifies that object was shut down properly
//-----------------------------------------------------------------------------
CVoiceCommunicator::~CVoiceCommunicator()
{
    assert( m_pMicrophoneXMO == NULL &&
            m_pHeadphoneXMO  == NULL &&
            m_pEncoderXMO    == NULL &&
            m_pDecoderXMO    == NULL );

    assert( m_pbMicrophoneBuffer  == NULL &&
            m_pbHeadphoneBuffer   == NULL );

    assert( m_adwMicrophonePacketStatus == NULL &&
            m_adwHeadphonePacketStatus == NULL );
}



//-----------------------------------------------------------------------------
// Name: Initialize
// Desc: Initializes the communicator object
//-----------------------------------------------------------------------------
HRESULT CVoiceCommunicator::Initialize( CVoiceManager* pManager )
{
    m_pManager = pManager;

    // Allocate our buffers and such
    m_pbMicrophoneBuffer = new BYTE[ m_pManager->GetPacketSize() * m_pManager->GetNumPackets() ];
    if( !m_pbMicrophoneBuffer )
        return E_OUTOFMEMORY;

    m_adwMicrophonePacketStatus = new DWORD[ m_pManager->GetNumPackets() ];
    if( !m_adwMicrophonePacketStatus )
        return E_OUTOFMEMORY;
    m_dwNextMicrophonePacket = 0;
    ZeroMemory( m_adwMicrophonePacketStatus, m_pManager->GetNumPackets() * sizeof( DWORD ) );

    m_pbHeadphoneBuffer = new BYTE[ m_pManager->GetPacketSize() * m_pManager->GetNumPackets() ];
    if( !m_pbHeadphoneBuffer )
        return E_OUTOFMEMORY;

    m_adwHeadphonePacketStatus = new DWORD[ m_pManager->GetNumPackets() ];
    if( !m_adwHeadphonePacketStatus )
        return E_OUTOFMEMORY;
    m_dwNextHeadphonePacket  = 0;
    ZeroMemory( m_adwHeadphonePacketStatus, m_pManager->GetNumPackets() * sizeof( DWORD ) );

    // Create the encoder
    if( FAILED( XVoiceCreateOneToOneEncoder( &m_pEncoderXMO ) ) )
        return E_OUTOFMEMORY;

    // Create a decoder to be used for loopback
    if( FAILED( XVoiceCreateOneToOneDecoder( &m_pDecoderXMO ) ) )
        return E_OUTOFMEMORY;

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: Shutdown
// Desc: Handles shutdown of the communicator object
//-----------------------------------------------------------------------------
HRESULT CVoiceCommunicator::Shutdown()
{
    OnRemoval();

    if( m_pbMicrophoneBuffer )
    {
        delete[] m_pbMicrophoneBuffer;
        m_pbMicrophoneBuffer = NULL;
    }

    if( m_adwMicrophonePacketStatus )
    {
        delete[] m_adwMicrophonePacketStatus;
        m_adwMicrophonePacketStatus = NULL;
    }

    if( m_pbHeadphoneBuffer )
    {
        delete[] m_pbHeadphoneBuffer;
        m_pbHeadphoneBuffer = NULL;
    }

    if( m_adwHeadphonePacketStatus )
    {
        delete[] m_adwHeadphonePacketStatus;
        m_adwHeadphonePacketStatus = NULL;
    }

    if( m_pEncoderXMO )
    {
        m_pEncoderXMO->Release();
        m_pEncoderXMO = NULL;
    }

    if( m_pDecoderXMO )
    {
        m_pDecoderXMO->Release();
        m_pDecoderXMO = NULL;
    }

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: ResetMicrophone
// Desc: Resets the voice communicator's microphone.  This should be called 
//          when the communicator is connected, as well as when we detect
//          a glitch in processing the microphone
//-----------------------------------------------------------------------------
HRESULT CVoiceCommunicator::ResetMicrophone()
{
    m_pManager->VoiceLog( L"Resetting microphone %d", m_lSlot );
    
    // To reset the microphone, we fill up all non-pending packets, up
    // until the first pending one
    XMEDIAPACKET xmp;
    while( m_adwMicrophonePacketStatus[ m_dwNextMicrophonePacket ] != XMEDIAPACKET_STATUS_PENDING )
    {
        GetMicrophonePacket( &xmp );
        if( FAILED( SubmitMicrophonePacket( &xmp ) ) )
            break;
    }
    
    // Flush the encoder to reset any state
    m_pEncoderXMO->Flush();

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: ResetHeadphone
// Desc: Resets the voice communicator's headphone.  This should be called 
//          when the communicator is connected, as well as when we detect
//          a glitch in processing the headphone
//-----------------------------------------------------------------------------
HRESULT CVoiceCommunicator::ResetHeadphone()
{
    m_pManager->VoiceLog( L"Resetting headphone %d", m_lSlot );
    
    // Reset our SRC pointer
    m_pManager->GetSRCInfo( (WORD)m_lSlot, NULL, NULL, &m_dwSRCReadPosition );

    // To reset the headphone, we fill up all non-pending packets, up
    // until the first pending one
    XMEDIAPACKET xmp;
    while( m_adwHeadphonePacketStatus[ m_dwNextHeadphonePacket ] != XMEDIAPACKET_STATUS_PENDING )
    {
        GetHeadphonePacket( &xmp );
        ZeroMemory( xmp.pvBuffer, m_pManager->GetPacketSize() );
        if( FAILED( SubmitHeadphonePacket( &xmp ) ) )
            break;
    }

    // Whenever the headphone is reset, we should reset m_dwRampTime,
    // so that we start the headphone off at full volume
    m_dwRampTime = 0;

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: OnInsertion
// Desc: Handles insertion of a voice communicator:
//       1) Create the microphone and headphone devices
//       2) Create XMEDIAPACKET queues for both the microphone and headphone
//       3) Create voice encoders and decoders
//       4) Handle updating voice web
//-----------------------------------------------------------------------------
HRESULT CVoiceCommunicator::OnInsertion( DWORD dwSlot )
{
    m_pManager->VoiceLog( L"Communicator inserted into slot %d", dwSlot );

    m_lSlot = LONG( dwSlot );

    // 1) Create the microphone and headphone devices
    WAVEFORMATEX wfx;
    wfx.cbSize = 0;
    wfx.nChannels = 1;
    wfx.nSamplesPerSec = 8000;
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = wfx.wBitsPerSample / 8 * wfx.nChannels;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    wfx.wFormatTag = WAVE_FORMAT_PCM;

    HRESULT hr;
    hr = XVoiceCreateMediaObject( XDEVICE_TYPE_VOICE_MICROPHONE, 
                                  m_lSlot,
                                  m_pManager->GetNumPackets(),
                                  &wfx,
                                  &m_pMicrophoneXMO );
    if( FAILED( hr ) )
        return E_FAIL;

    hr = XVoiceCreateMediaObject( XDEVICE_TYPE_VOICE_HEADPHONE, 
                                  m_lSlot,
                                  m_pManager->GetNumPackets(),
                                  &wfx,
                                  &m_pHeadphoneXMO );
    if( FAILED( hr ) )
        return E_FAIL;

    ResetMicrophone();
    ResetHeadphone();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: OnRemoval
// Desc: Handles removal of a voice communicator:
//       1) Notifies remote chatters that we're not here anymore
//       2) Frees all XMOs and buffers associated with the communicator
//-----------------------------------------------------------------------------
HRESULT CVoiceCommunicator::OnRemoval()
{
    m_pManager->VoiceLog( L"Communicator removed from slot %d", m_lSlot );

    if( m_pMicrophoneXMO )
    {
        m_pMicrophoneXMO->Release();
        m_pMicrophoneXMO = NULL;
    }
    if( m_pHeadphoneXMO )
    {
        m_pHeadphoneXMO->Release();
        m_pHeadphoneXMO = NULL;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: MicrophonePacketStatus
// Desc: Returns the status of the current microphone packet
//-----------------------------------------------------------------------------
DWORD CVoiceCommunicator::MicrophonePacketStatus()
{
    return m_adwMicrophonePacketStatus[ m_dwNextMicrophonePacket ];
}



//-----------------------------------------------------------------------------
// Name: GetMicrophonePacket
// Desc: Fills out an XMEDIAPACKET structure for the current microphone packet
//-----------------------------------------------------------------------------
HRESULT CVoiceCommunicator::GetMicrophonePacket( XMEDIAPACKET* pPacket )
{
    ZeroMemory( pPacket, sizeof( XMEDIAPACKET ) );
    pPacket->pvBuffer   = m_pbMicrophoneBuffer + m_dwNextMicrophonePacket * m_pManager->GetPacketSize();
    pPacket->dwMaxSize  = m_pManager->GetPacketSize();
    pPacket->pdwStatus  = &m_adwMicrophonePacketStatus[ m_dwNextMicrophonePacket ];

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: SubmitMicrophonePacket
// Desc: Submits the XMEDIAPACKET to the microphone XMO
//-----------------------------------------------------------------------------
HRESULT CVoiceCommunicator::SubmitMicrophonePacket( XMEDIAPACKET* pPacket )
{
    HRESULT hr;

    hr = m_pMicrophoneXMO->Process( NULL, pPacket );

    m_dwNextMicrophonePacket = ( m_dwNextMicrophonePacket + 1 ) % m_pManager->GetNumPackets();

    return hr;
}



//-----------------------------------------------------------------------------
// Name: HeadphonePacketStatus
// Desc: Returns the status of the current headphone packet
//-----------------------------------------------------------------------------
DWORD CVoiceCommunicator::HeadphonePacketStatus()
{
    return m_adwHeadphonePacketStatus[ m_dwNextHeadphonePacket ];
}



//-----------------------------------------------------------------------------
// Name: GetHeadphonePacket
// Desc: Fills out an XMEDIAPACKET structure for the current headphone packet
//-----------------------------------------------------------------------------
HRESULT CVoiceCommunicator::GetHeadphonePacket( XMEDIAPACKET* pPacket )
{
    ZeroMemory( pPacket, sizeof( XMEDIAPACKET ) );
    pPacket->pvBuffer   = m_pbHeadphoneBuffer + m_dwNextHeadphonePacket * m_pManager->GetPacketSize();
    pPacket->dwMaxSize  = m_pManager->GetPacketSize();
    pPacket->pdwStatus  = &m_adwHeadphonePacketStatus[ m_dwNextHeadphonePacket ];

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: SubmitHeadphonePacket
// Desc: Submits the XMEDIAPACKET to the headphone XMO
//-----------------------------------------------------------------------------
HRESULT CVoiceCommunicator::SubmitHeadphonePacket( XMEDIAPACKET* pPacket )
{
    HRESULT hr;

    hr = m_pHeadphoneXMO->Process( pPacket, NULL );

    m_dwNextHeadphonePacket = ( m_dwNextHeadphonePacket + 1 ) % m_pManager->GetNumPackets();

    return hr;
}



