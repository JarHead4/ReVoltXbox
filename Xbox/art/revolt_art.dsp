# Microsoft Developer Studio Project File - Name="revolt_art" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Xbox Generic Project" 0x0b0a

CFG=revolt_art - Xbox Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "revolt_art.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "revolt_art.mak" CFG="revolt_art - Xbox Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "revolt_art - Xbox Debug" (based on "Xbox Generic Project")
!MESSAGE "revolt_art - Xbox Release" (based on "Xbox Generic Project")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
MTL=midl.exe

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "_Debug"
# PROP BASE Intermediate_Dir "_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "_Debug"
# PROP Intermediate_Dir "_Debug"
# PROP Target_Dir ""
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Copying art to the Xbox
PostBuild_Cmds=call copyart.bat
# End Special Build Tool

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "_Release"
# PROP BASE Intermediate_Dir "_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "_Release"
# PROP Intermediate_Dir "_Release"
# PROP Target_Dir ""
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Copying art to the Xbox
PostBuild_Cmds=call copyart.bat
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "revolt_art - Xbox Debug"
# Name "revolt_art - Xbox Release"
# Begin Group "Levels"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\levels\frontend\frontend.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# PROP BASE Intermediate_Dir "_Debug"
# PROP Intermediate_Dir "_Debug"
# Begin Custom Build - xbrc $(InputPath)
InputDir=.\levels\frontend
WkspDir=.
InputPath=.\levels\frontend\frontend.xdx
InputName=frontend

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\levels\frontend
WkspDir=.
InputPath=.\levels\frontend\frontend.xdx
InputName=frontend

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\garden1\garden1.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# PROP BASE Intermediate_Dir "_Debug"
# PROP Intermediate_Dir "_Debug"
# Begin Custom Build - xbrc $(InputPath)
InputDir=.\levels\garden1
WkspDir=.
InputPath=.\levels\garden1\garden1.xdx
InputName=garden1

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\levels\garden1
WkspDir=.
InputPath=.\levels\garden1\garden1.xdx
InputName=garden1

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\markar\markar.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# PROP BASE Intermediate_Dir "_Debug"
# PROP Intermediate_Dir "_Debug"
# Begin Custom Build - xbrc $(InputPath)
InputDir=.\levels\markar
WkspDir=.
InputPath=.\levels\markar\markar.xdx
InputName=markar

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\levels\markar
WkspDir=.
InputPath=.\levels\markar\markar.xdx
InputName=markar

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\market1\market1.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# PROP BASE Intermediate_Dir "_Debug"
# PROP Intermediate_Dir "_Debug"
# Begin Custom Build - xbrc $(InputPath)
InputDir=.\levels\market1
WkspDir=.
InputPath=.\levels\market1\market1.xdx
InputName=market1

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\levels\market1
WkspDir=.
InputPath=.\levels\market1\market1.xdx
InputName=market1

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\market2\market2.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# PROP BASE Intermediate_Dir "_Debug"
# PROP Intermediate_Dir "_Debug"
# Begin Custom Build - xbrc $(InputPath)
InputDir=.\levels\market2
WkspDir=.
InputPath=.\levels\market2\market2.xdx
InputName=market2

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\levels\market2
WkspDir=.
InputPath=.\levels\market2\market2.xdx
InputName=market2

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\muse1\muse1.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# PROP BASE Intermediate_Dir "_Debug"
# PROP Intermediate_Dir "_Debug"
# Begin Custom Build - xbrc $(InputPath)
InputDir=.\levels\muse1
WkspDir=.
InputPath=.\levels\muse1\muse1.xdx
InputName=muse1

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\levels\muse1
WkspDir=.
InputPath=.\levels\muse1\muse1.xdx
InputName=muse1

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\muse2\muse2.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# PROP BASE Intermediate_Dir "_Debug"
# PROP Intermediate_Dir "_Debug"
# Begin Custom Build - xbrc $(InputPath)
InputDir=.\levels\muse2
WkspDir=.
InputPath=.\levels\muse2\muse2.xdx
InputName=muse2

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\levels\muse2
WkspDir=.
InputPath=.\levels\muse2\muse2.xdx
InputName=muse2

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\muse_bat\muse_bat.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# PROP BASE Intermediate_Dir "_Debug"
# PROP Intermediate_Dir "_Debug"
# Begin Custom Build - xbrc $(InputPath)
InputDir=.\levels\muse_bat
WkspDir=.
InputPath=.\levels\muse_bat\muse_bat.xdx
InputName=muse_bat

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\levels\muse_bat
WkspDir=.
InputPath=.\levels\muse_bat\muse_bat.xdx
InputName=muse_bat

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\nhood1\nhood1.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# PROP BASE Intermediate_Dir "_Debug"
# PROP Intermediate_Dir "_Debug"
# Begin Custom Build - xbrc $(InputPath)
InputDir=.\levels\nhood1
WkspDir=.
InputPath=.\levels\nhood1\nhood1.xdx
InputName=nhood1

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\levels\nhood1
WkspDir=.
InputPath=.\levels\nhood1\nhood1.xdx
InputName=nhood1

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\nhood1_battle\nhood1_battle.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# PROP BASE Intermediate_Dir "_Debug"
# PROP Intermediate_Dir "_Debug"
# Begin Custom Build - xbrc $(InputPath)
InputDir=.\levels\nhood1_battle
WkspDir=.
InputPath=.\levels\nhood1_battle\nhood1_battle.xdx
InputName=nhood1_battle

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\levels\nhood1_battle
WkspDir=.
InputPath=.\levels\nhood1_battle\nhood1_battle.xdx
InputName=nhood1_battle

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\nhood2\nhood2.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# PROP BASE Intermediate_Dir "_Debug"
# PROP Intermediate_Dir "_Debug"
# Begin Custom Build - xbrc $(InputPath)
InputDir=.\levels\nhood2
WkspDir=.
InputPath=.\levels\nhood2\nhood2.xdx
InputName=nhood2

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\levels\nhood2
WkspDir=.
InputPath=.\levels\nhood2\nhood2.xdx
InputName=nhood2

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\ship1\ship1.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# PROP BASE Intermediate_Dir "_Debug"
# PROP Intermediate_Dir "_Debug"
# Begin Custom Build - xbrc $(InputPath)
InputDir=.\levels\ship1
WkspDir=.
InputPath=.\levels\ship1\ship1.xdx
InputName=ship1

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\levels\ship1
WkspDir=.
InputPath=.\levels\ship1\ship1.xdx
InputName=ship1

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\ship2\ship2.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# PROP BASE Intermediate_Dir "_Debug"
# PROP Intermediate_Dir "_Debug"
# Begin Custom Build - xbrc $(InputPath)
InputDir=.\levels\ship2
WkspDir=.
InputPath=.\levels\ship2\ship2.xdx
InputName=ship2

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\levels\ship2
WkspDir=.
InputPath=.\levels\ship2\ship2.xdx
InputName=ship2

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\stunts\stunts.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# PROP BASE Intermediate_Dir "_Debug"
# PROP Intermediate_Dir "_Debug"
# Begin Custom Build - xbrc $(InputPath)
InputDir=.\levels\stunts
WkspDir=.
InputPath=.\levels\stunts\stunts.xdx
InputName=stunts

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\levels\stunts
WkspDir=.
InputPath=.\levels\stunts\stunts.xdx
InputName=stunts

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\toy2\toy2.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# PROP BASE Intermediate_Dir "_Debug"
# PROP Intermediate_Dir "_Debug"
# Begin Custom Build - xbrc $(InputPath)
InputDir=.\levels\toy2
WkspDir=.
InputPath=.\levels\toy2\toy2.xdx
InputName=toy2

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\levels\toy2
WkspDir=.
InputPath=.\levels\toy2\toy2.xdx
InputName=toy2

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\toylite\toylite.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# PROP BASE Intermediate_Dir "_Debug"
# PROP Intermediate_Dir "_Debug"
# Begin Custom Build - xbrc $(InputPath)
InputDir=.\levels\toylite
WkspDir=.
InputPath=.\levels\toylite\toylite.xdx
InputName=toylite

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\levels\toylite
WkspDir=.
InputPath=.\levels\toylite\toylite.xdx
InputName=toylite

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\wild_west1\wild_west1.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# PROP BASE Intermediate_Dir "_Debug"
# PROP Intermediate_Dir "_Debug"
# Begin Custom Build - xbrc $(InputPath)
InputDir=.\levels\wild_west1
WkspDir=.
InputPath=.\levels\wild_west1\wild_west1.xdx
InputName=wild_west1

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\levels\wild_west1
WkspDir=.
InputPath=.\levels\wild_west1\wild_west1.xdx
InputName=wild_west1

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\wild_west2\wild_west2.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# PROP BASE Intermediate_Dir "_Debug"
# PROP Intermediate_Dir "_Debug"
# Begin Custom Build - xbrc $(InputPath)
InputDir=.\levels\wild_west2
WkspDir=.
InputPath=.\levels\wild_west2\wild_west2.xdx
InputName=wild_west2

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\levels\wild_west2
WkspDir=.
InputPath=.\levels\wild_west2\wild_west2.xdx
InputName=wild_west2

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Cars"

