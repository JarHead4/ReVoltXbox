//-----------------------------------------------------------------------------
// File: path.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef PATH_H
#define PATH_H

#define ANIM_FPS            30

// Position Key
typedef struct KeyPosStruct
{
    long        iFrame;     // Frame #
    VEC         vPos;       // Position
} KEY_POS;

// Rotation Key
typedef struct KeyQuatStruct
{
    long        iFrame;     // Frame #
    QUATERNION  qQuat;      // Quaternion
} KEY_QUAT;

// Scale Key
typedef struct KeyScaleStruct
{
    long        iFrame;     // Frame #
    VEC         vScale;     // Scale
} KEY_SCALE;


// Animation info
typedef struct AnimationDataStruct
{
    long        cFrames;        // Total number of frames in animation

    long        cPosKeys;       // Total number of position keys
    KEY_POS*    pPosKeyHead;    // Head of position key frames

    long        cQuatKeys;      // Total number of rotation keys
    KEY_QUAT*   pQuatKeyHead;   // Head of rotation key frames

    long        cScaleKeys;     // Total number of scale keys
    KEY_SCALE*  pScaleKeyHead;  // Head of scale key frames

} ANIMATION_DATA;



struct object_def;
extern void SetObjectAnimation(struct object_def *obj, ANIMATION_DATA *anim);
extern ANIMATION_DATA *CreateAnimation(long nFrames, long nPosKeys, long nQuatKeys, long nScaleKeys);
extern void DestroyAnimationData(ANIMATION_DATA *data);
extern ANIMATION_DATA *LoadAnimationData(FILE *fp);
extern void MoveAndScaleAnimationPath(ANIMATION_DATA *animData, VEC *shift, REAL scale);
extern void DrawAnimationPath(ANIMATION_DATA *animData);
extern void TransformAnimation(ANIMATION_DATA *animData);

extern void FollowPath(struct object_def *obj);


#endif // PATH_H

