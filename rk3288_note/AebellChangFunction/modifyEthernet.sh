#!/bin/sh
sudo cp ./ethernet/ipconfig.txt ./rk3288_system/etc/
sudo cp ./ethernet/set_ethernet.sh ./rk3288_system/bin/
sudo chmod 777 ./rk3288_system/bin/set_ethernet.sh
