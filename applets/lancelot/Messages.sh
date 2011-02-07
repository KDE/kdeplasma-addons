#! /usr/bin/env bash
$EXTRACTRC `find . -name \*.ui` >> rc.cpp || exit 11
$XGETTEXT `find . -name \*.cpp -o -name \*.h` -o $podir/lancelot.pot
rm -f rc.cpp
