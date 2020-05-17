#include "XAudioPlay.h"

void XAudioPlay::Stop()
{
	mutex.lock();
	if (output)//Ϊ��AudioOutput
	{
		output->stop();
	
		delete output;
		output = NULL;
		io = NULL;
	}
	mutex.unlock();
}

//�������ò��ŵĸ�ʽ�Լ�����
bool XAudioPlay::Start()
{
	Stop();
	mutex.lock();
	QAudioOutput *out;//������Ƶ
	QAudioFormat fmt;//������Ƶ�����ʽ
	fmt.setSampleRate(this->sampleRate);//1�����Ƶ������
	fmt.setSampleSize(this->sampleSize);//���������Ĵ�С
	fmt.setChannelCount(this->channel);//����
	fmt.setCodec("audio/pcm");//�����ʽ
	fmt.setByteOrder(QAudioFormat::LittleEndian);
	fmt.setSampleType(QAudioFormat::SignedInt);//������Ƶ����
	output = new QAudioOutput(fmt);
	io = output->start();//���ſ�ʼ
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
		output->resume();//�ָ�����
	}
	else
	{
		output->suspend();//��ͣ����
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
	int free = output->bytesFree();//ʣ��Ŀռ�

	mutex.unlock();

	return free;
}

bool XAudioPlay::Write(const char *data, int datasize)
{
	mutex.lock();
	if (io)
		io->write(data, datasize);//����ȡ����Ƶд�뵽��������
	mutex.unlock();
	return true;
}

XAudioPlay::XAudioPlay()
{
}


XAudioPlay::~XAudioPlay()
{
}