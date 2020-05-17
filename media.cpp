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
	int err = av_read_frame(pFormatCtx, &pkt);//��ȡ��Ƶ֡
	
	mutex.unlock();
	return pkt;

}
media::media()
{
	av_register_all();//ע��FFMpeg�Ŀ�
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
	Close();//��ǰ�ȹر�����
	mutex.lock();//��
	pFormatCtx = avformat_alloc_context();
	int re = avformat_open_input(&pFormatCtx,Path, 0, 0);//�򿪽��װ��
	
	memcpy_s((char *)this->filePath, strlen(Path), Path, strlen(Path));
	
	//����AVFormatContext������������صĽṹ��
	
	//��ʼ��pFormatCtx�ṹ
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
	//��ȡ�ļ���
	getName();
	//��ȡʱ�� ��λΪ��

	this->durationInfo = pFormatCtx->duration / (AV_TIME_BASE);//��ȡ��Ƶ����ʱ��

//������
	for (int i = 0; i < pFormatCtx->nb_streams; i++)
	{
		AVCodecContext *pCodecCtx = pFormatCtx->streams[i]->codec;//����������

		if (pCodecCtx->codec_type == AVMEDIA_TYPE_VIDEO)//�ж��Ƿ�Ϊ��Ƶ
		{
			videoStream = i;
			//videoCtx = enc;
			AVCodec *codec = avcodec_find_decoder(pCodecCtx->codec_id);//���ҽ�����
			if (!codec)//δ�ҵ�������
			{
				mutex.unlock();
				printf("video code not find\n");
				return 5;
			}
			int err = avcodec_open2(pCodecCtx, codec, NULL);//�򿪽�����
			if (err != 0)//δ�򿪽�����
			{
				mutex.unlock();
				return 5;
			}
			printf("open codec success!\n");
		}
		//�Ƿ�Ϊ��Ƶ��
		else if (pCodecCtx->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			audioStream = i;//��Ƶ��
			AVCodec *codec = avcodec_find_decoder(pCodecCtx->codec_id);//���ҽ�����
			if (avcodec_open2(pCodecCtx, codec, NULL) < 0)
			{
				mutex.unlock();
				return 0;
			}
			this->sampleRate = pCodecCtx->sample_rate;//������
			this->channel = pCodecCtx->channels;//ͨ����
			switch (pCodecCtx->sample_fmt)//������С
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
	}//����Ϊ�򿪽���������
		//��ÿ�͸�
	sizeheight = pFormatCtx->streams[0]->codec->height;

	sizewigth = pFormatCtx->streams[0]->codec->width;
	//this->sampleRate = pFormatCtx->streams[0]->time_base.num; 

	mutex.unlock();
	return 0;
}
void media::Close()
{
	mutex.lock();//��Ҫ�������Է����߳�����������close����һ���߳����ڶ�ȡ��

	if (rgb)
	{
		av_frame_free(&rgb);
	}
	if (yuv)
	{
		av_frame_free(&yuv);
	}
	if (pFormatCtx) 
		avformat_close_input(&pFormatCtx);//�ر�ic������
	//if (yuv) 
	//	av_frame_free(&yuv);//�ر�ʱ�ͷŽ�������Ƶ֡�ռ�
	if (pcm)
	{
		av_frame_free(&pcm);
	}
	if (aCtx)
	{
		swr_free(&aCtx);//�ͷ�ת���������Ŀռ�
		aCtx = NULL;
	}
	if (cCtx)
	{
		sws_freeContext(cCtx);//�ͷ�ת���������Ŀռ�
		cCtx = NULL;
	}
	mutex.unlock();
}

//λ��
void media::Seek(float pos)
{
	mutex.lock();
	if (!pFormatCtx)//δ����Ƶ
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
	stamp = pos * pFormatCtx->streams[videoStream]->duration;//��ǰ��ʵ�ʵ�λ��
	int re = av_seek_frame(pFormatCtx, videoStream, stamp,
		AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);//����Ƶ��������ǰ���������λ��
	avcodec_flush_buffers(pFormatCtx->streams[videoStream]->codec);//ˢ�»���,�����
	mutex.unlock();
	if (re > 0)
		return ;
	return ;
}

void media::upwwork() {
	time_base = pFormatCtx->streams[videoStream]->time_base; //
	int64_t  upta = 5 + ptsTime;
	av_seek_frame(pFormatCtx, videoStream, upta*44000, AVSEEK_FLAG_BACKWARD);
	avcodec_flush_buffers(pFormatCtx->streams[videoStream]->codec);//ˢ�»���,�����
	std::cout << "upta:" << upta << std::endl;
	std::cout << "ptstime:" << ptsTime << std::endl;
	std::cout << "upta*AV_TIME_BASE/10:" << upta*AV_TIME_BASE/10 << std::endl;


	av_seek_frame(pFormatCtx, audioStream, upta*44000, AVSEEK_FLAG_BACKWARD);
	avcodec_flush_buffers(pFormatCtx->streams[audioStream]->codec);//ˢ�»���,�����

}
void media::downwork() {
	time_base = pFormatCtx->streams[audioStream]->time_base; //
	int64_t  upc = ptsTime  - 5 ;
	av_seek_frame(pFormatCtx, videoStream, upc*44000, AVSEEK_FLAG_ANY);
	avcodec_flush_buffers(pFormatCtx->streams[videoStream]->codec);//ˢ�»���,�����

	av_seek_frame(pFormatCtx, audioStream, upc*44000, AVSEEK_FLAG_ANY);
	avcodec_flush_buffers(pFormatCtx->streams[audioStream]->codec);//ˢ�»���,�����

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
	
	if (yuv == NULL)//�������Ķ���ռ�
	{
		yuv = av_frame_alloc();
	}
	if (pcm == NULL)
	{
		pcm = av_frame_alloc();//��ʱframe�ǽ�������Ƶ��
	}
	//AVFrame *frame = yuv;//��ʱ��frame�ǽ�������Ƶ��
	if (pkt->stream_index == audioStream)//��δ��Ƶ
	{
		int gopicture ;
		AVCodecContext *ctx = pFormatCtx->streams[audioStream]->codec;
		avcodec_decode_audio4(ctx, pcm,&gopicture, pkt);
		return NULL;
	}
	if (pkt->stream_index == videoStream)
	{
		int re = avcodec_send_packet(pFormatCtx->streams[pkt->stream_index]->codec, pkt);//����֮ǰ��ȡ����Ƶ֡pkt
		if (re != 0)
		{
			mutex.unlock();
			return NULL;
		}
		
		re = avcodec_receive_frame(pFormatCtx->streams[pkt->stream_index]->codec, yuv);//����pkt�����yuv��
		if (re != 0)
		{
			mutex.unlock();

			//Ϊ�˱����ĳ����д����ڴ�
			return NULL;
		}
	}
	
	mutex.unlock();
	//ptsTime = yuv->pts+(pFormatCtx->streams[pkt->stream_index]->time_base) * 1000;
	ptsTime = yuv->pts* av_q2d(pFormatCtx->streams[pkt->stream_index]->time_base);  //��ǰ����ʱ��
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
	//������̬�ڴ�,�����洢ͼ�����ݵĿռ�
	//av_image_get_buffer_size��ȡһ֡ͼ����Ҫ�Ĵ�С


	if (out_buffer == NULL)  //�����ʱ�̣������ڴ�й©��ʼ��
	{
		out_buffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB32, videoCtx->width, videoCtx->height, 1));
	}
	av_image_fill_arrays(rgb->data,rgb->linesize, out_buffer,
		AV_PIX_FMT_RGB32, videoCtx->width, videoCtx->height, 1);   //av_image_fill_arrays������ǳ����

	cCtx = sws_getContext(videoCtx->width,//��ʼ��һ�������ʽ��SwsContext
		videoCtx->height,
		videoCtx->pix_fmt, //�������ظ�ʽ
		outwidth, outheight,
		AV_PIX_FMT_BGRA,//������ظ�ʽ
		SWS_BICUBIC,//ת����㷨
		NULL, NULL, NULL);
	if (!cCtx)
	{
		mutex.unlock();
		printf("sws_getCachedContext  failed!\n");
		return false;
	}
	//uint8_t *data[AV_NUM_DATA_POINTERS] = { 0 };
	//data[0] = (uint8_t *)out;//��һλ���RGB
	int linesize[AV_NUM_DATA_POINTERS] = { 0 };

	linesize[0] = outwidth * 4;//һ�еĿ�ȣ�32λ4���ֽ�
	int h = sws_scale(cCtx, yuv->data, //��ǰ���������ÿ��ͨ������ָ��
		yuv->linesize,//ÿ��ͨ�����ֽ���
		0, videoCtx->height,//ԭ��Ƶ֡�ĸ߶�
		rgb->data,//�����ÿ��ͨ������ָ��	
		linesize//ÿ��ͨ�����ֽ���
	);//��ʼת��
	sws_freeContext(cCtx);
	mutex.unlock();
	return rgb;
}
int media::ToPCM(char *out)
{
	AVCodecContext *ctx = pFormatCtx->streams[audioStream]->codec;//��Ƶ������������
	if (this->aCtx == NULL)
	{
		this->aCtx = swr_alloc();//��ʼ��
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

	//��Ƶ���ز�������
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