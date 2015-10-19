# Draw a pattern
r=0.0
g=0.9
b=1.0
while true
      do
for y in -0 1 2 3 4 5 6 7 8 7 6 5 4 3 2 1 0
do
replayOSC -h 127.0.0.1 7780 <<EOF
/laser/bg/begin
/pf/frame 101
/pf/set/minx -8.0
/pf/set/maxx 8.0
/pf/set/miny 0.0
/pf/set/maxy 8.0
/laser/bg/begin
/laser/set/color $r $g $b
/laser/line -8.0 $y 8.0 $y
/laser/line 8.0 $y 8.0 $y
/laser/bg/end
EOF
sleep .1
done
done
