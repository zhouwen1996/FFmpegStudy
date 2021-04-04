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
#include "libswresample/swresample.h"//����ͷ�ļ�
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

#pragma comment(lib,"avformat.lib")//��ӿ��ļ���Ҳ���������Դ����
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"swresample.lib")
#pragma comment(lib,"swscale.lib")
#pragma comment(lib,"avdevice.lib")
#pragma comment(lib,"avfilter.lib")

#pragma comment(lib,"SDL2.lib")
#pragma comment(lib,"SDL2main.lib")
#pragma comment(lib,"SDL2test.lib")

//SDL���¼�����
#define REFRESH_EVENT  (SDL_USEREVENT + 1) //ˢ���¼�
#define BREAK_EVENT  (SDL_USEREVENT + 2) // �˳��¼�
int thread_exit = 0;//��Ӧ��������
int thread_pause = 0;

//���vs2015���Ӵ���
#pragma comment(lib,"legacy_stdio_definitions.lib")
extern "C" { FILE __iob_func[3] = { *stdin,*stdout,*stderr }; }

//SDL�̺߳���
int video_refresh_thread(void *data) {
	thread_exit = 0;
	thread_pause = 0;

	while (!thread_exit) {
		if (!thread_pause) {
			SDL_Event event;
			event.type = REFRESH_EVENT;
			SDL_PushEvent(&event);// ����ˢ���¼�
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
	//1����������YUV����
	//1.1��ע���豸
	av_register_all();
	avformat_network_init();
	avdevice_register_all();//���һ��avdevice�豸ע��
							//1.2����ȡgdigrab��desktop�ķ�װ��������
	pFormatCtx = avformat_alloc_context();
	if (avformat_open_input(&pFormatCtx, "wx_camera_1615072849946.mp4", 0, NULL) != 0) {
		printf("Couldn't open output240X128.yuv stream.���޷�����������\n");
		return -1;
	}
	//1.3���õ���Ƶ����ID
	if (avformat_find_stream_info(pFormatCtx, NULL)<0)//��ʱ��Ҫ����һ�� ��ȻһЩ���ݿ���δ��ȡ��
	{
		printf("Couldn't find stream information.���޷���ȡ����Ϣ��\n");
		return -1;
	}
	videoindex = -1;
	videoindex = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);//ֱ�ӻ�ȡ δʹ��ѭ��
	if (videoindex == -1)
	{
		printf("Didn't find a video stream.��û���ҵ���Ƶ����\n");
		return -1;
	}
	//1.4����ȡ�򿪶�Ӧ��Ƶ���Ľ������ͽ�����������
	AVCodec *vcodec = avcodec_find_decoder(pFormatCtx->streams[videoindex]->codecpar->codec_id);
	if (!vcodec)
	{
		cout << "Codec not found.��û���ҵ�������" << endl;
		return -1;
	}
	pCodecCtx = avcodec_alloc_context3(vcodec);
	avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[videoindex]->codecpar);
	pCodecCtx->thread_count = 8;
	ret = avcodec_open2(pCodecCtx, NULL, 0);
	if (ret != 0)
	{
		cout << "Could not open codec.���޷��򿪽�����" << endl;
		return -1;
	}
	return 0;
}
int InitSDL(const AVFormatContext *pFormatCtx, SDL_Window * &win, SDL_Renderer * &renderer, SDL_Texture *&texture, int videoindex)
{
	int ret;
	//SDL��ر���
	int w_width = 640;	//Ĭ�ϴ��ڴ�С
	int w_height = 480;
	Uint32 pixformat;

	//2.1����ʼ��init
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not initialize SDL - %s\n", SDL_GetError());
		return ret;
	}

	//2.2����������
	w_width = pFormatCtx->streams[videoindex]->codecpar->width;//������Ļ���ﻹ��֪����ô������Ļ��С
	w_height = pFormatCtx->streams[videoindex]->codecpar->height;
	win = SDL_CreateWindow("Media Player",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		w_width, w_height,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (!win) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create window by SDL");
		//Ҫ�ͷ�ffmpeg������ڴ�
		return -1;
	}

	//2.3��������Ⱦ��
	renderer = SDL_CreateRenderer(win, -1, 0);
	if (!renderer) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create Renderer by SDL");
		//Ҫ�ͷ�ffmpeg������ڴ�
		return -1;
	}
	//2.4����������
	pixformat = SDL_PIXELFORMAT_IYUV;//YUV��ʽ			 
	texture = SDL_CreateTexture(renderer,
		pixformat,
		SDL_TEXTUREACCESS_STREAMING,
		w_width,
		w_height);

	//2.5������SDL�����߳�
	
	

	return 0;
}
int InitAvFilter(AVFilterGraph *&filter_graph, AVFilterContext * &bufferSrc_ctx, AVFilterContext* &bufferSink_ctx, const char *args)
{
	int ret = 0;
	avfilter_register_all();
	AVFilter *buffersrc = avfilter_get_by_name("buffer");
	AVFilter *buffersink = avfilter_get_by_name("buffersink");//ffbuffersink ʵ������ʱ����󷵻�-12
	AVFilterInOut *outputs = avfilter_inout_alloc();
	AVFilterInOut *inputs = avfilter_inout_alloc();
	enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE };
	AVBufferSinkParams *buffersink_params;

	filter_graph = avfilter_graph_alloc();
	/* buffer video source: the decoded frames from the decoder will be inserted here. */
	

	ret = avfilter_graph_create_filter(&bufferSrc_ctx, buffersrc, "in",
		args, NULL, filter_graph);
	if (ret < 0) {
		printf("Cannot create buffer source\n");
		return ret;
	}

	/* buffer video sink: to terminate the filter chain. */
	buffersink_params = av_buffersink_params_alloc();
	buffersink_params->pixel_fmts = pix_fmts;
	ret = avfilter_graph_create_filter(&bufferSink_ctx, buffersink, "out",
		NULL, buffersink_params, filter_graph);
	av_free(buffersink_params);
	if (ret < 0) {
		printf("Cannot create buffer sink\n");
		return ret;
	}

	/* Endpoints for the filter graph. */
	outputs->name = av_strdup("in");
	outputs->filter_ctx = bufferSrc_ctx;
	outputs->pad_idx = 0;
	outputs->next = NULL;

	inputs->name = av_strdup("out");
	inputs->filter_ctx = bufferSink_ctx;
	inputs->pad_idx = 0;
	inputs->next = NULL;


	//��ͼ��������ַ���������ͼ��hflip��ֱ��ת
	//����AVFilterInOut ���������
	const char *filter_descr = "movie=hello.png[wm];[in][wm]overlay=5:5[out]";
	if ((ret = avfilter_graph_parse_ptr(filter_graph, filter_descr,
		&inputs, &outputs, NULL)) < 0)
		return ret;

	if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
		return ret;

	//����ʹ�ã�
	//��ͼ��ת��Ϊ����ɶ����ַ�����ʾ��ʽ��
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
	//��ʼ��������
	ret = InitAVCode(pFormatCtx, pCodecCtx, videoindex);
	if (ret != 0)
	{
		cout << "Codec faile" << endl;
		return -1;
	}

	//2��SDL��ʾ��س�ʼ��
	ret = InitSDL(pFormatCtx, win, renderer, texture, videoindex);
	if (ret != 0)
	{
		cout << "InitSDL faile" << endl;
		return -1;
	}
		
	//��ʼ��avfilter�˲���
	AVFilterGraph *filter_graph = NULL;
	AVFilterContext * bufferSrc_ctx = NULL;
	AVFilterContext* bufferSink_ctx = NULL;
	char args[512];
	snprintf(args, sizeof(args),
		"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
		pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P,
		1, 25, 1, 1);
	ret = InitAvFilter(filter_graph, bufferSrc_ctx, bufferSink_ctx, args);
	if (ret != 0)
	{
		cout << "InitAvFilter faile" << endl;
		return -1;
	}

	//3����ȡ��������ݲ�SDL��ʾ
	//3.1�������߳�
	SDL_Event event;
	SDL_CreateThread(video_refresh_thread, "Video Thread", NULL);
	//3.2������
	//3.2.1������AVPacket��AVFrame����������ʾ��λ�ã����
	AVPacket *packet = av_packet_alloc();
	AVFrame *pFrame = NULL;
	AVFrame	*pFrameYUV = NULL;
	pFrameYUV = av_frame_alloc();
	pFrame = av_frame_alloc();
	//3.2.2������洢ת����һ֡�������ݻ�����
	//��һ��ʮ����Ҫ ֮ǰ֪��
	//����ʹ��sws_scale ��Ҫ�ڴ��ָ��ÿ�д�С��� ʹ��avpicture_fill�Ϳ��Բ����Զ��������
	uint8_t *out_buffer = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
	//avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);  �°汾������2
	//�洢һ֡�������ݻ�����
	av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);

	AVFrame *frame_out = av_frame_alloc();
	unsigned char *frame_buffer_out = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1));
	av_image_fill_arrays(frame_out->data, frame_out->linesize, frame_buffer_out,
		AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);

	rect.x = 0;
	rect.y = 0;
	rect.w = pFormatCtx->streams[videoindex]->codecpar->width;
	rect.h = pFormatCtx->streams[videoindex]->codecpar->height;
	//3.2.3��sws������Ƶ�ߴ�����ת������
	//��һ��ʮ����Ҫ ֮ǰ֪��  ��������Ľ���SDL��ʾ ���ظ�ʽ��Ҫת���������������
	//struct SwsContext *img_convert_ctx = NULL;
	//img_convert_ctx = sws_getCachedContext(img_convert_ctx, pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

	//д���ļ���
	//FILE *fp_yuv=fopen("output.yuv","wb+");

	//3.2.4��ѭ������
	for (;;) 
	{
		SDL_WaitEvent(&event);//ʹ��ʱ��������ÿ40msִ��һ�� ��SDL_PushEvent
		 //��ˢ���¼�REFRESH_EVENT�Ž���
		if (event.type == REFRESH_EVENT)
		{
			//Ԥ����һ��
			while (1) {
				if (av_read_frame(pFormatCtx, packet) < 0)//������
					thread_exit = 1;

				if (packet->stream_index == videoindex)
					break;
			}
			//���յİ�����Ƶ�������SDL��ʾ
			if (packet->stream_index == videoindex) {
				avcodec_send_packet(pCodecCtx, packet);//����packet�������߳� ���ض�֡
				while (avcodec_receive_frame(pCodecCtx, pFrame) == 0) {//ѭ����ȡ���뷵�ص�֡

					//sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);

					//�򻺳���Դ���һ��֡��
					if (av_buffersrc_add_frame(bufferSrc_ctx, pFrame) < 0) {
						printf("Error while add frame.\n");
						break;
					}

					//�ӽ��������һ�����˺������֡�����������֡�С�
					/* pull filtered pictures from the filtergraph */
					ret = av_buffersink_get_frame(bufferSink_ctx, frame_out);
					if (ret < 0)
						break;

					/*
					//д���ļ���
						int y_size=pCodecCtx->width*pCodecCtx->height;    
						fwrite(pFrameYUV->data[0],1,y_size,fp_yuv);    //Y   
						fwrite(pFrameYUV->data[1],1,y_size/4,fp_yuv);  //U  
						fwrite(pFrameYUV->data[2],1,y_size/4,fp_yuv);  //V  
					*/
				
					//�����ص����ݸ��µ�SDL������
					SDL_UpdateYUVTexture(texture, NULL,
						frame_out->data[0], frame_out->linesize[0],
						frame_out->data[1], frame_out->linesize[1],
						frame_out->data[2], frame_out->linesize[2]);

					//����SDLˢ����ʾ
					SDL_RenderClear(renderer);
					SDL_RenderCopy(renderer, texture, NULL, &rect);
					SDL_RenderPresent(renderer);
				}
				av_packet_unref(packet);
			}
		}
		else if (event.type == SDL_KEYDOWN) 
		{
			if (event.key.keysym.sym == SDLK_SPACE) { //�ո����ͣ
				thread_pause = !thread_pause;
			}
			if (event.key.keysym.sym == SDLK_ESCAPE) { // ESC���˳�
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
	//4���ͷ��ڴ�
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