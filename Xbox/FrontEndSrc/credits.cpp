//-----------------------------------------------------------------------------
// File: credits.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "ReVolt.h"
#include "Credits.h"
#include "Timing.h"
#include "Menu2.h"
#include "MenuDraw.h"
#include "TitleScreen.h"
#include "InitPlay.h"

#include "Gameloop.h"

#include "Text.h"
#include "Draw.h"



#define CREDIT_LOOP FALSE

#define CREDIT_MOVE_TIME            TO_TIME(Real(0.25))
#define CREDIT_FADE_TIME            TO_TIME(Real(0.25))
#define CREDIT_SHOW_TIME            TO_TIME(Real(1.5))
#define CREDIT_INIT_TIME            TO_TIME(Real(1.0))
#define CREDIT_EXIT_TIME            TO_TIME(Real(10.0))

#define CREDIT_MAX_TEXT_LINES       12
#define CREDIT_MIN_TEXT_LINES       4

bool SetNextCreditEntry();
void DrawCreditFadePoly();
void ChangeCreditCamera();




#ifndef _N64
static REAL GetTextLen(char *text) 
{
    return Real(CREDIT_TEXT_WIDTH) * strlen(text);
}
#endif


CREDIT_VARS CreditVars;


long NCreditPages = 0;





////////////////////////////////////////////////////////////////
//
// Text lines for each Credit page
//
////////////////////////////////////////////////////////////////

char *InitialMessageCredit[] = {
    "ACCLAIM STUDIOS LONDON",
    NULL,
};

char *PMCredit[] = {
    "Nick Baynes",
    NULL,
};

char *LeadProgrammerCredit[] = {
    "Balor Knight",
    NULL,
};

char *PCProgrammerCredit[] = {
    "Balor Knight",
    "Chris Caulfield",
    NULL,
};

char *N64ProgrammerCredit[] = {
    "Pablo Toledo",
    "Anthony Savva",
    "Matt Taylor",
    "Brian Watson",
    NULL,
};

char *PSXProgrammerCredit[] = {
    "Stefan Hopper",
    "Greg Modern",
    "Gari Biasillo",
    "Kim Watson",
    NULL,
};

char *RenderCredit[] = {
    "Balor Knight",
    NULL,
};

char *PhysicsCredit[] = {
    "Chris Caulfield",
    NULL,
};

char *AICredit[] = {
    "Gari Biasillo",
    NULL,
};

char *EditorProgrammerCredit[] = {
    "Bob Armour",
    NULL,
};

char *ExtraProgrammerCredit[] = {
    "Balor Knight",
    "Chris Caulfield",
    "Gari Biasillo",
    NULL,
};

char *LeadArtistCredit[] = {
    "Matt Tracey",
    NULL,
};

char *TrackModelCredit[] = {
    "Simon Harrison",
    "Shahid Malik",
    "Bliz",
    NULL,
};

char *TrackTextureCredit[] = {
    "Paul Phippen",
    "Jeremy White",
    "James Vale",
    "Jason Green",
    "Jon Trafford",
    "Eoin Rogan",
    "Errol Gale",
    "Colin Mulhearn",
    "Bliz",
    NULL,
};

char *CarCredit[] = {
    "Matt Tracey",
    "Joe Groombridge",
    "James Wallace Duncan",
    NULL,
};

char *CarConceptCredit[] = {
    "Paul Phippen",
    NULL,
};

char *ConceptCredit[] = {
    "Paul Phippen",
    "Simon Harrison",
    NULL,
};

char *LeadDesignerCredit[] = {
    "Simon Harrison",
    NULL,
};

char *DesignerCredit[] = {
    "Nick (NutNut) McGee",
    "Mike Patrick",
    "Kevin Row",
    "Richard Foster",
    "Serkan Hassan",
    NULL,
};

char *SoundTrackCredit[] = {
    "Simon Robertson",
    "Stephen Root",
    "Larry Lush",
    "Phil Earl",
    "Dominic Glynn",
    NULL,
};

char *SoundEffectsCredit[] = {
    "Andy Brock",
    NULL,
};

char *N64SoundCredit[] = {
    "Simon Robertson",
    NULL,
};

char *QAManagerCredit[] = {
    "Geoff Cullen",
    NULL,
};

char *QATeamCredit[] = {
    "Andrew Aish",
    "Stuart Ryall",
    "Louis Amore",
    "Darren Bennett",
    NULL,
};

char *TechNetManCredit[] = {
    "James Hawkins",
    NULL,
};

char *TechPCCredit[] = {
    "Peter Large",
    "Tim Hawkins",
    NULL,
};

char *CreativeDirectorCredit[] = {
    "Robert O'Farrell",
    NULL,
};

char *ArtDirectorCredit[] = {
    "Mark Knowles",
    NULL,
};

char *DesignManagerCredit[] = {
    "Andy Squirell",
    NULL,
};

char *AudioDirectorCredit[] = {
    "Steve Root",
    NULL,
};

char *BusinessManagerCredit[] = {
    "Tim Gummer",
    NULL,
};

char *ActorCredit[] = {
    "Kish Hirani",
    NULL,
};

char *SpecialThanksCredit[] = {
    "Teresa Rosbottom",
    "Jade, Asha, Amy and Anya",
    "Rhonda Baptiste",
    "Brenda and Gemma",
    "Emma",
    "John & Eileen Tracey (& S.W.)",
    "Atsuko Daido Biasillo",
    "Fox W. and the Mogsters",
    NULL,
};

#ifndef _PC
char *FinalMessageCredit[] = {
    "",
    "Developed by Acclaim Studios London",
    "",
    "All characters herein and distict",
    "likenesses thereof are trademark of",
    "Acclaim Entertainment, inc.",
    "",
    "TM , (R) & (C) 1999",
    "ACCLAIM ENTERTAINMENT, INC.",
    "ALL RIGHTS RESERVED",
    "",
    "Distributed by",
    "Acclaim Distribution, inc.",
    "",
    "THANK YOU FOR PLAYING!",
    NULL,
};

#else
char *FinalMessageCredit[] = {
    "",
    "Developed by Acclaim Studios London",
    "",
    "All Rights Reserved",
    "",
    "Uses Miles Sound System. @ 1991-1999 by Rad Game Tools, inc.",
    "",
    "All characters herein and distict likenesses thereof are",
    "trademark of Acclaim Entertainment, inc.",
    "",
    "TM, (R) & @ 1999 ACCLAIM ENTERTAINMENT, INC. ALL RIGHTS RESERVED",
    ""
    "Distributed by Acclaim Distribution, inc.",
    "",
    "",
    "THANK YOU FOR PLAYING!",
    NULL,
};
#endif

