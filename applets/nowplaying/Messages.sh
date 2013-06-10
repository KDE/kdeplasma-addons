#! /usr/bin/env bash
$EXTRACTRC `find . -name \*.ui` >> rc.cpp || exit 11
$XGETTEXT *.cpp -o `find . -name \*.qml` -o $podir/plasma_applet_nowplaying.pot
