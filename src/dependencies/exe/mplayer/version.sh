#!/bin/sh

test "$1" && extra="-$1"

svn_revision=`LC_ALL=C svn info 2> /dev/null | grep Revision | cut -d' ' -f2`
test $svn_revision || svn_revision=`grep revision .svn/entries 2>/dev/null | cut -d '"' -f2`
test $svn_revision || svn_revision=`sed -n -e '/^dir$/{n;p;q;}' .svn/entries 2>/dev/null`
test $svn_revision || svn_revision=UNKNOWN

NEW_REVISION="#define VERSION \"dev-SVN-r${svn_revision}${extra}\""
OLD_REVISION=`cat version.h 2> /dev/null`
TITLE="#define MP_TITLE \"MPlayer dev-SVN-r${svn_revision}(volume patched)${extra} (C) 2000-2008 MPlayer Team\""

# Update version.h only on revision changes to avoid spurious rebuilds
if test "$NEW_REVISION" != "$OLD_REVISION"; then
    echo "$NEW_REVISION" > version.h
    echo "$TITLE" >> version.h
fi
