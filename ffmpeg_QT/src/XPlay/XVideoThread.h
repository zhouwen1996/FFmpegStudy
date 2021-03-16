#ifndef XVIDEOTHREAD_H
#define XVIDEOTHREAD_H
#include <QThread>
#include <mutex>
#include <list>
#include "IVideoCall.h"
#include "XDecodeThread.h"

//音频相关的操作都封装到这个类中
struct AVCodecParameters;
class XAudioPlay;
class XResample;
class XVideoWidget;

//解码和显示视频
class XVideoThread :public XDecodeThread
{
public:
    XVideoThread();
    virtual ~XVideoThread();

    //打开音频  不管成功与否都清理para
	virtual bool Open(AVCodecParameters *para,IVideoCall *call,int width,int height);

    //解码pts，如果接收到的解码数据pts大于seekPts return true 并显示
    virtual bool RepaintPts(AVPacket *Pkt, long long seekPts);

    //读取
    void run();

    void SetPause(bool isPause);
    bool isPause = false;


    //同步时间由外部传入
    long long synpts = 0;

protected:
    std::mutex vmux;

    IVideoCall *call = NULL;
};

#endif // XVIDEOTHREAD_H
