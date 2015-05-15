#!/bin/sh
# Run laser
LOGDIR=~/Desktop/CRSLogs
[ -d $LOGDIR ] || mkdir -p $LOGDIR
while true
do
    LOGFILE=$LOGDIR/$(date +%Y%m%dT%H%M%S)-osclaser.log
    echo Running osclaser with output to $LOGFILE
    ./osclaser -n4 -D $LOGFILE -d1
    echo Osclaser exitted, pausing 2 seconds to restart
    sleep 2
done
