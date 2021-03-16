#ifndef XDECODETHREAD_H
#define XDECODETHREAD_H
#include <QThread>
#include <mutex>
#include <list>
//音频相关的操作都封装到这个类中
struct AVPacket;
class XDcode;

class XDecodeThread :public QThread
{
public:
    XDecodeThread();
    virtual ~XDecodeThread();
    //产生
    virtual void Push(AVPacket *pkt);

    //取出一帧数据并入栈，没有贼返回NULL
    virtual AVPacket *Pop();

    //清理队列
    virtual void Clear();

    //关闭清理  要将XDcode关闭
    virtual void Close();

    //最大队列
    int maxList = 500;
    //线程退出
    bool isExit = false;

protected:
    std::list <AVPacket *> packts;
    std::mutex mux;

    XDcode *decode = 0;
};

#endif // XDECODETHREAD_H
