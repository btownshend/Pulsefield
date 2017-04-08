#!/bin/sh
for i in 20140413T125104 20140413T130449 20140413T124555 20140404T161327
do
  if [ ! -r ../Recordings/$i.ferec ]
  then
     echo $i.ferec not found
  elif [ ! -r ../Recordings/$i.osc ]
  then
	sh regen.sh ../Recordings/$i.ferec
  fi
done
