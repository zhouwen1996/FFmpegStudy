#include "XResample.h"
#ifdef __cplusplus
extern "C"
{
#endif // !__cplusplus

#include <libswresample/swresample.h>
#include <libavcodec/avcodec.h>
#ifdef __cplusplus
}
#endif // !__cplusplus
#include <iostream>
using namespace std;

#pragma comment(lib,"swresample.lib")

XResample::XResample()
{

}
XResample::~XResample()
{

}
void XResample::Close()
{
    mux.lock();
    if(actx)
    {
        swr_free(&actx);
    }
    mux.unlock();
}

//输出参数和输入参数一致 除了采样格式 由FLAT输出为S16 会释放XResample
bool XResample::Open(AVCodecParameters *para, bool isClearPara)
{
    if(!para) return false;

    mux.lock();

    //音频重采样 上下文初始化
    //通过返回值方式创建内存，就不需要像前面上下文一样传入双重指针了
    actx = swr_alloc_set_opts(
        actx,
        av_get_default_channel_layout(2),//输出格式 2通道
        (AVSampleFormat)outFormat,//输出的样本格式  16位两个字节表示
        para->sample_rate, //输出采样率
        av_get_default_channel_layout(para->channels),//输入格式
        (AVSampleFormat)para->format,
        para->sample_rate,
        0,0
    );
    //释放内存
    if(isClearPara) avcodec_parameters_free(&para);

    //注意要先设置属性，再初始化，就是要先创建内存
    int re = swr_init(actx);
    mux.unlock();
    if (re != 0)
    {
        char buf[1024] = { 0 };
        av_strerror(re, buf, sizeof(buf) - 1);
        cout << "swr_init  failed! :" << buf << endl;
        return false;
    }
    //unsigned char *pcm = NULL;//代码处理音视频 要定义位无符号类型
    return true;
}
//重采样动作  返回重采样后大小 不管成功与否都释放AVFrame indata
int XResample::Resample(AVFrame *indata, unsigned char *d)
{
    if(!indata) return 0;
    if(!d)
    {
        av_frame_free(&indata);
        return 0;
    }
    uint8_t *data[2] = {0};
    data[0] = (uint8_t *)d;
    //返回单通道样本数的数量
    int len = swr_convert(
        actx,
        data,indata->nb_samples,//输出数据的存放地址，样本数量
        (const uint8_t**)indata->data,indata->nb_samples//输入数据的存放地址，样本数量
    );
    //cout << "swr_convert = " << len << endl;

    if(len <= 0)
    {
        av_frame_free(&indata);
        return len;
    }

    //单样本设为s16 2个字节
    int outsize =  len*indata->channels*av_get_bytes_per_sample((AVSampleFormat)outFormat);
    av_frame_free(&indata);
    return outsize;
}