char *NYProducerCredit[] = {
    "James Daly",
    NULL,
};

char *NYDirectorCredit[] = {
    "Brett Gow",
    NULL,
};

char *NYSeniorProducersCredit[] = {
    "Mike Archer",
    "Doug Yellin",
    NULL,
};

char *NYProductDevelopmentCredit[] = {
    "Howard Perlman",
    "Shawn Rosen",
    "Peter Wanat",
    "James Craddock",
    NULL,
};

char *NYQAManagerCredit[] = {
    "Carol Caracciolo",
    NULL,
};

char *NYQASupervisorCredit[] = {
    "Dale Taylor",
    "Jeff Rosa",
    NULL,
};

char *NYQASeniorAnalystCredit[] = {
    "Mike Patterson",
    "Brian Regan",
    NULL,
};

char *NYQALeadAnalystCredit[] = {
    "Matt Canonico",
    "Bonchi Martinez",
    "Mark Garone",
    NULL,
};

char *NYQALeadCredit[] = {
    "Marty Glantz",
    "Ted Kaplan",
    NULL,
};

#ifdef _PC
char *NYQATesterCredit[] = {
    "Bill Handworth",
    "Jameel Vega",
    "Orland Apiado",
    "Steve Guillaume",
    "Christian Johnson",
    "Mike Galatioto",
    "John Karnay",
    "Carlos Mejia",
    "Mike Lynch",
    "Chris Knowles",
    "Jay Capozello",
    "Adam Davidson",
    "Anthony Raccuglia",
    NULL,
};
#endif

char *NYTSManagerCredit[] = {
    "Harry Reimer",
    NULL,
};

char *NYTSSupervisorCredit[] = {
    "Kevin Denehy",
    "Andrew Fullaytor",
    NULL,
};

char *NYTSTurnoverCoordCredit[] = {
    "Chris Maher",
    NULL,
};

char *NYTSTurnoverTechCredit[] = {
    "Anthony Gentile",
    NULL,
};

char *NYTSSeniorTechnicianCredit[] = {
    "Leigh Busch",
    "David Pollick",
    NULL,
};

char *NYTSTechniciansCredit[] = {
    "Chris Zino",
    "Mike Sterzel",
    "Christopher Coppola",
    "Chris Frisone",
    "Ulises Batalla",
    NULL,
};

char *NYTSGuruCredit[] = {
    "John Zigmont",
    NULL,
};

char *NYTSHardwareCredit[] = {
    "Andy Basile",
    NULL,
};

char *NYTSNetworkCredit[] = {
    "Mike DeLieto",
    "Andy Skalka",
    "Karenga Smith",
    NULL,
};

char *ManualAuthorCredit[] = {
    "Bill Dickson",
    NULL,
};

char *USProductMarketingCredit[] = {
    "Noah Ullman",
    NULL,
};

char *USMarketingAnalystCredit[] = {
    "JP Carnovale",
    NULL,
};

char *USActionMarketingCredit[] = {
    "Evan Stein",
    NULL,
};

char *USMarketingServicesCredit[] = {
    "Dorian Rehfield",
    NULL,
};

char *USMarketingVP[] = {
    "Steven Lux",
    NULL,
};

#ifdef _PC
char *USPRCredit[] = {
    "Adam Kahn",
    NULL,
};
#else
char *USConsolePRManagerCredit[] = {
    "Michelle Seebach",
    NULL,
};

char *USConsolePRAsstCredit[] = {
    "Dora Radwick",
    NULL,
};
#endif // _PC

char *EUProdCoordCredit[] = {
    "Harvey Elliott",
    NULL,
};

char *EUMarketingCredit[] = {
    "Emmanuel Melero",
    "Vincent Mercier",
    "Ned Browning",
    "Simon Downing",
    "Mary Bond",
    "Reza Memari",
    "Rainer Zipp",
    "Sigfrido Buttner-Garcia",
    "Phil Brannelly",
    NULL,
};

char *PRCredit[] = {
    "Simon Smith-Wright",
    "Stefan Luludes",
    NULL,
};

char *OtherThanksCredit[] = {
    "Greg Hounsom",
    "Doug McConkey",
    "Matthew Marshall",
    "Paul Cross",
    NULL,
};

char *OnlineManagerCredit[] = {
    "Ben Fischbach",
    NULL,
};

char *WebTechCredit[] = {
    "Bobby Saha",
    NULL,
};

char *SrGraphicDesignerCredit[] = {
    "Sang Jin Bae",
    NULL,
};

char *GraphicDesignerCredit[] = {
    "Jarrett Brilliant",
    NULL,
};

char *OnlineEditorCredit[] = {
    "Ed Fortune",
    NULL,
};

char *ConsumerSupervisorCredit[] = {
    "Tara Schiraldi",
    NULL,
};

char *ConsumerTechnicianCredit[] = {
    "Jack Shen",
    "James Giambrone",
    "Jack Scalici",
    NULL,
};

char *CounselorCredit[] = {
    "Dan DeCarolis",
    "Jason Brown",
    "Brennen Vega",
    NULL,
};

char *ExecAdminCredit[] = {
    "Debbie Diraison",
    NULL,
};

char *AdminCredit[] = {
    "Jenny Racanelli",
    NULL,
};

char *ProductTrackingCredit[] = {
    "Kathleen Burne",
    "Nicoli Ruisi",
    NULL,
};

char *EvenMoreThanksCredit[] = {
    "Samantha and Darby",
    NULL,
};























////////////////////////////////////////////////////////////////
//
// Credit Page data
//
////////////////////////////////////////////////////////////////

CREDIT_PAGE_DATA InitialMessageCredits[] = {
    {
        "A Game by",
        InitialMessageCredit,
    },
    {
        "",
        NULL,
    },
};

CREDIT_PAGE_DATA ProjectManagerCredits[] = {
    {
        "Project Manager",
        PMCredit,
    },
    {
        "",
        NULL,
    },
};

