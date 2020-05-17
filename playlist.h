#pragma once
#include "QStringList"
#include "QFileDialog"
#include "QVector"
#include "QString"

class playlist
{public:
	static playlist *Get()//单件模式
	{
		static playlist ff;
		return &ff;
	}

	//三个播放列表文件播放列表的路径
	QStringList current;
	QStringList fileList;
	QStringList history;

	//三个播放列表文件播放列表的名字
	QStringList currentName;
	QStringList fileListName;
	QStringList historyName;

	playlist();
	~playlist();
	void addplaylist(QStringList *);
	void readtohistorylist();
	void storyFileFromCurrentName(int i);
};

