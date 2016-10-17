#!/bin/sh
#增加讯飞语音,删除默认自带语音输入
sudo cp apk/xfyyhc/libttsaisound.so ./rk3288_system/lib/
sudo cp apk/xfyyhc/libwrapper.so ./rk3288_system/lib/
sudo mkdir -p ./rk3288_system/app/XfTts
sudo cp apk/xfyyhc/xfyyhc.apk ./rk3288_system/app/XfTts/
sudo rm ./rk3288_system/app/PicoTts -fr
#增加sogou输入法
sudo cp apk/typeWriting/libNinepatch.so ./rk3288_system/lib/
sudo cp apk/typeWriting/libsogouupdcore.so ./rk3288_system/lib/
sudo cp apk/typeWriting/libweibosdkcore.so ./rk3288_system/lib/
sudo mkdir -p ./rk3288_system/app/Sougou
sudo cp apk/typeWriting/sougoushurufa.apk ./rk3288_system/app/Sougou

