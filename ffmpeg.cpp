#pragma once
#include "ffmpeg.h"
#include <QWidget>
#include <QMenu>
#include <QAction>
#include "iostream"

bool ffmpeg::isPressSlider = true;
bool ffmpeg::isPlay = false;//是否播放
double ffmpeg::doubleSpeed = 1;
#define  PAUSE "QPushButton\
{border-image: url\
(:/ffmpeg/Resources/pause.png);}"//css语法，暂停按钮
#define  PLAY "QPushButton\
{border-image: url\
(:/ffmpeg/Resources/play.png);}"//播放按钮

ffmpeg::ffmpeg(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	ui.label->installEventFilter(this);
	InittreeList();
	//---------调用read文件将文件的内容提取到storylist中；
	playlist::Get()->readtohistorylist();
	addtoTreehistory();
	ui.pushButtonplayandstop->setStyleSheet(PLAY);
	//暂停播放
	connect(ui.pushButtonplayandstop,&QPushButton::clicked , this, [=]() {
		playstaus();
	});
	//重新读取文件，任何添加到tree中
	connect(ui.pushButton_7, &QPushButton::clicked, this, [=]() {
		playlist::Get()->readtohistorylist();
		addtoTreehistory();
	});
	//添加文件槽函数
	connect(ui.action1, &QAction::triggered, this, [=]() {
		QStringList *fileName = new QStringList;
		*fileName = QFileDialog::getOpenFileNames(this, "Movies", "/", "*.mp4 *.avi");
		playlist::Get()->addplaylist(/*in*/fileName);
		addtoTreeCuttent();
		playListIndex(&playlist::Get()->current);
	});
	connect(ui.pushButton_3, &QPushButton::clicked,this,[=](){
		media::Get()->Close();
		QImage img;
		img = img.scaled(ui.label->size());
		ui.label->setPixmap(QPixmap::fromImage(img));
		ui.totleTime->setText("0");
		ui.playTime->setText("0");
	});
	//终止
	connect(ui.pushButton_5, &QPushButton::clicked, this, [=]() {
		media::Get()->downwork();

	});
	connect(ui.pushButton_4, &QPushButton::clicked, this, [=]() {
		media::Get()->upwwork();
	});
	//播放下一项
	connect(ui.pushButton_6, &QPushButton::clicked, this, [=]() {
		media::Get()->Close();
		clickIndex = clickIndex + 1;
		playListIndex(&playlist::Get()->current,clickIndex);
	});
	connect(ui.pushButton, &QPushButton::clicked, this, [=]() {
		media::Get()->Close();
		clickIndex = clickIndex - 1;
		playListIndex(&playlist::Get()->current, clickIndex);
	});
	connect(ui.doubleSpinBox, SIGNAL(signQSpinBox::valueChanged(double)), this, SLOT(setspeed(double)));
	//connect(ui.horizontalSlider,&QSlider::)
	connect(ui.pushButton_2, &QPushButton::clicked, this, [=]() {
		i++;
		if (i % 2 == 0) //此处为双击一次全屏，再双击一次退出
		{
			ui.label->setWindowFlags(Qt::Dialog);
			ui.label->showFullScreen();//全屏显示
		}
		else
		{
			ui.label->setWindowFlags(Qt::SubWindow);
			ui.label->showNormal();//退出全屏
		};
	});
	//treewidget点击事件
	//connect(ui.treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), 
	//	this, SLOT(checkself(QTreeWidgetItem*, int)));
	//sectionClicked
	connect(ui.treeWidget, SIGNAL(customContextMenuRequested(const QPoint&)), 
		this, SLOT(OnListContextMenu(const QPoint&)));
	startTimer(40);
}
void ffmpeg::on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
	std::cout << (item->text(column)).toStdString()<<"\n";
}

