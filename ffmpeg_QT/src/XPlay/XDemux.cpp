#include "XDemux.h"
#include <iostream>

using namespace std;

#ifdef __cplusplus
extern "C"
{
#endif // !__cplusplus

#include "libavformat/avformat.h"
#include <libavcodec/avcodec.h>
#ifdef __cplusplus
}
#endif // !__cplusplus

#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"avcodec.lib")

static double r2d(AVRational r)
{
    return r.den == 0 ? 0 : (double)r.num / (double)r.den;
}

bool XDemux::Open(const char *url)
{
    Close();//每次打开先关闭 注意不能放到锁里面，形成死锁
    //参数设置
    AVDictionary *opts = NULL;
    //设置rtsp流已tcp协议打开
    av_dict_set(&opts, "rtsp_transport", "tcp", 0);
    //网络延时时间
    av_dict_set(&opts, "max_delay", "500", 0);

    mux.lock();
    int re = avformat_open_input(
        &ic,
        url,
        0,  // 0表示自动选择解封器
        &opts //参数设置，比如rtsp的延时时间
    );
    if (re != 0)
    {
        mux.unlock();
        char buf[1024] = { 0 };
        av_strerror(re, buf, sizeof(buf) - 1);
        cout << "open " << url << " failed! :" << buf << endl;
        return false;
    }
    cout << "open " << url << " success! " << endl;

    //获取流信息
    re = avformat_find_stream_info(ic, 0);

    //总时长 毫秒
    totalMs = ic->duration / (AV_TIME_BASE / 1000);
    cout << "totalMs = " << totalMs << endl;
    //打印视频流详细信息
    av_dump_format(ic, 0, url, 0);


    //获取视频流
    videoStream = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    width = ic->streams[videoStream]->codecpar->width;
    height = ic->streams[videoStream]->codecpar->height;
    cout << videoStream << "=video information" << endl;
    cout << "codec_id = " << ic->streams[videoStream]->codecpar->codec_id << endl;
    cout << "format = " << ic->streams[videoStream]->codecpar->format << endl;
    cout << "width=" << ic->streams[videoStream]->codecpar->width << endl;
    cout << "height=" << ic->streams[videoStream]->codecpar->height << endl;
    cout << "video fps = " << r2d(ic->streams[videoStream]->avg_frame_rate) << endl;
    //获取音频流
    cout << "===================================================="<<endl;
    audioStream = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    sampleRate = ic->streams[audioStream]->codecpar->sample_rate;
    channels = ic->streams[audioStream]->codecpar->channels;
    cout << audioStream << "=audio information" << endl;
    cout << "codec_id = " << ic->streams[audioStream]->codecpar->codec_id << endl;
    cout << "format = " << ic->streams[audioStream]->codecpar->format << endl;
    cout << "sample_rate = " << ic->streams[audioStream]->codecpar->sample_rate << endl;
    cout << "channels = " << ic->streams[audioStream]->codecpar->channels << endl;
    cout << "frame_size = " << ic->streams[audioStream]->codecpar->frame_size << endl;

    mux.unlock();
    return true;
}
//只读视频 音频丢弃空间释放
AVPacket* XDemux::ReadVedio()
{
    mux.lock();
    if(!ic)//保证容错性
    {
        mux.unlock();//注意释放
        return 0;
    }
    mux.unlock();//注意释放
    AVPacket *pkt = NULL;
    for(int i = 0; i < 20; i++)//防止阻塞 只读20帧一定会有视频帧
    {
        pkt = Read();
        if(pkt->stream_index == videoStream)
        {
            break;
        }
        av_packet_free(&pkt);//其余帧都释放
    }
    return pkt;
}
//空间需要调用者释放，释放AVPacket对象空间和数据空间  av_packet_free
AVPacket* XDemux::Read()
{
    mux.lock();//同样要加互斥锁 多线程，打开read不同线程 如果打开ic失败了 这边还在使用就会崩溃

    if(!ic)//保证容错性
    {
        mux.unlock();//注意释放
        return 0;
    }
    //读取一帧数据
    //1、创建AVPacket对象
    AVPacket *pkt = av_packet_alloc();
    //2、读取一包并分配数据空间
    int re = av_read_frame(ic, pkt);
    if(re != 0)//读取失败
    {
        mux.unlock();
        av_packet_free(&pkt);//注意内存释放
        return 0;
    }
    //将pts转换为毫秒 时间单位统一 则音视频同步的时候好一些
    pkt->pts = pkt->pts*(1000*(r2d(ic->streams[pkt->stream_index]->time_base)));
    pkt->dts = pkt->dts*(1000*(r2d(ic->streams[pkt->stream_index]->time_base)));
    //cout << "pkt->pts = "<< pkt->pts << flush << endl;
    mux.unlock();
    return pkt;
}

