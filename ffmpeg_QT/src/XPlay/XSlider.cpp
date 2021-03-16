#include "XSlider.h"
#include <QDebug>
XSlider::XSlider(QWidget  *parent) : QSlider(parent)
{

}
XSlider::~XSlider()
{

}
void XSlider::mousePressEvent(QMouseEvent *e)
{
    double pos = (double)e->pos().x() / (double)width();
    setValue(pos * this->maximum());
    //qDebug()<<"XSlider::mousePressEvent";
    //原有事件处理
    //QSlider::mousePressEvent(e);//不能移动到最后
    QSlider::sliderReleased();
}
