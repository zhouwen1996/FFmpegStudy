#ifndef IVIDEOCALL_H
#define IVIDEOCALL_H

struct AVFrame;
//创建接口，则以后用其他显示的时候 videoThread这个类是不用变的
class IVideoCall
{
public:
	virtual void Init(int width, int height) = 0;
	virtual void Repaint(AVFrame *frame) = 0;
};

#endif // IVIDEOCALL_H