CREDIT_PAGE_DATA ProgrammerCredits[] = {
    {
        "Lead",
        LeadProgrammerCredit,
    },
    {
        "Coders: PC",
        PCProgrammerCredit,
    },
#ifndef _PSX
    {
        "Nintendo 64 Coders",
        N64ProgrammerCredit,
    },
#endif
#ifndef _N64
    {
        "Coders: This Version",
        PSXProgrammerCredit,
    },
#endif
#ifdef _PC
    {
        "Visuals and Audio (R/C Eyes and Ears)",
        RenderCredit,
    },
#endif
    {
        "Dynamics (R/C Hands and Feet)",
        PhysicsCredit,
    },
    {
        "Intellect (R/C Brains)",
        AICredit,
    },
    {
        "Track Editor",
        EditorProgrammerCredit,
    },
    {
        "Additional Programming",
        ExtraProgrammerCredit,
    },
    {
        "",
        NULL,
    },
};


CREDIT_PAGE_DATA ArtistCredits[] = {
    {
        "Lead Artist",
        LeadArtistCredit,
    },
    {
        "Track Modelling",
        TrackModelCredit,
    },
    {
        "Track Textures",
        TrackTextureCredit,
    },
    {
        "Car Models and Artwork",
        CarCredit,
    },
    {
        "Car Concepts",
        CarConceptCredit,
    },
    {




        "",
        NULL,
    },
};

CREDIT_PAGE_DATA ConceptCredits[] = {
    {
        "Based on an original concept by",
        ConceptCredit,
    },
    {
        "",
        NULL,
    },
};


CREDIT_PAGE_DATA DesignerCredits[] = {
    {
        "Lead Designer",
        LeadDesignerCredit,
    },
    {
        "Designers",
        DesignerCredit,
    },
    {
        "",
        NULL,
    },
};

CREDIT_PAGE_DATA MusicCredits[] = {
    {
        "Sound Effects Design",
        SoundEffectsCredit,
    },
    {
        "Music Written By",
        SoundTrackCredit,
    },
#ifndef _PSX
    {
        "Special N64 Arrangement",
        N64SoundCredit,
    },
#endif
    {
        "",
        NULL,
    },
};

CREDIT_PAGE_DATA QACredits[] = {
    {
        "QA Manager",
        QAManagerCredit,
    },
    {
        "QA Team",
        QATeamCredit,
    },
    {
        "",
        NULL,
    },
};

CREDIT_PAGE_DATA TechCredits[] = {
    {
        "Network Manager",
        TechNetManCredit,
    },
    {
        "PC Technician",
        TechPCCredit,
    },
    {
        "",
        NULL,
    },
};


CREDIT_PAGE_DATA ManagementCredits[] = {
    {
        "Director of Product Development",
        CreativeDirectorCredit,
    },
    {
        "Art Director",
        ArtDirectorCredit,
    },
    {
        "Acting Creative Director",
        DesignManagerCredit,
    },
    {
        "Audio Director",
        AudioDirectorCredit,
    },
    {
        "Business Development Manager",
        BusinessManagerCredit,
    },
    {
        "Acting Technical Director",
        ActorCredit,
    },
    {
        "",
        NULL,
    },
};

CREDIT_PAGE_DATA SpecialThanksCredits[] = {
    {
        "Special Thanks",
        SpecialThanksCredit,
    },
    {
        "",
        NULL,
    },
};

CREDIT_PAGE_DATA FinalMessageCredits[] = {
    {
        "(C) ACCLAIM ENTERTAINMENT INC. 1999",
        FinalMessageCredit,
    },
    {
        "",
        NULL,
    },
};

CREDIT_PAGE_DATA NYProductCredits[] = {
    {
        "Producer",
        NYProducerCredit,
    },
    {
        "Director",
        NYDirectorCredit,
    },
    {
        "Senior Producers",
        NYSeniorProducersCredit,
    },
    {
        "Product Development",
        NYProductDevelopmentCredit,
    },
    {
        "",
        NULL,
    },
};

CREDIT_PAGE_DATA NYQACredits[] = {
    {
        "Manager",
        NYQAManagerCredit,
    },
    {
        "Supervisors",
        NYQASupervisorCredit,
    },
    {
        "Senior Lead Analysts",
        NYQASeniorAnalystCredit,
    },
    {
        "Lead Analyst",
        NYQALeadAnalystCredit,
    },
    {
        "Project Leads",
#ifndef _PSX
        NYQALeadCredit,
#else
        NYPSXQALeadCredit,
#endif
    },
#if defined (_N64)
    {
        "Game Analyst",
        NYN64QAGameAnalystCredit,
    },
    {
        "Asst. Project Lead",
        NYN64QAAsstLeadCredit,
    },
#elif defined (_PSX)
    {
        "Asst. Project Lead",
        NYPSXQAAsstLeadCredit,
    },
#endif
    {
        "Testers",
#if defined (_PC)
        NYQATesterCredit,
#elif defined (_N64)
        NYN64QATesterCredit,
#elif defined (_PSX)
        NYPSXQATesterCredit,
#endif
    },
    {
        "",
        NULL,
    },
};

CREDIT_PAGE_DATA NYTechSupportCredits[] = {
    {
        "Manager",
        NYTSManagerCredit,
    },
    {
        "Supervisors",
        NYTSSupervisorCredit,
    },
    {
        "Turnover Coordinator",
        NYTSTurnoverCoordCredit,
    },
    {
        "Turnover Technician",
        NYTSTurnoverTechCredit,
    },
    {
        "Senior Technicians",
        NYTSSeniorTechnicianCredit,
    },
    {
        "Technicians",
        NYTSTechniciansCredit,
    },
    {
        "Sr. Hardware Guru",
        NYTSGuruCredit,
    },
    {
        "Hardware Tech.",
        NYTSHardwareCredit,
    },
    {
        "Network Support",
        NYTSNetworkCredit,
    },
    {
        "",
        NULL,
    },
};

CREDIT_PAGE_DATA ManualCredits[] = {
    {
        "Author",
        ManualAuthorCredit,
    },
    {
        "",
        NULL
    },
};

CREDIT_PAGE_DATA USMarketingCredits[] = {
    {
        "Product Marketing",
        USProductMarketingCredit,
    },
    {
        "Marketing Analyst",
        USMarketingAnalystCredit,
    },
    {
        "Director of Marketing (Action)",
        USActionMarketingCredit,
    },
    {
        "Director of Marketing Services",
        USMarketingServicesCredit,
    },
    {
        "Vice President, Marketing",
        USMarketingVP,
    },
    {
        "",
        NULL
    },
};


CREDIT_PAGE_DATA USPRCredits[] = {
    {
        "PR Manager",
#ifdef _PC
        USPRCredit,
#else
        USConsolePRManagerCredit,
#endif
    },
#ifndef _PC
    {
        "Asst. PR Manager",
        USConsolePRAsstCredit,
    },
#endif
    {
        "",
        NULL
    },
};

