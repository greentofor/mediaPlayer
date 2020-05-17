#pragma once
#include "ffmpeg.h"
#include <QWidget>
#include <QMenu>
#include <QAction>
#include "iostream"

bool ffmpeg::isPressSlider = true;
bool ffmpeg::isPlay = false;//�Ƿ񲥷�
double ffmpeg::doubleSpeed = 1;
#define  PAUSE "QPushButton\
{border-image: url\
(:/ffmpeg/Resources/pause.png);}"//css�﷨����ͣ��ť
#define  PLAY "QPushButton\
{border-image: url\
(:/ffmpeg/Resources/play.png);}"//���Ű�ť

ffmpeg::ffmpeg(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	ui.label->installEventFilter(this);
	InittreeList();
	//---------����read�ļ����ļ���������ȡ��storylist�У�
	playlist::Get()->readtohistorylist();
	addtoTreehistory();
	ui.pushButtonplayandstop->setStyleSheet(PLAY);
	//��ͣ����
	connect(ui.pushButtonplayandstop,&QPushButton::clicked , this, [=]() {
		playstaus();
	});
	//���¶�ȡ�ļ����κ���ӵ�tree��
	connect(ui.pushButton_7, &QPushButton::clicked, this, [=]() {
		playlist::Get()->readtohistorylist();
		addtoTreehistory();
	});
	//����ļ��ۺ���
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
	//��ֹ
	connect(ui.pushButton_5, &QPushButton::clicked, this, [=]() {
		media::Get()->downwork();

	});
	connect(ui.pushButton_4, &QPushButton::clicked, this, [=]() {
		media::Get()->upwwork();
	});
	//������һ��
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
		if (i % 2 == 0) //�˴�Ϊ˫��һ��ȫ������˫��һ���˳�
		{
			ui.label->setWindowFlags(Qt::Dialog);
			ui.label->showFullScreen();//ȫ����ʾ
		}
		else
		{
			ui.label->setWindowFlags(Qt::SubWindow);
			ui.label->showNormal();//�˳�ȫ��
		};
	});
	//treewidget����¼�
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
	QByteArray byArrat = str.toLatin1(); // ת����char 
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
	//����ʱ��
	char buf[128] = { 0 };
	XAudioPlay::Get()->sampleRate = media::Get()->sampleRate;
	XAudioPlay::Get()->channel = media::Get()->channel;
	XAudioPlay::Get()->sampleSize = 16;
	XAudioPlay::Get()->Start();
//������ʱ��
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
			AVPacket pkt = media::Get()->Read();//ÿ�ζ�ȡ��Ƶ�ð�
			if (pkt.size <= 0)
				break;
			//printf("pts = %lld\n", pkt.pts);

			if (pkt.stream_index == media::Get()->videoStream) 
			{
				AVFrame *yuv = media::Get()->Decode(&pkt);//������Ƶ֡  //ÿһ��decode ���ᴴ��һ��yuv��rjb�����������ͷ�
				if (rgb != NULL)
				{
					av_frame_free(&rgb);
				}
				if (yuv)
				{
					rgb = media::Get()->ToRGB(rgb, 1920, 1080);//��Ƶת��
					QImage img((uchar*)rgb->data[0], 1920, 1080, QImage::Format_RGB32);
					img = img.scaled(ui.label->size());//
					//ui->label.setPixmap(QPixmap::fromImage(img));
					ui.label->setPixmap(QPixmap::fromImage(img));
				}
				av_free_packet(&pkt);
			} 
			else if (pkt.stream_index == media::Get()->audioStream)
			{
				media::Get()->Decode(&pkt);//������Ƶ
				
				int len = media::Get()->ToPCM(out);//�ز�����Ƶ
				XAudioPlay::Get()->Write(out, len);//д����Ƶ
				av_packet_unref(&pkt);//�ͷ�pkt��
				continue;
			}
			else
			{
					av_packet_unref(&pkt);
			}
			Delay(doubleSpeed);
				
			//media::Get()->Decode(&pkt);//������Ƶ֡
			

		}
		else
		{
			for (;isPlay==true;)
			{
				Delay(100);
			}
		}
		//av_packet_unref(&pkt);//������pktΪ0
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
	//std::cout << "ptstime�仯֮���ʱ��"<< media::Get()->ptsTime << std::endl;
	char buf[1024] = { 0 };
	sprintf(buf, "%03d:%02d / ", min, sec);//����buf��
	ui.playTime->setText(buf);//��ʾ�ڽ�����
	if (media::Get()->durationInfo > 0)//�ж���Ƶ����ʱ��
	{
		float rate = (float)media::Get()->ptsTime / (float)media::Get()->durationInfo;//��ǰ���ŵ�ʱ������Ƶ��ʱ��ı�ֵ
		if (isPressSlider) //���ɿ�ʱ����ˢ�½�����λ��
			ui.horizontalSlider->setValue(rate * 100);//���õ�ǰ������λ��
	}

}