void ffmpeg::playListIndex (QStringList *path, int index)
{
	playlist::Get()->storyFileFromCurrentName(index);
	QString str = (QString)path->value(index);
	//QString      str1 = str.join("..");
	QByteArray byArrat = str.toLatin1(); // 转换成char 
	char *temp = byArrat.data();
	char *filepath = temp;
	if (media::Get()->Open(filepath))
	{
		printf("open success");
	}
	else
	{
		printf("open failed!");
	}
	//设置时间
	char buf[128] = { 0 };
	XAudioPlay::Get()->sampleRate = media::Get()->sampleRate;
	XAudioPlay::Get()->channel = media::Get()->channel;
	XAudioPlay::Get()->sampleSize = 16;
	XAudioPlay::Get()->Start();
//设置总时长
	int min = media::Get()->durationInfo / 60;
	int sec = media::Get()->durationInfo % 60;
	sprintf(buf, "%03d:%02d", min, sec);
	ui.totleTime->setText(buf);

	char out[10000] = { 0 };
	AVFrame *rgb = NULL;

	for (;;)
	{
		if (isPlay==false)
		{
			int free = XAudioPlay::Get()->GetFree();
			AVPacket pkt = media::Get()->Read();//每次读取视频得包
			if (pkt.size <= 0)
				break;
			//printf("pts = %lld\n", pkt.pts);

			if (pkt.stream_index == media::Get()->videoStream) 
			{
				AVFrame *yuv = media::Get()->Decode(&pkt);//解码视频帧  //每一次decode 都会创建一个yuv和rjb，二期忘记释放
				if (rgb != NULL)
				{
					av_frame_free(&rgb);
				}
				if (yuv)
				{
					rgb = media::Get()->ToRGB(rgb, 1920, 1080);//视频转码
					QImage img((uchar*)rgb->data[0], 1920, 1080, QImage::Format_RGB32);
					img = img.scaled(ui.label->size());//
					//ui->label.setPixmap(QPixmap::fromImage(img));
					ui.label->setPixmap(QPixmap::fromImage(img));
				}
				av_free_packet(&pkt);
			} 
			else if (pkt.stream_index == media::Get()->audioStream)
			{
				media::Get()->Decode(&pkt);//解码音频
				
				int len = media::Get()->ToPCM(out);//重采样音频
				XAudioPlay::Get()->Write(out, len);//写入音频
				av_packet_unref(&pkt);//释放pkt包
				continue;
			}
			else
			{
					av_packet_unref(&pkt);
			}
			Delay(doubleSpeed);
				
			//media::Get()->Decode(&pkt);//解码视频帧
			

		}
		else
		{
			for (;isPlay==true;)
			{
				Delay(100);
			}
		}
		//av_packet_unref(&pkt);//重新置pkt为0
	}
	media::Get()->Close();
}

void ffmpeg::addtoTreeCuttent()
{
	for (int i = 0; i < playlist::Get()->currentName.length(); i++)
	{
		int j = playlist::Get()->currentName.count(playlist::Get()->currentName[i]);
		if (j<=1)
		{
			QTreeWidgetItem *vediaName1 = new QTreeWidgetItem(QStringList() << playlist::Get()->currentName[i]);
			this->liitem1 ->addChild(vediaName1);
		}
		
	}

}
void ffmpeg::addtoTreehistory()
{
	static int lasthistory = 0;
	for (int i =lasthistory ; i < playlist::Get()->historyName.length(); i++)
	{
		int j = playlist::Get()->historyName.count(playlist::Get()->historyName[i]);
		if (j <= 1)
		{
			QTreeWidgetItem *vediaName1 = new QTreeWidgetItem(QStringList() << playlist::Get()->historyName[i]);
			this->liitem3->addChild(vediaName1);
		}
	}
	lasthistory = playlist::Get()->historyName.length();
}

void ffmpeg::timerEvent(QTimerEvent * event)
{
	int min = (media::Get()->ptsTime) / 60;
	int sec = (media::Get()->ptsTime) % 60;
	//std::cout << "ptstime变化之后的时间"<< media::Get()->ptsTime << std::endl;
	char buf[1024] = { 0 };
	sprintf(buf, "%03d:%02d / ", min, sec);//存入buf中
	ui.playTime->setText(buf);//显示在界面中
	if (media::Get()->durationInfo > 0)//判断视频得总时间
	{
		float rate = (float)media::Get()->ptsTime / (float)media::Get()->durationInfo;//当前播放的时间与视频总时间的比值
		if (isPressSlider) //当松开时继续刷新进度条位置
			ui.horizontalSlider->setValue(rate * 100);//设置当前进度条位置
	}

}

bool ffmpeg::eventFilter(QObject  *obj, QEvent *event)
{
	if (obj == ui.label)//当事件发生在u1（为Qlabel型）控件上
	{
		if (event->type() == QEvent::MouseButtonDblClick)//当为双击事件时
		{
			i++;
			if (i % 2 == 0) //此处为双击一次全屏，再双击一次退出
			{
				ui.label->setWindowFlags(Qt::Dialog);
				ui.label->showFullScreen();//全屏显示
			}
			else
			{
				ui.label->setWindowFlags(Qt::SubWindow);
				ui.label->showNormal();//退出全屏
			};

		}
		else if (event->type() == QEvent::MouseButtonPress)
		{
			isPlay = !isPlay;
		}
		return QObject::eventFilter(obj, event);
	}
}