CREDIT_PAGE_DATA EUProdCoordCredits[] = {
    {
        "",
        EUProdCoordCredit,
    },
    {
        "",
        NULL
    },
};

CREDIT_PAGE_DATA EUMarketingCredits[] = {
    {
        "",
        EUMarketingCredit,
    },
    {
        "",
        NULL
    },
};

CREDIT_PAGE_DATA PRCredits[] = {
    {
        "",
        PRCredit,
    },
    {
        "",
        NULL
    },
};

CREDIT_PAGE_DATA OtherThanksCredits[] = {
    {
        "",
        OtherThanksCredit,
    },
    {
        "",
        NULL
    },
};

CREDIT_PAGE_DATA OnlineCredits[] = {
    {
        "Manager",
        OnlineManagerCredit,
    },
    {
        "Internet Architect",
        WebTechCredit,
    },
    {
        "Sr. Graphic Designer",
        SrGraphicDesignerCredit,
    },
    {
        "Graphic Designer",
        GraphicDesignerCredit,
    },
    {
        "Online Editor",
        OnlineEditorCredit,
    },
    {
        "",
        NULL
    },
};

CREDIT_PAGE_DATA ConsumerCredits[] = {
    {
        "Supervisor",
        ConsumerSupervisorCredit,
    },
    {
        "PC Technicians",
        ConsumerTechnicianCredit,
    },
    {
        "Game Counselors",
        CounselorCredit,
    },
    {
        "",
        NULL
    },
};

CREDIT_PAGE_DATA AdminCredits[] = {
    {
        "Executive AA",
        ExecAdminCredit,
    },
    {
        "",
        AdminCredit,
    },
    {
        "",
        NULL
    },
};

CREDIT_PAGE_DATA TrackingCredits[] = {
    {
        "",
        ProductTrackingCredit,
    },
    {
        "",
        NULL
    },
};

CREDIT_PAGE_DATA EvenMoreThanksCredits[] = {
    {
        "",
        EvenMoreThanksCredit,
    },
    {
        "",
        NULL
    },
};


////////////////////////////////////////////////////////////////
//
// Credit Pages
//
////////////////////////////////////////////////////////////////

CREDIT_PAGE Credits[] = {
    ////////////////////////////////////////////////////////////////
    //
    // Lead-in message
    //
    ////////////////////////////////////////////////////////////////
    {
        "RE-VOLT",
        InitialMessageCredits,
        CREDIT_CENTRE_X | CREDIT_CENTRE_Y | CREDIT_CENTRE_TEXT,
    },

    ////////////////////////////////////////////////////////////////
    //
    // Credit Takers Nanon.
    //
    ////////////////////////////////////////////////////////////////
    {
        "RE-VOLT TEAM",
        ProjectManagerCredits,
        CREDIT_CENTRE_X | CREDIT_CENTRE_Y | CREDIT_CENTRE_TEXT,
    },

    ////////////////////////////////////////////////////////////////
    //
    // Programmers
    //
    ////////////////////////////////////////////////////////////////
    {
        "CODING",
        ProgrammerCredits,
        CREDIT_SCROLL | CREDIT_RANDOM_POS,
    },

    ////////////////////////////////////////////////////////////////
    //
    // Artists
    //
    ////////////////////////////////////////////////////////////////
    {
        "ARTWORK",
        ArtistCredits,
        CREDIT_SCROLL | CREDIT_RANDOM_POS,
    },

    ////////////////////////////////////////////////////////////////
    //
    // Concept
    //
    ////////////////////////////////////////////////////////////////
    {
        "RE-VOLT",
        ConceptCredits,
        CREDIT_CENTRE_X | CREDIT_CENTRE_Y | CREDIT_CENTRE_TEXT,
    },

    ////////////////////////////////////////////////////////////////
    //
    // Design
    //
    ////////////////////////////////////////////////////////////////
    {
        "DESIGN",
        DesignerCredits,
        CREDIT_SCROLL | CREDIT_RANDOM_POS,
    },

    ////////////////////////////////////////////////////////////////
    //
    // Music
    //
    ////////////////////////////////////////////////////////////////
    {
        "MUSIC DEPARTMENT",
        MusicCredits,
        CREDIT_SCROLL | CREDIT_RANDOM_POS,
    },

    ////////////////////////////////////////////////////////////////
    //
    // QA
    //
    ////////////////////////////////////////////////////////////////
    {
        "QUALITY ASSURANCE",
        QACredits,
        CREDIT_SCROLL | CREDIT_RANDOM_POS,
    },

    ////////////////////////////////////////////////////////////////
    //
    // Tech Support
    //
    ////////////////////////////////////////////////////////////////
    {
        "Technical Support",
        TechCredits,
        CREDIT_SCROLL | CREDIT_RANDOM_POS,
    },

    ////////////////////////////////////////////////////////////////
    //
    // Da Manijmont
    //
    ////////////////////////////////////////////////////////////////
    {
        "MANAGEMENT",
        ManagementCredits,
        CREDIT_SCROLL | CREDIT_RANDOM_POS,
    },

    ////////////////////////////////////////////////////////////////
    //
    // Special Thanks
    //
    ////////////////////////////////////////////////////////////////
    {
        "SPECIAL THANKS TO",
        SpecialThanksCredits,
        CREDIT_SCROLL | CREDIT_RANDOM_POS,
    },

    ////////////////////////////////////////////////////////////////
    //
    // NY CREDITS START HERE
    //
    ////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////
    //
    // Product Development
    //
    ////////////////////////////////////////////////////////////////
    {
        "NY Product Development",
        NYProductCredits,
        CREDIT_SCROLL | CREDIT_RANDOM_POS,
    },

    ////////////////////////////////////////////////////////////////
    //
    // QA
    //
    ////////////////////////////////////////////////////////////////
    {
        "NY Quality Assurance",
        NYQACredits,
        CREDIT_SCROLL | CREDIT_RANDOM_POS,
    },

    ////////////////////////////////////////////////////////////////
    //
    // Tech Support
    //
    ////////////////////////////////////////////////////////////////
    {
        "NY Technical Support",
        NYTechSupportCredits,
        CREDIT_SCROLL | CREDIT_RANDOM_POS,
    },

    ////////////////////////////////////////////////////////////////
    //
    // Manual
    //
    ////////////////////////////////////////////////////////////////
    {
        "Manual",
        ManualCredits,
        CREDIT_SCROLL | CREDIT_RANDOM_POS,
    },

    ////////////////////////////////////////////////////////////////
    //
    // US Marketing
    //
    ////////////////////////////////////////////////////////////////
    {
        "US Marketing",
        USMarketingCredits,
        CREDIT_SCROLL | CREDIT_RANDOM_POS,
    },

    ////////////////////////////////////////////////////////////////
    //
    // US PR
    //
    ////////////////////////////////////////////////////////////////
    {
        "US Public Relations",
        USPRCredits,
        CREDIT_CENTRE_X | CREDIT_CENTRE_Y | CREDIT_CENTRE_TEXT,
    },

    ////////////////////////////////////////////////////////////////
    //
    // EU Product Coord
    //
    ////////////////////////////////////////////////////////////////
    {
        "European Product Coordinator",
        EUProdCoordCredits,
        CREDIT_CENTRE_X | CREDIT_CENTRE_Y | CREDIT_CENTRE_TEXT,
    },

    ////////////////////////////////////////////////////////////////
    //
    // EU Marketing
    //
    ////////////////////////////////////////////////////////////////
    {
        "European Marketing",
        EUMarketingCredits,
        CREDIT_SCROLL | CREDIT_RANDOM_POS,
    },

    ////////////////////////////////////////////////////////////////
    //
    // PR
    //
    ////////////////////////////////////////////////////////////////
    {
        "Public Relations",
        PRCredits,
        CREDIT_SCROLL | CREDIT_RANDOM_POS,
    },

    ////////////////////////////////////////////////////////////////
    //
    // Thanks
    //
    ////////////////////////////////////////////////////////////////
    {
        "With thanks to...",
        OtherThanksCredits,
        CREDIT_SCROLL | CREDIT_RANDOM_POS,
    },

    ////////////////////////////////////////////////////////////////
    //
    // Acclaim Online
    //
    ////////////////////////////////////////////////////////////////
    {
        "Acclaim Online",
        OnlineCredits,
        CREDIT_SCROLL | CREDIT_RANDOM_POS,
    },

    ////////////////////////////////////////////////////////////////
    //
    // Consumer Service
    //
    ////////////////////////////////////////////////////////////////
    {
        "Consumer Services",
        ConsumerCredits,
        CREDIT_SCROLL | CREDIT_RANDOM_POS,
    },

    ////////////////////////////////////////////////////////////////
    //
    // Admin Assistants
    //
    ////////////////////////////////////////////////////////////////
    {
        "Administrative Assistants",
        AdminCredits,
        CREDIT_SCROLL | CREDIT_RANDOM_POS,
    },

    ////////////////////////////////////////////////////////////////
    //
    // Product Tracking
    //
    ////////////////////////////////////////////////////////////////
    {
        "Product Tracking",
        TrackingCredits,
        CREDIT_SCROLL | CREDIT_RANDOM_POS,
    },

    ////////////////////////////////////////////////////////////////
    //
    // Even more thanks
    //
    ////////////////////////////////////////////////////////////////
    {
        "Special Thanks",
        EvenMoreThanksCredits,
        CREDIT_SCROLL | CREDIT_RANDOM_POS,
    },

    ////////////////////////////////////////////////////////////////
    //
    // Final Message
    //
    ////////////////////////////////////////////////////////////////
    {
        "RE-VOLT",
        FinalMessageCredits,
        CREDIT_CENTRE_X | CREDIT_CENTRE_Y | CREDIT_CENTRE_TEXT,
    },

    ////////////////////////////////////////////////////////////////
    //
    // Thats all folks
    //
    ////////////////////////////////////////////////////////////////
    { // No more...
        "The End",
        NULL,
    }
};




