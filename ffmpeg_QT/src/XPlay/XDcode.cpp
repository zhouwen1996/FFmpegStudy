#include "XDcode.h"
#ifdef __cplusplus
extern "C"
{
#endif // !__cplusplus

#include <libavcodec/avcodec.h>

#ifdef __cplusplus
}
#endif // !__cplusplus

#include <iostream>

using namespace std;

void XFreePacket(AVPacket **pkt)
{
    if(!pkt || !(*pkt)) return ;
    av_packet_free(pkt);
}
void XFreeFrame(AVFrame **frame)
{
    if(!frame || !(*frame)) return ;
    av_frame_free(frame);
}
//打开解码器  并不管成功与否释放para空间，内存注释非常有必要的
bool XDcode::Open(AVCodecParameters *para)
{
    if(!para) return false;
    Close();
    //找到解码器 根据AVCodecParameters的ID
    AVCodec *vcodec = avcodec_find_decoder(para->codec_id);
    if (!vcodec)
    {
        avcodec_parameters_free(&para);//不管成功与否都要释放空间
        cout << "can't find the codec id " << para->codec_id;
        return false;
    }
    cout << "find the AVCodec " << para->codec_id << endl;

    mux.lock();
    codec = avcodec_alloc_context3(vcodec);

    //配置解码器上下文参数
    avcodec_parameters_to_context(codec, para);
    avcodec_parameters_free(&para);
    //八线程解码
    codec->thread_count = 8;

    ///打开解码器上下文
    int re = avcodec_open2(codec, 0, 0);
    if (re != 0)
    {
        avcodec_free_context(&codec);//也将para也释放了
        mux.unlock();
        //avcodec_parameters_free(&para);
        char buf[1024] = { 0 };
        av_strerror(re, buf, sizeof(buf) - 1);
        cout << "avcodec_open2  failed! :" << buf << endl;
        return false;
    }
    mux.unlock();
    cout << "codec avcodec_open2 success!" << endl;
    return true;
}

//发生到解码线程，不管成功与否都要释放pkt(包含两个空间对象和内部数据)
bool XDcode::Send(AVPacket *pkt)
{
    //容错处理
    if(!pkt || pkt->size <= 0 || !pkt->data) return false;

    mux.lock();
    if(!codec)
    {
        mux.unlock();
        return false;
    }
    int ret = avcodec_send_packet(codec, pkt);
    av_packet_free(&pkt);//这边释放有限定条件 pkt在解封装那边申请的方式要是av_packet_alloc的
    if(ret != 0) return false;
    mux.unlock();
    return true;
}

//获取解码后数据 一次Send需要多次Recv 最后获取缓冲的数据需Send(NULL)再Recv多次
//每次复制一份要由调用者释放 av_frame_free
AVFrame* XDcode::Recv()
{
    mux.lock();
    if(!codec)
    {
        mux.unlock();
        return NULL;
    }

    //采用内部创建空间复制的方法
    AVFrame *frame = av_frame_alloc();
    if(frame == NULL)
    {
        mux.unlock();
        return NULL;
    }
    int ret = avcodec_receive_frame(codec, frame);//每次都创建空间 传入进去 最后会被内容传入里面
    if(ret != 0)
    {
        mux.unlock();//锁是针对codec的
        av_frame_free(&frame);
        return NULL;
    }
    //cout << "-----------Frame Linesize = " << frame->linesize[0] << endl;
    pts = frame->pts;
    mux.unlock();//锁是针对codec的
    return frame;
}

void XDcode::Close()
{
    mux.lock();
    if(codec)
    {
        avcodec_close(codec);
        avcodec_free_context(&codec);
    }
    pts = 0;
    mux.unlock();
}
void XDcode::Clear()
{
    mux.lock();

    if(codec)
    {
        avcodec_flush_buffers(codec);
    }

    mux.unlock();
}

XDcode::XDcode()
{
}
XDcode::~XDcode()
{

}
