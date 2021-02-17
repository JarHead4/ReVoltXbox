//-----------------------------------------------------------------------------
// File: ObjInfo.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "obj_init.h"
#include "car.h"

// file object model list

char *FileObjectModelList[] = {
    "D:\\models\\barrel.m",
    "D:\\models\\beachball.m",
    "D:\\models\\mercury.m",
    "D:\\models\\venus.m",
    "D:\\models\\earth.m",
    "D:\\models\\mars.m",
    "D:\\models\\jupiter.m",
    "D:\\models\\saturn.m",
    "D:\\models\\uranus.m",
    "D:\\models\\neptune.m",
    "D:\\models\\pluto.m",
    "D:\\models\\moon.m",
    "D:\\models\\rings.m",
    "D:\\models\\plane.m",
    "D:\\models\\copter.m",
    "D:\\models\\dragon1.m",
    "D:\\models\\water.m",
    "D:\\cars\\trolley\\trollbod.m",
    "D:\\models\\boat1.m",
    "D:\\models\\speedup.m",
    "D:\\models\\radar.m",
    "D:\\models\\balloon.m",
    "D:\\models\\horse.m",
    "D:\\models\\train.m",
    "D:\\models\\light1.m",
    "D:\\models\\light2.m",
    "D:\\models\\light3.m",
    "D:\\models\\football.m",   // Football
    "D:\\edit\\spot.m",         // Spark generator - no model
    "D:\\models\\spaceman.m",
    "D:\\models\\pickup.m",
    "D:\\models\\flap.m",
    "D:\\edit\\spot.m",
    "D:\\models\\cone.m",
    "D:\\models\\Probe.m",
    "D:\\levels\\FrontEnd\\namestand.m",
    "D:\\models\\spbase.m",
    "D:\\models\\sphose.m",
    "D:\\edit\\spot.m",
    "D:\\models\\basketball.m",
    "D:\\levels\\frontend\\bigscreen.m",
    "D:\\levels\\frontend\\clockbody.m",
    "D:\\levels\\frontend\\CarBox.m",
    "D:\\models\\stream.m",
    "D:\\levels\\frontend\\cup04.m",
    "D:\\levels\\frontend\\cup02.m",
    "D:\\levels\\frontend\\cup01.m",
    "D:\\levels\\frontend\\cup03.m",
    "D:\\models\\star.m",
    "D:\\models\\tumble_weed.m",
    "D:\\levels\\frontend\\smallscreen.m",
    "D:\\models\\lantern.m",
    "D:\\models\\slider.m",
    "D:\\models\\bottle.m",
    "D:\\models\\poolwater.m",
    "D:\\models\\gardenwater1.m",
    "D:\\models\\gardenwater2.m",
    "D:\\models\\gardenwater3.m",
    "D:\\models\\gardenwater4.m",
    "D:\\models\\bucket.m",
    "D:\\models\\trafficcone.m",
    "D:\\models\\lantern.m",//"models\\can.m",
    "D:\\models\\lantern.m",//"models\\lilo.m",
    "D:\\models\\shiplight.m",
    "D:\\models\\packet.m",
    "D:\\models\\abcblock.m",
    "D:\\models\\lantern.m",
    NULL
};

// type lists

static char *TypeYesNo[] = {
    "No",
    "Yes",
};

static char *TypeOnOff[] = {
    "Off",
    "On",
};

static char *TypeAxis[] = {
    "X",
    "Y",
    "Z",
};

static char *TypePlanet[] = {
    "Mercury",
    "Venus",
    "Earth",
    "Mars",
    "Jupiter",
    "Saturn",
    "Uranus",
    "Neptune",
    "Pluto",
    "Moon",
    "Rings",
    "Sun",
};

static char *TypeBoat[] = {
    "Sail",
    "Tug",
};

static char *TypeStrobe[] = {
    "Muse Post",
    "Muse Wall",
    "Hood Tunnel",
};

