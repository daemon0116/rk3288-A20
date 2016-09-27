adb shell mkdir /data/local/tmp/nanda
adb shell busybox mount /dev/block/nanda /data/local/tmp/nanda
adb pull /data/local/tmp/nanda/script.bin
fexc.exe -I bin -O fex script.bin script.fex