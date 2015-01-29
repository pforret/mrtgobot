#!/bin/sh
# program requires:
# * mrtg
# * awk
# * can run on sh/bash/ash (e.g. Synology/QNAP)

#set -ex
PROGNAME=mrtgobot
PROGVERSION=1.0
PROGDATE="2015-01-29"

if [ "$1" == "" ] ; then
# USAGE
cat <<END >&2
=== $PROGNAME $PROGVERSION - $PROGDATE
=== USAGE
$PROGNAME index [file.cfg|folder]: create index file for file.cfg or for all .cfg files in a folder

$PROGNAME run   [file.cfg|folder]: run mrtg on file.cfg or for all .cfg files in a folder

$PROGNAME rsync [file.cfg|folder]: rsync generated html/png for file.cfg or for all .cfg files in a folder

$PROGNAME probe [metric]: give back a measurement in mrtg format

END
exit
fi

BNAME=$(basename $0)
DNAME=$(dirname  $0)
if [ "$DNAME" == "" ] ; then
    DNAME=$(pwd)
fi
ACTION=$1
. $DNAME/lib/tools.inc

case "$ACTION" in
"run")
. $DNAME/lib/mrtgobot.run.inc
;;

"index")
. $DNAME/lib/mrtgobot.index.inc
;;

"rsync")
. $DNAME/lib/mrtgobot.rsync.inc
;;

"probe")
. $DNAME/lib/mrtgobot.probe.inc
;;

*)
echo "ERROR: cannot interpret action [$ACTION] - use run/index/rsync/probe" >&2
;;
esac