# PROP Default_Filter "xdx"
# Begin Source File

SOURCE=.\cars\adeon\adeon.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\adeon
WkspDir=.
InputPath=.\cars\adeon\adeon.xdx
InputName=adeon

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\adeon
WkspDir=.
InputPath=.\cars\adeon\adeon.xdx
InputName=adeon

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cars\amw\amw.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\amw
WkspDir=.
InputPath=.\cars\amw\amw.xdx
InputName=amw

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\amw
WkspDir=.
InputPath=.\cars\amw\amw.xdx
InputName=amw

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cars\beatall\beatall.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\beatall
WkspDir=.
InputPath=.\cars\beatall\beatall.xdx
InputName=beatall

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\beatall
WkspDir=.
InputPath=.\cars\beatall\beatall.xdx
InputName=beatall

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cars\candy\candy.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\candy
WkspDir=.
InputPath=.\cars\candy\candy.xdx
InputName=candy

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\candy
WkspDir=.
InputPath=.\cars\candy\candy.xdx
InputName=candy

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cars\cougar\cougar.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\cougar
WkspDir=.
InputPath=.\cars\cougar\cougar.xdx
InputName=cougar

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\cougar
WkspDir=.
InputPath=.\cars\cougar\cougar.xdx
InputName=cougar

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cars\dino\dino.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\dino
WkspDir=.
InputPath=.\cars\dino\dino.xdx
InputName=dino

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\dino
WkspDir=.
InputPath=.\cars\dino\dino.xdx
InputName=dino

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cars\flag\flag.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\flag
WkspDir=.
InputPath=.\cars\flag\flag.xdx
InputName=flag

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\flag
WkspDir=.
InputPath=.\cars\flag\flag.xdx
InputName=flag

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cars\fone\fone.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\fone
WkspDir=.
InputPath=.\cars\fone\fone.xdx
InputName=fone

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\fone
WkspDir=.
InputPath=.\cars\fone\fone.xdx
InputName=fone

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cars\gencar\gencar.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\gencar
WkspDir=.
InputPath=.\cars\gencar\gencar.xdx
InputName=gencar

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\gencar
WkspDir=.
InputPath=.\cars\gencar\gencar.xdx
InputName=gencar

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cars\mite\mite.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\mite
WkspDir=.
InputPath=.\cars\mite\mite.xdx
InputName=mite

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\mite
WkspDir=.
InputPath=.\cars\mite\mite.xdx
InputName=mite

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cars\moss\moss.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\moss
WkspDir=.
InputPath=.\cars\moss\moss.xdx
InputName=moss

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\moss
WkspDir=.
InputPath=.\cars\moss\moss.xdx
InputName=moss

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cars\mouse\mouse.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\mouse
WkspDir=.
InputPath=.\cars\mouse\mouse.xdx
InputName=mouse

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\mouse
WkspDir=.
InputPath=.\cars\mouse\mouse.xdx
InputName=mouse

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cars\mud\mud.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\mud
WkspDir=.
InputPath=.\cars\mud\mud.xdx
InputName=mud

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\mud
WkspDir=.
InputPath=.\cars\mud\mud.xdx
InputName=mud

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cars\panga\panga.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\panga
WkspDir=.
InputPath=.\cars\panga\panga.xdx
InputName=panga

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\panga
WkspDir=.
InputPath=.\cars\panga\panga.xdx
InputName=panga

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cars\phat\phat.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\phat
WkspDir=.
InputPath=.\cars\phat\phat.xdx
InputName=phat

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\phat
WkspDir=.
InputPath=.\cars\phat\phat.xdx
InputName=phat

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cars\q\q.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\q
WkspDir=.
InputPath=.\cars\q\q.xdx
InputName=q

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\q
WkspDir=.
InputPath=.\cars\q\q.xdx
InputName=q

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cars\r5\r5.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\r5
WkspDir=.
InputPath=.\cars\r5\r5.xdx
InputName=r5

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\r5
WkspDir=.
InputPath=.\cars\r5\r5.xdx
InputName=r5

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cars\rc\rc.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\rc
WkspDir=.
InputPath=.\cars\rc\rc.xdx
InputName=rc

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\rc
WkspDir=.
InputPath=.\cars\rc\rc.xdx
InputName=rc

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cars\rotor\rotor.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\rotor
WkspDir=.
InputPath=.\cars\rotor\rotor.xdx
InputName=rotor

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\rotor
WkspDir=.
InputPath=.\cars\rotor\rotor.xdx
InputName=rotor

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cars\sgt\sgt.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\sgt
WkspDir=.
InputPath=.\cars\sgt\sgt.xdx
InputName=sgt

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\sgt
WkspDir=.
InputPath=.\cars\sgt\sgt.xdx
InputName=sgt

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cars\sugo\sugo.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\sugo
WkspDir=.
InputPath=.\cars\sugo\sugo.xdx
InputName=sugo

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\sugo
WkspDir=.
InputPath=.\cars\sugo\sugo.xdx
InputName=sugo

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cars\tc1\tc1.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\tc1
WkspDir=.
InputPath=.\cars\tc1\tc1.xdx
InputName=tc1

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\tc1
WkspDir=.
InputPath=.\cars\tc1\tc1.xdx
InputName=tc1

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cars\tc2\tc2.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\tc2
WkspDir=.
InputPath=.\cars\tc2\tc2.xdx
InputName=tc2

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\tc2
WkspDir=.
InputPath=.\cars\tc2\tc2.xdx
InputName=tc2

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cars\tc3\tc3.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\tc3
WkspDir=.
InputPath=.\cars\tc3\tc3.xdx
InputName=tc3

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\tc3
WkspDir=.
InputPath=.\cars\tc3\tc3.xdx
InputName=tc3

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cars\tc4\tc4.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\tc4
WkspDir=.
InputPath=.\cars\tc4\tc4.xdx
InputName=tc4

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\tc4
WkspDir=.
InputPath=.\cars\tc4\tc4.xdx
InputName=tc4

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cars\tc5\tc5.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\tc5
WkspDir=.
InputPath=.\cars\tc5\tc5.xdx
InputName=tc5

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\tc5
WkspDir=.
InputPath=.\cars\tc5\tc5.xdx
InputName=tc5

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cars\tc6\tc6.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\tc6
WkspDir=.
InputPath=.\cars\tc6\tc6.xdx
InputName=tc6

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\tc6
WkspDir=.
InputPath=.\cars\tc6\tc6.xdx
InputName=tc6

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cars\toyeca\toyeca.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\toyeca
WkspDir=.
InputPath=.\cars\toyeca\toyeca.xdx
InputName=toyeca

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\toyeca
WkspDir=.
InputPath=.\cars\toyeca\toyeca.xdx
InputName=toyeca

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cars\trolley\trolley.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\trolley
WkspDir=.
InputPath=.\cars\trolley\trolley.xdx
InputName=trolley

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\trolley
WkspDir=.
InputPath=.\cars\trolley\trolley.xdx
InputName=trolley

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cars\ufo\ufo.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\ufo
WkspDir=.
InputPath=.\cars\ufo\ufo.xdx
InputName=ufo

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\ufo
WkspDir=.
InputPath=.\cars\ufo\ufo.xdx
InputName=ufo

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cars\volken\volken.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\volken
WkspDir=.
InputPath=.\cars\volken\volken.xdx
InputName=volken

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\volken
WkspDir=.
InputPath=.\cars\volken\volken.xdx
InputName=volken

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cars\wincar\wincar.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\wincar
WkspDir=.
InputPath=.\cars\wincar\wincar.xdx
InputName=wincar

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\wincar
WkspDir=.
InputPath=.\cars\wincar\wincar.xdx
InputName=wincar

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cars\wincar2\wincar2.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\wincar2
WkspDir=.
InputPath=.\cars\wincar2\wincar2.xdx
InputName=wincar2

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\wincar2
WkspDir=.
InputPath=.\cars\wincar2\wincar2.xdx
InputName=wincar2

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cars\wincar3\wincar3.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\wincar3
WkspDir=.
InputPath=.\cars\wincar3\wincar3.xdx
InputName=wincar3

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\wincar3
WkspDir=.
InputPath=.\cars\wincar3\wincar3.xdx
InputName=wincar3

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cars\wincar4\wincar4.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\wincar4
WkspDir=.
InputPath=.\cars\wincar4\wincar4.xdx
InputName=wincar4

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\cars\wincar4
WkspDir=.
InputPath=.\cars\wincar4\wincar4.xdx
InputName=wincar4

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Gfx"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\gfx\gamepad.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\gfx
WkspDir=.
InputPath=.\gfx\gamepad.xdx
InputName=gamepad

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\gfx
WkspDir=.
InputPath=.\gfx\gamepad.xdx
InputName=gamepad

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Gfx\splash.xdx

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\Gfx
WkspDir=.
InputPath=.\Gfx\splash.xdx
InputName=splash

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - xbrc $(InputPath)
InputDir=.\Gfx
WkspDir=.
InputPath=.\Gfx\splash.xdx
InputName=splash

