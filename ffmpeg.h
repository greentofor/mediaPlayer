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
		treewidget相关
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
	void timerEvent(QTimerEvent *event); //当前播放时间
	bool eventFilter(QObject  *obj, QEvent *event);//全屏双击事件

	//进度条设置
	static bool isPressSlider;//是否按下进度条
	void sliderPressed();//按下进度条时		
	void sliderReleased();//松开进度条时
	void  playstaus();  //暂停或者播放

	int i = 1; //控制全屏
	static double doubleSpeed;
	static bool isPlay;
	/*
	三个播放列表
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
