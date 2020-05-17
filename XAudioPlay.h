#pragma once
#include "QtMultimedia/QAudioOutput"
#include "QtMultimedia/QAudioFormat"
#include "pch.h"
#include<QMutex>

#include <QIODevice>

class XAudioPlay
{
public:
	static XAudioPlay *Get()
	{
		static XAudioPlay ap;
		return &ap;
	};//����ģʽ
	bool Start();//����
	void  Play(bool isplay);//��ͣ
	bool Write(const char *data, int datasize);//����Ƶд��
	void Stop() ;//ֹͣ
	int GetFree() ;//��ȡʣ��ռ�
	~XAudioPlay();
	int sampleRate = 48000;//������
	int sampleSize = 16;//������С
	int channel = 2;///ͨ����


	QAudioOutput *output = NULL;
	QIODevice *io = NULL;
	QMutex mutex;
protected:
	XAudioPlay();
};