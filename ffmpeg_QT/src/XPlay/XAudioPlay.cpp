#include "XAudioPlay.h"
#include <QAudioFormat>
#include <QAudioOutput>
#include <mutex>
//建立一个播放类，基于QT的，关于QT的播放都在这个文件当中
class CXAudiaoPlay : public XAudioPlay
{
public:
    QAudioOutput *output = NULL;
    QIODevice *io = NULL;
    std::mutex mux;//因为打开关闭不一定在一个线程

    virtual long long GetNoPlayMs()
    {
        mux.lock();

        if(!output)
        {
            mux.unlock();
            return 0;
        }

        long long pts;
        //还未播放的字节数
        double size = output->bufferSize() - output->bytesFree();
        //1秒音频字节大小
        double secSize = sampleRate*(samleSize/8)*channels;
        if(secSize <= 0)
		{
			pts = 0;
		}
        else
        {
            pts = (size/secSize)*1000;
        }
        mux.unlock();
        return pts;
    }


    virtual bool Open()
    {
        Close();//每次打开前进行关闭
        QAudioFormat fmt;
        fmt.setSampleRate(sampleRate);//样本率
        fmt.setSampleSize(samleSize);  //样本大小 S16
        fmt.setChannelCount(channels); //双通道
        fmt.setCodec("audio/pcm"); //设置pcm编码器
        fmt.setByteOrder(QAudioFormat::LittleEndian);//这是字节序
        fmt.setSampleType(QAudioFormat::UnSignedInt);//设置样本类型
        mux.lock();
        output = new QAudioOutput(fmt);//创建QAudioOutput
        io = output->start(); //开始播放
        mux.unlock();
        if(io)
            return true;
        return false;
    }
    //清理
    virtual void Clear()
    {
        mux.lock();
        if(io)
        {
            io->reset();
        }
        mux.unlock();
        return ;
    }
    virtual bool Close()
    {
        mux.lock();
        if(io)
        {
            io->close();
            io = NULL;//由谁打开就由谁关闭
        }
        if(output)
        {
            output->stop();
            delete output;
            output = NULL;
        }
        mux.unlock();
        return true;
    }
    //设置音频播放的缓冲暂停
    virtual void SetPause(bool isPause)
    {

        mux.lock();

        if(!output)
        {
            mux.unlock();
            return ;
        }
        if(isPause)
        {
            output->suspend();//output设备挂起
        }
        else
        {
            output->resume();
        }
        mux.unlock();
    }
    virtual bool Write(const unsigned char *data, int datasize)
    {
        if(!data || datasize <= 0)return false;
        mux.lock();

        if(!output || !io)
        {
            mux.unlock();
            return  false;
        }
        int size = io->write((char *)data, datasize);
        if(datasize != size)
        {
            mux.unlock();
            return false;
        }
        mux.unlock();
        return true;
    }
    virtual int GetFree(){
        mux.lock();

        if(!output)
        {
            mux.unlock();
            return  0;
        }
        int free = output->bytesFree();
        mux.unlock();
        return free;
    }
};
XAudioPlay::XAudioPlay()
{

}
XAudioPlay::~XAudioPlay()
{

}
XAudioPlay *XAudioPlay::Get()
{
    static CXAudiaoPlay play;
    return &play;
}
