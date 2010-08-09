#! /usr/bin/env bash
$EXTRACTRC $(find . -name "*.ui") >> rc.cpp
$XGETTEXT $(find . -name "*.cpp") -o $podir/libplasma_groupingcontainment.pot
rm -f rc.cpp
