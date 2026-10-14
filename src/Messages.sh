#! /usr/bin/env bash
# Extracts the translatable strings (KDE convention, $XGETTEXT and $podir come from the caller)
$XGETTEXT $(find . -name \*.cpp -o -name \*.h) -o $podir/pla8ma_kwin_deco.pot
