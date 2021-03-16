#include "widget.h"
#include "ui_widget.h"//如果要在外面使用UI则要跟过去看那边是怎么调用的，要添加这个头文件
#include <QApplication>
#include "XDemux.h"
#include <iostream>
#include "XDcode.h"
#include "XVideoWidget.h"
#include <QThread>
#include "XResample.h"
#include "XAudioPlay.h"
#include <XAudioThread.h>
#include "XVideoThread.h"
#include "XDemuxThread.h"
using namespace std;
//wx_camera_1615072849946.mp4

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.show();
    return a.exec();
}
