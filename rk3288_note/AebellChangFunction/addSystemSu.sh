#!/bin/sh
#add root for everyone
#1.service daemonsu /system/xbin/su --daemonsu
#		class main
#		oneshot
#2.following
sudo rm ./rk3288_system/xbin/su -fr
sudo cp root_files/su ./rk3288_system/xbin/
sudo chown root ./rk3288_system/xbin/su
sudo chgrp root ./rk3288_system/xbin/su
sudo chmod 06755 ./rk3288_system/xbin/su

