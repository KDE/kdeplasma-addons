#! /usr/bin/env bash
$XGETTEXT `find . -name \*.js -o -name \*.qml -o -name \*.cpp` -o $podir/plasma_applet_org.kde.plasma.activitypager.pot
rm -f rc.cpp