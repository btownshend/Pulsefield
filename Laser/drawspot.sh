# Draw a pattern
replayOSC -h 127.0.0.1 7780 <<EOF
/pf/frame 101
/pf/set/minx -3.0
/pf/set/maxx 3.0
/pf/set/miny 0.0
/pf/set/maxy 3.0
/laser/set/color 1.0 1.0 1.0
/laser/circle 0.0 0.0 1.0
/laser/circle 1.0 0.0 0.5 
EOF