"$(InputDir)\$(InputName).xbr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\xbrc $(InputName).xdx 
	
# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Wave Banks"

# PROP Default_Filter ".xwp"
# Begin Source File

SOURCE=.\levels\COMMON\sounds_common.xwp

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build
InputDir=.\levels\COMMON
WkspDir=.
InputPath=.\levels\COMMON\sounds_common.xwp
InputName=sounds_common

"$(InputDir)\$(InputName).xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\wavbndlr.exe /L $(InputName).xwp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build
InputDir=.\levels\COMMON
WkspDir=.
InputPath=.\levels\COMMON\sounds_common.xwp
InputName=sounds_common

"$(InputDir)\$(InputName).xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\wavbndlr.exe /L $(InputName).xwp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\garden1\sounds_garden1.xwp

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build
InputDir=.\levels\garden1
WkspDir=.
InputPath=.\levels\garden1\sounds_garden1.xwp
InputName=sounds_garden1

"$(InputDir)\$(InputName).xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\wavbndlr.exe /L $(InputName).xwp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build
InputDir=.\levels\garden1
WkspDir=.
InputPath=.\levels\garden1\sounds_garden1.xwp
InputName=sounds_garden1

"$(InputDir)\$(InputName).xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\wavbndlr.exe /L $(InputName).xwp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\markar\sounds_markar.xwp

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build
InputDir=.\levels\markar
WkspDir=.
InputPath=.\levels\markar\sounds_markar.xwp
InputName=sounds_markar

