#include "playlist.h"
#include "QFile"
#include "QTextStream"

playlist::playlist()
{
}

playlist::~playlist()
{
}

void playlist::addplaylist(QStringList *plist)
{
	char  * temp = NULL;
	int fileNum = plist->length();
	current = *plist;
	//QStringList temp = plist->filter("\\");
	QString *p1 = NULL;
	QString str= plist[0].join("..");  //×ª»»³É×Ö·û´®
	//ÇÐ¸î×Ö·û´®
	for (int i =0;i<fileNum;i++)
	{
		p1 = new QString;
		*p1 = str.section("..", i, i);

		QByteArray byArrat = p1->toLatin1(); // ×ª»»³Échar 
		temp = byArrat.data();
		char *name = new char(p1->size());  //»¹Î´ÊÍ·Å  ---£¡£¡£¡£¡

		int fileLength = p1->size();
		int endt = fileLength;
		int sart = endt;

		for (int i = 0; i < 128; i++)
		{
			if (temp[sart] == '/')
			{
				break;
			}
			sart--;
		}
		for (int i = 0; endt > sart; sart++, i++)
		{
				
				name[i] = temp[sart + 1];
		}
		QString nema = QString(QLatin1String(name));

		this->currentName.append(nema);
		
		delete p1;
		//delete named;
		p1 = NULL;
	}

}

void playlist::readtohistorylist()
{
	this->history.clear();
	this->historyName.clear();
	QFile file("binary.txt");
	char buf[128];
	file.open(QIODevice::ReadOnly);
	for (int i = 0; i < 20; i++)
	{
		if (i % 2 == 0)
		{
			memset(buf, 0, sizeof(buf));
			if (file.readLine(buf, sizeof(buf)) == -1)
				return;
			QString a;
			a.append(buf);
			a = a.simplified();
			history.append(a);
		}
		else
		{
			memset(buf, 0, sizeof(buf));
			if (file.readLine(buf, sizeof(buf)) == -1)
				return;
			QString a;
			a.append(buf);
			a = a.simplified();
			historyName.append(a);
		}
	}
}
void playlist::storyFileFromCurrentName(int i)
{
	this->history.append(this->current[i]);
	this->historyName.append(this->currentName[i]);
	QFile file("binary.txt");
	file.open(QIODevice::WriteOnly | QIODevice::Truncate);
	QTextStream  out(&file);
	for (int i=0;i<historyName.count();i++)
	{
		out << history[i]<<"\n";
		out << historyName[i]<<"\n";
	}
	file.close();
}
