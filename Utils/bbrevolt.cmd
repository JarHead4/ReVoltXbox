@echo off
if (%1)==(lastxdk) (
   call %_NTDRIVE%%_NTROOT%\private\atg\utils\buddybuild.cmd lastxdk %2 %3 %4 %5 %6 %7 %8 %9
) ELSE (
   call %_NTDRIVE%%_NTROOT%\private\atg\utils\buddybuild.cmd revolt %1 %2 %3 %4 %5 %6 %7 %8 %9
)
