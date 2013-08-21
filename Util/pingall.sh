#!/bin/sh
SUBNET=192.168
/bin/echo "My address:  $(/sbin/ifconfig en0 | grep netmask)"
for TGT in $(grep -v '^-' ~/Dropbox/Pulsefield/config/urlconfig.txt)
do
   host=$(echo $TGT | sed -e 's/,.*//')
   ip=$(echo $TGT | sed -e 's/.*,\(.*\),.*/\1/')
   port=$(echo $TGT | sed -e 's/.*,//')
   /bin/echo -n "${host}	${ip}	${port}	"
   if /sbin/ping -t 1 -c 2 -q -o ${ip} >/dev/null 
   then
       echo OK
   else
       echo Failed
   fi
done
   
arp -an
