#include "widget.h"
#include "ui_widget.h"
#include <QFileDialog>
#include <QString>
#include <QDebug>
#include <QMessageBox>
#include "XDemuxThread.h"

static XDemuxThread dt;

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    dt.Start();
    startTimer(40);
}

Widget::~Widget()
{
    dt.Close();
    delete ui;
}
//重载窗口尺寸大小变化
void Widget::resizeEvent(QResizeEvent *e)
{
    ui->playPos->move(0, this->height()-50);
    ui->playPos->resize(this->width()-50, ui->playPos->height());
    ui->pushButton->move(100, this->height()-150);
    ui->isplay->move(ui->pushButton->x()+ui->pushButton->width()+10, ui->pushButton->y());
    ui->vedio->resize(this->size());
}
void Widget::SetPause(bool isPause)
{
    if(isPause)
        ui->isplay->setText("Stop");
    else
        ui->isplay->setText("Play");
}
//双击全屏
void Widget::mouseDoubleClickEvent(QMouseEvent *e)
{
    if(isFullScreen())
        this->showNormal();
    else
        this->showFullScreen();
}

//滑动条的按下与松开
void Widget::on_playPos_sliderPressed()
{
    isSliderPress = true;
}

void Widget::on_playPos_sliderReleased()
{
    isSliderPress = false;
    double pos = 0.0;
    pos = (double)ui->playPos->value()/(double)ui->playPos->maximum();
    dt.Seek(pos);
    int v = ui->playPos->maximum() * pos;
    ui->playPos->setValue(v);
}

//重载定时器函数的方法 滑动条进行显示
void Widget::timerEvent(QTimerEvent *e)
{
    //如果滑动条按下的时候就不在计时
    if(isSliderPress) return ;
    long long total = dt.totalMs;
    //播放的pts
    if(total > 0)
    {
        double pos = (double)dt.pts/(double)total;
        int v = ui->playPos->maximum() * pos;
        ui->playPos->setValue(v);
    }

}
//打开文件
void Widget::on_pushButton_clicked()
{
    //选择文件
    QString name =  QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("选择视频文件"));
    qDebug() << name ;
    if(name.isEmpty()) return ;
    this->setWindowTitle(name);
    //打开
    if(!dt.Open(name.toLocal8Bit(), ui->vedio))
    {
        QMessageBox::information(0,"error","open file faile");
        return ;
    }
    SetPause(dt.isPause);
}

void Widget::on_isplay_clicked()
{
    bool isPause = !dt.isPause;
    SetPause(isPause);
    dt.SetPause(isPause);
    qDebug()<<"on_isplay_clicked = " << isPause;
}

