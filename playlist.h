#pragma once
#include "QStringList"
#include "QFileDialog"
#include "QVector"
#include "QString"

class playlist
{public:
	static playlist *Get()//����ģʽ
	{
		static playlist ff;
		return &ff;
	}

	//���������б��ļ������б��·��
	QStringList current;
	QStringList fileList;
	QStringList history;

	//���������б��ļ������б������
	QStringList currentName;
	QStringList fileListName;
	QStringList historyName;

	playlist();
	~playlist();
	void addplaylist(QStringList *);
	void readtohistorylist();
	void storyFileFromCurrentName(int i);
};

