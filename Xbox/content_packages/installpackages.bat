@echo off
for /d %%i in (packages\*_raw.*) do (
	echo installing %%i --------------------------------------------------------------------
	xbinstcont %%i
)

