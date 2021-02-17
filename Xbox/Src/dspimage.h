
#pragma once

typedef enum _DSP_IMAGE_DSPImage_FX_INDICES {
    Graph0_ZeroTempMixbins0 = 0,
    Graph1_SRCforheadphone1 = 1,
    Graph2_SRCforheadphone2 = 2,
    Graph3_SRCforheadphone3 = 3,
    Graph4_SRCforheadphone4 = 4,
    Graph5_I3DL2Reverb24K = 5,
    Graph5_XTalk = 6,
    Graph5_XTalk2LFE_A = 7,
    Graph5_XTalk2LFE_B = 8
} DSP_IMAGE_DSPImage_FX_INDICES;

typedef struct _Graph0_FX0_ZeroTempMixbins0_STATE {
    DWORD dwScratchOffset;        // Offset in bytes, of scratch area for this FX
    DWORD dwScratchLength;        // Length in DWORDS, of scratch area for this FX
    DWORD dwYMemoryOffset;        // Offset in DSP WORDS, of Y memory area for this FX
    DWORD dwYMemoryLength;        // Length in DSP WORDS, of Y memory area for this FX
    DWORD dwFlags;                // FX bitfield for various flags. See xgpimage documentation
    DWORD dwInMixbinPtrs[1];      // XRAM offsets in DSP WORDS, of input mixbins
    DWORD dwOutMixbinPtrs[8];     // XRAM offsets in DSP WORDS, of output mixbins
} Graph0_FX0_ZeroTempMixbins0_STATE, *LPGraph0_FX0_ZeroTempMixbins0_STATE;

typedef const Graph0_FX0_ZeroTempMixbins0_STATE *LPCGraph0_FX0_ZeroTempMixbins0_STATE;

typedef struct _Graph1_FX0_SRCforheadphone1_STATE {
    DWORD dwScratchOffset;        // Offset in bytes, of scratch area for this FX
    DWORD dwScratchLength;        // Length in DWORDS, of scratch area for this FX
    DWORD dwYMemoryOffset;        // Offset in DSP WORDS, of Y memory area for this FX
    DWORD dwYMemoryLength;        // Length in DSP WORDS, of Y memory area for this FX
    DWORD dwFlags;                // FX bitfield for various flags. See xgpimage documentation
    DWORD dwInMixbinPtrs[1];      // XRAM offsets in DSP WORDS, of input mixbins
    DWORD dwOutMixbinPtrs[1];     // XRAM offsets in DSP WORDS, of output mixbins
} Graph1_FX0_SRCforheadphone1_STATE, *LPGraph1_FX0_SRCforheadphone1_STATE;

typedef const Graph1_FX0_SRCforheadphone1_STATE *LPCGraph1_FX0_SRCforheadphone1_STATE;

typedef struct _Graph2_FX0_SRCforheadphone2_STATE {
    DWORD dwScratchOffset;        // Offset in bytes, of scratch area for this FX
    DWORD dwScratchLength;        // Length in DWORDS, of scratch area for this FX
    DWORD dwYMemoryOffset;        // Offset in DSP WORDS, of Y memory area for this FX
    DWORD dwYMemoryLength;        // Length in DSP WORDS, of Y memory area for this FX
    DWORD dwFlags;                // FX bitfield for various flags. See xgpimage documentation
    DWORD dwInMixbinPtrs[1];      // XRAM offsets in DSP WORDS, of input mixbins
    DWORD dwOutMixbinPtrs[1];     // XRAM offsets in DSP WORDS, of output mixbins
} Graph2_FX0_SRCforheadphone2_STATE, *LPGraph2_FX0_SRCforheadphone2_STATE;

typedef const Graph2_FX0_SRCforheadphone2_STATE *LPCGraph2_FX0_SRCforheadphone2_STATE;

typedef struct _Graph3_FX0_SRCforheadphone3_STATE {
    DWORD dwScratchOffset;        // Offset in bytes, of scratch area for this FX
    DWORD dwScratchLength;        // Length in DWORDS, of scratch area for this FX
    DWORD dwYMemoryOffset;        // Offset in DSP WORDS, of Y memory area for this FX
    DWORD dwYMemoryLength;        // Length in DSP WORDS, of Y memory area for this FX
    DWORD dwFlags;                // FX bitfield for various flags. See xgpimage documentation
    DWORD dwInMixbinPtrs[1];      // XRAM offsets in DSP WORDS, of input mixbins
    DWORD dwOutMixbinPtrs[1];     // XRAM offsets in DSP WORDS, of output mixbins
} Graph3_FX0_SRCforheadphone3_STATE, *LPGraph3_FX0_SRCforheadphone3_STATE;

typedef const Graph3_FX0_SRCforheadphone3_STATE *LPCGraph3_FX0_SRCforheadphone3_STATE;