////////////////////////////////////////////////////////////////
//
// Initialise the credit entries
//
////////////////////////////////////////////////////////////////

#if defined _PC
#define CREDIT_X_BOUNDARY   64
#define CREDIT_X_INDENT     32
#define CREDIT_Y_BOUNDARY   64
#define CREDIT_Y_INDENT     32
#elif defined(_N64)
#define CREDIT_X_BOUNDARY   40
#define CREDIT_X_INDENT     20
#define CREDIT_Y_BOUNDARY   80
#define CREDIT_Y_INDENT     16
#elif defined(_PSX)
#define CREDIT_X_BOUNDARY   64
#define CREDIT_X_INDENT     32
#define CREDIT_Y_BOUNDARY   80
#define CREDIT_Y_INDENT     16
#endif

void InitCreditEntries()
{
    CREDIT_PAGE *page;
    CREDIT_PAGE_DATA *pageData;
    REAL width;
    int iSection, iPage, iLine, iName;



    iPage = 0;
    do {
        page = &Credits[iPage];

        // Is this the last credit entry?
        if (page->PageData == NULL) {
            break;
        }

        // Get width of title
        page->XSize = GetTextLen(page->Title);

        // Count sections in this page
        iSection = 0;
        iLine = 0;
        do {

            pageData = &page->PageData[iSection];

            if (pageData->NameList == NULL) {
                break;
            }

            width = GetTextLen(pageData->SubTitle) + Real(CREDIT_TEXT_INDENT);
            if (width > page->XSize) page->XSize = width;

            // Count name entries in this section
            iName = 0;
            do {
                if (pageData->NameList[iName] == NULL) {
                    break;
                }

                width = GetTextLen(pageData->NameList[iName]) + Real(CREDIT_TEXT_INDENT);
                if (width > page->XSize) page->XSize = width;

                iName++;
                iLine++;

            } while (TRUE);
            pageData->NNames = iName;


            // Count Sections
            iSection++;
            iLine += 2;             // For sub-heading and space

        } while (TRUE);
        page->NSubSections = iSection;
        page->NTextLines = iLine;

        // Fix Scroll speed for now
        page->ScrollSpeed = FROM_TIME(Real(40));

        // Get Height
        if ((page->PageType & CREDIT_SCROLL) == 0) {
            page->YSize = (page->NTextLines + 1) * Real(CREDIT_TEXT_HEIGHT);
        } else if (page->NTextLines > CREDIT_MAX_TEXT_LINES) {
            page->YSize = (CREDIT_MAX_TEXT_LINES + 1) * Real(CREDIT_TEXT_HEIGHT);
        } else if (page->NTextLines < CREDIT_MIN_TEXT_LINES) {
            page->YSize = (CREDIT_MIN_TEXT_LINES) * Real(CREDIT_TEXT_HEIGHT);
        } else {
            page->YSize = (page->NTextLines + 1) * Real(CREDIT_TEXT_HEIGHT);
        }

        // Choose a random position that fits on the screen if told to do so
        if (page->PageType & CREDIT_RANDOM_POS) {
            REAL spare;
            // X position
            spare = Real(MENU_SCREEN_WIDTH) - page->XSize - Real(MENU_FRAME_WIDTH) * 2 - Real(CREDIT_X_BOUNDARY);
            if (spare < ZERO) {
                page->PageType |= CREDIT_CENTRE_X;
            } else {
#ifndef _PSX
                page->XPos = frand(spare) + Real(MENU_FRAME_WIDTH) + Real(CREDIT_X_INDENT);
#else
                page->XPos = (rand() % spare) + Real(MENU_FRAME_WIDTH) + Real(CREDIT_X_INDENT);
#endif
            }
            // Y Position
            spare = Real(MENU_SCREEN_HEIGHT) - page->YSize - Real(MENU_FRAME_HEIGHT) * 2 - Real(CREDIT_Y_BOUNDARY);
            if (spare < ZERO) {
                page->PageType |= CREDIT_CENTRE_Y;
            } else {
#ifndef _PSX
                page->YPos = frand(spare) + Real(MENU_FRAME_HEIGHT) + Real(CREDIT_Y_INDENT);
#else
                page->YPos = (rand() % spare) + Real(MENU_FRAME_HEIGHT) + Real(CREDIT_Y_INDENT);
#endif
            }
        }


        // Centre if necessary
        if (page->PageType & CREDIT_CENTRE_X) {
            page->XPos = (Real(MENU_SCREEN_WIDTH) - page->XSize) / 2;
        }
        if (page->PageType & CREDIT_CENTRE_Y) {
            page->YPos = (Real(MENU_SCREEN_HEIGHT) - page->YSize) / 2;
        }



        // Count Credits
        iPage++;

    } while (TRUE);
    NCreditPages = iPage;

}



