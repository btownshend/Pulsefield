#!/bin/sh
i=1
while true
do
   file=20131115-c2b/c2-$i.jpg
   if [ ! -r $file ]
   then
       read -p "Press return to acquire $file: " x
       curl 'http://192.168.0.72/image?res=full&quality=21&doublescan=0&x0=0&x1=9999&y0=0&y1=9999' >$file
       say done
   fi
   i=$(expr $i + 1)
done