void ffmpeg::sliderPressed()
{
	isPressSlider = true;
}
void ffmpeg::sliderReleased()
{
	isPressSlider = false;
	float pos = 0;

	//松开时此时滑动条的位置与滑动条的总长度
	pos = (float)ui.horizontalSlider->value() / (float)(ui.horizontalSlider->maximum() + 1);
	media::Get()->Seek(pos);
}

void ffmpeg::playstaus()
{
	isPlay = !isPlay;
	if (isPlay)
	{
		ui.pushButtonplayandstop->setStyleSheet(PLAY);
	}
	else
	{
		ui.pushButtonplayandstop->setStyleSheet(PAUSE);
	}	
}

void ffmpeg::Delay(int msec)
{
	if (doubleSpeed == 1.5)
	{
		msec = 2;
	}
	else if (doubleSpeed == 0.5)
	{
		msec = 20;
	}
	else
	{
		msec = 5;
	}
	QTime dieTime = QTime::currentTime().addMSecs(msec);
	while (QTime::currentTime() < dieTime)
		QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}
int ffmpeg::InittreeList()
{
	ui.treeWidget->setHeaderLabels(QStringList() << "Table");
	liitem1 = new QTreeWidgetItem(QStringList() << "current");
	liitem2 = new QTreeWidgetItem(QStringList() << "fileList");
	liitem3 = new QTreeWidgetItem(QStringList() << "history");
	ui.treeWidget->addTopLevelItem(liitem1);
	ui.treeWidget->addTopLevelItem(liitem2);
	ui.treeWidget->addTopLevelItem(liitem3);
	return ret;
}

//
//

/*
void ffmpeg::ffon_btnPlay_clicked()
{
	AVFormatContext    *pFormatCtx;
	int                i, videoindex;
	AVCodecContext    *pCodecCtx;
	AVCodec            *pCodec;
	AVFrame    *pFrame, *pFrameRGB;
	unsigned char *out_buffer;
	AVPacket *packet;
	int ret, got_picture;
	struct SwsContext *img_convert_ctx;

	char filepath[] = "H:\\152.mp4";
	//初始化编解码库
	av_register_all();//创建AVFormatContext对象，与码流相关的结构。
	pFormatCtx = avformat_alloc_context();
	//初始化pFormatCtx结构
	if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0) {
		printf("Couldn't open input stream.\n");
		return;
	}
	//获取音视频流数据信息
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
		printf("Couldn't find stream information.\n");
		return;
	}
	videoindex = -1;
	//nb_streams视音频流的个数，这里当查找到视频流时就中断了。
	for (i = 0; i < pFormatCtx->nb_streams; i++)
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoindex = i;
			break;
		}
	if (videoindex == -1) {
		printf("Didn't find a video stream.\n");
		return;
	}
	//获取视频流编码结构
	pCodecCtx = pFormatCtx->streams[videoindex]->codec;
	//查找解码器
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL) {
		printf("Codec not found.\n");
		return;
	}
	//用于初始化pCodecCtx结构
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		printf("Could not open codec.\n");
		return;
	}
	//创建帧结构，此函数仅分配基本结构空间，图像数据空间需通过av_malloc分配
	pFrame = av_frame_alloc();
	pFrameRGB = av_frame_alloc();
	//创建动态内存,创建存储图像数据的空间
	//av_image_get_buffer_size获取一帧图像需要的大小
	out_buffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB32, pCodecCtx->width, pCodecCtx->height, 1));
	av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, out_buffer,
		AV_PIX_FMT_RGB32, pCodecCtx->width, pCodecCtx->height, 1);

	packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	//Output Info-----------------------------
	printf("--------------- File Information ----------------\n");
	//此函数打印输入或输出的详细信息
	av_dump_format(pFormatCtx, 0, filepath, 0);
	printf("-------------------------------------------------\n");
	//初始化img_convert_ctx结构
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
		pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);
	//av_read_frame读取一帧未解码的数据
	while (av_read_frame(pFormatCtx, packet) >= 0) {
		//如果是视频数据
		if (packet->stream_index == videoindex) {
			//解码一帧视频数据
			ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
			if (ret < 0) {
				printf("Decode Error.\n");
				return;
			}
			if (got_picture) {
				sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
					pFrameRGB->data, pFrameRGB->linesize);
				QImage img((uchar*)pFrameRGB->data[0], pCodecCtx->width, pCodecCtx->height, QImage::Format_RGB32);
				img = img.scaled(ui.label->size());//
				//ui->label.setPixmap(QPixmap::fromImage(img));
				ui.label->setPixmap(QPixmap::fromImage(img));
			}
		} 
		av_free_packet(packet);
	}
	sws_freeContext(img_convert_ctx);
	av_frame_free(&pFrameRGB);
	av_frame_free(&pFrame);
	avcodec_close(pCode);
	avformat_close_input(&pFormatCtx);
}
*/




