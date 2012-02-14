#! /usr/bin/env bash
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT */*.cpp *.cpp -o $podir/plasma_applet_icontasks.pot
