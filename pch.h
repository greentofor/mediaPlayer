#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_ffmpeg.h"
#include "QPixmap"
#include "QPushButton"
#include <QTime>
#include "QDebug"



extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
#include <libavutil/mathematics.h>
#include <libavdevice/avdevice.h>
#include <libswresample/swresample.h>
#include "stdio.h"

}
