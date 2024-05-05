# DarkWechat_Win32

深色模式化微信的dll

仅适用于旧版本的wechat，实测为3.4.5

目前已知可用版本为2.8.0 3.4.5 3.7.6.44，注意需要32位微信

64位微信无法使用，高于3.9的微信无法使用

提供一个可以适配的微信版本https://soft-10-1.xiaoguaniu.com/soft/202112/WX_V3.4.5.45_XiTongZhiJia.zip?time=1714873884&ip=113.57.176.205&secret=f3985716e9f83fd40702e5dad14cef80

请注意若提示缺少相应的dll，这是正常的，因为这是debug模式的编译，不知道为什么MinHook package的Release编译不能通过，你可以找到这些文件或者安装带有v141编译器的vs

新版本不知道什么时候开始Hook gdi相关的api失去效果了。

本项目仅供学习交流使用
