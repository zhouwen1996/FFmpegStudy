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
����취��
������ı������汾���ڵ�ǰ����汾����Ҫ��������Դ����vs2017���±��룬����û�а������Դ�룬��·��ͨ��
Ȼ��鵽˵��stdin, stderr, stdout �⼸������vs2015����ǰ�Ķ���ò�һ�������Ա���
��������أ�����ʹ��{*stdin,*stdout,*stderr}�����Լ�����__iob_func()
*/
#pragma comment(lib,"legacy_stdio_definitions.lib")
extern "C" { FILE __iob_func[3] = { *stdin,*stdout,*stderr }; }
/*
	��ʱ����ʵ��main����������sdl����Ĳ�һ�������磺
	int main()
	��ʱ����ʱ���ͻ��������ĳ�����Ҫ�޸�Ϊ������
	int main(int      argc, char    *argv[])

��û����������ˡ�
*/
//int main()
int main(int      argc, char    *argv[])
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
	SDL_Window *win = NULL;
	win = SDL_CreateWindow("Hello world!" ,100, 100, 640, 480, SDL_WINDOW_SHOWN);
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
	SDL_Renderer *ren = NULL;
	ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (ren == NULL)
	{
		cout << "SDL_CreateRenderer error" << endl;
		return -1;
	}

	//��ͼ  �м����SDL_Surface������ ����Ӳ������
	//��ͼƬ���ص�SDL_Surface��
	SDL_Surface *bmp = NULL;
	bmp = SDL_LoadBMP("hello.bmp");
	if (bmp == NULL)
	{
		cout << "SDL_LoadBMP error" << endl;
		return -1;
	}
	//��SDL_Surface���ݸ�SDL_Texture
	SDL_Texture * tex = NULL;
	tex = SDL_CreateTextureFromSurface(ren, bmp);
	SDL_FreeSurface(bmp);

	//��SDL_Texture����SDL_Renderer��
	SDL_RenderClear(ren);//�����
	/*
	//����NULL�ֱ��ǵ�һ��NULL��һ��ָ��Դ���ε�ָ�룬��ͼ���ϲü��µ�һ����Σ�����һ����ָ��Ŀ����ε�ָ�롣
	���ǽ�NULL����������������
	�Ǹ���SDL��������Դͼ�񣨵�һ��NULL����������������Ļ�ϣ�0��0 ����λ�ã�
	�������ͼ�����������������ڣ��ڶ���NULL��
	*/
	SDL_RenderCopy(ren, tex, NULL, NULL);//����
	SDL_RenderPresent(ren);//ˢ��

	SDL_Delay(10000);//�ӳ�չʾ

	//����ڴ�
	SDL_DestroyTexture(tex);
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);

	SDL_Quit();

	return 0;
}