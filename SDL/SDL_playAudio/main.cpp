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

static Uint8 *audio_chunk;
static Uint32 audio_len;
static Uint8 *audio_pos;
int pcm_buffer_size = 4096;

//回调函数，音频设备需要更多数据的时候会调用该回调函数
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
	//初始化音频的SDL  SDL_INIT_TIMER定时器
	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER))
	{
		cout << "SDL_Init error" << endl;
		return -1;
	}
	
	//SDL音频播放就是需要该结构体进行参数记录
	SDL_AudioSpec spec;
	spec.freq = 44100;//根据你录制的PCM采样率决定
	spec.format = AUDIO_S16SYS;//音频样式  16位
	spec.channels = 1; //单声道
	spec.silence = 0;
	spec.samples = 1024;
	spec.callback = read_audio_data;//回调函数  有对应格式的
	spec.userdata = NULL;

	//打开音频设备
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

	//播放 传入0播放  1暂停
	SDL_PauseAudio(0);

	while (1) {
		//从文件中读取数据，剩下的就交给音频设备去完成了，它播放完一段数据后会执行回调函数，获取等多的数据
		if (fread(pcm_buffer, 1, pcm_buffer_size, fp) != pcm_buffer_size) { 
			break;
		}

		audio_chunk = (Uint8 *)pcm_buffer;
		audio_len = pcm_buffer_size; //长度为读出数据长度，在read_audio_data中做减法
		audio_pos = audio_chunk;//音频数据存放的地址

		while (audio_len > 0) //判断是否播放完毕
			SDL_Delay(1);
	}

	free(pcm_buffer);

	//退出
	SDL_Quit();

	return 0;
}