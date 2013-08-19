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
   
for ADDR in 0.162 0.70 0.71 0.72 0.73 0.74 0.75 0.76 0.29 3.29 0.31 3.31 3.148
do
   /bin/echo -n "${SUBNET}.${ADDR} ..."
   if /sbin/ping -t 1 -c 2 -q -o ${SUBNET}.${ADDR} >/dev/null 
   then
       echo OK
   else
       echo Failed
   fi
done
arp -an
