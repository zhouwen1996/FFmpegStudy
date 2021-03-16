#ifndef XDCODE_H
#define XDCODE_H

#include <mutex>
//头文件当中别引入命名空间，命名空间就是为了解决同名冲突，如果加入到头文件当中 然后别人多次引入这个头文件，命名空间就没有意义了

struct AVCodecParameters;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;
#include <mutex>

extern void XFreePacket(AVPacket **pkt);
extern void XFreeFrame(AVFrame **frame);
class XDcode
{
public:
    //标识音频还是视频
    bool isAudio = false;
    //打开解码器  并不管成功与否释放para空间
    virtual bool Open(AVCodecParameters *para);

    //解码相关的函数
    //发生到解码线程，不管成功与否都要释放pkt(包含两个空间对象和内部数据)
    virtual bool Send(AVPacket *pkt);

    //获取解码后数据 一次Send需要多次Recv 最后获取缓冲的数据需Send(NULL)再Recv多次
    //每次复制一份要由调用者释放 av_frame_free
    virtual AVFrame* Recv();

    //清理关闭
    virtual void Close();
    virtual void Clear();

    long long pts = 0;

    XDcode();
    virtual ~XDcode();

protected:
    AVCodecContext *codec = NULL;
    std::mutex mux;
};

#endif // XDCODE_H
