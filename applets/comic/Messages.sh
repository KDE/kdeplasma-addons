#! /usr/bin/env bash
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT *.cpp `find . -name '*.qml'` -o $podir/plasma_applet_comic.pot
