#! /usr/bin/env bash
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT *.cpp  -o $podir/ksysguardlsofwidgets.pot
rm -f rc.cpp
