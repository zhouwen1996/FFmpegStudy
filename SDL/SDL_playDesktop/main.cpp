#include <iostream>
#include <thread>

using namespace std;

#ifdef __cplusplus
extern "C"
{
#endif // !__cplusplus
	
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"//包含头文件
#include "libavdevice/avdevice.h"
#include "libavutil/imgutils.h"
	
#include "SDL.h"
#include "SDL_thread.h"

#ifdef __cplusplus
}
#endif // !__cplusplus

#pragma comment(lib,"avformat.lib")//添加库文件，也可以在属性处添加
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"swresample.lib")
#pragma comment(lib,"swscale.lib")
#pragma comment(lib,"avdevice.lib")

#pragma comment(lib,"SDL2.lib")
#pragma comment(lib,"SDL2main.lib")
#pragma comment(lib,"SDL2test.lib")

//SDL的事件定义
#define REFRESH_EVENT  (SDL_USEREVENT + 1) //刷新事件
#define BREAK_EVENT  (SDL_USEREVENT + 2) // 退出事件
int thread_exit = 0;//相应变量定义
int thread_pause = 0;

//解决vs2015链接错误
#pragma comment(lib,"legacy_stdio_definitions.lib")
extern "C" { FILE __iob_func[3] = { *stdin,*stdout,*stderr }; }