////////////////////////////////////////////////////////////////
//
// Init Credit Control vars to inactive state
//
////////////////////////////////////////////////////////////////

void InitCreditStateInactive()
{

    CreditVars.State = CREDIT_STATE_INACTIVE;
    CreditVars.Timer = ZERO;
    CreditVars.MaxTime = ZERO;
    CreditVars.CurrentPage = 0;

}



////////////////////////////////////////////////////////////////
//
// Init Credit Control vars to inactive state
//
////////////////////////////////////////////////////////////////

void InitCreditStateActive()
{
    CREDIT_PAGE *page;

    CreditVars.State = CREDIT_STATE_INIT;
    CreditVars.Timer = ZERO;
    CreditVars.ScrollTimer = ZERO;
    CreditVars.MaxTime = CREDIT_INIT_TIME;
    CreditVars.CurrentPage = -1;

    CreditVars.TimerCurrent = 0;
    CreditVars.TimerLast = 0;

    page = &Credits[CreditVars.CurrentPage];
    
    CreditVars.XPos = Real(320);
    CreditVars.YPos = Real(240);
    CreditVars.XSize = ZERO;
    CreditVars.YSize = ZERO;
    
    SetNextCreditEntry();

    CreditVars.ColIndex = SPRU_COL_YELLOW;

}



////////////////////////////////////////////////////////////////
//
// Process Credits In Game
//
////////////////////////////////////////////////////////////////

void ProcessCredits()
{
    CREDIT_PAGE *page;
    
#if 0
    
    unsigned long timerDiff;
    CreditVars.TimerLast = CreditVars.TimerCurrent;
    CreditVars.TimerCurrent = CurrentTimer();

    if (CreditVars.TimerLast != 0) {
        timerDiff = CreditVars.TimerCurrent - CreditVars.TimerLast;
    } else {
        timerDiff = 0;
    }

    CreditVars.Timer += TIME2MS(timerDiff) / Real(1000);
    CreditVars.ScrollTimer += TIME2MS(timerDiff) / Real(1000);

#else

    CreditVars.Timer += TimeStep;
    CreditVars.ScrollTimer += TimeStep;

#endif

    switch (CreditVars.State) {

    case CREDIT_STATE_INACTIVE:
        // Do nothing
        break;

    case CREDIT_STATE_INIT:                         // Wait for Countdown
        if (CountdownTime == 0) {
            ChangeCreditCamera();
            CreditVars.State = CREDIT_STATE_MOVING;
            CreditVars.MaxTime = CREDIT_MOVE_TIME;
            CreditVars.Timer = ZERO;
            return;
        }

        CreditVars.Alpha = ZERO;
        break;

    case CREDIT_STATE_MOVING:                       // Move Box
        // Change State?
        if (CreditVars.Timer > CreditVars.MaxTime) {
            page = &Credits[CreditVars.CurrentPage];

            CreditVars.State = CREDIT_STATE_FADE_IN;
            CreditVars.Timer = ZERO;
            CreditVars.ScrollTimer = ZERO;
            
            CreditVars.ScrollMaxTime = DivScalar(page->NTextLines * Real(CREDIT_TEXT_HEIGHT) + page->YSize, page->ScrollSpeed);
            
            if ((page->PageType & CREDIT_SCROLL) != 0) {
                CreditVars.MaxTime = ZERO;
            } else {
                CreditVars.MaxTime = CREDIT_FADE_TIME;
            }

            CreditVars.XPos = CreditVars.DestXPos;
            CreditVars.YPos = CreditVars.DestYPos;
            CreditVars.XSize = CreditVars.DestXSize;
            CreditVars.YSize = CreditVars.DestYSize;

            return;
        }

        CreditVars.XPos += MulScalar(CreditVars.XVel, TimeStep);
        CreditVars.YPos += MulScalar(CreditVars.YVel, TimeStep);
        CreditVars.XSize += MulScalar(CreditVars.XGrow, TimeStep);
        CreditVars.YSize += MulScalar(CreditVars.YGrow, TimeStep);

        CreditVars.Alpha = ZERO;
        break;

    case CREDIT_STATE_FADE_IN:                      // Fade Text In
        // Change State?
        if (CreditVars.Timer > CreditVars.MaxTime) {
            page = &Credits[CreditVars.CurrentPage];
            
            CreditVars.State = CREDIT_STATE_SHOWING;

            if ((page->PageType & CREDIT_SCROLL) != 0) {
                CreditVars.MaxTime = CreditVars.ScrollMaxTime;
            } else {
                if (CreditVars.CurrentPage == NCreditPages - 1) {
                    CreditVars.MaxTime = CREDIT_EXIT_TIME;
                } else {
                    CreditVars.MaxTime = CREDIT_SHOW_TIME;
                }
            }

            CreditVars.Timer = ZERO;

            CreditVars.Alpha = ONE;
            return;
        }

        CreditVars.Alpha = DivScalar(CreditVars.Timer, CreditVars.MaxTime);
        break;

    case CREDIT_STATE_SHOWING:                      // Continue Showing Text
        // Scroll the credits

        // Change State?
        if (CreditVars.Timer > CreditVars.MaxTime) {
            page = &Credits[CreditVars.CurrentPage];

            CreditVars.State = CREDIT_STATE_FADE_OUT;

            if ((page->PageType & CREDIT_SCROLL) != 0) {
                CreditVars.MaxTime = ZERO;
            } else {
                CreditVars.MaxTime = CREDIT_FADE_TIME;
            }

            CreditVars.Timer = ZERO;
            return;
        }
        break;

    case CREDIT_STATE_FADE_OUT:                     // Fade Text Out
        // Change State?
        if (CreditVars.Timer > CreditVars.MaxTime) {
            CreditVars.Timer = ZERO;
            CreditVars.Alpha = ZERO;

            if (SetNextCreditEntry()) {
                CreditVars.State = CREDIT_STATE_MOVING;
                CreditVars.MaxTime = CREDIT_MOVE_TIME;
            } else {
                CreditVars.State = CREDIT_STATE_DONE;
#if !CREDIT_LOOP
                SetFadeEffect(FADE_DOWN);
                GameLoopQuit = GAMELOOP_QUIT_FRONTEND;
#endif
            }

            ChangeCreditCamera();
            return;
        }

        CreditVars.Alpha = ONE - DivScalar(CreditVars.Timer, CreditVars.MaxTime);       
        break;

    case CREDIT_STATE_DONE:
#if CREDIT_LOOP
        CreditVars.CurrentPage = -1;
        SetNextCreditEntry();

        CreditVars.State = CREDIT_STATE_MOVING;
        CreditVars.MaxTime = CREDIT_MOVE_TIME;
        CreditVars.Timer = ZERO;
#endif
        break;
    }
}


