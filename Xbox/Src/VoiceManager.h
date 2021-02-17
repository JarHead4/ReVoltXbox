//-----------------------------------------------------------------------------
// File: VoiceManager.h
//
// Desc: Class and strucuture definitions related to VoiceCommunicator support
//
// Hist: 04.29.02 - New for June XDK
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#ifndef VOICEMANAGER_H
#define VOICEMANAGER_H

#include <xtl.h>
#include <xvoice.h>
#include <xonline.h>
#include "VoiceCommunicator.h"
#include <vector>


// Enumeration for communicator-related events
enum VOICE_COMMUNICATOR_EVENT
{
    VOICE_COMMUNICATOR_INSERTED,
    VOICE_COMMUNICATOR_REMOVED,
    VOICE_COMMUNICATOR_MICROPHONESTARVED,
};

// Callback function signature for communicator-related events
typedef VOID (*PFNCOMMUNICATORCALLBACK)( DWORD dwPort, VOICE_COMMUNICATOR_EVENT event, VOID* pContext );

// Callback function signature for voice data 
typedef VOID (*PFNVOICEDATACALLBACK)( DWORD dwPort, DWORD dwSize, VOID* pvData, VOID* pContext );

struct VOICE_MANAGER_CONFIG
{
    DWORD   dwVoiceSamplingRate;    // Sampling rate, in Hz
    DWORD   dwVoicePacketTime;      // Packet time, in ms
    DWORD   dwNumBuffers;           // # of buffers
    DWORD   dwMaxRemotePlayers;     // Maximum # of remote players
    DWORD   dwQueueResetThreshold;  // # of bad packets before resetting queue

    LPDIRECTSOUND8      pDSound;                // DirectSound object
    LPDSEFFECTIMAGEDESC pEffectImageDesc;       // DSP Effect Image Desc
    DWORD               dwFirstSRCEffectIndex;  // Effect index of first SRC effect in DSP

    // Will need callbacks for notifying of certain events
    VOID*                   pCallbackContext;
    PFNCOMMUNICATORCALLBACK pfnCommunicatorCallback;
    PFNVOICEDATACALLBACK    pfnVoiceDataCallback;
};

struct REMOTE_CHATTER
{
    XUID                xuid;
    DWORD               dwPacketsIn;
    DWORD               dwPacketsOut;
    DWORD               dwPacketsWithoutOutput;
    XMediaObject*       pVoiceQueueXMO;
    LPXVOICEDECODER     pDecoderXMO;
    LPDIRECTSOUNDSTREAM pOutputStream;
    BOOL                bHasOutput;
    BOOL                bIsTalking;

    BYTE*               pbStreamBuffer;
    DWORD*              adwStatus;
    DWORD               dwCurrentPacket;
    BOOL                bMuted[XGetPortCount()]; // cache of who's listening
};

typedef std::vector<XUID> MuteList;

class CXBFont;

class CVoiceManager
{
public:
    CVoiceManager();
    ~CVoiceManager();

    //////////////////////////////////////////////////////////////////////////
    // Routines for controlling overall voice state
    //////////////////////////////////////////////////////////////////////////
    HRESULT Initialize( VOICE_MANAGER_CONFIG* pConfig );
    HRESULT Shutdown();
    VOID    EnterChatSession();
    VOID    LeaveChatSession();
    BOOL    IsInChatSession() { return m_bIsInChatSession; }

    HRESULT ProcessVoice();
    HRESULT ReceivePacket( XUID xuidFromPlayer, VOID* pvData, INT nSize );
    HRESULT EnableCommunicator( DWORD dwControllerPort, BOOL bEnabled );
    HRESULT SetVoiceMask( DWORD dwControllerPort, XVOICE_MASK mask );
    HRESULT SetLoopback( DWORD dwControllerPort, BOOL bLoopback );

    //////////////////////////////////////////////////////////////////////////
    // Helper routines - only really needed by VoiceCommunicators
    //////////////////////////////////////////////////////////////////////////
    HRESULT GetSRCInfo( DWORD dwControllerPort, DWORD* pdwBufferSize, VOID** ppvBufferData, DWORD* pdwWritePosition );
    DWORD   GetSamplingRate() { return m_dwSamplingRate; }
    DWORD   GetNumPackets() { return m_dwNumBuffers; }
    DWORD   GetPacketSize() { return m_dwPacketSize; }
    const WAVEFORMATEX* GetWaveFormat() { return &m_wfx; }

    //////////////////////////////////////////////////////////////////////////
    // Functions for managing the chatter list
    //////////////////////////////////////////////////////////////////////////
    HRESULT AddChatter( XUID xuidPlayer );
    HRESULT RemoveChatter( XUID xuidPlayer );

