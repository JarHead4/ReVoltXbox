@echo OFF
::Get dest dir on Xbox from environment var, or use default if not specified.
::(We use temp var to avoid creating/stomping an env var in default-dir case.)
set __XBOX_DIR=%REVOLT_XBOX_DIR%
if "%__XBOX_DIR%" == ""  set __XBOX_DIR=xe:\revolt



REM ---------------------------------------------------------------------------
REM                  !!! WARNING !!!
REM  If you update the list of files copied, be sure to
REM  also update the script used by the build lab to create
REM  a Re-Volt CD (public\tools\buildlab\makerevolt.cmd)
REM ---------------------------------------------------------------------------

REM === Copy art to Xbox ===

xbmkdir  %__XBOX_DIR% 2>nul

REM Copy original PC assets
REM (Note: copy before Xbox assets because we want some PC assets to be overwritten.)
pushd ..\..\PC\assets\final
xbcp -r -d -t -y -f  *  %__XBOX_DIR%
popd

REM Copy new Xbox assets
xbcp -r -d -t -y -f  *.xbr  %__XBOX_DIR%
xbcp -r -d -t -y -f  *.xwb  %__XBOX_DIR%
xbcp -r -d -t -y -f  *.xsb  %__XBOX_DIR%
xbcp -r -d -t -y -f  *.wma  %__XBOX_DIR%
xbcp -r -d -t -y -f  *.sfx  %__XBOX_DIR%
xbcp -r -d -t -y -f  *.inf  %__XBOX_DIR%
xbcp -r -d -t -y -f  carinfo.txt   %__XBOX_DIR%
xbcp -r -d -t -y -f  cars\*.txt    %__XBOX_DIR%\cars
xbcp -r -d -t -y -f  cars\*.xpr    %__XBOX_DIR%\cars
xbcp    -d -t -y -f  gfx\*.bmp     %__XBOX_DIR%\gfx
xbcp    -d -t -y -f  gfx\*.xpr     %__XBOX_DIR%\gfx
xbcp -r -d -t -y -f  strings\*.bin %__XBOX_DIR%\strings
REM Temporary hack to create D:\times\...
xbcp -r -d -t -y -f  times\*     %__XBOX_DIR%\times

REM return to xbox\art directory just to be safe
rem - we're already there.



::Cleanup. Don't want to create stray environment vars.
set __XBOX_DIR=