////////////////////////////////////////////////////////////////
//
// SetNextCreditEntry
//
////////////////////////////////////////////////////////////////

bool SetNextCreditEntry()
{
    bool notdone = TRUE;
    CREDIT_PAGE *page;

    if (CreditVars.CurrentPage >= NCreditPages - 1) {
        return FALSE;
        /*CreditVars.DestXPos = Real(MENU_SCREEN_WIDTH)/2;
        CreditVars.DestYPos = Real(MENU_SCREEN_HEIGHT)*2;
        CreditVars.DestXSize = ZERO;
        CreditVars.DestYSize = ZERO;
        notdone = FALSE;*/
    } else {
        CreditVars.CurrentPage++;
        page = &Credits[CreditVars.CurrentPage];
        CreditVars.DestXPos = page->XPos;
        CreditVars.DestYPos = page->YPos;
        CreditVars.DestXSize = page->XSize;
        CreditVars.DestYSize = page->YSize;
        notdone = TRUE;
    }

    CreditVars.XVel = DivScalar((CreditVars.DestXPos - CreditVars.XPos), CREDIT_MOVE_TIME);
    CreditVars.YVel = DivScalar((CreditVars.DestYPos - CreditVars.YPos), CREDIT_MOVE_TIME);
    CreditVars.XGrow = DivScalar((CreditVars.DestXSize - CreditVars.XSize), CREDIT_MOVE_TIME);
    CreditVars.YGrow = DivScalar((CreditVars.DestYSize - CreditVars.YSize), CREDIT_MOVE_TIME);

    return notdone;
}



////////////////////////////////////////////////////////////////
//
// Change the camera mode and refereced object
//
////////////////////////////////////////////////////////////////

void ChangeCreditCamera()
{
    PLAYER *player;
    bool done;
    int cam;
    int iCount = 0;

    do {
        done = TRUE;
        player = &Players[rand()%StartData.PlayerNum];

        if ((player->CarAI.AIState == CAI_S_REVERSE_CORRECT) ||
            (player->CarAI.AIState == CAI_S_FORWARD_CORRECT) ||
            (player->CarAI.AIState == CAI_S_CORRECT_FORWARDLEFT) ||
            (player->CarAI.AIState == CAI_S_CORRECT_FORWARDRIGHT) ||
            (player->CarAI.AIState == CAI_S_CORRECT_REVERSELEFT) ||
            (player->CarAI.AIState == CAI_S_CORRECT_REVERSERIGHT))
        {
            done = FALSE;
        }

        if (++iCount >= StartData.PlayerNum) 
        {
            done = TRUE;
        }
    } while (!done);

    cam = rand() % 10;

    if (cam > 5) {
        SetCameraRail(CAM_MainCamera, player->ownobj, CAM_RAIL_DYNAMIC_MONO);
    } else {
        SetCameraFollow(CAM_MainCamera, player->ownobj, cam);
    }

}
////////////////////////////////////////////////////////////////
//
// Draw Credits
//
////////////////////////////////////////////////////////////////

