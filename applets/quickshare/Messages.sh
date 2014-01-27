#!/bin/sh
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT *.cpp *.h -o $podir/plasma_applet_org.kde.plasma.quickshare.pot
rm -f rc.cpp
