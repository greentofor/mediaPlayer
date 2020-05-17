#pragma once


#include "pch.h"
#include <QMutex>


class media
{
public:
	AVFrame *yuv = NULL;//解码后的视频帧数据
	SwsContext  *cCtx = NULL;//转码器上下文
	AVFormatContext *pFormatCtx = NULL;
	AVFrame * rgb = NULL;//解码后的视频数据
	unsigned char *out_buffer =NULL;//用于缓冲空间
	SwrContext *aCtx = NULL;
	AVFrame *pcm = NULL;
	void getName();
	/*
		媒体的信息
	*/
	int videoStream = 0;//视频流
	const char filePath[128] = { 0 };
	char name[128] = { 0 };
	int  durationInfo;
	int sizeheight;
	int sizewigth;
	int ptsTime; //显示时间搓
	int timebase;

	int audioStream = 0;   //音频流
	int sampleRate = 48000;//样本率
	int sampleSize = 16;//样本大小
	int channel = 2;   //通道数

	AVRational time_base;//多少的
	int mediaState = 0;
	QMutex mutex;//互斥变量，多线程时避免同时间的读写
	/*
	暂时只能使用函数回掉
	*/
	static media *Get()//单件模式
	{
		static media ff;
		return &ff;
	}
	int Open(const char *path);//打开视频文件
	 //读取视频帧
	AVPacket Read();
	//包解码成帧
	AVFrame *Decode(const AVPacket *pkt); 
	//将解码后的YUV视频帧转化为RGB格式
	AVFrame * ToRGB(AVFrame *rgb, int outwidth, int outheight);
	int ToPCM(char *out);
	//关闭文件
	
	void Close();

	//视频进度条的位置
	void Seek(float pos);


	void upwwork();
	void downwork();
	typedef int(*fullsrc)(int);
	typedef int(*playSpeed)(int);

	//int mediaplayer(const char * filePath, 
	//	media::playSpeed playerSpeedCallBack ,
	//	media::fullsrc fullSrcCallBack );
	media();
	~media();
};

