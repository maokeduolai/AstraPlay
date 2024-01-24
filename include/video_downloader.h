#ifndef VIDEO_DOWNLOAD_H
#define VIDEO_DOWNLOAD_H

#include <QObject>
#include <QProcess>
#include <QCoreApplication>
#include <QDir>
#include <QMessageBox>
#include <QDebug>

#include "output_window.h"

class VideoDownloader : public QObject {
Q_OBJECT

public:
    explicit VideoDownloader(QObject *parent = nullptr);

    void downloadVideo(const QString &videoUrl);

signals:

    void downloadStarted();

    void downloadProgress(const QString &status);

    void downloadFinished(const QString &filePath);

    void downloadError(const QString &error);

private slots:

    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void processError(QProcess::ProcessError error);

private:
    QProcess *downloadProcess;
    QString downloadedFilePath;
};

#endif //VIDEO_DOWNLOAD_H
