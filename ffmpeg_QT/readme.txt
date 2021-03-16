1、工程前首先把ffmpeg的环境配置好，准备好下面库和头文件 并且在windows下能运行ffmpeg
lib/win32  lib/win64
include
bin/win32  bin/win64

2、vs工程头文件目录包含添加、链接库的目录添加
   QTCreat在pro管理文件添加使用模块以及头文件路径添加、链接库路径添加
   
3、工程介绍
VeiwFFmpegTest  进行测试ffmpeg环境是否安装成功
TestDemux       使用ffmpeg完成音视频文件解封装解码以及视频像素尺寸变化和音频重采样
TestQAVdio		使用QTCreat基于QAudioFormat完成音频重采样
TestQTOpenGL	使用QTCreat基于QOPenGL完成视频显示
XPlay			使用QTCreat实现音视频播放器，基于FFMpeg+QAudioFormat+QOpenGLWidget实现音视频编解码及显示播放