"$(InputDir)\$(InputName).xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\wavbndlr.exe /L $(InputName).xwp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build
InputDir=.\levels\markar
WkspDir=.
InputPath=.\levels\markar\sounds_markar.xwp
InputName=sounds_markar

"$(InputDir)\$(InputName).xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\wavbndlr.exe /L $(InputName).xwp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\market1\sounds_market1.xwp

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build
InputDir=.\levels\market1
WkspDir=.
InputPath=.\levels\market1\sounds_market1.xwp
InputName=sounds_market1

"$(InputDir)\$(InputName).xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\wavbndlr.exe /L $(InputName).xwp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build
InputDir=.\levels\market1
WkspDir=.
InputPath=.\levels\market1\sounds_market1.xwp
InputName=sounds_market1

"$(InputDir)\$(InputName).xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\wavbndlr.exe /L $(InputName).xwp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\market2\sounds_market2.xwp

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build
InputDir=.\levels\market2
WkspDir=.
InputPath=.\levels\market2\sounds_market2.xwp
InputName=sounds_market2

"$(InputDir)\$(InputName).xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\wavbndlr.exe /L $(InputName).xwp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build
InputDir=.\levels\market2
WkspDir=.
InputPath=.\levels\market2\sounds_market2.xwp
InputName=sounds_market2

"$(InputDir)\$(InputName).xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\wavbndlr.exe /L $(InputName).xwp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\muse1\sounds_muse1.xwp

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build
InputDir=.\levels\muse1
WkspDir=.
InputPath=.\levels\muse1\sounds_muse1.xwp
InputName=sounds_muse1

"$(InputDir)\$(InputName).xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\wavbndlr.exe /L $(InputName).xwp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build
InputDir=.\levels\muse1
WkspDir=.
InputPath=.\levels\muse1\sounds_muse1.xwp
InputName=sounds_muse1

"$(InputDir)\$(InputName).xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\wavbndlr.exe /L $(InputName).xwp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\muse2\sounds_muse2.xwp

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build
InputDir=.\levels\muse2
WkspDir=.
InputPath=.\levels\muse2\sounds_muse2.xwp
InputName=sounds_muse2

