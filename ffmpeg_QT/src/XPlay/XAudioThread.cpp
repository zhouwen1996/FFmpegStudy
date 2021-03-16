#include "XAudioThread.h"
#include "XDcode.h"
#include "XAudioPlay.h"
#include "XResample.h"
#include <iostream>
#include <QDebug>
using namespace std;
#ifdef __cplusplus
extern "C"
{
#endif // !__cplusplus

#include <libavcodec/avcodec.h>
#ifdef __cplusplus
}
#endif // !__cplusplus

//音频播放有两部分缓冲 要重载Clear
void XAudioThread::Clear()
{
    XDecodeThread::Clear();
    mux.lock();

    if(ap) ap->Clear();

    mux.unlock();
}

void XAudioThread::Close()
{
    XDecodeThread::Close();
    if(res)
    {
        res->Close();
        amux.lock();
        delete res;
        res = NULL;
        amux.unlock();
    }

    if(ap)
    {
        ap->Close();
        amux.lock();
        ap = NULL;
        amux.unlock();
    }

}
bool XAudioThread::Open(AVCodecParameters *para,int sampleRate, int channels)
{
    if(!para) return false;

    Clear();

    amux.lock();
    pts = 0;

    bool ret = true;
    if(!res->Open(para, false))
    {
        cout << "XResample Open faile" << endl;
        ret = false;
    }
    ap->sampleRate = sampleRate;
    ap->channels = channels;
    if(!ap->Open())
    {
        cout << "XResample Open faile" << endl;
        ret = false;
    }
    if(!decode->Open(para))//参数在这里释放
    {
        cout << "audio XDcode Open faile" << endl;
        ret = false;
    }

    amux.unlock();
    qDebug() << "XAudioThread::Open Open = " << ret;
    return ret;
}
//设置音频线程的缓冲暂停
void XAudioThread::SetPause(bool isPause)
{
    //amux.lock();  不能要锁 因为视频最多缓冲一帧 音频存储很大的缓冲

    //本线程的缓冲暂停
    this->isPause = isPause;

    //设置音频播放设备的缓冲暂停
    if(ap)
        ap->SetPause(isPause);

    //amux.unlock();
}

//线程函数
void XAudioThread::run()
{
    qDebug()<<"XAudioThread::run()";
    //申请重采样输出数据内存
    unsigned char *pcm = new unsigned char[1024*1024*50];
    while (!isExit) {
        amux.lock();

        if(isPause)
        {
            amux.unlock();
            msleep(5);
            continue;
        }

        //取数据
        AVPacket *pkt = Pop();
        bool ret = decode->Send(pkt);
        if(!ret)
        {
            amux.unlock();
            msleep(1);
            continue;
        }
        //av_packet_free(&pkt);

        //一次Send多次Recv
        while(!isExit)
        {
            AVFrame *frame = decode->Recv();
            if(!frame) break;

            //解码的显示时间-播放器还未播放的pts
            pts = decode->pts - ap->GetNoPlayMs();

            //重采样
            int size = res->Resample(frame, pcm);
            //av_frame_free(&frame);
            //播放音频
            while(!isExit)
            {
                if(size <= 0) break;
                //缓冲位播放完，空间不够
                if(ap->GetFree() < size || isPause)
                {
                    msleep(1);
                    continue;
                }
                ap->Write(pcm,size);
				break;
            }
        }

        amux.unlock();
    }
	delete pcm;
}
XAudioThread::XAudioThread()
{
    if(!res) res = new XResample();
    if(!ap) ap = XAudioPlay::Get();
}
XAudioThread::~XAudioThread()
{

}
