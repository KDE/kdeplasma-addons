#!/bin/sh
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT *.cpp *.h -o $podir/plasma_applet_quickshare.pot
rm -f rc.cpp