"$(InputDir)\$(InputName).xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\wavbndlr.exe /L $(InputName).xwp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build
InputDir=.\levels\muse2
WkspDir=.
InputPath=.\levels\muse2\sounds_muse2.xwp
InputName=sounds_muse2

"$(InputDir)\$(InputName).xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\wavbndlr.exe /L $(InputName).xwp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\Muse_Bat\sounds_muse_bat.xwp

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build
InputDir=.\levels\Muse_Bat
WkspDir=.
InputPath=.\levels\Muse_Bat\sounds_muse_bat.xwp
InputName=sounds_muse_bat

"$(InputDir)\$(InputName).xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\wavbndlr.exe /L $(InputName).xwp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build
InputDir=.\levels\Muse_Bat
WkspDir=.
InputPath=.\levels\Muse_Bat\sounds_muse_bat.xwp
InputName=sounds_muse_bat

"$(InputDir)\$(InputName).xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\wavbndlr.exe /L $(InputName).xwp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\nhood1\sounds_nhood1.xwp

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build
InputDir=.\levels\nhood1
WkspDir=.
InputPath=.\levels\nhood1\sounds_nhood1.xwp
InputName=sounds_nhood1

"$(InputDir)\$(InputName).xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\wavbndlr.exe /L $(InputName).xwp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build
InputDir=.\levels\nhood1
WkspDir=.
InputPath=.\levels\nhood1\sounds_nhood1.xwp
InputName=sounds_nhood1

"$(InputDir)\$(InputName).xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\wavbndlr.exe /L $(InputName).xwp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\nhood1_battle\sounds_nhood1_battle.xwp

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build
InputDir=.\levels\nhood1_battle
WkspDir=.
InputPath=.\levels\nhood1_battle\sounds_nhood1_battle.xwp
InputName=sounds_nhood1_battle

"$(InputDir)\$(InputName).xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\wavbndlr.exe /L $(InputName).xwp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build
InputDir=.\levels\nhood1_battle
WkspDir=.
InputPath=.\levels\nhood1_battle\sounds_nhood1_battle.xwp
InputName=sounds_nhood1_battle

"$(InputDir)\$(InputName).xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\wavbndlr.exe /L $(InputName).xwp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\nhood2\sounds_nhood2.xwp

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build
InputDir=.\levels\nhood2
WkspDir=.
InputPath=.\levels\nhood2\sounds_nhood2.xwp
InputName=sounds_nhood2

"$(InputDir)\$(InputName).xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\wavbndlr.exe /L $(InputName).xwp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build
InputDir=.\levels\nhood2
WkspDir=.
InputPath=.\levels\nhood2\sounds_nhood2.xwp
InputName=sounds_nhood2

"$(InputDir)\$(InputName).xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\wavbndlr.exe /L $(InputName).xwp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\ship1\sounds_ship1.xwp

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build
InputDir=.\levels\ship1
WkspDir=.
InputPath=.\levels\ship1\sounds_ship1.xwp
InputName=sounds_ship1

"$(InputDir)\$(InputName).xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\wavbndlr.exe /L $(InputName).xwp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build
InputDir=.\levels\ship1
WkspDir=.
InputPath=.\levels\ship1\sounds_ship1.xwp
InputName=sounds_ship1

"$(InputDir)\$(InputName).xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\wavbndlr.exe /L $(InputName).xwp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\ship2\sounds_ship2.xwp

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build
InputDir=.\levels\ship2
WkspDir=.
InputPath=.\levels\ship2\sounds_ship2.xwp
InputName=sounds_ship2

"$(InputDir)\$(InputName).xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\wavbndlr.exe /L $(InputName).xwp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build
InputDir=.\levels\ship2
WkspDir=.
InputPath=.\levels\ship2\sounds_ship2.xwp
InputName=sounds_ship2

"$(InputDir)\$(InputName).xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\wavbndlr.exe /L $(InputName).xwp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\toy2\sounds_toy2.xwp

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build
InputDir=.\levels\toy2
WkspDir=.
InputPath=.\levels\toy2\sounds_toy2.xwp
InputName=sounds_toy2

"$(InputDir)\$(InputName).xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\wavbndlr.exe /L $(InputName).xwp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build
InputDir=.\levels\toy2
WkspDir=.
InputPath=.\levels\toy2\sounds_toy2.xwp
InputName=sounds_toy2

"$(InputDir)\$(InputName).xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\wavbndlr.exe /L $(InputName).xwp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\toylite\sounds_toylite.xwp

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build
InputDir=.\levels\toylite
WkspDir=.
InputPath=.\levels\toylite\sounds_toylite.xwp
InputName=sounds_toylite

"$(InputDir)\$(InputName).xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\wavbndlr.exe /L $(InputName).xwp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build
InputDir=.\levels\toylite
WkspDir=.
InputPath=.\levels\toylite\sounds_toylite.xwp
InputName=sounds_toylite

"$(InputDir)\$(InputName).xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\wavbndlr.exe /L $(InputName).xwp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\wild_west1\sounds_wild_west1.xwp

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build
InputDir=.\levels\wild_west1
WkspDir=.
InputPath=.\levels\wild_west1\sounds_wild_west1.xwp
InputName=sounds_wild_west1

"$(InputDir)\$(InputName).xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\wavbndlr.exe /L $(InputName).xwp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build
InputDir=.\levels\wild_west1
WkspDir=.
InputPath=.\levels\wild_west1\sounds_wild_west1.xwp
InputName=sounds_wild_west1

