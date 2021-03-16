#include "XVideoWidget.h"
#include <QDebug>
#include <QTimer>

FILE *fp = NULL;

//定义一个宏表示数组
#define A_VER 3
#define T_VER 4
//自动加双引号 定义常量串都可以使用
#define GET_STR(x) #x
//顶点着色器的shader代码
const char *vString = GET_STR(
    attribute vec4 vertexIn;
    attribute vec2 textureIn;
    varying vec2 textureOut;//varying顶点和片元shader共享变量
    void main(void)
    {
        gl_Position = vertexIn;//传入的顶点坐标记录
        textureOut = textureIn;//传入的材质坐标保存到textureOut，从而传出去了
    }
);
//片元着色器的shader代码
const char *tString = GET_STR(
    varying vec2 textureOut;//刚刚顶点着色器算出来的坐标  共享的
    uniform sampler2D tex_y;//uniform是外部传入的变量
    uniform sampler2D tex_u;//sampler2D是一个2d图像
    uniform sampler2D tex_v;
    void main(void)
    {
    //420P是平面存储的，最后都是要转换为每个像素都是有yuv再方便转换
    //用的是灰度图的形式存储的
    //如安装那边硬解码出来的都是yuv420sp，uv是打包存在一起的，则不能使用这套shader代码了，要增加透明度存储方式。
        vec3 yuv;
        vec3 rgb;
        //根据那个坐标把材质坐标计算出来
        //传入材质，和坐标   返回材质当中的颜色rgb，但是用灰度图存储的，都是一样的。
        //三个材质就可以拼出一个yuv的数据
        yuv.x = texture2D(tex_y, textureOut).r;//获取材质当中的颜色
        yuv.y = texture2D(tex_u, textureOut).r - 0.5;//四舍五入
        yuv.z = texture2D(tex_v, textureOut).r - 0.5;
        //转换公式
        rgb = mat3(1.0, 1.0, 1.0,
            0.0, -0.39465, 2.03211,
            1.13983, -0.58060, 0.0) * yuv;
        gl_FragColor = vec4(rgb, 1.0);//转换成显示的颜色，再设置一下
    }

);
XVideoWidget::XVideoWidget(QWidget *parent)
    :QOpenGLWidget(parent)//初始化列表调用父类构造方法调用paint画出OpenGLWidget
{

    int a = 1;
}
XVideoWidget::~XVideoWidget()
{

}
//刷新初始化，每次移动都会调用一次，进行刷新
void XVideoWidget::paintGL()
{
    if(feof(fp))
    {
     fseek(fp, 0, SEEK_SET);
    }
    //读取帧数据
    fread(datas[0],1,width*height, fp);
    fread(datas[1], 1, width*height/4, fp);
    fread(datas[2], 1, width*height/4, fp);

    //在显卡中创建材质   并绑定到了0层渲染材质
    glActiveTexture(GL_TEXTURE0);//激活第0层
    glBindTexture(GL_TEXTURE_2D, texs[0]);//把0层绑定Y材质
    //修改材质(复制内存内存)
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED, GL_UNSIGNED_BYTE, datas[0]);
    //与shader uni变量关联起来
    glUniform1i(unis[0], 0);

    glActiveTexture(GL_TEXTURE0+1);//激活第1层
    glBindTexture(GL_TEXTURE_2D, texs[1]);//把1层绑定Y材质
    //修改材质(复制内存内存)
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width/2, height/2, GL_RED, GL_UNSIGNED_BYTE, datas[1]);
    //与shader uni变量关联起来
    glUniform1i(unis[1], 1);

    glActiveTexture(GL_TEXTURE0+2);//激活第0层
    glBindTexture(GL_TEXTURE_2D, texs[2]);//把0层绑定Y材质
    //修改材质(复制内存内存)
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width/2, height/2, GL_RED, GL_UNSIGNED_BYTE, datas[2]);
    //与shader uni变量关联起来
    glUniform1i(unis[2], 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);//画三角形 从0开始 四个顶点，

    qDebug() << "paintGL";
}

