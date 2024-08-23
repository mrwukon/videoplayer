﻿

#include "VideoPlayerWidget.h"
#include "ui_VideoPlayerWidget.h"

#include <QPainter>
#include <QPaintEvent>
#include <QFileDialog>
#include <QDebug>
#include <QScreen>
#include <QFontDatabase>
#include <QMouseEvent>
#include <QMessageBox>

Q_DECLARE_METATYPE(VideoPlayer::PlayerState)

VideoPlayerWidget::VideoPlayerWidget(QWidget *parent) :
    DragAbleWidget(parent),
    ui(new Ui::VideoPlayerWidget)
{
    ui->setupUi(this->getContainWidget());

    ///初始化播放器
    VideoPlayer::initPlayer();

    setWindowFlags(Qt::FramelessWindowHint);//|Qt::WindowStaysOnTopHint);  //使窗口的标题栏隐藏
    setAttribute(Qt::WA_TranslucentBackground);

    //因为VideoPlayer::PlayerState是自定义的类型 要跨线程传递需要先注册一下
    qRegisterMetaType<VideoPlayer::PlayerState>();

    connect(this, &VideoPlayerWidget::sig_OpenVideoFileFailed, this, &VideoPlayerWidget::slotOpenVideoFileFailed);
    connect(this, &VideoPlayerWidget::sig_OpenSdlFailed, this, &VideoPlayerWidget::slotOpenSdlFailed);
    connect(this, &VideoPlayerWidget::sig_TotalTimeChanged, this, &VideoPlayerWidget::slotTotalTimeChanged);
    connect(this, &VideoPlayerWidget::sig_PlayerStateChanged, this, &VideoPlayerWidget::slotStateChanged);
    connect(this, &VideoPlayerWidget::sig_DisplayVideo, this, &VideoPlayerWidget::slotDisplayVideo);

    connect(ui->pushButton_open, &QPushButton::clicked, this, &VideoPlayerWidget::slotBtnClick);
    connect(ui->toolButton_open, &QPushButton::clicked, this, &VideoPlayerWidget::slotBtnClick);
    connect(ui->pushButton_play, &QPushButton::clicked, this, &VideoPlayerWidget::slotBtnClick);
    connect(ui->pushButton_pause,&QPushButton::clicked, this, &VideoPlayerWidget::slotBtnClick);
    connect(ui->pushButton_stop, &QPushButton::clicked, this, &VideoPlayerWidget::slotBtnClick);
    connect(ui->pushButton_volume, &QPushButton::clicked, this, &VideoPlayerWidget::slotBtnClick);

    connect(ui->horizontalSlider, SIGNAL(sig_valueChanged(int)), this, SLOT(slotSliderMoved(int)));
    connect(ui->horizontalSlider_volume, SIGNAL(valueChanged(int)), this, SLOT(slotSliderMoved(int)));

    ui->page_video->setMouseTracking(true);
    ui->page_video->installEventFilter(this);
    ui->widget_container->installEventFilter(this);

    mPlayer = this;

    mTimer = new QTimer; //定时器-获取当前视频时间
    connect(mTimer, &QTimer::timeout, this, &VideoPlayerWidget::slotTimerTimeOut);
    mTimer->setInterval(500);

    mTimer_CheckControlWidget = new QTimer; //用于控制控制界面的出现和隐藏
    connect(mTimer_CheckControlWidget, &QTimer::timeout, this, &VideoPlayerWidget::slotTimerTimeOut);
    mTimer_CheckControlWidget->setInterval(1500);

    mAnimation_ControlWidget  = new QPropertyAnimation(ui->widget_controller, "geometry");

    ui->stackedWidget->setCurrentWidget(ui->page_open);
    ui->pushButton_pause->hide();

    resize(1024,768);
    setTitle(QStringLiteral("我的播放器-V%1").arg(AppConfig::VERSION_NAME));

    mVolume = mPlayer->getVolume();

}

VideoPlayerWidget::~VideoPlayerWidget()
{
    delete ui;
}

void VideoPlayerWidget::showOutControlWidget()
{

    mAnimation_ControlWidget->setDuration(800);

    int w = ui->widget_controller->width();
    int h = ui->widget_controller->height();
    int x = 0;
    int y = ui->widget_container->height() - ui->widget_controller->height();

    if (ui->widget_controller->isHidden())
    {
        ui->widget_controller->show();
        mAnimation_ControlWidget->setStartValue(ui->widget_controller->geometry());
    }
    else
    {
        mAnimation_ControlWidget->setStartValue(ui->widget_controller->geometry());
    }

//    mAnimation_ControlWidget->setKeyValueAt(0, QRect(0, 0, 00, 00));
//    mAnimation_ControlWidget->setKeyValueAt(0.4, QRect(20, 250, 20, 30));
//    mAnimation_ControlWidget->setKeyValueAt(0.8, QRect(100, 250, 20, 30));
//    mAnimation_ControlWidget->setKeyValueAt(1, QRect(250, 250, 100, 30));
    mAnimation_ControlWidget->setEndValue(QRect(x, y, w, h));
    mAnimation_ControlWidget->setEasingCurve(QEasingCurve::Linear); //设置动画效果

    mAnimation_ControlWidget->start();

}

