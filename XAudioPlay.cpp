#include "XAudioPlay.h"

void XAudioPlay::Stop()
{
	mutex.lock();
	if (output)//为打开AudioOutput
	{
		output->stop();
	
		delete output;
		output = NULL;
		io = NULL;
	}
	mutex.unlock();
}

//首先设置播放的格式以及参数
bool XAudioPlay::Start()
{
	Stop();
	mutex.lock();
	QAudioOutput *out;//播放音频
	QAudioFormat fmt;//设置音频输出格式
	fmt.setSampleRate(this->sampleRate);//1秒的音频采样率
	fmt.setSampleSize(this->sampleSize);//声音样本的大小
	fmt.setChannelCount(this->channel);//声道
	fmt.setCodec("audio/pcm");//解码格式
	fmt.setByteOrder(QAudioFormat::LittleEndian);
	fmt.setSampleType(QAudioFormat::SignedInt);//设置音频类型
	output = new QAudioOutput(fmt);
	io = output->start();//播放开始
	mutex.unlock();
	return true;
}

void  XAudioPlay::Play(bool isplay)
{
	mutex.lock();
	if (!output)
	{
		mutex.unlock();
		return;
	}

	if (isplay)
	{
		output->resume();//恢复播放
	}
	else
	{
		output->suspend();//暂停播放
	}
	mutex.unlock();
}


int XAudioPlay::GetFree()
{
	mutex.lock();
	if (!output)
	{
		mutex.unlock();
		return 0;
	}
	int free = output->bytesFree();//剩余的空间

	mutex.unlock();

	return free;
}

bool XAudioPlay::Write(const char *data, int datasize)
{
	mutex.lock();
	if (io)
		io->write(data, datasize);//将获取的音频写入到缓冲区中
	mutex.unlock();
	return true;
}

XAudioPlay::XAudioPlay()
{
}


XAudioPlay::~XAudioPlay()
{
}