typedef struct _Graph4_FX0_SRCforheadphone4_STATE {
    DWORD dwScratchOffset;        // Offset in bytes, of scratch area for this FX
    DWORD dwScratchLength;        // Length in DWORDS, of scratch area for this FX
    DWORD dwYMemoryOffset;        // Offset in DSP WORDS, of Y memory area for this FX
    DWORD dwYMemoryLength;        // Length in DSP WORDS, of Y memory area for this FX
    DWORD dwFlags;                // FX bitfield for various flags. See xgpimage documentation
    DWORD dwInMixbinPtrs[1];      // XRAM offsets in DSP WORDS, of input mixbins
    DWORD dwOutMixbinPtrs[1];     // XRAM offsets in DSP WORDS, of output mixbins
} Graph4_FX0_SRCforheadphone4_STATE, *LPGraph4_FX0_SRCforheadphone4_STATE;

typedef const Graph4_FX0_SRCforheadphone4_STATE *LPCGraph4_FX0_SRCforheadphone4_STATE;

typedef struct _Graph5_FX0_I3DL2Reverb24K_STATE {
    DWORD dwScratchOffset;        // Offset in bytes, of scratch area for this FX
    DWORD dwScratchLength;        // Length in DWORDS, of scratch area for this FX
    DWORD dwYMemoryOffset;        // Offset in DSP WORDS, of Y memory area for this FX
    DWORD dwYMemoryLength;        // Length in DSP WORDS, of Y memory area for this FX
    DWORD dwFlags;                // FX bitfield for various flags. See xgpimage documentation
    DWORD dwInMixbinPtrs[2];      // XRAM offsets in DSP WORDS, of input mixbins
    DWORD dwOutMixbinPtrs[35];     // XRAM offsets in DSP WORDS, of output mixbins
} Graph5_FX0_I3DL2Reverb24K_STATE, *LPGraph5_FX0_I3DL2Reverb24K_STATE;

typedef const Graph5_FX0_I3DL2Reverb24K_STATE *LPCGraph5_FX0_I3DL2Reverb24K_STATE;

typedef struct _Graph5_FX1_XTalk_STATE {
    DWORD dwScratchOffset;        // Offset in bytes, of scratch area for this FX
    DWORD dwScratchLength;        // Length in DWORDS, of scratch area for this FX
    DWORD dwYMemoryOffset;        // Offset in DSP WORDS, of Y memory area for this FX
    DWORD dwYMemoryLength;        // Length in DSP WORDS, of Y memory area for this FX
    DWORD dwFlags;                // FX bitfield for various flags. See xgpimage documentation
    DWORD dwInMixbinPtrs[4];      // XRAM offsets in DSP WORDS, of input mixbins
    DWORD dwOutMixbinPtrs[4];     // XRAM offsets in DSP WORDS, of output mixbins
} Graph5_FX1_XTalk_STATE, *LPGraph5_FX1_XTalk_STATE;

typedef const Graph5_FX1_XTalk_STATE *LPCGraph5_FX1_XTalk_STATE;

typedef struct _Graph5_FX2_XTalk2LFE_A_STATE {
    DWORD dwScratchOffset;        // Offset in bytes, of scratch area for this FX
    DWORD dwScratchLength;        // Length in DWORDS, of scratch area for this FX
    DWORD dwYMemoryOffset;        // Offset in DSP WORDS, of Y memory area for this FX
    DWORD dwYMemoryLength;        // Length in DSP WORDS, of Y memory area for this FX
    DWORD dwFlags;                // FX bitfield for various flags. See xgpimage documentation
    DWORD dwInMixbinPtrs[2];      // XRAM offsets in DSP WORDS, of input mixbins
    DWORD dwOutMixbinPtrs[1];     // XRAM offsets in DSP WORDS, of output mixbins
} Graph5_FX2_XTalk2LFE_A_STATE, *LPGraph5_FX2_XTalk2LFE_A_STATE;

typedef const Graph5_FX2_XTalk2LFE_A_STATE *LPCGraph5_FX2_XTalk2LFE_A_STATE;

typedef struct _Graph5_FX3_XTalk2LFE_B_STATE {
    DWORD dwScratchOffset;        // Offset in bytes, of scratch area for this FX
    DWORD dwScratchLength;        // Length in DWORDS, of scratch area for this FX
    DWORD dwYMemoryOffset;        // Offset in DSP WORDS, of Y memory area for this FX
    DWORD dwYMemoryLength;        // Length in DSP WORDS, of Y memory area for this FX
    DWORD dwFlags;                // FX bitfield for various flags. See xgpimage documentation
    DWORD dwInMixbinPtrs[2];      // XRAM offsets in DSP WORDS, of input mixbins
    DWORD dwOutMixbinPtrs[1];     // XRAM offsets in DSP WORDS, of output mixbins
} Graph5_FX3_XTalk2LFE_B_STATE, *LPGraph5_FX3_XTalk2LFE_B_STATE;

typedef const Graph5_FX3_XTalk2LFE_B_STATE *LPCGraph5_FX3_XTalk2LFE_B_STATE;
