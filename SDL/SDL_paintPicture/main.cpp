#include <iostream>
#include <string>

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
#pragma comment(lib,"legacy_stdio_definitions.lib")

extern "C" { FILE __iob_func[3] = { *stdin,*stdout,*stderr }; }

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

SDL_Window *win = NULL;
SDL_Renderer *ren = NULL;

//加载图片 返回指定渲染器的图片纹理
SDL_Texture * LoadImage(string str_path)
{
	SDL_Texture * tex = NULL;
	SDL_Surface *bmp = NULL;

	//加载图片到Surface  便于硬件加速
	bmp = SDL_LoadBMP(str_path.c_str());
	if (bmp == NULL)
	{
		cout << "SDL_LoadBMP error" << endl;
		return tex;
	}

	//在指定渲染器Renderer上创建纹理Texture
	tex = SDL_CreateTextureFromSurface(ren, bmp);
	
	//释放Surface内存
	SDL_FreeSurface(bmp);

	return tex;
}

//绘制图像 指定位置
void ApplySurface(int x, int y, SDL_Texture *tex, SDL_Renderer *rend)
{
	//为了指定纹理Texture的绘制位置  我们需要创建一个SDL_Rect表示一个矩形 有位置宽高参数
	//矩形
	SDL_Rect pos;
	pos.x = x;//坐标
	pos.y = y;

	//通过纹理Texture查询纹理图片的宽高
	SDL_QueryTexture(tex, NULL, NULL, &pos.w, &pos.h);

	/*
	extern DECLSPEC int SDLCALL SDL_RenderCopy(SDL_Renderer * renderer,
	SDL_Texture * texture,
	const SDL_Rect * srcrect,
	const SDL_Rect * dstrect);
	//两个NULL分别是第一个NULL是一个指向源矩形的指针，从图像上裁剪下的一块矩形；而另一个是指向目标矩形的指针。
	我们将NULL传入这两个参数，
	是告诉SDL绘制整个源图像（第一个NULL），并把它画在屏幕上（0，0 ）的位置，
	拉伸这个图像让它填满整个窗口（第二个NULL）
	SDL_RenderCopy(ren, tex, NULL, NULL);
	*/
	//在渲染器ren的pos位置绘制纹理图像tex
	SDL_RenderCopy(ren, tex, NULL, &pos);

	return;
}

int main(int argc, char *argv[])
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
	//SDL_WINDOWPOS_CENTERED表示sdl把窗口设定到指定坐标轴的中央
	win = SDL_CreateWindow("Paint Picture!" ,SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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
	ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (ren == NULL)
	{
		cout << "SDL_CreateRenderer error" << endl;
		return -1;
	}

	SDL_Texture *background = NULL, *image = NULL;
	background = LoadImage("4.bmp");
	image = LoadImage("5.bmp");
	if (background == NULL || image == NULL)
	{
		cout << "LoadImage error" << endl;
		return -1;
	}
		

	SDL_RenderClear(ren);//先清除
	
	int bw, bh;
	SDL_QueryTexture(background, NULL, NULL, &bw, &bh);
	for (int i = 0; i < SCREEN_WIDTH; i+= bw)
	{
		for (int j = 0; j < SCREEN_HEIGHT; j += bh)
		{
			ApplySurface(i, j, background, ren);
		}
	}

	SDL_QueryTexture(image, NULL, NULL, &bw, &bh);
	int x = SCREEN_WIDTH / 2 - bw / 2;
	int y = SCREEN_HEIGHT / 2 - bh / 2;
	ApplySurface(x, y, image, ren);

	SDL_RenderPresent(ren);//刷新

	SDL_Delay(10000);//延迟展示

	//清除内存
	SDL_DestroyTexture(background);
	SDL_DestroyTexture(image);
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);

	SDL_Quit();

	return 0;
}