//SDL线程函数
int video_refresh_thread(void *data) {
	thread_exit = 0;
	thread_pause = 0;

	while (!thread_exit) {
		if (!thread_pause) {
			SDL_Event event;
			event.type = REFRESH_EVENT;
			SDL_PushEvent(&event);// 发送刷新事件
		}
		SDL_Delay(40);
	}
	thread_exit = 0;
	thread_pause = 0;
	//Break
	SDL_Event event;
	event.type = BREAK_EVENT;
	SDL_PushEvent(&event);

	return 0;
}
int main(int argc, char *argv[])
{
	AVFormatContext	*pFormatCtx;
	int				i, videoindex;
	AVCodecContext	*pCodecCtx;
	AVCodec			*pCodec;
	int ret;

	//SDL相关变量
	int w_width = 640;	//默认窗口大小
	int w_height = 480;
	SDL_Rect rect;
	Uint32 pixformat;
	SDL_Window *win = NULL;
	SDL_Renderer *renderer = NULL;
	SDL_Texture *texture = NULL;

	//1、首先ffmpeg的gdigrab解封装器对desktop进行解码获取对应的视频流
	//1.1、注册设备
	av_register_all();
	avformat_network_init();
	avdevice_register_all();//添加一个avdevice设备注册
	//1.2、获取gdigrab的desktop的封装器上下文
	pFormatCtx = avformat_alloc_context();
	/*
	//使用gdigrab进行录屏
	AVDictionary* options = NULL;
	//Set some options
	//grabbing frame rate
	//av_dict_set(&options,"framerate","5",0);
	//The distance from the left edge of the screen or desktop
	//av_dict_set(&options,"offset_x","20",0);
	//The distance from the top edge of the screen or desktop
	//av_dict_set(&options,"offset_y","40",0);
	//Video frame size. The default is to capture the full screen
	//av_dict_set(&options,"video_size","640x480",0);
	AVInputFormat *ifmt = av_find_input_format("gdigrab");
	if (avformat_open_input(&pFormatCtx, "desktop", ifmt, &options) != 0) {
		printf("Couldn't open input stream.（无法打开输入流）\n");
		return -1;
	}
	*/
	//使用dshow进行录屏
	AVInputFormat *ifmt = av_find_input_format("dshow");
	if (avformat_open_input(&pFormatCtx, "video=screen-capture-recorder", ifmt, NULL) != 0) {
		printf("Couldn't open input stream.（无法打开输入流）\n");
		return -1;
	}

	//1.3、得到视频流的ID
	if (avformat_find_stream_info(pFormatCtx, NULL)<0)//有时候要调用一下 不然一些数据可能未读取到
	{
		printf("Couldn't find stream information.（无法获取流信息）\n");
		return -1;
	}
	videoindex = -1;
	videoindex = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);//直接获取 未使用循环
	if (videoindex == -1)
	{
		printf("Didn't find a video stream.（没有找到视频流）\n");
		return -1;
	}
	//1.4、获取打开对应视频流的解码器和解码器上下文
	AVCodec *vcodec = avcodec_find_decoder(pFormatCtx->streams[videoindex]->codecpar->codec_id);
	if (!vcodec)
	{
		cout << "Codec not found.（没有找到解码器" << endl;
		return -1;
	}
	pCodecCtx = avcodec_alloc_context3(vcodec);
	avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[videoindex]->codecpar);
	pCodecCtx->thread_count = 8;
	ret = avcodec_open2(pCodecCtx, NULL, 0);
	if (ret != 0)
	{
		cout << "Could not open codec.（无法打开解码器" << endl;
		return -1;
	}
	//avformat_find_stream_info(pFormatCtx, NULL);
	//2、SDL显示相关初始化
	//2.1、初始化init
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not initialize SDL - %s\n", SDL_GetError());
		return ret;
	}

	//2.2、创建窗口
	w_width = pFormatCtx->streams[videoindex]->codecpar->width;//关于屏幕这里还不知道怎么设置屏幕大小
	w_height = pFormatCtx->streams[videoindex]->codecpar->height;
	win = SDL_CreateWindow("Media Player",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		w_width, w_height,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (!win) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create window by SDL");
		//要释放ffmpeg的相关内存
		return -1;
	}

	//2.3、创建渲染器
	renderer = SDL_CreateRenderer(win, -1, 0);
	if (!renderer) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create Renderer by SDL");
		//要释放ffmpeg的相关内存
		return -1;
	}
	//2.4、创建纹理
	pixformat = SDL_PIXELFORMAT_IYUV;//YUV格式			 
	texture = SDL_CreateTexture(renderer,
		pixformat,
		SDL_TEXTUREACCESS_STREAMING,
		w_width,
		w_height);
	
	//3、读取编解码数据并SDL显示
	//3.1、创建SDL管理线程
	SDL_Event event;
	SDL_CreateThread(video_refresh_thread, "Video Thread", NULL);
	//3.2、播放
	//3.2.1、定义AVPacket、AVFrame设置纹理显示的位置，宽高
	AVPacket *packet = av_packet_alloc();
	AVFrame *pFrame = NULL;
	AVFrame	*pFrameYUV = NULL;
	pFrameYUV = av_frame_alloc();

	//3.2.2、定义存储转换后一帧像素数据缓冲区
	//这一步十分重要 之前知道
	//后面使用sws_scale 需要内存和指定每行大小存放 使用avpicture_fill就可以不用自定义计算了
	pFrame = av_frame_alloc();
	uint8_t *out_buffer = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
	//avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);  新版本被遗弃2
	//存储一帧像素数据缓冲区
	av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);

	rect.x = 0;
	rect.y = 0;
	rect.w = w_width;
	rect.h = w_height;
	//3.2.3、sws进行视频尺寸像素转换定义
	//这一步十分重要 之前知道  解码出来的进行SDL显示 像素格式需要转换，否则就是绿屏
	struct SwsContext *img_convert_ctx = NULL;
	img_convert_ctx = sws_getCachedContext(img_convert_ctx, pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

	//写入文件中
	//FILE *fp_yuv=fopen("output.yuv","wb+");

	//3.2.4、循环播放
	for (;;) 
	{
		SDL_WaitEvent(&event);//使用时间驱动，每40ms执行一次 等SDL_PushEvent
		 //是刷新事件REFRESH_EVENT才进入
		if (event.type == REFRESH_EVENT)
		{
			//预处理一下
			while (1) {
				if (av_read_frame(pFormatCtx, packet) < 0)//读完了
					thread_exit = 1;

				if (packet->stream_index == videoindex)
					break;
			}
			//接收的包是视频包则进入SDL显示
			if (packet->stream_index == videoindex) {
				avcodec_send_packet(pCodecCtx, packet);//发送packet到解码线程 返回多帧
				while (avcodec_receive_frame(pCodecCtx, pFrame) == 0) {//循环读取解码返回的帧

					sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);

					/*
					//写入文件中
						int y_size=pCodecCtx->width*pCodecCtx->height;    
						fwrite(pFrameYUV->data[0],1,y_size,fp_yuv);    //Y   
						fwrite(pFrameYUV->data[1],1,y_size/4,fp_yuv);  //U  
						fwrite(pFrameYUV->data[2],1,y_size/4,fp_yuv);  //V  
					*/
				
					//将返回的数据更新到SDL纹理当中
					SDL_UpdateYUVTexture(texture, NULL,
						pFrameYUV->data[0], pFrameYUV->linesize[0],
						pFrameYUV->data[1], pFrameYUV->linesize[1],
						pFrameYUV->data[2], pFrameYUV->linesize[2]);

					//进行SDL刷新显示
					SDL_RenderClear(renderer);
					SDL_RenderCopy(renderer, texture, NULL, &rect);
					SDL_RenderPresent(renderer);
				}
				av_packet_unref(packet);
			}
		}
		else if (event.type == SDL_KEYDOWN) 
		{
			if (event.key.keysym.sym == SDLK_SPACE) { //空格键暂停
				thread_pause = !thread_pause;
			}
			if (event.key.keysym.sym == SDLK_ESCAPE) { // ESC键退出
				thread_exit = 1;
			}
		}
		else if (event.type == SDL_QUIT) 
		{
			thread_exit = 1;
		}
		else if (event.type == BREAK_EVENT) 
		{
			break;
		}
	}
	//4、释放内存
	if (pFrame)
	{
		av_frame_free(&pFrame);
	}
	if (pFrameYUV)
	{
		av_frame_free(&pFrameYUV);
	}
	if (packet)
	{
		av_packet_free(&packet);
	}
	// Close the codec
	if (pCodecCtx)
	{
		avcodec_close(pCodecCtx);
	}
	// Close the video file
	if (pFormatCtx) 
	{
		avformat_close_input(&pFormatCtx);
	}
	if (win) 
	{
		SDL_DestroyWindow(win);
	}
	if (renderer) 
	{
		SDL_DestroyRenderer(renderer);
	}
	if (texture) 
	{
		SDL_DestroyTexture(texture);
	}

	SDL_Quit();

	return 0;
}