void VideoPlayerWidget::hideControlWidget()
{
    mAnimation_ControlWidget->setTargetObject(ui->widget_controller);

    mAnimation_ControlWidget->setDuration(300);

    int w = ui->widget_controller->width();
    int h = ui->widget_controller->height();
    int x = 0;
    int y = ui->widget_container->height() + h;

    mAnimation_ControlWidget->setStartValue(ui->widget_controller->geometry());
    mAnimation_ControlWidget->setEndValue(QRect(x, y, w, h));
    mAnimation_ControlWidget->setEasingCurve(QEasingCurve::Linear); //设置动画效果

    mAnimation_ControlWidget->start();
}


void VideoPlayerWidget::slotSliderMoved(int value)
{
    if (QObject::sender() == ui->horizontalSlider)
    {
        mPlayer->seek((qint64)value * 1000000);
    }
    else if (QObject::sender() == ui->horizontalSlider_volume)
    {
        mPlayer->setVolume(value / 100.0);
        ui->label_volume->setText(QString("%1").arg(value));
    }
}

void VideoPlayerWidget::slotTimerTimeOut()
{
    if (QObject::sender() == mTimer)
    {
        qint64 Sec = mPlayer->getCurrentTime();

        ui->horizontalSlider->setValue(Sec);
        qDebug()<<Sec<<"\n";
    //    QString hStr = QString("00%1").arg(Sec/3600);
        QString mStr = QString("00%1").arg(Sec/60);
        QString sStr = QString("00%1").arg(Sec%60);

        QString str = QString("%1:%2").arg(mStr.right(2)).arg(sStr.right(2));
        ui->label_currenttime->setText(str);
    }
    else if (QObject::sender() == mTimer_CheckControlWidget)
    {
        mTimer_CheckControlWidget->stop();
        hideControlWidget();
    }
}

void VideoPlayerWidget::slotBtnClick(bool isChecked)
{
    if (QObject::sender() == ui->pushButton_play)
    {
        mPlayer->play();
    }
    else if (QObject::sender() == ui->pushButton_pause)
    {
        mPlayer->pause();
    }
    else if (QObject::sender() == ui->pushButton_stop)
    {
        mPlayer->stop(true);
    }
    else if (QObject::sender() == ui->pushButton_open || QObject::sender() == ui->toolButton_open)
    {
        QString s = QFileDialog::getOpenFileName(
                   this, QStringLiteral("选择要播放的文件"),
                    "/",//初始目录
                    QStringLiteral("视频文件 (*.flv *.rmvb *.avi *.MP4 *.mkv);;")
                    +QStringLiteral("音频文件 (*.mp3 *.wma *.wav);;")
                    +QStringLiteral("所有文件 (*.*)"));
        if (!s.isEmpty())
        {
            s.replace("/","\\");

            mPlayer->stop(true); //如果在播放则先停止
            mPlayer->startPlay(s.toStdString());
        }
    }
    else if (QObject::sender() == ui->pushButton_volume)
    {
       qDebug()<<isChecked;

        bool isMute = isChecked;
        mPlayer->setMute(isMute);

        if (isMute)
        {
            mVolume = mPlayer->getVolume();

            ui->horizontalSlider_volume->setValue(0);
            ui->horizontalSlider_volume->setEnabled(false);
            ui->label_volume->setText(QString("%1").arg(0));
        }
        else
        {
            int volume = mVolume * 100.0;
            ui->horizontalSlider_volume->setValue(volume);
            ui->horizontalSlider_volume->setEnabled(true);
            ui->label_volume->setText(QString("%1").arg(volume));
        }

    }

}

///打开文件失败
void VideoPlayerWidget::doOpenVideoFileFailed(const int &code)
{
    emit sig_OpenVideoFileFailed(code);
}

///打开SDL失败的时候回调此函数
void VideoPlayerWidget::doOpenSdlFailed(const int &code)
{
    emit sig_OpenSdlFailed(code);
}

///获取到视频时长的时候调用此函数
void VideoPlayerWidget::doTotalTimeChanged(const int64_t &uSec)
{
    emit sig_TotalTimeChanged(uSec);
}

