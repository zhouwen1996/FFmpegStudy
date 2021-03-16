#include "XVideoThread.h"
#include "XDcode.h"
#include <iostream>
#include <QDebug>
#include "XVideoWidget.h"
using namespace std;
#ifdef __cplusplus
extern "C"
{
#endif // !__cplusplus

#include <libavcodec/avcodec.h>
#ifdef __cplusplus
}
#endif // !__cplusplus
//打开音频  不管成功与否都清理para
bool XVideoThread::Open(AVCodecParameters *para, IVideoCall *call, int width, int height)
{
    int ret = true;
    if(!para) return false;

    Clear();

    mux.lock();
    //初始化显示窗口
    this->call = call;
    if(call)
    {
        call->Init(width, height);
    }

    mux.unlock();
    if(!decode->Open(para))//参数在这里释放
    {
        cout << "XVideoThread XDcode Open faile" << endl;
        ret = false;
    }

    qDebug() << "XVideoThread::Open Open = " << ret ;
    return ret;
}

void XVideoThread::SetPause(bool isPause)
{
    vmux.lock();
    this->isPause = isPause;
    vmux.unlock();
}
//解码pts，如果接收到的解码数据pts大于seekPts return true 并显示
bool XVideoThread::RepaintPts(AVPacket *Pkt, long long seekPts)
{
    vmux.lock();
    bool ret = decode->Send(Pkt);
    if(!ret)
    {
        vmux.unlock();
        return true;//表示解码结束
    }
    AVFrame *frame = decode->Recv();
    if(!frame)
    {
        vmux.unlock();
        return false;
    }
    //达到位置 显示
    if(decode->pts >= seekPts)
    {
        //显示
        if(call)
            call->Repaint(frame);
        vmux.unlock();
        return true;
    }
    XFreeFrame(&frame);
    vmux.unlock();
    return false;
}
//读取
void XVideoThread::run()
{
    qDebug()<<"XVideoThread::run()";
    while (!isExit)
    {
        vmux.lock();

        //暂停状态
        if(isPause)
        {
            vmux.unlock();
            msleep(5);
            continue ;
        }

        //音频同步
        if(synpts > 0 && synpts < decode->pts)
        {
            vmux.unlock();
            msleep(1);
            continue;
        }

        //取数据
        AVPacket *pkt = Pop();

        //解码
        bool ret = decode->Send(pkt);
        if(!ret)
        {
            vmux.unlock();
            msleep(1);
            continue;
        }
        //一次Send多次Recv
        while(!isExit)
        {
            AVFrame *frame = decode->Recv();

            if(!frame) break;
            //显示视频
            if(call)
            {
                //qDebug()<<"decode->Recv success ";
                call->Repaint(frame);
            }
            else
            {
                av_frame_free(&frame);
            }
        }
        vmux.unlock();
    }
}
XVideoThread::XVideoThread()
{

}

XVideoThread::~XVideoThread()
{

}
