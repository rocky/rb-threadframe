#!/bin/bash
# Removes files our patches add. 
#
# Typically I get the Ruby source, check that into git: 
#   git init; git add -f *; git commit -m"Boo!" . 
# And then after patching to go back I use "git reset --hard".
# But this leaves new files added. So to remove those, do this: 
/bin/rm -fvr brkpt.c frame.c tracehook.c test/debugger-ext
