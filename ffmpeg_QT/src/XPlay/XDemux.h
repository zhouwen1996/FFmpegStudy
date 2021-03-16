#ifndef XDEMUX_H
#define XDEMUX_H

#include <mutex>

//前置声明就可以了
struct AVFormatContext;
struct AVPacket;
struct AVCodecParameters;
class XDemux
{
public:
    //打开媒体文件或流媒体文件如rtsp
    virtual bool Open(const char *url);//使用虚函数在继承中实现

    //read属于高频操作 要注意内存的申请释放 结合多线程交替使用的问题，引用计数+1
    //空间需要调用者释放，释放AVPacket对象空间和数据空间  av_packet_free
    virtual AVPacket* Read();

    //只读视频 音频丢弃空间释放
    virtual AVPacket* ReadVedio();

    //获取视频参数 返回的空气需要清理  注意一下AVCodecParameters中存在一个uint8_t *extradata;而外的数据空间
    //则要是有配套的赋值函数 涉及深拷贝 avcodec_parameters_copy
    virtual AVCodecParameters* CopyVPara();

    //获取音频参数
    virtual AVCodecParameters* CopyAPara();

    //seek 位置 pos0.0-1.0 类似百分比的值
    virtual bool Seek(double pos);

    //清空读取缓存
    virtual void Clear();
    virtual void Close();

    //新增一个接口判断是音频还是视频
    virtual bool IsAudio(AVPacket *pkt);

    XDemux();
    virtual ~XDemux();

    //媒体总时长 毫秒
    int totalMs = 0;
    //宽高度
    int width = 0;
    int height = 0;
    //音频相关
    int sampleRate = 0;
    int channels = 0;


protected:
    //音视频索引，读取时区分音视频
    int videoStream = 0;
    int audioStream = 1;
    std::mutex mux;//防止多线程open的时候互斥问题 锁要尽晚开启尽早释放
    //解封装上下文
    AVFormatContext *ic = NULL;
};

#endif // XDEMUX_H
