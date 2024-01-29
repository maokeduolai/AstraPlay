#ifndef VIDEO_DOWNLOAD_H
#define VIDEO_DOWNLOAD_H

#include <QObject>
#include <QProcess>
#include <QCoreApplication>
#include <QDir>
#include <QMessageBox>
#include <QDebug>
#include <QString>

#include "output_window.h"

class VideoDownloader : public QObject {
Q_OBJECT

public:
    explicit VideoDownloader(QObject *parent = nullptr);

    ~VideoDownloader() override {
        delete outputWindow;
    }

    void downloadVideo(const QString &videoUrl);

signals:

    void downloadFinished(const QString &filePath);

    void downloadError(const QString &error);

private:

    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void processError(QProcess::ProcessError error);

private:
    QString downloadFolderPath;

    OutputWindow *outputWindow;
};

#endif //VIDEO_DOWNLOAD_H
