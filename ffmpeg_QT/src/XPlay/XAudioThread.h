#ifndef XAUDIOTHREAD_H
#define XAUDIOTHREAD_H
#include <QThread>
#include <mutex>
#include <list>
#include "XDecodeThread.h"
//音频相关的操作都封装到这个类中
struct AVCodecParameters;
class XAudioPlay;
class XResample;

class XAudioThread : public XDecodeThread
{
public:
    //打开音频  不管成功与否都清理para
    virtual bool Open(AVCodecParameters *para,int sampleRate, int channels);

    //停止线程 清理资源
    virtual void Close();

    //音频播放有两部分缓冲 要重载Clear
    virtual void Clear();

    //读取
    void run();

    void SetPause(bool isPause);
    bool isPause = false;

    XAudioThread();
    virtual ~XAudioThread();

    //当前音频播放的pts
    long long pts = 0;

protected:
    std::mutex amux;
    XAudioPlay *ap = 0;
    XResample *res = 0;
};

#endif // XAUDIOTHREAD_H
