#ifndef XAUDIOPLAY_H
#define XAUDIOPLAY_H


class XAudioPlay
{
public:
    //工程模式
    static XAudioPlay *Get();
    //打开时需要的参数都通过成员变量来传递
    int sampleRate = 44100;
    int samleSize = 16;//固定的重采样出来的应该是16位的
    int channels = 2;
    //打开音频播放
    virtual bool Open() = 0;//纯虚函数 在子类中做实现
    //关闭
    virtual bool Close() = 0;
    //清理
    virtual void Clear() = 0;

    //返回缓冲中还没有播放的时间 毫秒单位
    virtual  long long GetNoPlayMs() = 0;

    //播放音频
    virtual bool Write(const unsigned char *data, int datasize) = 0;
    virtual int GetFree() = 0;

    //设置音频播放的缓冲暂停
    virtual void SetPause(bool isPause) = 0;

	XAudioPlay();
	virtual ~XAudioPlay();
};

#endif // XAUDIOPLAY_H
