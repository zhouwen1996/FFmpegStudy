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

//����ͼƬ ����ָ����Ⱦ����ͼƬ����
SDL_Texture * LoadImage(string str_path)
{
	SDL_Texture * tex = NULL;
	SDL_Surface *bmp = NULL;

	//����ͼƬ��Surface  ����Ӳ������
	bmp = SDL_LoadBMP(str_path.c_str());
	if (bmp == NULL)
	{
		cout << "SDL_LoadBMP error" << endl;
		return tex;
	}

	//��ָ����Ⱦ��Renderer�ϴ�������Texture
	tex = SDL_CreateTextureFromSurface(ren, bmp);
	
	//�ͷ�Surface�ڴ�
	SDL_FreeSurface(bmp);

	return tex;
}

//����ͼ�� ָ��λ��
void ApplySurface(int x, int y, SDL_Texture *tex, SDL_Renderer *rend)
{
	//Ϊ��ָ������Texture�Ļ���λ��  ������Ҫ����һ��SDL_Rect��ʾһ������ ��λ�ÿ�߲���
	//����
	SDL_Rect pos;
	pos.x = x;//����
	pos.y = y;

	//ͨ������Texture��ѯ����ͼƬ�Ŀ��
	SDL_QueryTexture(tex, NULL, NULL, &pos.w, &pos.h);

	/*
	extern DECLSPEC int SDLCALL SDL_RenderCopy(SDL_Renderer * renderer,
	SDL_Texture * texture,
	const SDL_Rect * srcrect,
	const SDL_Rect * dstrect);
	//����NULL�ֱ��ǵ�һ��NULL��һ��ָ��Դ���ε�ָ�룬��ͼ���ϲü��µ�һ����Σ�����һ����ָ��Ŀ����ε�ָ�롣
	���ǽ�NULL����������������
	�Ǹ���SDL��������Դͼ�񣨵�һ��NULL����������������Ļ�ϣ�0��0 ����λ�ã�
	�������ͼ�����������������ڣ��ڶ���NULL��
	SDL_RenderCopy(ren, tex, NULL, NULL);
	*/
	//����Ⱦ��ren��posλ�û�������ͼ��tex
	SDL_RenderCopy(ren, tex, NULL, &pos);

	return;
}

int main(int argc, char *argv[])
{
	//SDL��ʼ��
	if (SDL_Init(SDL_INIT_VIDEO))
	{
		cout << "SDL_Init error" << endl;
	}
	else
	{
		cout << "SDL_Init success" << endl;
	}
	/*SDL����windows����
	extern DECLSPEC SDL_Window * SDLCALL SDL_CreateWindow(const char *title,
	int x, int y, int w,
	int h, Uint32 flags);
	���� ���� ��� ����
	SDL_WINDOW_SHOWN  ��ʾ����֮�����ϵ�����ʾ
	*/
	//SDL_WINDOWPOS_CENTERED��ʾsdl�Ѵ����趨��ָ�������������
	win = SDL_CreateWindow("Paint Picture!" ,SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (win == NULL)
	{
		cout << "SDL_CreateWindow error" << endl;
		return -1;
	}
	/*
	������Ⱦ�� ��ָ�������������ƵĴ���win
	extern DECLSPEC SDL_Renderer * SDLCALL SDL_CreateRenderer(SDL_Window * window,
	int index, Uint32 flags);
	ָ����Ⱦ���󶨵Ĵ��� ָ����ѡ�õ��Կ����� -1��ʾsdl�Զ�ѡ�� ѡ���������
	SDL_RENDERER_ACCELERATED  ��ʾʹ��Ӳ������ ʹ���Կ�
	SDL_RENDERER_PRESENTVSYNC  ��ʾʹ����ʾ����ˢ���������»���
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
		

	SDL_RenderClear(ren);//�����
	
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

	SDL_RenderPresent(ren);//ˢ��

	SDL_Delay(10000);//�ӳ�չʾ

	//����ڴ�
	SDL_DestroyTexture(background);
	SDL_DestroyTexture(image);
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);

	SDL_Quit();

	return 0;
}