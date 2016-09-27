#!/bin/sh
if [ "$1"x = "extend"x ]
	then
	echo "extend system.img"
	./extendSystemSpace.sh
elif [ "$1"x = "noextend"x ]
	then
	echo "no extend system.img"
else
	echo "usage[$0 extend/noextend ...]"
	exit 1
fi
./mountRk3288System.sh
./addSystemSu.sh
./addXfyyhcRmTtsApk.sh
./modifyLangAndTimezone.sh
./modifyEthernet.sh
./umountRk3288System.sh

