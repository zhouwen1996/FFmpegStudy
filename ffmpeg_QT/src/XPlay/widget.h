#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTimerEvent>
namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

    //重载定时器函数的方法 滑动条进行显示
    void timerEvent(QTimerEvent *e);

    //重载窗口尺寸大小变化
    void resizeEvent(QResizeEvent *e);

    //双击全屏
    void mouseDoubleClickEvent(QMouseEvent *e);

    //设置播放暂停
    void SetPause(bool isPause);

    bool isSliderPress = false;

private slots:
    void on_pushButton_clicked();

    void on_isplay_clicked();

    void on_playPos_sliderPressed();

    void on_playPos_sliderReleased();

private:
    Ui::Widget *ui;
};

#endif // WIDGET_H