//获取视频参数 返回的空气需要清理  注意一下AVCodecParameters中存在一个uint8_t *extradata;而外的数据空间
//使用 avcodec_parameters_free()释放
AVCodecParameters* XDemux::CopyVPara()
{
    //因为要使用到ic中的数据 则要上锁访问
    mux.lock();
    if(!ic)
    {
        mux.unlock();
        return NULL;
    }
    AVCodecParameters* pa = avcodec_parameters_alloc();
    int ret = avcodec_parameters_copy(pa, ic->streams[videoStream]->codecpar);
    if(ret >= 0)
    {
        mux.unlock();
        return pa;
    }
    mux.unlock();
    return NULL;
}

//获取音频参数
AVCodecParameters* XDemux::CopyAPara()
{
    //因为要使用到ic中的数据 则要上锁访问
    mux.lock();
    if(!ic)
    {
        mux.unlock();
        return NULL;
    }
    AVCodecParameters* pa = avcodec_parameters_alloc();
    int ret = avcodec_parameters_copy(pa, ic->streams[audioStream]->codecpar);
    if(ret >= 0)
    {
        mux.unlock();
        return pa;
    }
    mux.unlock();
    return NULL;
}

bool XDemux::Seek(double pos)
{
    mux.lock();
    if(!ic)
    {
        mux.unlock();
        return false;
    }
    //清理读取缓冲 因为都已经移动了那么就要重头开始 ，主要是针对网络传输怕捏包现象
    avformat_flush(ic);//不能在这里调用clear因为那里也要获取锁，这里引进上锁了会卡住死锁的
    long long seek_pos = 0;
    seek_pos = (double)ic->streams[videoStream]->duration*pos;
    //av_seek_frame注释可以知道首先time_base没有再以AV_TIME_BASE为基本单位
    //往后跳到关键帧，后续要跳到真正的seek帧在后面处理
    int ret = av_seek_frame(ic, videoStream, seek_pos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
     mux.unlock();
    if(ret >= 0)
    {
        return true;
    }
    return false;
}

bool XDemux::IsAudio(AVPacket *pkt)
{
    //每部都要做好容错
    if(!pkt) return false;
    if(pkt->stream_index == audioStream)
    {
        return true;
    }
    return false;
}

//清空读取缓存
void XDemux::Clear()
{
    mux.lock();
    if(!ic)
    {
        mux.unlock();
        return ;
    }
    //清理读取缓冲 因为都已经移动了那么就要重头开始 ，主要是针对网络传输怕捏包现象
    avformat_flush(ic);
    mux.unlock();
}

void XDemux::Close()
{
    mux.lock();
    if(!ic)
    {
        mux.unlock();
        return ;
    }

    avformat_close_input(&ic);//传入的地址进去的，因此会释放内存并置NULL，注意有的不会置NULL
    totalMs = 0;//初始化

    mux.unlock();
}

XDemux::XDemux()
{
    static bool isFirst = true;
    static std::mutex dmux;
    dmux.lock();
    if(isFirst)//保护线程安全并且初始化库只做一次
    {
        //初始化封装库
        av_register_all();

        //初始化网络库 （可以打开rtsp rtmp http 协议的流媒体视频）
        avformat_network_init();
		isFirst = false;
    }
    dmux.unlock();
}
XDemux::~XDemux()
{
}
