#include "XDecodeThread.h"
#include "XDcode.h"



XDecodeThread::XDecodeThread()
{
    //打开解码器
    if(!decode) decode = new XDcode();
}
XDecodeThread::~XDecodeThread()
{
    //等待线程退出
    isExit = true;
    wait();
}

//取出一帧数据并入栈，没有贼返回NULL
AVPacket *XDecodeThread::Pop()
{
    mux.lock();
    if(packts.empty())
    {
        mux.unlock();
        return NULL;
    }
    AVPacket *pkt = packts.front();
    packts.pop_front();
    mux.unlock();
    return pkt;
}
//产生
void XDecodeThread::Push(AVPacket *pkt)
{
    if(!pkt) return ;

    //阻塞
    while(!isExit)
    {
        mux.lock();
        if(packts.size() < maxList)
        {
            packts.push_back(pkt);
            mux.unlock();
            break;
        }
        mux.unlock();
        msleep(1);
    }
}
//清理队列
void XDecodeThread::Clear()
{
    mux.lock();
    decode->Clear();
    while (!packts.empty()) {
        AVPacket *pkt = packts.front();
        XFreePacket(&pkt);
        packts.pop_front();
    }
    mux.unlock();
}

//清理资源 停止线程
void XDecodeThread::Close()
{
    Clear();

    //等待线程退出
    isExit = true;
    wait();
    decode->Close();

    mux.lock();
    delete decode;
    decode = NULL;
    mux.unlock();
}