static char *TypeSparkGen[] = {
    "SPARK_SPARK",
    "SPARK_SPARK2",
    "SPARK_SNOW",
    "SPARK_POPCORN",
    "SPARK_GRAVEL",
    "SPARK_SAND",
    "SPARK_GRASS",
    "SPARK_ELECTRIC",
    "SPARK_WATER",
    "SPARK_DIRT",
    "SPARK_SMOKE1",
    "SPARK_SMOKE2",
    "SPARK_SMOKE3",
    "SPARK_BLUE",
    "SPARK_BIGBLUE",
    "SPARK_SMALLORANGE",
    "SPARK_SMALLRED",
    "SPARK_EXPLOSION1",
    "SPARK_EXPLOSION2",
    "SPARK_STAR",
};

static char *TypeCupType[] = {
    "BRONZE",
    "SILVER",
    "GOLD",
    "SPECIAL",
};

static char *TypeWav[] = {
    "Hood Birds 1",
    "Hood Dog Bark",
    "Hood Kids",
    "Hood TV",
    "Hood Lawnmower",
    "Hood Digger",
    "Hood Birds 2",
    "Hood Birds 3",
    "Toy Arcade",
    "Hood Stream",
    "Garden tropics2",
    "Garden tropics3",
    "Garden tropics4",
    "Muse ambience",
    "Market air cond",
    "Market cabinet hum",
    "Market car park",
    "Market freezer",
    "Market icy",
    "Muse escalator",
    "Muse barrel",
    "Muse door",
    "Garden stream",
    "Ghost coyote",
    "Ghost bats",
    "Ghost eagle",
    "Ghost drip",
    "Ghost rattler",
    "Ship internal ambience",
    "Ship seagulls",
    "Ship foghorn",
    "Ship thunder",
    "Ship storm",
    "Ship calm",
    "Garden animal wierd",
    "Garden animal bird1",
    "Garden animal bird2",
    "Garden animal frog",
    "Hood ambience",
    "Ghost bell",
};

static char *TypeStar[] = {
    "Global weapon",
    "Practice star",
};

static char *TypeSkyboxLevel[] = {
    "Toytanic day ",
    "Toytanic night",
    "Wild West",
    "Neighborhood",
};

static char *TypeRipple[] = {
    "Hood Stream",
    "Toytanic Pool",
    "Garden 1",
    "Garden 2",
    "Garden 3",
    "Garden 4",
};

// file object info

