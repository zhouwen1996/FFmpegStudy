#include <QCoreApplication>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QThread>
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QAudioFormat fmt;
    fmt.setSampleRate(44100);//样本率
    fmt.setSampleSize(16);  //样本大小 S16
    fmt.setChannelCount(2); //双通道
    fmt.setCodec("audio/pcm"); //设置pcm编码器
    fmt.setByteOrder(QAudioFormat::LittleEndian);//这是字节序
    fmt.setSampleType(QAudioFormat::UnSignedInt);//设置样本类型
    QAudioOutput *out = new QAudioOutput(fmt);//创建QAudioOutput
    QIODevice *io = out->start(); //开始播放

    int size = out->periodSize();//一个周期的大小，硬件设备一次读多大
    char *buf = new char[size];

    FILE *fp = fopen("out.pcm", "rb");
    while (!feof(fp))
    {
        //如果缓冲空间剩余内存不够size，硬件还没取走
        if (out->bytesFree() < size)
        {
            QThread::msleep(1);
            continue;
        }
        int len = fread(buf, 1, size, fp);
        if (len <= 0)break;
        io->write(buf, len);
    }
    fclose(fp);
    delete buf;
    buf = 0;

    return a.exec();
}