"$(InputDir)\$(InputName).xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\wavbndlr.exe /L $(InputName).xwp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\wild_west2\sounds_wild_west2.xwp

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build
InputDir=.\levels\wild_west2
WkspDir=.
InputPath=.\levels\wild_west2\sounds_wild_west2.xwp
InputName=sounds_wild_west2

"$(InputDir)\$(InputName).xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\wavbndlr.exe /L $(InputName).xwp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build
InputDir=.\levels\wild_west2
WkspDir=.
InputPath=.\levels\wild_west2\sounds_wild_west2.xwp
InputName=sounds_wild_west2

"$(InputDir)\$(InputName).xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\wavbndlr.exe /L $(InputName).xwp 
	
# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Sound Effects"

# PROP Default_Filter ".sdf"
# Begin Source File

SOURCE=.\levels\COMMON\sounds_common.sdf

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build
InputDir=.\levels\COMMON
WkspDir=.
InputPath=.\levels\COMMON\sounds_common.sdf
InputName=sounds_common

"$(InputDir)\$(InputName).sfx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\sfxparse.exe $(InputName).sdf $(InputName).sfx $(WkspDir)\Src\$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build
InputDir=.\levels\COMMON
WkspDir=.
InputPath=.\levels\COMMON\sounds_common.sdf
InputName=sounds_common

"$(InputDir)\$(InputName).sfx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\sfxparse.exe $(InputName).sdf $(InputName).sfx $(WkspDir)\Src\$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\garden1\sounds_garden1.sdf

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build
InputDir=.\levels\garden1
WkspDir=.
InputPath=.\levels\garden1\sounds_garden1.sdf
InputName=sounds_garden1

"$(InputDir)\$(InputName).sfx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\sfxparse.exe $(InputName).sdf $(InputName).sfx $(WkspDir)\Src\$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build
InputDir=.\levels\garden1
WkspDir=.
InputPath=.\levels\garden1\sounds_garden1.sdf
InputName=sounds_garden1

"$(InputDir)\$(InputName).sfx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\sfxparse.exe $(InputName).sdf $(InputName).sfx $(WkspDir)\Src\$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\markar\sounds_markar.sdf

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build
InputDir=.\levels\markar
WkspDir=.
InputPath=.\levels\markar\sounds_markar.sdf
InputName=sounds_markar

"$(InputDir)\$(InputName).sfx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\sfxparse.exe $(InputName).sdf $(InputName).sfx $(WkspDir)\Src\$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build
InputDir=.\levels\markar
WkspDir=.
InputPath=.\levels\markar\sounds_markar.sdf
InputName=sounds_markar

"$(InputDir)\$(InputName).sfx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\sfxparse.exe $(InputName).sdf $(InputName).sfx $(WkspDir)\Src\$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\market1\sounds_market1.sdf

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build
InputDir=.\levels\market1
WkspDir=.
InputPath=.\levels\market1\sounds_market1.sdf
InputName=sounds_market1

"$(InputDir)\$(InputName).sfx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\sfxparse.exe $(InputName).sdf $(InputName).sfx $(WkspDir)\Src\$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build
InputDir=.\levels\market1
WkspDir=.
InputPath=.\levels\market1\sounds_market1.sdf
InputName=sounds_market1

"$(InputDir)\$(InputName).sfx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\sfxparse.exe $(InputName).sdf $(InputName).sfx $(WkspDir)\Src\$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\market2\sounds_market2.sdf

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build
InputDir=.\levels\market2
WkspDir=.
InputPath=.\levels\market2\sounds_market2.sdf
InputName=sounds_market2

"$(InputDir)\$(InputName).sfx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\sfxparse.exe $(InputName).sdf $(InputName).sfx $(WkspDir)\Src\$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build
InputDir=.\levels\market2
WkspDir=.
InputPath=.\levels\market2\sounds_market2.sdf
InputName=sounds_market2

"$(InputDir)\$(InputName).sfx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\sfxparse.exe $(InputName).sdf $(InputName).sfx $(WkspDir)\Src\$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\muse1\sounds_muse1.sdf

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build
InputDir=.\levels\muse1
WkspDir=.
InputPath=.\levels\muse1\sounds_muse1.sdf
InputName=sounds_muse1

"$(InputDir)\$(InputName).sfx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\sfxparse.exe $(InputName).sdf $(InputName).sfx $(WkspDir)\Src\$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build
InputDir=.\levels\muse1
WkspDir=.
InputPath=.\levels\muse1\sounds_muse1.sdf
InputName=sounds_muse1

"$(InputDir)\$(InputName).sfx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\sfxparse.exe $(InputName).sdf $(InputName).sfx $(WkspDir)\Src\$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\muse2\sounds_muse2.sdf

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build
InputDir=.\levels\muse2
WkspDir=.
InputPath=.\levels\muse2\sounds_muse2.sdf
InputName=sounds_muse2

"$(InputDir)\$(InputName).sfx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\sfxparse.exe $(InputName).sdf $(InputName).sfx $(WkspDir)\Src\$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build
InputDir=.\levels\muse2
WkspDir=.
InputPath=.\levels\muse2\sounds_muse2.sdf
InputName=sounds_muse2

