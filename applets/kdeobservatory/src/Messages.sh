#! /usr/bin/env bash
$EXTRACTRC `find -name \*.ui` >> rc.cpp || exit 11
$XGETTEXT *.cpp -o $podir/plasma_applet_kdeobservatory.pot
rm -f rc.cpp
