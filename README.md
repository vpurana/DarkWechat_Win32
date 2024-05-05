# DarkWechat_Win32

深色模式化微信的dll

仅适用于旧版本的wechat，实测为3.4.5

目前已知可用版本为2.8.0 3.4.5 3.7.6.44，注意需要32位微信

64位微信无法使用，高于3.9的微信无法使用

可以通过修改TrashHook.cpp文件中的FOREGOUND_COLOR和BACKGOUND_COLOR实现配色变化

提供一个可以适配的微信版本https://www.pcsoft.com.cn/soft/15638.html

请注意若提示缺少相应的dll，这是正常的，因为这是debug模式的编译，不知道为什么MinHook package的Release编译不能通过，你可以找到这些文件或者安装带有v141编译器的vs

新版本不知道什么时候开始Hook gdi相关的api失去效果了。

本项目仅供学习交流使用

正确使用的效果图：

![WeChat Screenshot_20240505105539](https://github.com/vpurana/DarkWechat_Win32/assets/69775280/f958dd8d-f2db-4855-b802-b932feb4c01d)

VS背景图作者：翼@Tsubasachyan