bool ffmpeg::eventFilter(QObject  *obj, QEvent *event)
{
	if (obj == ui.label)//���¼�������u1��ΪQlabel�ͣ��ؼ���
	{
		if (event->type() == QEvent::MouseButtonDblClick)//��Ϊ˫���¼�ʱ
		{
			i++;
			if (i % 2 == 0) //�˴�Ϊ˫��һ��ȫ������˫��һ���˳�
			{
				ui.label->setWindowFlags(Qt::Dialog);
				ui.label->showFullScreen();//ȫ����ʾ
			}
			else
			{
				ui.label->setWindowFlags(Qt::SubWindow);
				ui.label->showNormal();//�˳�ȫ��
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

	//�ɿ�ʱ��ʱ��������λ���뻬�������ܳ���
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
	//��ʼ��������
	av_register_all();//����AVFormatContext������������صĽṹ��
	pFormatCtx = avformat_alloc_context();
	//��ʼ��pFormatCtx�ṹ
	if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0) {
		printf("Couldn't open input stream.\n");
		return;
	}
	//��ȡ����Ƶ��������Ϣ
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
		printf("Couldn't find stream information.\n");
		return;
	}
	videoindex = -1;
	//nb_streams����Ƶ���ĸ��������ﵱ���ҵ���Ƶ��ʱ���ж��ˡ�
	for (i = 0; i < pFormatCtx->nb_streams; i++)
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoindex = i;
			break;
		}
	if (videoindex == -1) {
		printf("Didn't find a video stream.\n");
		return;
	}
	//��ȡ��Ƶ������ṹ
	pCodecCtx = pFormatCtx->streams[videoindex]->codec;
	//���ҽ�����
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL) {
		printf("Codec not found.\n");
		return;
	}
	//���ڳ�ʼ��pCodecCtx�ṹ
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		printf("Could not open codec.\n");
		return;
	}
	//����֡�ṹ���˺�������������ṹ�ռ䣬ͼ�����ݿռ���ͨ��av_malloc����
	pFrame = av_frame_alloc();
	pFrameRGB = av_frame_alloc();
	//������̬�ڴ�,�����洢ͼ�����ݵĿռ�
	//av_image_get_buffer_size��ȡһ֡ͼ����Ҫ�Ĵ�С
	out_buffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB32, pCodecCtx->width, pCodecCtx->height, 1));
	av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, out_buffer,
		AV_PIX_FMT_RGB32, pCodecCtx->width, pCodecCtx->height, 1);

	packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	//Output Info-----------------------------
	printf("--------------- File Information ----------------\n");
	//�˺�����ӡ������������ϸ��Ϣ
	av_dump_format(pFormatCtx, 0, filepath, 0);
	printf("-------------------------------------------------\n");
	//��ʼ��img_convert_ctx�ṹ
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
		pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);
	//av_read_frame��ȡһ֡δ���������
	while (av_read_frame(pFormatCtx, packet) >= 0) {
		//�������Ƶ����
		if (packet->stream_index == videoindex) {
			//����һ֡��Ƶ����
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




