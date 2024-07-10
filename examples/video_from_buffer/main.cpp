﻿/*********************************************************
 * Copyright (C) 2020, Val Doroshchuk <valbok@gmail.com> *
 *                                                       *
 * This file is part of QtAVPlayer.                      *
 * Free Qt Media Player based on FFmpeg.                 *
 *********************************************************/

#include <QtAVPlayer/qavplayer.h>
#include <QtAVPlayer/qavvideoframe.h>
#include <QtAVPlayer/qavaudiooutput.h>
#include <QtAVPlayer/qaviodevice.h>
#include <QVideoWidget>
#include <QApplication>
#include <QDebug>
#include <QThread>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QAbstractVideoSurface>
#include <QVideoSurfaceFormat>
#include <QMediaService>
#include <QMediaObject>
#include <QVideoRendererControl>
#include "file1.h"
#include "rawdataparser.h"

class VideoRenderer : public QVideoRendererControl
{
public:
    QAbstractVideoSurface *surface() const override
    {
        return m_surface;
    }

    void setSurface(QAbstractVideoSurface *surface) override
    {
        m_surface = surface;
    }

    QAbstractVideoSurface *m_surface = nullptr;
};

class MediaService : public QMediaService
{
public:
    MediaService(VideoRenderer *vr, QObject* parent = nullptr)
        : QMediaService(parent)
        , m_renderer(vr)
    {
    }

    QMediaControl* requestControl(const char *name) override
    {
        if (qstrcmp(name, QVideoRendererControl_iid) == 0)
            return m_renderer;

        return nullptr;
    }

    void releaseControl(QMediaControl *) override
    {
    }

    VideoRenderer *m_renderer = nullptr;
};

class MediaObject : public QMediaObject
{
public:
    explicit MediaObject(VideoRenderer *vr, QObject* parent = nullptr)
        : QMediaObject(parent, new MediaService(vr, parent))
    {
    }
};

class VideoWidget : public QVideoWidget
{
public:
    bool setMediaObject(QMediaObject *object) override
    {
        return QVideoWidget::setMediaObject(object);
    }
};
#else
#include <QVideoSink>
#endif

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    VideoRenderer vr;

    VideoWidget w;
    w.resize(1280, 720);
    w.show();

    MediaObject mo(&vr);
    w.setMediaObject(&mo);
#else
    QVideoWidget w;
    w.show();
#endif
    QAVPlayer p;
    QSharedPointer<File1> file1(new File1());
    QSharedPointer<QAVIODevice> dev(new QAVIODevice(file1));

    QString fileName = QStringLiteral("C:\\workspace\\l2xx\\QtAVPlayer\\video_1_2_10Mbps.dat");
    RawDataParser *parser = new RawDataParser(fileName);
    parser->setFile1(file1);
    QThread *parseThread = new QThread();
    parser->moveToThread(parseThread);
    parseThread->start();
    QMetaObject::invokeMethod(parser, "start", Qt::QueuedConnection);

    p.setSource(fileName, dev);
    p.play();

    QAVAudioOutput audioOutput;
    //audioOutput.setBufferSize(128 * 1024);
    QObject::connect(&p, &QAVPlayer::audioFrame, &audioOutput, [&audioOutput](const QAVAudioFrame &frame) { audioOutput.play(frame); }, Qt::DirectConnection);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QObject::connect(&p, &QAVPlayer::videoFrame, &p, [&](const QAVVideoFrame &frame) {
        if (vr.m_surface == nullptr)
            return;
        QVideoFrame videoFrame = frame.convertTo(AV_PIX_FMT_RGB32);
        if (!vr.m_surface->isActive() || vr.m_surface->surfaceFormat().frameSize() != videoFrame.size()) {
            QVideoSurfaceFormat f(videoFrame.size(), videoFrame.pixelFormat(), videoFrame.handleType());
            vr.m_surface->start(f);
        }
        if (vr.m_surface->isActive())
            vr.m_surface->present(videoFrame);
    }, Qt::DirectConnection);
#else
    QObject::connect(&p, &QAVPlayer::videoFrame, &p, [&](const QAVVideoFrame &frame) {
        QVideoFrame videoFrame = frame;
        w.videoSink()->setVideoFrame(videoFrame);
    }, Qt::DirectConnection);
#endif

    QObject::connect(&p, &QAVPlayer::mediaStatusChanged, [&](auto status) {
        qDebug() << "mediaStatusChanged"<< status << p.state();
        if (status == QAVPlayer::LoadedMedia) {
            qDebug() << "Video streams:" << p.currentVideoStreams().size();
            for (const auto &s: p.currentVideoStreams())
                qDebug() << "[" << s.index() << "]" << s.metadata() << s.framesCount() << "frames," << s.frameRate() << "frame rate";
            qDebug() << "Audio streams:" << p.currentAudioStreams().size();
            for (const auto &s: p.currentAudioStreams())
                qDebug() << "[" << s.index() << "]" << s.metadata() << s.framesCount() << "frames," << s.frameRate() << "frame rate";
        } else if (status == QAVPlayer::EndOfMedia) {
            audioOutput.stop();
        }
    });

    return app.exec();
}