"$(InputDir)\$(InputName).sfx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\sfxparse.exe $(InputName).sdf $(InputName).sfx $(WkspDir)\Src\$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\Muse_Bat\sounds_muse_bat.sdf

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build
InputDir=.\levels\Muse_Bat
WkspDir=.
InputPath=.\levels\Muse_Bat\sounds_muse_bat.sdf
InputName=sounds_muse_bat

"$(InputDir)\$(InputName).sfx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\sfxparse.exe $(InputName).sdf $(InputName).sfx $(WkspDir)\Src\$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build
InputDir=.\levels\Muse_Bat
WkspDir=.
InputPath=.\levels\Muse_Bat\sounds_muse_bat.sdf
InputName=sounds_muse_bat

"$(InputDir)\$(InputName).sfx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\sfxparse.exe $(InputName).sdf $(InputName).sfx $(WkspDir)\Src\$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\nhood1\sounds_nhood1.sdf

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build
InputDir=.\levels\nhood1
WkspDir=.
InputPath=.\levels\nhood1\sounds_nhood1.sdf
InputName=sounds_nhood1

"$(InputDir)\$(InputName).sfx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\sfxparse.exe $(InputName).sdf $(InputName).sfx $(WkspDir)\Src\$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build
InputDir=.\levels\nhood1
WkspDir=.
InputPath=.\levels\nhood1\sounds_nhood1.sdf
InputName=sounds_nhood1

"$(InputDir)\$(InputName).sfx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\sfxparse.exe $(InputName).sdf $(InputName).sfx $(WkspDir)\Src\$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\nhood1_battle\sounds_nhood1_battle.sdf

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build
InputDir=.\levels\nhood1_battle
WkspDir=.
InputPath=.\levels\nhood1_battle\sounds_nhood1_battle.sdf
InputName=sounds_nhood1_battle

"$(InputDir)\$(InputName).sfx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\sfxparse.exe $(InputName).sdf $(InputName).sfx $(WkspDir)\Src\$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build
InputDir=.\levels\nhood1_battle
WkspDir=.
InputPath=.\levels\nhood1_battle\sounds_nhood1_battle.sdf
InputName=sounds_nhood1_battle

"$(InputDir)\$(InputName).sfx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\sfxparse.exe $(InputName).sdf $(InputName).sfx $(WkspDir)\Src\$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\nhood2\sounds_nhood2.sdf

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build
InputDir=.\levels\nhood2
WkspDir=.
InputPath=.\levels\nhood2\sounds_nhood2.sdf
InputName=sounds_nhood2

"$(InputDir)\$(InputName).sfx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\sfxparse.exe $(InputName).sdf $(InputName).sfx $(WkspDir)\Src\$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build
InputDir=.\levels\nhood2
WkspDir=.
InputPath=.\levels\nhood2\sounds_nhood2.sdf
InputName=sounds_nhood2

"$(InputDir)\$(InputName).sfx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\sfxparse.exe $(InputName).sdf $(InputName).sfx $(WkspDir)\Src\$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\ship1\sounds_ship1.sdf

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build
InputDir=.\levels\ship1
WkspDir=.
InputPath=.\levels\ship1\sounds_ship1.sdf
InputName=sounds_ship1

"$(InputDir)\$(InputName).sfx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\sfxparse.exe $(InputName).sdf $(InputName).sfx $(WkspDir)\Src\$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build
InputDir=.\levels\ship1
WkspDir=.
InputPath=.\levels\ship1\sounds_ship1.sdf
InputName=sounds_ship1

"$(InputDir)\$(InputName).sfx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\sfxparse.exe $(InputName).sdf $(InputName).sfx $(WkspDir)\Src\$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\ship2\sounds_ship2.sdf

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build
InputDir=.\levels\ship2
WkspDir=.
InputPath=.\levels\ship2\sounds_ship2.sdf
InputName=sounds_ship2

"$(InputDir)\$(InputName).sfx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\sfxparse.exe $(InputName).sdf $(InputName).sfx $(WkspDir)\Src\$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build
InputDir=.\levels\ship2
WkspDir=.
InputPath=.\levels\ship2\sounds_ship2.sdf
InputName=sounds_ship2

"$(InputDir)\$(InputName).sfx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\sfxparse.exe $(InputName).sdf $(InputName).sfx $(WkspDir)\Src\$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\toy2\sounds_toy2.sdf

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build
InputDir=.\levels\toy2
WkspDir=.
InputPath=.\levels\toy2\sounds_toy2.sdf
InputName=sounds_toy2

"$(InputDir)\$(InputName).sfx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\sfxparse.exe $(InputName).sdf $(InputName).sfx $(WkspDir)\Src\$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build
InputDir=.\levels\toy2
WkspDir=.
InputPath=.\levels\toy2\sounds_toy2.sdf
InputName=sounds_toy2

"$(InputDir)\$(InputName).sfx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\sfxparse.exe $(InputName).sdf $(InputName).sfx $(WkspDir)\Src\$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\toylite\sounds_toylite.sdf

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build
InputDir=.\levels\toylite
WkspDir=.
InputPath=.\levels\toylite\sounds_toylite.sdf
InputName=sounds_toylite

"$(InputDir)\$(InputName).sfx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\sfxparse.exe $(InputName).sdf $(InputName).sfx $(WkspDir)\Src\$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build
InputDir=.\levels\toylite
WkspDir=.
InputPath=.\levels\toylite\sounds_toylite.sdf
InputName=sounds_toylite