//初始化gl
void XVideoWidget::initializeGL()
{
    qDebug() << "initializeGL";

    //初始化 opengl(QOpenGLFunctions继承过来的函数)
    initializeOpenGLFunctions();

    //program(opengl和qt都提供了)加载shader脚本文件 顶点shader 片元shader
    // qt 提供的 QGLShaderProgram
    qDebug() << "addShaderFromSourceCode :" <<program.addShaderFromSourceCode(QGLShader::Fragment,tString);//在源代码中添加好一些，从文件怕有泄露
    qDebug() << "addShaderFromSourceCode :" << program.addShaderFromSourceCode(QGLShader::Vertex,vString);

    //设置顶点坐标的变量
    program.bindAttributeLocation("vertexIn", A_VER);//这个vertexIn变量对应的本地位置 vertexIn是顶点shader定义的 3的位置

    //材质坐标
    program.bindAttributeLocation("textureIn",T_VER);//下标四，等下往4的位置存

    //编译shader
    qDebug() << "program.link():" << program.link();
    //绑定shader
    qDebug() << "program.link():" << program.bind();

    //传递顶点和材质坐标,
    //这个坐标是在这个函数中时候，之后draw的时候还要使用的,只传入二维
    static const GLfloat ver[] = {
        -1.0f,-1.0f,
        1.0f,-1.0f,
        -1.0f, 1.0f,
        1.0f,1.0f
    };
    static const GLfloat tex[] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f
    };

    //将坐标写入GL当中
    glVertexAttribPointer(A_VER,//索引地址
                          2,//表示一点顶点几个元素，
                          GL_FLOAT,//存放的类型，浮点数
                          0,
                          0,
                          ver //顶点地址
                          );
    //顶点坐标生效
    glEnableVertexAttribArray(A_VER);

    //材质  就是shader准备数据
    glVertexAttribPointer(T_VER,2, GL_FLOAT, 0,0,tex);
    glEnableVertexAttribArray(T_VER);

    //从shader当中获取材质
    unis[0] = program.uniformLocation("tex_y");
    unis[1] = program.uniformLocation("tex_u");
    unis[2] = program.uniformLocation("tex_v");

    //创建材质
    glGenTextures(3,texs);//创建三个对象
    //分别对每个材质进行设置

    //Y
    glBindTexture(GL_TEXTURE_2D, texs[0]);//绑定2D
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);//设置属性  放大 线性差值的属性
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);//设置属性  放大 线性差值的属性
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED,width,height,0,GL_RED,GL_UNSIGNED_BYTE,0);//创建材质显卡的空间 ，与内存的空间是不一样的

    //width/2 height/2根据yuv420的特性来的 uv是y的四分之一
    //U
    glBindTexture(GL_TEXTURE_2D, texs[1]);//绑定2D
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);//设置属性  放大 线性差值的属性
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);//设置属性  放大 线性差值的属性
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED,width/2,height/2,0,GL_RED,GL_UNSIGNED_BYTE,0);//创建材质显卡的空间 ，与内存的空间是不一样的

    //V
    glBindTexture(GL_TEXTURE_2D, texs[2]);//绑定2D
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);//设置属性  放大 线性差值的属性
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);//设置属性  放大 线性差值的属性
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED,width/2,height/2,0,GL_RED,GL_UNSIGNED_BYTE,0);//创建材质显卡的空间 ，与内存的空间是不一样的

    //分配材质的内存空间
    datas[0] = new unsigned char[width*height];
    datas[1] = new unsigned char[width*height/4];
    datas[2] = new unsigned char[width*height/4];

    //读取材质并显示
    //再通过opengl接口，实现内存空间和显卡空间的转换
    fp = fopen("output240X128.yuv", "rb");
    if(!fp)
    {
        qDebug() << "fopen error";
    }

    //启动定时器
    QTimer *ti = new QTimer(this);
    connect(ti, SIGNAL(timeout()), this, SLOT(update()));//定时器刷新到update()里面取
    ti->start(40);
}

//窗口大小变化
void XVideoWidget::resizeGL(int width,int height)
{
    qDebug() << "resizeGL width = " << width << "  height = " << height;
}
