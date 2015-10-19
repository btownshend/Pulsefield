# Draw a pattern
r=0.0
g=0.9
b=1.0
while true
      do
for x in -4 -3 -2 -1 0 1 2 3 4 3 2 1 0 -1 -2 -3
do
    x2=$(expr $x + 1)
    echo x= $x x2= $x2
replayOSC -h 127.0.0.1 7780 <<EOF
/laser/bg/begin
/pf/frame 101
/pf/set/minx -8.0
/pf/set/maxx 8.0
/pf/set/miny 0.0
/pf/set/maxy 8.0
/laser/bg/begin
/laser/set/color 1.0 0.0 1.0
/laser/line $x 0.0 $x 8.0
/laser/set/color 0.0 1.0 0.0
/laser/line $x2 8.0 $x2 0.0
/laser/bg/end
EOF
sleep 1
done
done
