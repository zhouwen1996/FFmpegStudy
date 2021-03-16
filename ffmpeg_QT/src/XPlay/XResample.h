#ifndef XRESAMPLE_H
#define XRESAMPLE_H

#include <mutex>

struct AVCodecParameters;
struct SwrContext;
struct AVFrame;
class XResample
{
public:
    //输出参数和输入参数一致 除了采样格式 由FLAT输出为S16
    virtual bool Open(AVCodecParameters *para, bool isClearPara = false);

    //重采样动作  返回重采样后大小  不管成功与否都释放AVFrame indata
    virtual int Resample(AVFrame *indata, unsigned char *data);

    //关闭
    virtual void Close();
    XResample();
    ~XResample();

    //设置重采样输出的格式  1=AV_SAMPLE_FMT_S16
    int outFormat = 1;
protected:
    std::mutex mux;
    //音频重采样
    SwrContext *actx = NULL;
};

#endif // XRESAMPLE_H