    //////////////////////////////////////////////////////////////////////////
    // Functions for managing muting, locking out, etc.
    //////////////////////////////////////////////////////////////////////////
    HRESULT MutePlayer( XUID xuidPlayer, DWORD dwControllerPort );
    HRESULT UnMutePlayer( XUID xuidPlayer, DWORD dwControllerPort );
    BOOL    IsPlayerMuted( XUID xuidPlayer, DWORD dwControllerPort );

    HRESULT RemoteMutePlayer( XUID xuidPlayer, DWORD dwControllerPort );
    HRESULT UnRemoteMutePlayer( XUID xuidPlayer, DWORD dwControllerPort );
    BOOL    IsPlayerRemoteMuted( XUID xuidPlayer, DWORD dwControllerPort );

    //////////////////////////////////////////////////////////////////////////
    // Utility functions for game code
    //////////////////////////////////////////////////////////////////////////
    BOOL    IsCommunicatorInserted( DWORD dwControllerPort ) { return m_dwConnectedCommunicators & ( 1 << dwControllerPort ); }
    BOOL    DoesPlayerHaveVoice( XUID xuidPlayer );
    BOOL    IsPlayerTalking( XUID xuidPlayer );
    HRESULT FlushQueues() { m_bFlushQueuesOnNextProcess = TRUE; return S_OK; }

#if _DEBUG
    // Debug output will throw timing off, but we need to be able 
    // to watch what's going on...
    VOID    VoiceLog( WCHAR* format, ... );
    VOID    RenderDebugInfo( CXBFont* pFont );

    const static DWORD s_dwLogEntrySize = 256;
    const static DWORD s_dwNumLogEntries = 5;

    WCHAR   m_strDebugLog[s_dwNumLogEntries][256];
    DWORD   m_dwCurrentLogEntry;
#else
    VOID    VoiceLog( WCHAR* format, ... ) {}
    VOID    RenderDebugInfo( CXBFont* pFont ) {}
#endif

private:
    HRESULT OnCommunicatorInserted( DWORD dwControllerPort );
    HRESULT OnCommunicatorRemoved( DWORD dwControllerPort );
    HRESULT OnCommunicatorEvent( DWORD dwControllerPort, VOICE_COMMUNICATOR_EVENT event );
    HRESULT CheckDeviceChanges();
    HRESULT ProcessMicrophones();
    HRESULT ProcessQueues();
    HRESULT ProcessHeadphones();
    HRESULT BroadcastPacket( VOID* pvData, INT nSize, DWORD dwControllerPort );

    HRESULT ResetChatter( DWORD dwChatterIndex );
    HRESULT FlushQueuesInternal();
    HRESULT GetTemporaryPacket( XMEDIAPACKET* pPacket );
    HRESULT GetStreamPacket( XMEDIAPACKET* pPacket, DWORD dwChatterIndex );
    HRESULT SubmitStreamPacket( XMEDIAPACKET* pPacket, DWORD dwChatterIndex );
    
    DWORD   ChatterIndexFromXUID( XUID xuidPlayer);
    HRESULT ToggleListenToChatter( XUID xuidPlayer, BOOL bMute, DWORD dwPort );

    VOID*                   m_pCallbackContext;
    PFNCOMMUNICATORCALLBACK m_pfnCommunicatorCallback;
    PFNVOICEDATACALLBACK    m_pfnVoiceDataCallback;

    WAVEFORMATEX m_wfx;
    DWORD   m_dwSamplingRate;
    DWORD   m_dwPacketTime;
    DWORD   m_dwPacketSize;
    DWORD   m_dwBufferSize;
    DWORD   m_dwCompressedSize;
    DWORD   m_dwNumBuffers;
    DWORD   m_dwMaxChatters;
    DWORD   m_dwQueueResetThreshold;
    DWORD   m_dwFirstSRCEffectIndex;
    LPDSEFFECTIMAGEDESC m_pDSPImageDesc;

    REMOTE_CHATTER* m_pChatters;
    DWORD   m_dwNumChatters;
    BOOL    m_bIsInChatSession;

    DWORD   m_dwConnectedCommunicators;
    DWORD   m_dwMicrophoneState;
    DWORD   m_dwHeadphoneState;
    DWORD   m_dwLoopback;
    DWORD   m_dwEnabled;
    CVoiceCommunicator  m_aVoiceCommunicators[XGetPortCount()];

    BOOL    m_bFlushQueuesOnNextProcess;

    BYTE*   m_pbTempEncodedPacket;

    MuteList    m_MuteList[XGetPortCount()];
    MuteList    m_RemoteMuteList[XGetPortCount()];

#if _DEBUG
    HRESULT ValidateStateDbg();
#endif // _DEBUG
};

extern CVoiceManager g_VoiceManager;

#endif // VOICEMANAGER_H