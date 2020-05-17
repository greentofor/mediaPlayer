#pragma once


#include "pch.h"
#include <QMutex>


class media
{
public:
	AVFrame *yuv = NULL;//��������Ƶ֡����
	SwsContext  *cCtx = NULL;//ת����������
	AVFormatContext *pFormatCtx = NULL;
	AVFrame * rgb = NULL;//��������Ƶ����
	unsigned char *out_buffer =NULL;//���ڻ���ռ�
	SwrContext *aCtx = NULL;
	AVFrame *pcm = NULL;
	void getName();
	/*
		ý�����Ϣ
	*/
	int videoStream = 0;//��Ƶ��
	const char filePath[128] = { 0 };
	char name[128] = { 0 };
	int  durationInfo;
	int sizeheight;
	int sizewigth;
	int ptsTime; //��ʾʱ���
	int timebase;

	int audioStream = 0;   //��Ƶ��
	int sampleRate = 48000;//������
	int sampleSize = 16;//������С
	int channel = 2;   //ͨ����

	AVRational time_base;//���ٵ�
	int mediaState = 0;
	QMutex mutex;//������������߳�ʱ����ͬʱ��Ķ�д
	/*
	��ʱֻ��ʹ�ú����ص�
	*/
	static media *Get()//����ģʽ
	{
		static media ff;
		return &ff;
	}
	int Open(const char *path);//����Ƶ�ļ�
	 //��ȡ��Ƶ֡
	AVPacket Read();
	//�������֡
	AVFrame *Decode(const AVPacket *pkt); 
	//��������YUV��Ƶ֡ת��ΪRGB��ʽ
	AVFrame * ToRGB(AVFrame *rgb, int outwidth, int outheight);
	int ToPCM(char *out);
	//�ر��ļ�
	
	void Close();

	//��Ƶ��������λ��
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

