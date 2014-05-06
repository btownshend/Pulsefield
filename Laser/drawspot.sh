# Draw a pattern
replayOSC -h 127.0.0.1 7780 <<EOF
/laser/circle 0.0 0.0 500.0 0.0 1.0 1.0
/laser/circle 2000.0 0.0 500.0 0.0 1.0 1.0
/laser/update 500
EOF
