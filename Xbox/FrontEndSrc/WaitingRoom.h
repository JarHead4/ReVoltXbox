//-----------------------------------------------------------------------------
// File: WaitingRoom.h
//
// Desc: 
//
// Hist: 02.08.02 - Port/reorg from origional revolt source
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


#pragma once

#define WAITING_ROOM_MESSAGE_LEN 28
#define WAITING_ROOM_MESSAGE_NUM 5

extern MENU Menu_WaitingRoom;

extern char WaitingRoomMessages[][WAITING_ROOM_MESSAGE_LEN + MAX_PLAYER_NAME + 2];
extern char WaitingRoomCurrentMessage[];
extern long WaitingRoomMessagePos;
extern long WaitingRoomMessageRGB[];
extern REAL WaitingRoomMessageTimer;
extern long WaitingRoomMessageActive;

