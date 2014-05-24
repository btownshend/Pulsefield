# Draw a pattern
replayOSC -h 127.0.0.1 7780 <<EOF
/laser/set/points 0 $*
/laser/set/points 1 $*
EOF
