#!/bin/sh
dd if=/dev/zero bs=1M count=20>>/home/plg/firefly-rk3288-lollipop/rockdev/Image-rk3288_box/system.img
e2fsck -f /home/plg/firefly-rk3288-lollipop/rockdev/Image-rk3288_box/system.img
resize2fs /home/plg/firefly-rk3288-lollipop/rockdev/Image-rk3288_box/system.img
