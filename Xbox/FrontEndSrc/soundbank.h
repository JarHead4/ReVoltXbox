//-----------------------------------------------------------------------------
// File: SoundBank.h
//
// Desc: Header file containing class, structure, and constant definitions
//       for the CSoundBank class
//
// Hist: 12.06.01 - Created
//
// TODO: * Need to think about different buffer pools - if we end up w/
//          several different pools, will need to manage them more cleanly.
//          Ideally, I'd rather just have one pool.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef SOUNDBANK_H
#define SOUNDBANK_H

#include <xtl.h>
#include <dsound.h>
#include "wavbndlr.h"
#include "sounds.h"



#define BUFFERFLAGS_2D          0x000000001     // 2D buffer
#define BUFFERFLAGS_3D          0x000000002     // 3D buffer
#define BUFFERFLAGS_NOMIX       0x000000004     // Source buffer
#define BUFFERFLAGS_MIXIN       0x000000008     // MixIn dest buffer
#define BUFFERFLAGS_FXIN        0x000000010     // FXIn dest buffer

//-----------------------------------------------------------------------------
// Name: struct BufferListNode
// Desc: This structure represents one node in a list of buffers, used for our
//       free/busy buffer lists
//-----------------------------------------------------------------------------
struct BufferListNode
{
    BufferListNode*     pNext;
    LPDIRECTSOUNDBUFFER pBuffer;
    DWORD               dwFlags;
};




//-----------------------------------------------------------------------------
// Name: class CSoundBank
// Desc: The CSoundBank class manages all sound effects in the game
//-----------------------------------------------------------------------------
class CSoundBank
{
public:
    CSoundBank();
    ~CSoundBank();

    HRESULT Initialize();
    HRESULT LoadSoundBank( LPCSTR strFilename );            // Load a sound bank
    HRESULT FreeSoundBank();                                // Frees the loaded bank
    HRESULT InitializeBufferPool( DWORD dw2DBuffers,        // Set up buffer pool
                                  DWORD dw3DBuffers );
    HRESULT FreeBufferPool();

    HRESULT PlaySound( DWORD dwIndex,                       // Play a sound
                       BOOL bLooping,
                       BOOL b3D,
                       LPDIRECTSOUNDBUFFER pBuffer = NULL );
    HRESULT GetBuffer( BOOL b3D,                            // Grab a buffer
                       LPDIRECTSOUNDBUFFER* ppBuffer );
    HRESULT ReturnBuffer( LPDIRECTSOUNDBUFFER pBuffer );    // Return a buffer to pool

    HRESULT PauseAll();                                     // Pauses all sounds
    HRESULT ResumeAll();                                    // Resumes all sounds
    HRESULT StopAll();                                      // Stops all sounds
    HRESULT ScrubBusyList();                                // Scrubs the busy list

    LPDIRECTSOUND GetDSound() { return m_pDSound; }

protected:
    BufferListNode* GetNodeFromList( BufferListNode** ppList );
    VOID            AddNodeToList( BufferListNode** ppList, BufferListNode* pNode );

    LPDIRECTSOUND       m_pDSound;                          // DirectSound object
    WAVEBANKENTRY*      m_pSoundBankEntries;                // Sound entries
    DWORD               m_dwNumSounds;                      // # of sounds in bank
    BYTE*               m_pbSampleData;                     // Pointer to sample data
    DWORD               m_dwSampleLength;                   // Length of sample data

    DWORD               m_dwNumNodes;                       // Number of nodes allocated
    BufferListNode*     m_pAllNodes;                        // Pointer to node allocation
    BufferListNode*     m_pFree2DBufferList;                // Free 2D buffers
    BufferListNode*     m_pFree3DBufferList;                // Free 3D buffers
    BufferListNode*     m_pBusyBufferList;                  // Busy buffers
    BufferListNode*     m_pReservedBufferList;

#if _DEBUG
    HRESULT ValidateStateDbg();                             // Validates everything is OK

    // Buffers given out through GetBuffer
    DWORD               m_dwNum2DBuffersDbg;
    DWORD               m_dwNum3DBuffersDbg;
#endif // _DEBUG
};

extern CSoundBank g_SoundBank;
#endif // SOUNDBANK_H