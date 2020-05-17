#pragma once
#include "ffmpeg.h"
#include <QtWidgets/QApplication>


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	ffmpeg w;
	w.show();
	return a.exec();
}
