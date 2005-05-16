#!/bin/bash

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
