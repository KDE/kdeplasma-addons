#! /usr/bin/env bash
$XGETTEXT `find . -name \*.qml` -L Java -o $podir/konqprofiles.pot
rm -f rc.cpp

