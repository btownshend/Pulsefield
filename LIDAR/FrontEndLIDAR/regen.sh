#!/bin/sh
# Regenerate .mat and .osc files from a run
# Usage 'regen filename.ferec'
base=$(basename $1 .ferec)
recdir=../Recordings
if [ ! -r ${recdir}/${base}.ferec ]
then
    echo Usage: $0 file.ferec
    exit 1
fi

# Start OSC program
~/Dropbox/Pulsefield/OSC/recordOSC/recordOSC 7002 > $recdir/$base.osc &
PID=$!
sleep 2

# Start frontend
./frontend -d1 -x1 -p "$recdir/$base.ferec" 

# Kill OSC program
kill $!

# Run again to generate matlab
#./frontend -d1 -x10 -p "$recdir/$base.ferec" -m 0

echo Now run snapmovie on $recdir/$base-*.mat
