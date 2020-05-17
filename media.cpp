#pragma once
#include "media.h"
#include "iostream"
AVPacket media::Read()
{
	AVPacket pkt;
	memset(&pkt, 0, sizeof(AVPacket));
	mutex.lock();
	if (!pFormatCtx)
	{
		mutex.unlock();
		return pkt;
	}
	int err = av_read_frame(pFormatCtx, &pkt);//读取视频帧
	
	mutex.unlock();
	return pkt;

}
media::media()
{
	av_register_all();//注册FFMpeg的库
}
void media::getName()
{
	char *temp = (char *)this->filePath;
	int endt = strlen(this->filePath)-1;
	int sart = endt;
	for (int i = 0;i<128;i++)
	{
		if (temp[sart] =='\\')
		{
			break;
		}
		
		//printf("%c", temp[sart]);
		sart--;
	}
	for (int i = 0; endt > sart; sart++,i++)
	{
		this->name[i] = temp[sart+1];
	}
	


}
int media::Open(const char *Path)
{
	Close();//打开前先关闭清理
	mutex.lock();//锁
	pFormatCtx = avformat_alloc_context();
	int re = avformat_open_input(&pFormatCtx,Path, 0, 0);//打开解封装器
	
	memcpy_s((char *)this->filePath, strlen(Path), Path, strlen(Path));
	
	//创建AVFormatContext对象，与码流相关的结构。
	
	//初始化pFormatCtx结构
	if (avformat_open_input(&pFormatCtx,Path, NULL, NULL) != 0)
	{
		printf("Couldn't open input stream.\n");
		return   1;
	}
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
	{
		printf("Couldn't find stream information.\n");
		return  2;
	}
	//读取文件名
	getName();
	//读取时长 单位为秒

	this->durationInfo = pFormatCtx->duration / (AV_TIME_BASE);//获取视频的总时间

//解码器
	for (int i = 0; i < pFormatCtx->nb_streams; i++)
	{
		AVCodecContext *pCodecCtx = pFormatCtx->streams[i]->codec;//解码上下文

		if (pCodecCtx->codec_type == AVMEDIA_TYPE_VIDEO)//判断是否为视频
		{
			videoStream = i;
			//videoCtx = enc;
			AVCodec *codec = avcodec_find_decoder(pCodecCtx->codec_id);//查找解码器
			if (!codec)//未找到解码器
			{
				mutex.unlock();
				printf("video code not find\n");
				return 5;
			}
			int err = avcodec_open2(pCodecCtx, codec, NULL);//打开解码器
			if (err != 0)//未打开解码器
			{
				mutex.unlock();
				return 5;
			}
			printf("open codec success!\n");
		}
		//是否为音频流
		else if (pCodecCtx->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			audioStream = i;//音频流
			AVCodec *codec = avcodec_find_decoder(pCodecCtx->codec_id);//查找解码器
			if (avcodec_open2(pCodecCtx, codec, NULL) < 0)
			{
				mutex.unlock();
				return 0;
			}
			this->sampleRate = pCodecCtx->sample_rate;//样本率
			this->channel = pCodecCtx->channels;//通道数
			switch (pCodecCtx->sample_fmt)//样本大小
			{
			case AV_SAMPLE_FMT_S16://signed 16 bits
				this->sampleSize = 16;
				break;
			case  AV_SAMPLE_FMT_S32://signed 32 bits
				this->sampleSize = 32;
			default:
				break;
			}
			printf("audio sample rate:%d sample size:%d chanle:%d\n", 
				this->sampleRate, this->sampleSize, this->channel);
		}
	}//至此为打开解码器过程
		//获得宽和高
	sizeheight = pFormatCtx->streams[0]->codec->height;

	sizewigth = pFormatCtx->streams[0]->codec->width;
	//this->sampleRate = pFormatCtx->streams[0]->time_base.num; 

	mutex.unlock();
	return 0;
}
void media::Close()
{
	mutex.lock();//需要上锁，以防多线程中你这里在close，另一个线程中在读取，

	if (rgb)
	{
		av_frame_free(&rgb);
	}
	if (yuv)
	{
		av_frame_free(&yuv);
	}
	if (pFormatCtx) 
		avformat_close_input(&pFormatCtx);//关闭ic上下文
	//if (yuv) 
	//	av_frame_free(&yuv);//关闭时释放解码后的视频帧空间
	if (pcm)
	{
		av_frame_free(&pcm);
	}
	if (aCtx)
	{
		swr_free(&aCtx);//释放转码器上下文空间
		aCtx = NULL;
	}
	if (cCtx)
	{
		sws_freeContext(cCtx);//释放转码器上下文空间
		cCtx = NULL;
	}
	mutex.unlock();
}

