1.�滻������su(5.1)

2.������Ӷ�Ӧ�Ĵ��뵽init.rk30board.rc
diff --git a/device/rockchip/common/init.rk30board.rc b/device/rockchip/common/init.rk30board.rc
index 8a44718..db978a0 100644
--- a/device/rockchip/common/init.rk30board.rc
+++ b/device/rockchip/common/init.rk30board.rc
@@ -164,3 +164,8 @@ on property:app.logsave.start=1
 
 on property:app.logsave.start=0
     stop catlog
+
+
+service daemonsu /system/xbin/su --daemon
+    class main
+    oneshot
3.�޸�
+++ b/system/core/include/private/android_filesystem_config.h
@@ -245,7 +245,7 @@ static const struct fs_path_config android_files[] = {
 
     /* the following five files are INTENTIONALLY set-uid, but they
      * are NOT included on user builds. */
-    { 04750, AID_ROOT,      AID_SHELL,     0, "system/xbin/su" },
+    { 06755, AID_ROOT,      AID_ROOT,     0, "system/xbin/su" },


4.�������д�̼������޷���ȡroot����鿴ls -l system/xbin/su ��Ȩ���Ƿ�Ϊ6755