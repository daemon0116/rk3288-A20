#!/bin/sh
if [ -f /data/misc/ethernet/ipconfig.txt ]; then
	cp /system/etc/ipconfig.txt /data/misc/ethernet/
	echo "copy /system/etc/ipconfig.txt to /data/misc/ethernet/."
else
	echo "/data/misc/ethernet/ipconfig.txt is already exist."
fi