///播放器状态改变的时候回调此函数
void VideoPlayerWidget::doPlayerStateChanged(const VideoPlayer::PlayerState &state, const bool &hasVideo, const bool &hasAudio)
{
    emit sig_PlayerStateChanged(state, hasVideo, hasAudio);
}

void VideoPlayerWidget::slotOpenVideoFileFailed(const int &code)
{
    QMessageBox::critical(NULL, "tips", QString("open file failed %1").arg(code));
}

void VideoPlayerWidget::slotOpenSdlFailed(const int &code)
{
    QMessageBox::critical(NULL, "tips", QString("open Sdl failed %1").arg(code));
}

void VideoPlayerWidget::slotTotalTimeChanged(const qint64 &uSec)
{
    qint64 Sec = uSec/1000000;

    ui->horizontalSlider->setRange(0,Sec);

//    QString hStr = QString("00%1").arg(Sec/3600);
    QString mStr = QString("00%1").arg(Sec/60);
    QString sStr = QString("00%1").arg(Sec%60);

    QString str = QString("%1:%2").arg(mStr.right(2)).arg(sStr.right(2));
    ui->label_totaltime->setText(str);

}

void VideoPlayerWidget::slotStateChanged(const PlayerState &state, const bool &hasVideo, const bool &hasAudio)
{
    if (state == VideoPlayer::Stop)
    {
        ui->stackedWidget->setCurrentWidget(ui->page_open);

        ui->pushButton_pause->hide();
        ui->widget_videoPlayer->clear();

        ui->horizontalSlider->setValue(0);
        ui->label_currenttime->setText("00:00");
        ui->label_totaltime->setText("00:00");

        mTimer->stop();

    }
    else if (state == VideoPlayer::Playing)
    {
        if (hasVideo)
        {
            ui->stackedWidget->setCurrentWidget(ui->page_video);
        }
        else
        {
            ui->stackedWidget->setCurrentWidget(ui->page_audio);
        }

        ui->pushButton_play->hide();
        ui->pushButton_pause->show();

        mTimer->start();
    }
    else if (state == VideoPlayer::Pause)
    {
        ui->pushButton_pause->hide();
        ui->pushButton_play->show();
    }
}

///显示rgb数据，此函数不宜做耗时操作，否则会影响播放的流畅性，传入的brgb32Buffer，在函数返回后既失效。
void VideoPlayerWidget::doDisplayVideo(const uint8_t *brgb32Buffer, const int &width, const int &height)
{
//    qDebug()<<__FUNCTION__<<width<<height;

    //把这个RGB数据 用QImage加载
    QImage tmpImg((uchar *)brgb32Buffer, width, height, QImage::Format_RGB32);
//  QImage image = tmpImg.copy(); //把图像复制一份 传递给界面显示
    QImage image = tmpImg.convertToFormat(QImage::Format_RGB888,Qt::NoAlpha); //去掉透明的部分 有些奇葩的视频会透明

    emit sig_DisplayVideo(image);
}

void VideoPlayerWidget::slotDisplayVideo(const QImage &image)
{
//    qDebug()<<__FUNCTION__<<image;
    ui->widget_videoPlayer->inputOneFrame(image);
}

//图片显示部件时间过滤器处理
bool VideoPlayerWidget::eventFilter(QObject *target, QEvent *event)
{
    if(target == ui->widget_container)
    {
        if(event->type() == QEvent::Resize)
        {
            ///停止动画，防止此时刚好开始动画，导致位置出错
            mAnimation_ControlWidget->stop();

            QResizeEvent * e = (QResizeEvent*)event;
            int w = e->size().width();
            int h = e->size().height();
            ui->stackedWidget->move(0, 0);
            ui->stackedWidget->resize(w, h);

            int x = 0;
            int y = h - ui->widget_controller->height();
            ui->widget_controller->move(x, y);
            ui->widget_controller->resize(w, ui->widget_controller->height());
        }
    }
    else if(target == ui->page_video)
    {
        if(event->type() == QEvent::MouseMove)
        {
            if (!mTimer_CheckControlWidget->isActive())
            {
                showOutControlWidget();
            }

            mTimer_CheckControlWidget->stop();
            mTimer_CheckControlWidget->start();
        }
        else if(event->type() == QEvent::Enter)
        {
            ui->widget_controller->show();
        }
        else if(event->type() == QEvent::Leave)
        {
            mTimer_CheckControlWidget->stop();
            mTimer_CheckControlWidget->start();
        }
    }

    //其它部件产生的事件则交给基类处理
    return DragAbleWidget::eventFilter(target, event);
}
