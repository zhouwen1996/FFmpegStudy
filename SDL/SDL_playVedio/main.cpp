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

//���VS2015�ⲻ��Ӧ����
#pragma comment(lib,"legacy_stdio_definitions.lib")
extern "C" { FILE __iob_func[3] = { *stdin,*stdout,*stderr }; }

//ÿ�����ص�ռ��λ��  YUV420����12��λ 8+2+2
const int bpp = 12;
//���ڴ�С
const int screen_w = 640, screen_h = 360;
//��Ƶ��С
const int pixel_w = 640, pixel_h = 360;
//ÿ֡ͼ����ڴ���
unsigned char buffer[pixel_w*pixel_h*bpp / 8];

int main(int argc, char *argv[])
{
	//SDL��ʼ��
	if (SDL_Init(SDL_INIT_VIDEO))
	{
		cout << "SDL_Init error" << endl;
		return -1;
	}
	else
	{
		cout << "SDL_Init success" << endl;
	}

	//��������
	//SDL_WINDOWPOS_CENTERED  ����  
	//SDL_WINDOW_OPENGL ʹ��openGL  SDL_WINDOW_RESIZABLE�ޱ߿�
	SDL_Window *screen = NULL;
	screen = SDL_CreateWindow("SDL Play Vedio", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		screen_w, screen_h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (!screen)
	{
		cout << "SDL_CreateWindow error" << endl;
		return -1;
	}
	
	//�������ڵ���Ⱦ�� -1��ʾ��Ⱦ�����Զ�ѡ�� 
	SDL_Renderer* sdlRenderer = SDL_CreateRenderer(screen, -1, 0);

	Uint32 pixformat = 0;
	//IYUV: Y + U + V  (3 planes)
	//YV12: Y + V + U  (3 planes)
	pixformat = SDL_PIXELFORMAT_IYUV;  //YUV422   ��ÿ������ռ12λ
	//�������� ֮ǰҲ����ʹ�ô�suface�л�ȡ 
	/*
	extern DECLSPEC SDL_Texture * SDLCALL SDL_CreateTexture(SDL_Renderer * renderer,
                                                        Uint32 format, //����ĸ�ʽ
                                                        int access,  //SDL_TEXTUREACCESS_STREAMING ��ʾ�仯Ƶ��
                                                        int w, int h);//�������ݵĿ��
	*/
	SDL_Texture* sdlTexture = NULL;
	sdlTexture = SDL_CreateTexture(sdlRenderer, pixformat, SDL_TEXTUREACCESS_STREAMING, pixel_w, pixel_h);
	if (sdlTexture == NULL)
	{
		cout << "SDL_CreateWindow error" << endl;
		return -1;
	}
	
	//����Ƶ�ļ�
	FILE *fp = NULL;
	fp = fopen("sintel_640_360.yuv", "rb+");
	if (fp == NULL) {
		printf("cannot open this file\n");
		return -1;
	}

	//ʹ�þ�������λ������ʾ����Ⱦ��������
	SDL_Rect sdlRect;

	while (1) {
		//ÿ�ζ�ȡһ��֡����
		if (fread(buffer, 1, pixel_w*pixel_h*bpp / 8, fp) != pixel_w*pixel_h*bpp / 8) {
			// Loop
			fseek(fp, 0, SEEK_SET);
			fread(buffer, 1, pixel_w*pixel_h*bpp / 8, fp);
		}

		//��������䵽������
		/*
		extern DECLSPEC int SDLCALL SDL_UpdateTexture(SDL_Texture * texture,
                                              const SDL_Rect * rect,
                                              const void *pixels,//�����ڴ��ַ
											  int pitch);//����������֮����ֽ���
		//�����������ʹ��
		extern DECLSPEC int SDLCALL SDL_UpdateYUVTexture(SDL_Texture * texture,
                                                 const SDL_Rect * rect,
                                                 const Uint8 *Yplane, int Ypitch,
                                                 const Uint8 *Uplane, int Upitch,
                                                 const Uint8 *Vplane, int Vpitch);									  
		 */
		SDL_UpdateTexture(sdlTexture, NULL, buffer, pixel_w);

		//��ʾ��λ��
		sdlRect.x = 0;
		sdlRect.y = 0;
		sdlRect.w = screen_w;
		sdlRect.h = screen_h;

		SDL_RenderClear(sdlRenderer);//�����Ⱦ��
		SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect);//������������䵽��Ⱦ����
		SDL_RenderPresent(sdlRenderer);//��ʾˢ��

		//Delay 40ms
		SDL_Delay(40);//�ӳ�

	}

	//����ڴ�
	SDL_DestroyTexture(sdlTexture);
	SDL_DestroyRenderer(sdlRenderer);
	SDL_DestroyWindow(screen);
	//�˳�
	SDL_Quit();

	return 0;
}