//位置
void media::Seek(float pos)
{
	mutex.lock();
	if (!pFormatCtx)//未打开视频
	{
		mutex.unlock();
		return ;
	}
	//auto time_base = context->streams[defaultStreamIndex]->time_base;
	//auto seekTime = context->streams[defaultStreamIndex]->start_time +
	// av_rescale(seekFrame, time_base.den, time_base.num);
	//int ret;
	//if (seekTime > ist->cur_dts)
	//{
	//	ret = av_seek_frame(context, defaultStreamIndex, seekTime, AVSEEK_FLAG_ANY);
	//}
	int64_t stamp = 0;
	//timescalle = pFormatCtx->streams[videoStream]->time_base; //
	stamp = pos * pFormatCtx->streams[videoStream]->duration;//当前它实际的位置
	int re = av_seek_frame(pFormatCtx, videoStream, stamp,
		AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);//将视频移至到当前点击滑动条位置
	avcodec_flush_buffers(pFormatCtx->streams[videoStream]->codec);//刷新缓冲,清理掉
	mutex.unlock();
	if (re > 0)
		return ;
	return ;
}

void media::upwwork() {
	time_base = pFormatCtx->streams[videoStream]->time_base; //
	int64_t  upta = 5 + ptsTime;
	av_seek_frame(pFormatCtx, videoStream, upta*44000, AVSEEK_FLAG_BACKWARD);
	avcodec_flush_buffers(pFormatCtx->streams[videoStream]->codec);//刷新缓冲,清理掉
	std::cout << "upta:" << upta << std::endl;
	std::cout << "ptstime:" << ptsTime << std::endl;
	std::cout << "upta*AV_TIME_BASE/10:" << upta*AV_TIME_BASE/10 << std::endl;


	av_seek_frame(pFormatCtx, audioStream, upta*44000, AVSEEK_FLAG_BACKWARD);
	avcodec_flush_buffers(pFormatCtx->streams[audioStream]->codec);//刷新缓冲,清理掉

}
void media::downwork() {
	time_base = pFormatCtx->streams[audioStream]->time_base; //
	int64_t  upc = ptsTime  - 5 ;
	av_seek_frame(pFormatCtx, videoStream, upc*44000, AVSEEK_FLAG_ANY);
	avcodec_flush_buffers(pFormatCtx->streams[videoStream]->codec);//刷新缓冲,清理掉

	av_seek_frame(pFormatCtx, audioStream, upc*44000, AVSEEK_FLAG_ANY);
	avcodec_flush_buffers(pFormatCtx->streams[audioStream]->codec);//刷新缓冲,清理掉

}
media::~media()
{
}

//int media::mediaplayer(const char * filePath, 
//	media::playSpeed playerSpeedCallBack ,
//	media::fullsrc fullSrcCallBack)
//{
//	return 9;
//}

