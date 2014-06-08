#!/bin/sh
# Run conductor
LOGDIR=~/Desktop/CRSLogs
[ -d $LOGDIR ] || mkdir -p $LOGDIR
while true
do
    LOGFILE=$LOGDIR/$(date +%Y%m%dT%H%M%S)-osclaser.log
    echo Running osclaser with output to $LOGFILE
    ./osclaser -n2 -D $LOGFILE -d2
    echo Osclaser exitted, pausing 2 seconds to restart
    sleep 2
done
