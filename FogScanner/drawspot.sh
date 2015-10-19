# Draw a pattern
replayOSC -h 127.0.0.1 7780 <<EOF
/laser/bg/begin
/pf/frame 101
/pf/set/minx -8.0
/pf/set/maxx 8.0
/pf/set/miny 0.0
/pf/set/maxy 8.0
/laser/bg/begin
/laser/set/color 1.0 1.0 1.0
/laser/circle 0.0 1.0 1.0
/laser/circle 1.0 1.0 0.5 
/laser/circle 1.0 2.0 0.5 
/laser/bg/end
EOF
