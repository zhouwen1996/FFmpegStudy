#ifndef XVIDEOWIDGET_H
#define XVIDEOWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QGLShaderProgram>
/*
 * //引入这个需要在pro文件添加模块
 * QT += opengl
 * QT += openglextensions
*/

class XVideoWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    //注意Q_OBJECT这个变量如果没有添加进来则槽函数无法调用
    Q_OBJECT

public:
    XVideoWidget(QWidget *parent);
    ~XVideoWidget();
    //从shader的yuv变量地址
    GLuint unis[3] = {0};
    //opengl的texture地址
    GLuint texs[3] = {0};

    //材质的内存空间
    unsigned char *datas[3] = {0};

    //宽高度
    int width = 240;
    int height = 128;
protected:
     //重载那三个函数
    //刷新初始化
    void paintGL();//具体绘制在这里面实现

    //初始化gl
    void initializeGL();//初始化

    //窗口大小变化
    void resizeGL(int width,int height);//当窗口发生变化的时候调用，这个函数


private:
    QGLShaderProgram program;

};

#endif // XVIDEOWIDGET_H
