#!/bin/sh
# just a wrapper to start the php program
DIR=$(dirname $0)
PROG=$(basename $0 .sh)
REAL=$(cd $DIR; pwd)

if [ -f "$REAL/$PROG.php" ] ; then
	php "$REAL/$PROG.php" $*
else
	echo "ERROR: cannot find program [$REAL/$PROG.php]" >&2
fi