"$(InputDir)\$(InputName).sfx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\sfxparse.exe $(InputName).sdf $(InputName).sfx $(WkspDir)\Src\$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\wild_west1\sounds_wild_west1.sdf

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build
InputDir=.\levels\wild_west1
WkspDir=.
InputPath=.\levels\wild_west1\sounds_wild_west1.sdf
InputName=sounds_wild_west1

"$(InputDir)\$(InputName).sfx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\sfxparse.exe $(InputName).sdf $(InputName).sfx $(WkspDir)\Src\$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build
InputDir=.\levels\wild_west1
WkspDir=.
InputPath=.\levels\wild_west1\sounds_wild_west1.sdf
InputName=sounds_wild_west1

"$(InputDir)\$(InputName).sfx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\sfxparse.exe $(InputName).sdf $(InputName).sfx $(WkspDir)\Src\$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\levels\wild_west2\sounds_wild_west2.sdf

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build
InputDir=.\levels\wild_west2
WkspDir=.
InputPath=.\levels\wild_west2\sounds_wild_west2.sdf
InputName=sounds_wild_west2

"$(InputDir)\$(InputName).sfx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\sfxparse.exe $(InputName).sdf $(InputName).sfx $(WkspDir)\Src\$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build
InputDir=.\levels\wild_west2
WkspDir=.
InputPath=.\levels\wild_west2\sounds_wild_west2.sdf
InputName=sounds_wild_west2

"$(InputDir)\$(InputName).sfx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	$(WkspDir)\..\utils\sfxparse.exe $(InputName).sdf $(InputName).sfx $(WkspDir)\Src\$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "XACT Projects"

# PROP Default_Filter "*.xap"
# Begin Source File

SOURCE=.\XACTProject\revolt.xap

!IF  "$(CFG)" == "revolt_art - Xbox Debug"

# Begin Custom Build
InputPath=.\XACTProject\revolt.xap

BuildCmds= \
	xactbld /L "$(InputPath)"

"..\src\revolt_audio.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\COMMON\sounds_xact_common.xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\garden1\sounds_xact_garden1.xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\markar\sounds_xact_markar.xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\market1\sounds_xact_market1.xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\market2\sounds_xact_market2.xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\muse1\sounds_xact_muse1.xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\muse2\sounds_xact_muse2.xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\Muse_Bat\sounds_xact_muse_bat.xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\nhood1\sounds_xact_nhood1.xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\nhood1_battle\sounds_xact_nhood1_battle.xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\nhood2\sounds_xact_nhood2.xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\ship1\sounds_xact_ship1.xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\ship2\sounds_xact_ship2.xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\toy2\sounds_xact_toy2.xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\toylite\sounds_xact_toylite.xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\wild_west1\sounds_xact_wild_west1.xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\wild_west2\sounds_xact_wild_west2.xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\COMMON\sounds_xact_common.xsb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\garden1\sounds_xact_garden1.xsb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\markar\sounds_xact_markar.xsb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\market1\sounds_xact_market1.xsb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\market2\sounds_xact_market2.xsb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\muse1\sounds_xact_muse1.xsb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\muse2\sounds_xact_muse2.xsb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\Muse_Bat\sounds_xact_muse_bat.xsb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\nhood1\sounds_xact_nhood1.xsb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\nhood1_battle\sounds_xact_nhood1_battle.xsb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\nhood2\sounds_xact_nhood2.xsb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\ship1\sounds_xact_ship1.xsb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\ship2\sounds_xact_ship2.xsb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\toy2\sounds_xact_toy2.xsb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\toylite\sounds_xact_toylite.xsb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\wild_west1\sounds_xact_wild_west1.xsb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\wild_west2\sounds_xact_wild_west2.xsb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "revolt_art - Xbox Release"

# Begin Custom Build - Build XACT Projects
InputPath=.\revolt.xap

BuildCmds= \
	xactbld /L "$(InputPath)"

"..\src\revolt_audio.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\COMMON\sounds_xact_common.xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\garden1\sounds_xact_garden1.xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\markar\sounds_xact_markar.xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\market1\sounds_xact_market1.xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\market2\sounds_xact_market2.xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\muse1\sounds_xact_muse1.xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\muse2\sounds_xact_muse2.xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\Muse_Bat\sounds_xact_muse_bat.xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\nhood1\sounds_xact_nhood1.xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\nhood1_battle\sounds_xact_nhood1_battle.xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\nhood2\sounds_xact_nhood2.xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\ship1\sounds_xact_ship1.xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\ship2\sounds_xact_ship2.xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\toy2\sounds_xact_toy2.xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\toylite\sounds_xact_toylite.xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\wild_west1\sounds_xact_wild_west1.xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\wild_west2\sounds_xact_wild_west2.xwb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\COMMON\sounds_xact_common.xsb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\garden1\sounds_xact_garden1.xsb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\markar\sounds_xact_markar.xsb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\market1\sounds_xact_market1.xsb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\market2\sounds_xact_market2.xsb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\muse1\sounds_xact_muse1.xsb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\muse2\sounds_xact_muse2.xsb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\Muse_Bat\sounds_xact_muse_bat.xsb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\nhood1\sounds_xact_nhood1.xsb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\nhood1_battle\sounds_xact_nhood1_battle.xsb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\nhood2\sounds_xact_nhood2.xsb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\ship1\sounds_xact_ship1.xsb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\ship2\sounds_xact_ship2.xsb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\toy2\sounds_xact_toy2.xsb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\toylite\sounds_xact_toylite.xsb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\wild_west1\sounds_xact_wild_west1.xsb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"levels\wild_west2\sounds_xact_wild_west2.xsb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
