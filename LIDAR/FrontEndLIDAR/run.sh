#!/bin/sh
# Run frontend
export DYLD_LIBRARY_PATH=.
LOGDIR=~/Desktop/PFLogs
[ -d $LOGDIR ] || mkdir -p $LOGDIR
while true
do
    DSTR=$(date +%Y%m%dT%H%M%S)
    LOGFILE=$LOGDIR/$DSTR-frontend.log
    RECFILE=$LOGDIR/$DSTR.ferec
    echo Running frontend with output to $LOGFILE
    ./frontend -B10 -D $LOGFILE -d2 -r $RECFILE
    echo frontend exitted, pausing 2 seconds to restart
    sleep 2
done
