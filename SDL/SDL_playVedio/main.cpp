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

//解决VS2015库不对应问题
#pragma comment(lib,"legacy_stdio_definitions.lib")
extern "C" { FILE __iob_func[3] = { *stdin,*stdout,*stderr }; }

//每个像素点占的位数  YUV420就是12个位 8+2+2
const int bpp = 12;
//窗口大小
const int screen_w = 640, screen_h = 360;
//视频大小
const int pixel_w = 640, pixel_h = 360;
//每帧图像的内存存放
unsigned char buffer[pixel_w*pixel_h*bpp / 8];

int main(int argc, char *argv[])
{
	//SDL初始化
	if (SDL_Init(SDL_INIT_VIDEO))
	{
		cout << "SDL_Init error" << endl;
		return -1;
	}
	else
	{
		cout << "SDL_Init success" << endl;
	}

	//创建窗口
	//SDL_WINDOWPOS_CENTERED  居中  
	//SDL_WINDOW_OPENGL 使用openGL  SDL_WINDOW_RESIZABLE无边框
	SDL_Window *screen = NULL;
	screen = SDL_CreateWindow("SDL Play Vedio", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		screen_w, screen_h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (!screen)
	{
		cout << "SDL_CreateWindow error" << endl;
		return -1;
	}
	
	//创建窗口的渲染器 -1表示渲染驱动自动选择 
	SDL_Renderer* sdlRenderer = SDL_CreateRenderer(screen, -1, 0);

	Uint32 pixformat = 0;
	//IYUV: Y + U + V  (3 planes)
	//YV12: Y + V + U  (3 planes)
	pixformat = SDL_PIXELFORMAT_IYUV;  //YUV422   则每个像素占12位
	//创建纹理 之前也可以使用从suface中获取 
	/*
	extern DECLSPEC SDL_Texture * SDLCALL SDL_CreateTexture(SDL_Renderer * renderer,
                                                        Uint32 format, //纹理的格式
                                                        int access,  //SDL_TEXTUREACCESS_STREAMING 表示变化频繁
                                                        int w, int h);//纹理数据的宽高
	*/
	SDL_Texture* sdlTexture = NULL;
	sdlTexture = SDL_CreateTexture(sdlRenderer, pixformat, SDL_TEXTUREACCESS_STREAMING, pixel_w, pixel_h);
	if (sdlTexture == NULL)
	{
		cout << "SDL_CreateWindow error" << endl;
		return -1;
	}
	
	//打开视频文件
	FILE *fp = NULL;
	fp = fopen("sintel_640_360.yuv", "rb+");
	if (fp == NULL) {
		printf("cannot open this file\n");
		return -1;
	}

	//使用矩形来定位纹理显示在渲染器的坐标
	SDL_Rect sdlRect;

	while (1) {
		//每次读取一个帧画面
		if (fread(buffer, 1, pixel_w*pixel_h*bpp / 8, fp) != pixel_w*pixel_h*bpp / 8) {
			// Loop
			fseek(fp, 0, SEEK_SET);
			fread(buffer, 1, pixel_w*pixel_h*bpp / 8, fp);
		}

		//将数据填充到纹理中
		/*
		extern DECLSPEC int SDLCALL SDL_UpdateTexture(SDL_Texture * texture,
                                              const SDL_Rect * rect,
                                              const void *pixels,//数据内存地址
											  int pitch);//像素数据行之间的字节数
		//填充纹理还可以使用
		extern DECLSPEC int SDLCALL SDL_UpdateYUVTexture(SDL_Texture * texture,
                                                 const SDL_Rect * rect,
                                                 const Uint8 *Yplane, int Ypitch,
                                                 const Uint8 *Uplane, int Upitch,
                                                 const Uint8 *Vplane, int Vpitch);									  
		 */
		SDL_UpdateTexture(sdlTexture, NULL, buffer, pixel_w);

		//显示的位置
		sdlRect.x = 0;
		sdlRect.y = 0;
		sdlRect.w = screen_w;
		sdlRect.h = screen_h;

		SDL_RenderClear(sdlRenderer);//清空渲染器
		SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect);//将纹理数据填充到渲染器中
		SDL_RenderPresent(sdlRenderer);//显示刷新

		//Delay 40ms
		SDL_Delay(40);//延迟

	}

	//清除内存
	SDL_DestroyTexture(sdlTexture);
	SDL_DestroyRenderer(sdlRenderer);
	SDL_DestroyWindow(screen);
	//退出
	SDL_Quit();

	return 0;
}