#pragma once
#include "pch.h"
#include "playlist.h"
#include "QStringList"
#include "QSlider"
#include "media.h"
#include "XAudioPlay.h"

using namespace std;

class ffmpeg : public QMainWindow
{
	Q_OBJECT

public:
	/*
		treewidget���
	*/
	int clickIndex = 0;


	ffmpeg(QWidget *parent = Q_NULLPTR);
//	void ffon_btnPlay_clicked();
	void Delay(int msec);
	int InittreeList();
	void playListIndex(QStringList *,int a=0 );
//	void getName(const char * filepath);
	void addtoTreeCuttent();
	void addtoTreehistory();
	char medianame[128] = { 0 };
	int ret = 0;
	void timerEvent(QTimerEvent *event); //��ǰ����ʱ��
	bool eventFilter(QObject  *obj, QEvent *event);//ȫ��˫���¼�

	//����������
	static bool isPressSlider;//�Ƿ��½�����
	void sliderPressed();//���½�����ʱ		
	void sliderReleased();//�ɿ�������ʱ
	void  playstaus();  //��ͣ���߲���

	int i = 1; //����ȫ��
	static double doubleSpeed;
	static bool isPlay;
	/*
	���������б�
	*/
public:
	QTreeWidgetItem *liitem1 = NULL;
	QTreeWidgetItem *liitem2 = NULL;
	QTreeWidgetItem *liitem3 = NULL;

private:
	Ui::ffmpegClass ui;
public slots:
	void on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);
};
