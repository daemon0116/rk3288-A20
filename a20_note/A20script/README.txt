1.开发板启动完毕，连上usb线
2.执行脚本 script_pull.bat ，会从开发板的nanda中导出配置script.bin文件，并转换成script.fex文件
3.编辑script.fex文件，修改成想要的配置
4.执行脚本script_push.bat 会把配置重新写回到nanda中并重启