AVFrame* media::Decode(const AVPacket *pkt)
{
	mutex.lock();
	
	if (yuv == NULL)//申请解码的对象空间
	{
		yuv = av_frame_alloc();
	}
	if (pcm == NULL)
	{
		pcm = av_frame_alloc();//此时frame是解码后的音频流
	}
	//AVFrame *frame = yuv;//此时的frame是解码后的视频流
	if (pkt->stream_index == audioStream)//若未音频
	{
		int gopicture ;
		AVCodecContext *ctx = pFormatCtx->streams[audioStream]->codec;
		avcodec_decode_audio4(ctx, pcm,&gopicture, pkt);
		return NULL;
	}
	if (pkt->stream_index == videoStream)
	{
		int re = avcodec_send_packet(pFormatCtx->streams[pkt->stream_index]->codec, pkt);//发送之前读取的视频帧pkt
		if (re != 0)
		{
			mutex.unlock();
			return NULL;
		}
		
		re = avcodec_receive_frame(pFormatCtx->streams[pkt->stream_index]->codec, yuv);//解码pkt后存入yuv中
		if (re != 0)
		{
			mutex.unlock();

			//为了避免别的程序读写这个内存
			return NULL;
		}
	}
	
	mutex.unlock();
	//ptsTime = yuv->pts+(pFormatCtx->streams[pkt->stream_index]->time_base) * 1000;
	ptsTime = yuv->pts* av_q2d(pFormatCtx->streams[pkt->stream_index]->time_base);  //当前解码时间
	return yuv;
}
AVFrame * media::ToRGB( /*out*/AVFrame *rgb, int outwidth, int outheight)
{

	mutex.lock();
	if (rgb == NULL)
	{
		rgb = av_frame_alloc();
	}
	
	//cCtx = pFormatCtx->streams[this->videoStream]->codec;
	AVCodecContext *videoCtx = pFormatCtx->streams[this->videoStream]->codec;
	//创建动态内存,创建存储图像数据的空间
	//av_image_get_buffer_size获取一帧图像需要的大小


	if (out_buffer == NULL)  //神奇的时刻，就是内存泄漏的始终
	{
		out_buffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB32, videoCtx->width, videoCtx->height, 1));
	}
	av_image_fill_arrays(rgb->data,rgb->linesize, out_buffer,
		AV_PIX_FMT_RGB32, videoCtx->width, videoCtx->height, 1);   //av_image_fill_arrays是属于浅拷贝

	cCtx = sws_getContext(videoCtx->width,//初始化一个输出格式的SwsContext
		videoCtx->height,
		videoCtx->pix_fmt, //输入像素格式
		outwidth, outheight,
		AV_PIX_FMT_BGRA,//输出像素格式
		SWS_BICUBIC,//转码的算法
		NULL, NULL, NULL);
	if (!cCtx)
	{
		mutex.unlock();
		printf("sws_getCachedContext  failed!\n");
		return false;
	}
	//uint8_t *data[AV_NUM_DATA_POINTERS] = { 0 };
	//data[0] = (uint8_t *)out;//第一位输出RGB
	int linesize[AV_NUM_DATA_POINTERS] = { 0 };

	linesize[0] = outwidth * 4;//一行的宽度，32位4个字节
	int h = sws_scale(cCtx, yuv->data, //当前处理区域的每个通道数据指针
		yuv->linesize,//每个通道行字节数
		0, videoCtx->height,//原视频帧的高度
		rgb->data,//输出的每个通道数据指针	
		linesize//每个通道行字节数
	);//开始转码
	sws_freeContext(cCtx);
	mutex.unlock();
	return rgb;
}
int media::ToPCM(char *out)
{
	AVCodecContext *ctx = pFormatCtx->streams[audioStream]->codec;//音频解码器上下文
	if (this->aCtx == NULL)
	{
		this->aCtx = swr_alloc();//初始化
		swr_alloc_set_opts(this->aCtx, ctx->channel_layout,
			AV_SAMPLE_FMT_S16,
			ctx->sample_rate,
			ctx->channels,
			ctx->sample_fmt,
			ctx->sample_rate,
			0, 0
		);
		swr_init(this->aCtx);
	}
	uint8_t  *data[1];
	data[0] = (uint8_t *)out;

	//音频的重采样过程
	int len = swr_convert(this->aCtx, data, 10000,
		(const uint8_t **)pcm->data,
		pcm->nb_samples
	);
	if (len <= 0)
	{
		mutex.unlock();
		return 0;
	}
	int outsize = av_samples_get_buffer_size(NULL, ctx->channels,
		pcm->nb_samples,
		AV_SAMPLE_FMT_S16,
		0);

	mutex.unlock();
	return outsize;
}