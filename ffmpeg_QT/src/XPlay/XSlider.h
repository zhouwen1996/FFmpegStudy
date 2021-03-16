#ifndef XSLIDER_H
#define XSLIDER_H

#include <QObject>
#include <QSlider>
#include <QWidget>
#include <QMouseEvent>
//注意这个重载滑动条的类要与ui界面的滑动条绑定 提升为的方式
class XSlider : public QSlider
{
    Q_OBJECT
public:
    explicit XSlider(QWidget *parent = nullptr);
    virtual ~XSlider();
    //重载滑动进度条的鼠标点击事件的bug  每次点击只能往后移动一下，不能移动到点击位置
    void mousePressEvent(QMouseEvent *e);
signals:

public slots:
};

#endif // XSLIDER_H
