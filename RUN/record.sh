#!/bin/sh
# Run conductor
LOGDIR=~/Desktop/PF_LOGS
[ -d $LOGDIR ] || mkdir -p $LOGDIR
while true
do
    LOGFILE=$LOGDIR/$(date +%Y%m%dT%H%M%S)-recorder.osc
    echo Running recorder with output to $LOGFILE
    echo "/pf/datestamp $(date +%Y%m%dT%H%M%S)" >$LOGFILE
    ./recordOSC 7790 >> $LOGFILE
    echo Recorder exitted, pausing 2 seconds to restart
    sleep 2
done
