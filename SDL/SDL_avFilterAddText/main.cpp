#include <iostream>
#include <thread>
#include <ctime>
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
#include <libavfilter/avfiltergraph.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
	
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
#pragma comment(lib,"avfilter.lib")

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
int InitAVCode(AVFormatContext * &pFormatCtx, AVCodecContext * &pCodecCtx, int &videoindex)
{
	int ret = 0;
	//1、解码生成YUV数据
	//1.1、注册设备
	av_register_all();
	avformat_network_init();
	avdevice_register_all();//添加一个avdevice设备注册
							//1.2、获取gdigrab的desktop的封装器上下文
	pFormatCtx = avformat_alloc_context();
	if (avformat_open_input(&pFormatCtx, "dushuhu.mp4", 0, NULL) != 0) {
		printf("Couldn't open output240X128.yuv stream.（无法打开输入流）\n");
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
	return 0;
}
int InitSDL(const AVFormatContext *pFormatCtx, SDL_Window * &win, SDL_Renderer * &renderer, SDL_Texture *&texture, int videoindex)
{
	int ret;
	//SDL相关变量
	int w_width = 640;	//默认窗口大小
	int w_height = 480;
	Uint32 pixformat;

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

	//2.5、创建SDL管理线程
	
	

	return 0;
}
int InitAvFilter(AVFilterGraph *&filter_graph, AVFilterContext * &bufferSrc_ctx, AVFilterContext* &bufferSink_ctx, const char *args)
{
	int ret = 0;

	avfilter_register_all();

	filter_graph = avfilter_graph_alloc();
	if (!filter_graph)
	{
		cout << "create filter graph Fial" << endl;
		return -1;
	}

	//创建一个buffer源过滤器并创建对应的过滤器实列并加入过滤器图中，之后就需要将图里面的不同过滤器链接到一起即可
	AVFilter* bufferSrc = avfilter_get_by_name("buffer");
	ret = avfilter_graph_create_filter(&bufferSrc_ctx, bufferSrc ,"in", args, NULL, filter_graph);
	if (ret < 0) {
		printf("Fail to create filter\n");
		return -1;
	}

	//创建buffersink目的过滤器
	//bufferSink需要设置参数 允许的像素格式列表，以AV_PIX_FMT_NONE结束
	AVBufferSinkParams *bufferSink_params = NULL;
	enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE };
	bufferSink_params = av_buffersink_params_alloc();
	bufferSink_params->pixel_fmts = pix_fmts;
	AVFilter* bufferSink = avfilter_get_by_name("buffersink"); //ffbuffersink带缓冲的buffersink
	ret = avfilter_graph_create_filter(&bufferSink_ctx, bufferSink, "out", NULL, bufferSink_params, filter_graph);
	if (ret < 0) {
		printf("Fail to create filter sink filter\n");
		return -1;
	}
	AVFilterInOut *outputs = avfilter_inout_alloc();
	AVFilterInOut *inputs = avfilter_inout_alloc();
	outputs->name = av_strdup("in");
	outputs->filter_ctx = bufferSrc_ctx;
	outputs->pad_idx = 0;
	outputs->next = NULL;

	inputs->name = av_strdup("out");
	inputs->filter_ctx = bufferSink_ctx;
	inputs->pad_idx = 0;
	inputs->next = NULL;

	time_t now = time(0);
	string strTime = "";
	char ch[100] = { 0 };
	tm *ltm = localtime(&now);
	// 输出 tm 结构的各个组成部分
	itoa(ltm->tm_sec, ch, 10);
	strTime = ch;
	string m_filters_descr = "drawtext=fontfile=STSONG.TTF:fontcolor=red:fontsize=50:x=0:y=0:text=";
	m_filters_descr += "\'%{localtime\\:%Y-%m-%d %H.%M.%S}\'";
	//string m_filters_descr = "settb=AVTB,setpts='trunc(PTS/1K)*1K+st(1,trunc(RTCTIME/1K))-1K*trunc(ld(1)/1K)',drawtext=fontsize=100:fontcolor=white:text='%{localtime}.%{eif\:1M*t-1K*trunc(t*1K)\:d}'";
	if ((ret = avfilter_graph_parse_ptr(filter_graph, m_filters_descr.c_str(),
		&inputs, &outputs, NULL)) < 0)
		return -1;

	if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
		return -1;
	return ret;

	////创建split分割过滤器  可以传入参数outputs=2表示分割出两路 注意链接的时候需要指定哪一路 从0开始计数
	//AVFilter *splitFilter = avfilter_get_by_name("split");
	//AVFilterContext *splitFilter_ctx;
	//ret = avfilter_graph_create_filter(&splitFilter_ctx, splitFilter, "split", "outputs=2", NULL, filter_graph);
	//if (ret < 0) {
	//	printf("Fail to create split filter\n");
	//	return -1;
	//}

	////创建crop裁剪处理过滤器
	//AVFilter *cropFilter = avfilter_get_by_name("crop");
	//AVFilterContext *cropFilter_ctx;
	//ret = avfilter_graph_create_filter(&cropFilter_ctx, cropFilter, "crop", "out_w=iw:out_h=ih/2:x=0:y=0", NULL, filter_graph);
	//if (ret < 0) {
	//	printf("Fail to create crop filter\n");
	//	return -1;
	//}

	////创建vflip水平翻转过滤器
	//AVFilter *vflipFilter = avfilter_get_by_name("vflip");
	//AVFilterContext *vflipFilter_ctx;
	//ret = avfilter_graph_create_filter(&vflipFilter_ctx, vflipFilter, "vflip", NULL, NULL, filter_graph);
	//if (ret < 0) {
	//	printf("Fail to create vflip filter\n");
	//	return -1;
	//}

	////创建overlay覆盖过滤器，将一个视频覆盖再另外一个视频上
	//AVFilter *overlayFilter = avfilter_get_by_name("overlay");
	//AVFilterContext *overlayFilter_ctx;
	//ret = avfilter_graph_create_filter(&overlayFilter_ctx, overlayFilter, "overlay", "y=0:H/2", NULL, filter_graph);
	//if (ret < 0) {
	//	printf("Fail to create overlay filter\n");
	//	return -1;
	//}

	////对上面添加到过滤器图中的过滤器进行链接生成逻辑关系
	////进行链接滤波器buffer -- split
	//ret = avfilter_link(bufferSrc_ctx, 0, splitFilter_ctx, 0);
	//if (ret != 0) {
	//	printf("Fail to link src filter and split filter\n");
	//	return -1;
	//}
	////链接split分出第1pad--overlay
	//ret = avfilter_link(splitFilter_ctx, 0, overlayFilter_ctx, 0);
	//if (ret != 0) {
	//	printf("Fail to link split filter and overlay filter main pad\n");
	//	return -1;
	//}
	////链接split分出第2pad--crop
	//ret = avfilter_link(splitFilter_ctx, 1, cropFilter_ctx, 0);
	//if (ret != 0) {
	//	printf("Fail to link split filter's second pad and crop filter\n");
	//	return -1;
	//}
	////链接crop到vflip
	//ret = avfilter_link(cropFilter_ctx, 0, vflipFilter_ctx, 0);
	//if (ret != 0) {
	//	printf("Fail to link crop filter and vflip filter\n");
	//	return -1;
	//}
	////链接vflip到overlay的第二个输入pad 主屏幕   第一个输入是覆盖第二个输入的“主”视频。
	//ret = avfilter_link(vflipFilter_ctx, 0, overlayFilter_ctx, 1);
	//if (ret != 0) {
	//	printf("Fail to link vflip filter and overlay filter's second pad\n");
	//	return -1;
	//}
	////链接overlay的输出到sink目的过滤器
	//ret = avfilter_link(overlayFilter_ctx, 0, bufferSink_ctx, 0);
	//if (ret != 0) {
	//	printf("Fail to link overlay filter and sink filter\n");
	//	return -1;
	//}

	////检查有效性并配置图中的所有链接和格式。
	//ret = avfilter_graph_config(filter_graph, NULL);
	//if (ret < 0) {
	//	printf("Fail in filter graph\n");
	//	return -1;
	//}

	//测试使用；
	//将图形转储为人类可读的字符串表示形式。
	char *graph_str = avfilter_graph_dump(filter_graph, NULL);
	FILE* graphFile = NULL;
	fopen_s(&graphFile, "graphFile.txt", "w");
	fprintf(graphFile, "%s", graph_str);
	av_free(graph_str);

	return 0;
}
int main(int argc, char *argv[])
{
	AVFormatContext	*pFormatCtx;
	int				i, videoindex;
	AVCodecContext	*pCodecCtx;
	AVCodec			*pCodec;
	int ret = 0;

	
	SDL_Rect rect;
	

	SDL_Window *win = NULL;
	SDL_Renderer *renderer = NULL;
	SDL_Texture *texture = NULL;
	//初始化解码器
	ret = InitAVCode(pFormatCtx, pCodecCtx, videoindex);
	if (ret != 0)
	{
		cout << "Codec faile" << endl;
		return -1;
	}

	//2、SDL显示相关初始化
	ret = InitSDL(pFormatCtx, win, renderer, texture, videoindex);
	if (ret != 0)
	{
		cout << "InitSDL faile" << endl;
		return -1;
	}
		
	//初始化avfilter滤波器
	AVFilterGraph *filter_graph = NULL;
	AVFilterContext * bufferSrc_ctx = NULL;
	AVFilterContext* bufferSink_ctx = NULL;
	char args[512];
	_snprintf_s(args, sizeof(args),
		"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
		pFormatCtx->streams[videoindex]->codecpar->width, pFormatCtx->streams[videoindex]->codecpar->height, AV_PIX_FMT_YUV420P,
		1, 25, 1, 1);
	ret = InitAvFilter(filter_graph, bufferSrc_ctx, bufferSink_ctx, args);
	if (ret != 0)
	{
		cout << "InitAvFilter faile" << endl;
		return -1;
	}

	//3、读取编解码数据并SDL显示
	//3.1、开启线程
	SDL_Event event;
	SDL_CreateThread(video_refresh_thread, "Video Thread", NULL);
	//3.2、播放
	//3.2.1、定义AVPacket、AVFrame设置纹理显示的位置，宽高
	AVPacket *packet = av_packet_alloc();
	AVFrame *pFrame = NULL;
	AVFrame	*pFrameYUV = NULL;
	pFrameYUV = av_frame_alloc();
	pFrame = av_frame_alloc();
	//3.2.2、定义存储转换后一帧像素数据缓冲区
	//这一步十分重要 之前知道
	//后面使用sws_scale 需要内存和指定每行大小存放 使用avpicture_fill就可以不用自定义计算了
	uint8_t *out_buffer = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
	//avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);  新版本被遗弃2
	//存储一帧像素数据缓冲区
	av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);

	AVFrame *frame_out = av_frame_alloc();
	unsigned char *frame_buffer_out = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1));
	av_image_fill_arrays(frame_out->data, frame_out->linesize, frame_buffer_out,
		AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);

	rect.x = 0;
	rect.y = 0;
	rect.w = pFormatCtx->streams[videoindex]->codecpar->width;
	rect.h = pFormatCtx->streams[videoindex]->codecpar->height;
	//3.2.3、sws进行视频尺寸像素转换定义
	//这一步十分重要 之前知道  解码出来的进行SDL显示 像素格式需要转换，否则就是绿屏
	//struct SwsContext *img_convert_ctx = NULL;
	//img_convert_ctx = sws_getCachedContext(img_convert_ctx, pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

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

					//sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);

					//向缓冲区源添加一个帧。
					if (av_buffersrc_add_frame(bufferSrc_ctx, pFrame) < 0) {
						printf("Error while add frame.\n");
						break;
					}

					//从接收器获得一个过滤后的数据帧，并将其放入帧中。
					/* pull filtered pictures from the filtergraph */
					ret = av_buffersink_get_frame(bufferSink_ctx, frame_out);
					if (ret < 0)
						break;

					/*
					//写入文件中
						int y_size=pCodecCtx->width*pCodecCtx->height;    
						fwrite(pFrameYUV->data[0],1,y_size,fp_yuv);    //Y   
						fwrite(pFrameYUV->data[1],1,y_size/4,fp_yuv);  //U  
						fwrite(pFrameYUV->data[2],1,y_size/4,fp_yuv);  //V  
					*/
				
					//将返回的数据更新到SDL纹理当中
					SDL_UpdateYUVTexture(texture, NULL,
						frame_out->data[0], frame_out->linesize[0],
						frame_out->data[1], frame_out->linesize[1],
						frame_out->data[2], frame_out->linesize[2]);

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