FILE_OBJECT_INFO FileObjectInfo[] = {

// barrel

    {
        0,                                              // model ID
        "Spinning Barrel",                              // name
        "Speed", NULL, -255, 255,                       // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// Beachball

    {
        1,                                              // model ID
        "Beachball",                                    // name
        NULL, NULL, 0, 0,                               // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// planet

    {
        2,                                              // model ID
        "Planet",                                       // name
        "Name", TypePlanet, 0, 11,                      // flag 1
        "Orbit", TypePlanet, 0, 11,                     // flag 2
        "Orbit Speed", NULL, -255, 255,                 // flag 3
        "Spin speed", NULL, -255, 255,                  // flag 4
    },

// plane

    {
        13,                                             // model ID
        "Plane",                                        // name
        "Speed", NULL, -256, 256,                       // flag 1
        "Radius", NULL, 0, 1024,                        // flag 2
        "Bank", NULL, -256, 256,                        // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// copter

    {
        14,                                             // model ID
        "Copter",                                       // name
        "X range", NULL, 0, 256,                        // flag 1
        "Y range", NULL, 0, 256,                        // flag 1
        "Z range", NULL, 0, 256,                        // flag 1
        "Y offset", NULL, -256, 256,                    // flag 4
    },

// dragon

    {
        15,                                             // model ID
        "Dragon",                                       // name
        NULL, NULL, 0, 0,                               // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// water

    {
        16,                                             // model ID
        "Water",                                        // name
        NULL, NULL, 0, 0,                               // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// Trolley

    {
        17,                                             // model ID
        "Trolley",                                      // name
        NULL, NULL, 0, 0,                               // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// boat

    {
        18,                                             // model ID
        "Boat",                                         // name
        "Type", TypeBoat, 0, 1,                         // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// speedup

    {
        19,                                             // model ID
        "Speedup",                                      // name
        "Width        ", NULL, 10, 100,                 // flag 2
        "LoSpeed (mph)", NULL, 0, 100,                  // flag 1
        "HiSpeed (mph)", NULL, 0, 100,                  // flag 1
        "Time (s)     ", NULL, 0, 50,                   // flag 4
    },

// radar

    {
        20,                                             // model ID
        "Radar",                                        // name
        NULL, NULL, 0, 0,                               // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// balloon

    {
        21,                                             // model ID
        "Balloon",                                      // name
        NULL, NULL, 0, 0,                               // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// horse

    {
        22,                                             // model ID
        "Horse",                                        // name
        NULL, NULL, 0, 0,                               // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// train

    {
        23,                                             // model ID
        "Train",                                        // name
        NULL, NULL, 0, 0,                               // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// strobe

    {
        24,                                             // model ID
        "Strobe",                                       // name
        "Type", TypeStrobe, 0, 2,                       // flag 1
        "Sequence Num", NULL, 0, 500,                   // flag 2
        "Sequence Count", NULL, 0, 500,                 // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// Football

    {
        27,                                             // model ID
        "Football",                                     // name
        NULL, NULL, 0, 0,                               // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// Spark Generator

    {
        28,                                             // model ID
        "Spark Generator",                              // name
        "Type", TypeSparkGen, 0, 19,                    // flag 1
        "Av. Speed", NULL, 0, 200,                      // flag 2
        "Var. Speed", NULL, 0, 200,                     // flag 3
        "Frequency", NULL, 1, 200,                      // flag 4
    },

// space man

    {
        29,                                             // model ID
        "Space Man",                                    // name
        NULL, NULL, 0, 0,                               // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// shockwave

    {
        -2,                                             // model ID
    },

// firework

    {
        -2,                                             // model ID
    },

// putty bomb

    {
        -2,                                             // model ID
    },

// water bomb

    {
        -2,                                             // model ID
    },

// electro pulse

    {
        -2,                                             // model ID
    },

// oil slick

    {
        -2,                                             // model ID
    },

// oil slick dropper

    {
        -2,                                             // model ID
    },

// chrome ball

    {
        -2,                                             // model ID
    },

// clone

    {
        -2,                                             // model ID
    },

// turbo

    {
        -2,                                             // model ID
    },

// electro zapped

    {
        -2,                                             // model ID
    },

// spring

    {
        -2,                                             // model ID
    },

// pickup generator

    {
        30,                                             // model ID
        "Pickup",                                       // name
        NULL, NULL, 0, 0,                               // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// dissolve model

    {
        -2,                                             // model ID
    },

// flap

    {
        31,                                             // model ID
        "Flappage",                                     // name
        NULL, NULL, 0, 0,                               // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// laser

    {
        32,                                             // model ID
        "Laser",                                        // name
        "Width", NULL, 1, 10,                           // flag 1
        "Rand", NULL, 1, 10,                            // flag 2
        "Object", TypeYesNo, 0, 1,                      // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// splash

    {
        -2,                                             // model ID
    },

// bomb glow

    {
        -2,                                             // model ID
    },

// wobbly cone weebel thingy

    {
        33,                                             // model ID
        "Wobbly Cone",                                  // name
        NULL, NULL, 0, 0,                               // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// Probe logo

    {
        34,                                             // model ID
        "Probe Logo",                                   // name
        NULL, NULL, 0, 0,                               // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// clouds

    {
        38,                                             // model ID
        "Clouds",                                       // name
        NULL, NULL, 0, 0,                               // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// name entry wheel and stand

    {
        35,                                             // model ID
        "Name Entry",                                   // name
        NULL, NULL, 0, 0,                               // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// sprinkler

    {
        36,                                             // model ID
        "Sprinkler",                                    // name
        "ID", NULL, 0, 255,                             // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// sprinkler hose

    {
        37,                                             // model ID
        "Sprinkler Hose",                               // name
        "ID", NULL, 0, 255,                             // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// object thrower

    {
        38,                                             // model ID
        "Object Thrower",                               // name
        "ID", NULL, 0, 255,                             // flag 1
        "Object", NULL, 0, OBJECT_TYPE_MAX - 1,         // flag 2
        "Speed", NULL, -1, 100,                         // flag 3
        "ReUse", TypeYesNo, 0, 1,                       // flag 4
    },

// basketball

    {
        39,                                             // model ID
        "BasketBall",                                   // name
        NULL, NULL, 0, 0,                               // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// Track Screen

    {
        40,                                             // model ID
        "TrackScreen",                                  // name
        NULL, NULL, 0, 0,                               // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// Clock

    {
        41,                                             // model ID
        "Clock",                                        // name
        NULL, NULL, 0, 0,                               // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// CarBox

    {
        42,                                             // model ID
        "CarBox",                                       // name
        "Car ID", NULL, -1, CARID_NTYPES-1,             // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// stream

    {
        43,                                             // model ID
        "Stream",                                       // name
        "Model", NULL, 0, 5,                            // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// cup

    {
        44, //also 45,46,47                             // model ID
        "Cup",                                          // name
        "Type", TypeCupType, 0, 3,                      // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// 3D sound

    {
        38,                                             // model ID
        "3D Sound",                                     // name
        "Name", TypeWav, 0, 39,                         // flag 1
        "Range", NULL, 0, 100,                          // flag 2
        "Random Play", TypeYesNo, 0, 1,                 // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// star pickup

    {
        48,                                             // model ID
        "Star",                                         // name
        "Type", TypeStar, 0, 1,                         // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// fox

    {
        -2,                                             // model ID
    },

// tumbleweed

    {
        49,                                             // model ID
        "Tumbleweed",                                   // name
        NULL, NULL, 0, 0,                               // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// small screen

    {
        50,                                             // model ID
        "Small Screen",                                 // name
        NULL, NULL, 0, 0,                               // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// lantern

    {
        51,                                             // model ID
        "Lantern",                                      // name
        NULL, NULL, 0, 0,                               // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// skybox

    {
        38,                                             // model ID
        "Skybox",                                       // name
        "Level", TypeSkyboxLevel, 0, 3,                 // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// sliding doors

    {
        52,                                             // model ID
        "Slider",                                       // name
        "ID", NULL, 0, 1,                               // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// bottle

    {
        53,                                             // model ID
        "Bottle",                                       // name
        "Stop", TypeYesNo, 0, 1,                        // flag 3
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// bucket

    {
        59,                                             // model ID
        "Bucket",                                       // name
        NULL, NULL, 0, 0,                               // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// cone

    {
        60,                                             // model ID
        "Cone",                                     // name
        NULL, NULL, 0, 0,                               // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// can

    {
        61,                                             // model ID
        "Can",                                      // name
        NULL, NULL, 0, 0,                               // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// lilo

    {
        62,                                             // model ID
        "Lilo",                                     // name
        NULL, NULL, 0, 0,                               // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// global

    {
        -2,                                             // model ID
    },

// rain

    {
        38,                                             // model ID
        "Rain",                                         // name
        NULL, NULL, 0, 0,                               // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// lightning

    {
        38,                                             // model ID
        "Lightning",                                    // name
        NULL, NULL, 0, 0,                               // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// ship light

    {
        63,                                             // model ID
        "Ship light",                                   // name
        NULL, NULL, 0, 0,                               // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// packet

    {
        64,                                             // model ID
        "Packet",                                       // name
        NULL, NULL, 0, 0,                               // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// abc

    {
        65,                                             // model ID
        "ABC Block",                                    // name
        NULL, NULL, 0, 0,                               // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// water box

    {
        66,                                             // model ID
        "Water Box",                                    // name
        "X range", NULL, 0, 10000,                      // flag 1
        "Y range", NULL, 0, 10000,                      // flag 2
        "Z range", NULL, 0, 10000,                      // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// ripple

    {
        66,                                             // model ID
        "Water Ripples",                                // name
        "Model", TypeRipple, 0, 5,                      // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// flag

    {
        38,                                             // model ID
        "Gari f(l)ag",                                  // name
        "Garyness", NULL, 0, 10,                            // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// dolphin

    {
        38,                                             // model ID
        "Dolphin",                                      // name
        NULL, NULL, 0, 0,                               // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// garden fog

    {
        38,                                             // model ID
        "Garden Fog",                                   // name
        NULL, NULL, 0, 0,                               // flag 1
        NULL, NULL, 0, 0,                               // flag 2
        NULL, NULL, 0, 0,                               // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// fog box

    {
        66,                                             // model ID
        "Fog Box",                                      // name
        "X range", NULL, 0, 20000,                      // flag 1
        "Y range", NULL, 0, 20000,                      // flag 2
        "Z range", NULL, 0, 20000,                      // flag 3
        NULL, NULL, 0, 0,                               // flag 4
    },

// end of list

    {
        -1
    }

};

