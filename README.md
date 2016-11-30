## BetterTelnet

Building BetterTelnet requires *CodeWarrior Gold version 10*, released in 1996 and downloadable from
[Macintosh Garden](http://macintoshgarden.org/apps/codewarrior-10-gold)
Note that this is a totally different version from CodeWarrior Development Studio 10, released in 2005.

- I personally used Mac OS X 10.4, with CodeWarrior running in the Classic environment.

- CodeWarrior IDE 1.7 requires that file type codes be set correctly, run the following commands in a terminal:
```
/Developer/Tools/SetFile -t TEXT source/*/*.c
/Developer/Tools/SetFile -t TEXT source/*/*.h
/Developer/Tools/SetFile -t TEXT source/*/*.pch
/Developer/Tools/SetFile -t TEXT source/*/*/*.c
/Developer/Tools/SetFile -t TEXT source/*/*/*.h
/Developer/Tools/SetFile -t TEXT source/TelnetHelp.bh
/Developer/Tools/SetFile -t TEXT source/*.r
/Developer/Tools/SetFile -t MPLF source/Libraries/ICGluePPC.lib
/Developer/Tools/SetFile -t "OBJ " source/Libraries/ICGlue.o
```

- Then, un-MacBinary the telnet.68k.µ.bin and telnet.ppc.µ.bin project files, open in CodeWarrior
IDE 1.7, and Make (cmd-M).
