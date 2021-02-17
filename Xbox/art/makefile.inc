# Makefile included by art subdirectories (car, gfx, levels)
# to compile resource description files (.xdx) to produce
# headers (.h) and packed resources files (.xbr)

.SUFFIXES: .xdx .xwp .sdf

# Path to tools
UTILS = C:\XDK\ReVolt\Utils
XBRC = $(UTILS)\xbrc.exe
WAVBNDLR = $(UTILS)\wavbndlr.exe
SFXPARSE = $(UTILS)\sfxparse.exe

# Note: The "rc i386" line below turns on error reporting in build.exe by
# using the same error filter as for rc.exe.  The errors reported by
# xbrc.exe are similar, so this seems to work fine.

.xdx.xbr:
    @type <<$(ECHO_RSP)
rc i386 $<
$(XBRC) $<
<<NOKEEP
    @$(XBRC) $<

.xwp.xwb:
    $(WAVBNDLR) /L $**

.sdf.sfx:
    $(SFXPARSE) $** $*.sfx C:\XDK\ReVolt\xbox\src\$*.h

xactmedia: *.xap
    !xactbld.exe /L $**