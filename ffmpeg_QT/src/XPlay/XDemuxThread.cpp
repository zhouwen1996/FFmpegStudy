#include "XDemuxThread.h"
#include "XAudioThread.h"
#include "XDemux.h"
#include "XVideoThread.h"
#include <iostream>
#include <QDebug>
using namespace std;
#ifdef __cplusplus
extern "C"
{
#endif // !__cplusplus

#include <libavformat/avformat.h>

#ifdef __cplusplus
}
#endif // !__cplusplus

#include "XDcode.h"

//进度移动
void XDemuxThread::Seek(double pos)
{
    //清理缓冲
    Clear();

    mux.lock();
    bool status = this->isPause;
    mux.unlock();

    //其余操作暂停
    SetPause(true);

    mux.lock();
    if(demux)
    {
        demux->Seek(pos);
    }
    else
    {
        mux.unlock();
        this->pts = pos;//用于刷新界面
        return ;
    }
    //实际要显示的位置pts
    long long seekpts = pos*demux->totalMs;
    while(!isExit)
    {
        AVPacket *pkt = demux->ReadVedio();
        if(!pkt) break;
        if(vt->RepaintPts(pkt, seekpts))
        {
            //如果解码到seekpts
            this->pts = seekpts;//用于刷新界面
            break;
        }
    }

    mux.unlock();
    //seek时非暂停状态
    if(!status)
        SetPause(status);
}

void XDemuxThread::SetPause(bool isPause)
{
    mux.lock();
    this->isPause = isPause;
    if(at) at->SetPause(isPause);
    if(vt) vt->SetPause(isPause);
    mux.unlock();
}

void XDemuxThread::run()
{
    qDebug()<<"XDemuxThread::run()";
    while(!isExit)
    {
        mux.lock();

        if(this->isPause)
        {
            mux.unlock();
            msleep(5);
            continue ;
        }

        if(!demux)
        {
            mux.unlock();
            msleep(2);
            continue;
        }

        //音视频同步
        if(vt && at)
        {
            pts = at->pts;
            vt->synpts = at->pts;
        }

        AVPacket *pkt = demux->Read();
        if(!pkt)
        {
            mux.unlock();
            msleep(2);
            continue;
        }

        if(demux->IsAudio(pkt))
        {
            if(at)//容错 如果不支持音频
                at->Push(pkt);
        }
        else
        {
            if(vt)
                vt->Push(pkt);
        }
        //cout << "Push(pkt);" << endl;
        mux.unlock();
        //msleep();
    }
}


//启动所有线程
void XDemuxThread::Start()
{
    mux.lock();
    qDebug()<<"XDemuxThread::Start()";
    //开启自己线程
    if(!demux) demux = new XDemux();
    if(!vt) vt = new XVideoThread();
    if(!at) at = new XAudioThread();

    QThread::start();
    if(vt) vt->start();
    if(at) at->start();

    mux.unlock();
}
//创建对象并打开
bool XDemuxThread::Open(const char *url, IVideoCall *call)
{
    if(url==0 || url[0] == '\0')return false;
    mux.lock();
	if (!demux) demux = new XDemux();
	if (!vt) vt = new XVideoThread();
	if (!at) at = new XAudioThread();
    //打开解封装
    bool ret = demux->Open(url);
    if(!ret)
    {
		mux.unlock();
        qDebug() << "demux->Open failed";
        return false;
    }

    //打开视频解码器和处理线程
    ret = vt->Open(demux->CopyVPara(), call, demux->width, demux->height);
    if(!ret)
    {
        ret = false;
        cout << "vt->Open faile" << endl;
    }

    //打开音频解码器和处理线程
    ret = at->Open(demux->CopyAPara(), demux->sampleRate, demux->channels);
    if(!ret)
    {
        ret = false;
        cout << "at->Open faile" << endl;
    }
    mux.unlock();
    totalMs = demux->totalMs;
    qDebug() << "XDemuxThread::Open :" << ret;

    return ret;
}
//清理缓冲资源
void XDemuxThread::Clear()
{
    mux.lock();

    if(demux) demux->Clear();
    if(vt) vt->Clear();
    if(at) at->Clear();

    mux.unlock();
}
//关闭线程 清理资源
void XDemuxThread::Close()
{
    isExit = true;
    wait();
    if(vt) vt->Close();
    if(at) at->Close();
    mux.lock();
    delete vt;
    vt = NULL;
    delete at;
    at = NULL;
    mux.unlock();
}

XDemuxThread::XDemuxThread()
{

}
XDemuxThread::~XDemuxThread()
{
    isExit = true;
    wait();
    qDebug()<<"XDemuxThread::~XDemuxThread()";
}
