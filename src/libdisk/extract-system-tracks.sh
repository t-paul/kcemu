#!/bin/bash
#
#  KCemu -- The emulator for the KC85 homecomputer series and much more.
#  Copyright (C) 1997-2010 Torsten Paul
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License along
#  with this program; if not, write to the Free Software Foundation, Inc.,
#  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

if test "x$1" = x
then
	echo "usage: $0 disk-dump <output-file>"
	exit 1
fi

TMP=`mktemp`
if test $? -ne 0
then
	echo "cannot create temporary file, abort."
	exit 1
fi

trap "rm -f \"$TMP\"" 0

SECT=0
while test $SECT -lt 20
do
	SRC_OFFSET=`expr $SECT \* \( 1024 + 8 \) + 8`
	DST_OFFSET=`expr $SECT \* 1024`
	dd if="$1" of="$TMP" bs=1 skip=$SRC_OFFSET seek=$DST_OFFSET count=1024 2>/dev/null
	if test $? -ne 0
	then
		echo "error while extracting system tracks from \"$1\", abort."
		exit 1
	fi

	SECT=`expr $SECT + 1`
done

if test "x$2" = x
then
	cat "$TMP"
else
	mv "$TMP" "$2"
fi
