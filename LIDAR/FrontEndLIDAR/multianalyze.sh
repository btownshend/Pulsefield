#!/bin/sh
# Analyze error performance by looking at multi1..4
# Use track in multi1 as ground truth and compare with others
# Analyze in matlab using multicompare
for i in 1 2 3 4 5 6 7 8
do
   ./frontend -p ../Recordings/multi${i}.ferec -x10 -B 8 -m 2000
done
echo Run multitest in matlab
