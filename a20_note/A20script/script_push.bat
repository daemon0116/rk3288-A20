fexc.exe -I fex -O bin script.fex script.bin
adb push script.bin /data/local/tmp/nanda
adb shell busybox umount /data/local/tmp/nanda
echo press any key to reboot
pause
adb reboot