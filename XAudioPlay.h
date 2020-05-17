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
	};//单例模式
	bool Start();//启动
	void  Play(bool isplay);//暂停
	bool Write(const char *data, int datasize);//将音频写入
	void Stop() ;//停止
	int GetFree() ;//获取剩余空间
	~XAudioPlay();
	int sampleRate = 48000;//样本率
	int sampleSize = 16;//样本大小
	int channel = 2;///通道数


	QAudioOutput *output = NULL;
	QIODevice *io = NULL;
	QMutex mutex;
protected:
	XAudioPlay();
};