void DrawCredits()
{
    REAL xPos, yPos, xSize, fade, xOff;
    int iSection, iName;
    long col, alpha;
    CREDIT_PAGE *page;

    page = &Credits[CreditVars.CurrentPage];

    // Draw Fade poly
    DrawCreditFadePoly();

    // Draw the title

    xSize = GetTextLen(page->Title);
    xPos = Real(50);
    yPos = Real(50);
/*
    DrawSpruBox(
        gMenuWidthScale * (xPos - MENU_FRAME_WIDTH), 
        gMenuHeightScale * (yPos - MENU_FRAME_HEIGHT),
        gMenuWidthScale * (xSize + 2*MENU_FRAME_WIDTH),
        gMenuHeightScale * (CREDIT_TEXT_HEIGHT*2 + 2*MENU_FRAME_HEIGHT),
        CreditVars.ColIndex,
        1);
    
    DrawExtraBigGameText(xPos, yPos, 0xff4040FF, page->Title);
*/

    // Don't draw credits whilst initialising
    if (CreditVars.State == CREDIT_STATE_INIT) return;


    // Draw the Credits
    xPos = CreditVars.XPos;
    yPos = CreditVars.YPos;

#ifndef _PSX
    DrawSpruBox(
        gMenuWidthScale * (xPos - MENU_FRAME_WIDTH), 
        gMenuHeightScale * (yPos - MENU_FRAME_HEIGHT),
        gMenuWidthScale * (CreditVars.XSize + 2*MENU_FRAME_WIDTH),
        gMenuHeightScale * (CreditVars.YSize + 2*MENU_FRAME_HEIGHT),
        CreditVars.ColIndex,
        0);
    
#else
    DrawSpruBox(
        ((xPos >> 16) - MENU_FRAME_WIDTH), 
        ((yPos >> 16) - MENU_FRAME_HEIGHT),
        (CreditVars.XSize + 2*Real(MENU_FRAME_WIDTH)) >>16,
        (CreditVars.YSize + 2*Real(MENU_FRAME_HEIGHT)) >>16,
        CreditVars.ColIndex,
        0);

    if (CreditVars.State != CREDIT_STATE_SHOWING) return;
#endif

    // Draw Title
    FTOL(CreditVars.Alpha * 255, alpha);
    if (alpha > 255) alpha = 255;
    col = alpha << 24 | 0x4040FF;
    DrawGameText(xPos + (page->XSize - xSize) / 2, yPos - CREDIT_TEXT_HEIGHT/2, col, page->Title);

    if ((page->PageType & CREDIT_SCROLL) != 0) {
        //int yOff = Int(MulScalar(CreditVars.ScrollTimer, page->ScrollSpeed));
        yPos += page->YSize - MulScalar(CreditVars.ScrollTimer, page->ScrollSpeed);//yOff;
    }   

    for (iSection = 0; iSection < page->NSubSections; iSection++) {

        CREDIT_PAGE_DATA *section = &page->PageData[iSection];

        fade = (yPos - CreditVars.YPos + Real(CREDIT_TEXT_HEIGHT));
        if (fade < CREDIT_TEXT_HEIGHT) {
            fade = ZERO;
        } else if (fade > CreditVars.YSize) {
            fade = ZERO;
        } else if (fade < CREDIT_TEXT_HEIGHT * 3) {
            fade = (fade - CREDIT_TEXT_HEIGHT) / (CREDIT_TEXT_HEIGHT * 2) ;
        } else if (fade > CreditVars.YSize - Real(CREDIT_TEXT_HEIGHT) * 2) {
            fade = (CreditVars.YSize - fade) / (CREDIT_TEXT_HEIGHT * 2);
        } else {
            fade = ONE;
        }

        if (fade > ONE) fade = ONE;
        if (fade < ZERO) fade = ZERO;

        if (!(page->PageType & CREDIT_SCROLL)) {
            fade = ONE;
        }

        yPos += Real(CREDIT_TEXT_HEIGHT);

        if (fade > ZERO) {
#ifndef _PSX
            FTOL(CreditVars.Alpha * fade * 255, alpha);
#else
            alpha = (CreditVars.Alpha * 255) >> 16;
#endif
            if (alpha > 255) alpha = 255;
        
            // Draw SubSection Header
            col = alpha << 24 | 0xffff00;
            if ((page->PageType & CREDIT_CENTRE_TEXT) != 0) {
                xOff = (page->XSize - GetTextLen(section->SubTitle)) / 2;
            } else {
                xOff = ZERO;
            }

            DrawGameText(xPos + xOff, yPos, col, section->SubTitle);
        }

        for (iName = 0; iName < section->NNames; iName++) {

            fade = (yPos - CreditVars.YPos + Real(CREDIT_TEXT_HEIGHT));
            if (fade < CREDIT_TEXT_HEIGHT) {
                fade = ZERO;
            } else if (fade > CreditVars.YSize) {
                fade = ZERO;
            } else if (fade < CREDIT_TEXT_HEIGHT * 3) {
                fade = (fade - CREDIT_TEXT_HEIGHT) / (CREDIT_TEXT_HEIGHT * 2) ;
            } else if (fade > CreditVars.YSize - Real(CREDIT_TEXT_HEIGHT) * 2) {
                fade = (CreditVars.YSize - fade) / (CREDIT_TEXT_HEIGHT * 2);
            } else {
                fade = ONE;
            }

            if (fade > ONE) fade = ONE;
            if (fade < ZERO) fade = ZERO;

            if (!(page->PageType & CREDIT_SCROLL)) {
                fade = ONE;
            }

            yPos += Real(CREDIT_TEXT_HEIGHT);
            
            if (fade > ZERO) {
#ifndef _PSX
                FTOL(CreditVars.Alpha * fade * 255, alpha);
#else
                alpha = (CreditVars.Alpha * 255) >> 16;
#endif
                if (alpha > 255) alpha = 255;


                // Draw Names

                col = alpha << 24 | 0xffffff;
                if ((page->PageType & CREDIT_CENTRE_TEXT) != 0) {
                    xOff = (page->XSize - GetTextLen(section->NameList[iName])) / 2;
                } else {
                    xOff = ZERO;
                }

                DrawGameText(xPos + xOff, yPos, col, section->NameList[iName]);
            }
        }

        yPos += Real(CREDIT_TEXT_HEIGHT);
    }

}




////////////////////////////////////////////////////////////////
//
// Draw Poly in background to darken image
//
////////////////////////////////////////////////////////////////

void DrawCreditFadePoly()
{
#ifdef _PC
    VERTEX_TEX1 vert[4];
    long col;

    if (CreditVars.State != CREDIT_STATE_MOVING) return;

    vert[0].sx = ZERO;
    vert[0].sy = ZERO;
    vert[1].sx = (REAL)ScreenXsize;
    vert[1].sy = ZERO;
    vert[2].sx = (REAL)ScreenXsize;
    vert[2].sy = (REAL)ScreenYsize;
    vert[3].sx = ZERO;
    vert[3].sy = (REAL)ScreenYsize;

    vert[0].sz = vert[1].sz = vert[2].sz = vert[3].sz = 300.0f;
    vert[0].rhw = vert[1].rhw = vert[2].rhw = vert[3].rhw = 1.0f;

    FTOL(CreditVars.Timer * 960, col);
    col = 255 - col;
    if (col < 0) return;
    col <<= 24;
    col |= 0xFFFFFF;

    vert[0].color = 
    vert[1].color = 
    vert[2].color = 
    vert[3].color = col;


    ZBUFFER_OFF();
    FOG_OFF();
    BLEND_ALPHA();
    BLEND_SRC(D3DBLEND_SRCALPHA);
    BLEND_DEST(D3DBLEND_INVSRCALPHA);
    SET_TPAGE(-1);

    DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, vert, 4, D3DDP_DONOTUPDATEEXTENTS);

#endif
}

