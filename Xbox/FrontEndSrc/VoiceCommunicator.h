//-----------------------------------------------------------------------------
// File: VoiceCommunicator.h
//
// Desc: Class and strucuture definitions related to VoiceCommunicator support
//
// Hist: 1.17.02 - Created
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#ifndef VOICECOMMUNICATOR_H
#define VOICECOMMUNICATOR_H

#include <xtl.h>
#include <xvoice.h>

//////////////////////////////////////////////////////////////////////////////
// Hack! DbgPrint is much nicer than OutputDebugString so JonT exported it from xapi.lib
// (as DebugPrint) but the prototype isn't in the public headers...
extern "C"
{
    ULONG
    DebugPrint(
        PCHAR Format,
        ...
        );
}

const LONG INACTIVE_SLOT = -1;
const DWORD NUM_VOICE_PACKETS = 4;
const DWORD VOICE_PACKET_SIZE = 20 * 8000 * 2 / 1000;

class CVoiceCommunicator
{
public:
    CVoiceCommunicator();
    ~CVoiceCommunicator();

    HRESULT OnInsertion( DWORD dwSlot );
    HRESULT OnRemoval();

    HRESULT BroadcastPacket( VOID* pvData, INT nSize );
    HRESULT ReceivePacket( VOID* pvData, INT nSize );

    static HRESULT InitCommunicators();
    static HRESULT ProcessVoice();

private:
    LONG            m_lSlot;
    XMediaObject*   m_pMicrophoneXMO;
    XMediaObject*   m_pHeadphoneXMO;
    XMediaObject*   m_pEncoderXMO;
    XVoiceQueueMediaObject* m_pVoiceQueueXMO;
    
    DWORD           m_adwMicrophonePacketStatus[NUM_VOICE_PACKETS];
    DWORD           m_dwNextMicrophonePacket;
    BYTE*           m_pbMicrophoneBuffer;

    DWORD           m_adwHeadphonePacketStatus[NUM_VOICE_PACKETS];
    DWORD           m_dwNextHeadphonePacket;
    BYTE*           m_pbHeadphoneBuffer;
};

extern CVoiceCommunicator g_VoiceCommunicators[4];

#endif // VOICECOMMUNICATOR_H