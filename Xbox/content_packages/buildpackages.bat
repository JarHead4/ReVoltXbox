@echo off

echo Remove old packages?-------------------------------------------------------------------------
rmdir /s packages
mkdir packages

echo building premium subscription ------------------------------------------------------------------------------------
for /d %%i in (subscriptions\*) do (
	echo building package %%i ----------------------------------------------------------------------------
	mkdir packages\%%~ni%_raw
	cd %%i
	copy ..\texture.rdf .\
	bundler texture.rdf
	del texture.rdf
	del resource.h
	cd packagefiles
	buildoffer N ..\..\..\packages\%%~ni%.xcp ..\metadata.xbx *.*
	cd ..\..\..
	copy %%i\packagefiles\*.* packages\%%~ni%_raw
)

echo building car keys ------------------------------------------------------------------------------------
for /d %%i in (carkeys\*) do (
	echo building package %%i ----------------------------------------------------------------------------
	mkdir packages\%%~ni%_raw
	cd %%i
	copy ..\details.rdf .\
	bundler details.rdf
	del details.rdf
	del resource.h
	copy ..\contentimage.rdf .\
	bundler contentimage.rdf
	del contentimage.rdf
	del resource.h
	cd packagefiles
	buildoffer N ..\..\..\packages\%%~ni%.xcp ..\metadata.xbx *.*
	cd ..\..\..
	copy %%i\packagefiles\*.* packages\%%~ni%_raw
)

echo building cars ------------------------------------------------------------------------------------
for /d %%i in (cars\*) do (
	echo building package %%i ----------------------------------------------------------------------------
	mkdir packages\%%~ni%_raw
	cd %%i
	copy ..\contentimage.rdf .\
	bundler contentimage.rdf
	del contentimage.rdf
	del resource.h
	copy ..\carbox.rdf .\
	bundler carbox.rdf
	del carbox.rdf
	del resource.h
	cd packagefiles
	
	buildoffer N ..\..\..\packages\%%~ni%.xcp ..\metadata.xbx *.*
	cd ..\..\..
	copy %%i\packagefiles\*.* packages\%%~ni%_raw
)