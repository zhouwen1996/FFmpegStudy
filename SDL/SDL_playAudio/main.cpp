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

static Uint8 *audio_chunk;
static Uint32 audio_len;
static Uint8 *audio_pos;
int pcm_buffer_size = 4096;

//�ص���������Ƶ�豸��Ҫ�������ݵ�ʱ�����øûص�����
void read_audio_data(void *udata, Uint8 *stream, int len)
{
	SDL_memset(stream, 0, len);
	if (audio_len == 0)
		return;
	len = (len > audio_len ? audio_len : len);

	SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
	audio_pos += len;
	audio_len -= len;
}

int main(int argc, char *argv[])
{
	//��ʼ����Ƶ��SDL  SDL_INIT_TIMER��ʱ��
	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER))
	{
		cout << "SDL_Init error" << endl;
		return -1;
	}
	
	//SDL��Ƶ���ž�����Ҫ�ýṹ����в�����¼
	SDL_AudioSpec spec;
	spec.freq = 44100;//������¼�Ƶ�PCM�����ʾ���
	spec.format = AUDIO_S16SYS;//��Ƶ��ʽ  16λ
	spec.channels = 1; //������
	spec.silence = 0;
	spec.samples = 1024;
	spec.callback = read_audio_data;//�ص�����  �ж�Ӧ��ʽ��
	spec.userdata = NULL;

	//����Ƶ�豸
	if (SDL_OpenAudio(&spec, NULL) < 0) {
		cout << "SDL_OpenAudio error" << endl;
		return -1;
	}

	FILE *fp = fopen("1.wav", "rb+");
	if (fp == NULL) {
		cout << "cannot open this file\n" << endl;
		return -1;
	}
	char *pcm_buffer = (char *)malloc(pcm_buffer_size);

	//���� ����0����  1��ͣ
	SDL_PauseAudio(0);

	while (1) {
		//���ļ��ж�ȡ���ݣ�ʣ�µľͽ�����Ƶ�豸ȥ����ˣ���������һ�����ݺ��ִ�лص���������ȡ�ȶ������
		if (fread(pcm_buffer, 1, pcm_buffer_size, fp) != pcm_buffer_size) { 
			break;
		}

		audio_chunk = (Uint8 *)pcm_buffer;
		audio_len = pcm_buffer_size; //����Ϊ�������ݳ��ȣ���read_audio_data��������
		audio_pos = audio_chunk;//��Ƶ���ݴ�ŵĵ�ַ

		while (audio_len > 0) //�ж��Ƿ񲥷����
			SDL_Delay(1);
	}

	free(pcm_buffer);

	//�˳�
	SDL_Quit();

	return 0;
}