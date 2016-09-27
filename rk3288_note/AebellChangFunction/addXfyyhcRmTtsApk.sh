#!/bin/sh
sudo cp apk/xfyyhc/libttsaisound.so ./rk3288_system/lib/
sudo cp apk/xfyyhc/libwrapper.so ./rk3288_system/lib/
sudo mkdir -p ./rk3288_system/app/XfTts
sudo cp apk/xfyyhc/xfyyhc.apk ./rk3288_system/app/XfTts/
sudo rm ./rk3288_system/app/PicoTts -fr

