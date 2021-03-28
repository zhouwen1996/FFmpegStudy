#include <iostream>

using namespace std;

#ifdef __cplusplus
extern "C"
{
#endif // !__cplusplus

#include "SDL.h"
//#include "SDL_main.h"

#ifdef __cplusplus
}
#endif // !__cplusplus

#pragma comment(lib,"SDL2.lib")
#pragma comment(lib,"SDL2main.lib")
#pragma comment(lib,"SDL2test.lib")
//https://www.freesion.com/article/63721074585/  
/*
解决办法：
包含库的编译器版本低于当前编译版本，需要将包含库源码用vs2017重新编译，由于没有包含库的源码，此路不通。
然后查到说是stdin, stderr, stdout 这几个函数vs2015和以前的定义得不一样，所以报错。
解决方法呢，就是使用{*stdin,*stdout,*stderr}数组自己定义__iob_func()
*/
#pragma comment(lib,"legacy_stdio_definitions.lib")
extern "C" { FILE __iob_func[3] = { *stdin,*stdout,*stderr }; }
/*
	这时，其实是main函数定义与sdl库里的不一样，比如：
	int main()
	这时编译时，就会出现上面的出错。需要修改为这样：
	int main(int      argc, char    *argv[])

就没有这个出错了。
*/
//int main()
int main(int      argc, char    *argv[])
{
	//SDL初始化
	if (SDL_Init(SDL_INIT_VIDEO))
	{
		cout << "SDL_Init error" << endl;
	}
	else
	{
		cout << "SDL_Init success" << endl;
	}

	/*SDL创建windows窗口  
	extern DECLSPEC SDL_Window * SDLCALL SDL_CreateWindow(const char *title,
	int x, int y, int w,
	int h, Uint32 flags);
	标题 坐标 宽高 属性
	SDL_WINDOW_SHOWN  表示创建之后马上弹出显示
	*/
	SDL_Window *win = NULL;
	win = SDL_CreateWindow("Hello world!" ,100, 100, 640, 480, SDL_WINDOW_SHOWN);
	if (win == NULL)
	{
		cout << "SDL_CreateWindow error" << endl;
		return -1;
	}

	/*
	创建渲染器 需指定我们用来绘制的窗口win
	extern DECLSPEC SDL_Renderer * SDLCALL SDL_CreateRenderer(SDL_Window * window,
	int index, Uint32 flags);
	指定渲染器绑定的窗口 指定可选用的显卡驱动 -1表示sdl自动选择 选择额外属性
	SDL_RENDERER_ACCELERATED  表示使用硬件加速 使用显卡  
	SDL_RENDERER_PRESENTVSYNC  表示使用显示器的刷新率来更新画面
	*/
	SDL_Renderer *ren = NULL;
	ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (ren == NULL)
	{
		cout << "SDL_CreateRenderer error" << endl;
		return -1;
	}

	//画图  中间采用SDL_Surface来加载 加速硬件绘制
	//将图片加载到SDL_Surface中
	SDL_Surface *bmp = NULL;
	bmp = SDL_LoadBMP("hello.bmp");
	if (bmp == NULL)
	{
		cout << "SDL_LoadBMP error" << endl;
		return -1;
	}
	//将SDL_Surface传递给SDL_Texture
	SDL_Texture * tex = NULL;
	tex = SDL_CreateTextureFromSurface(ren, bmp);
	SDL_FreeSurface(bmp);

	//将SDL_Texture画到SDL_Renderer上
	SDL_RenderClear(ren);//先清除
	/*
	//两个NULL分别是第一个NULL是一个指向源矩形的指针，从图像上裁剪下的一块矩形；而另一个是指向目标矩形的指针。
	我们将NULL传入这两个参数，
	是告诉SDL绘制整个源图像（第一个NULL），并把它画在屏幕上（0，0 ）的位置，
	拉伸这个图像让它填满整个窗口（第二个NULL）
	*/
	SDL_RenderCopy(ren, tex, NULL, NULL);//绘制
	SDL_RenderPresent(ren);//刷新

	SDL_Delay(10000);//延迟展示

	//清楚内存
	SDL_DestroyTexture(tex);
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);

	SDL_Quit();

	return 0;
}