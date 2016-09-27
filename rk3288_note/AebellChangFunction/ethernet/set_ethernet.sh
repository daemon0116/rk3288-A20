#!/system/bin/sh
if [ ! -f "/data/misc/ethernet/ipconfig.txt" ]; then
	cp /system/etc/ipconfig.txt /data/misc/ethernet/
	chmod 777 /data/misc/ethernet/ipconfig.txt
	echo "copy /system/etc/ipconfig.txt to /data/misc/ethernet/."
	reboot
else
	echo "/data/misc/ethernet/ipconfig.txt is already exist."
fi
