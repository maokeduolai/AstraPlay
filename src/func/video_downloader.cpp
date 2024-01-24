#include "video_downloader.h"

VideoDownloader::VideoDownloader(QObject *parent) : QObject(parent) {
    downloadProcess = new QProcess(this);
    connect(downloadProcess, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this,
            &VideoDownloader::processFinished);
    connect(downloadProcess, &QProcess::errorOccurred, this, &VideoDownloader::processError);
}

void VideoDownloader::downloadVideo(const QString &videoUrl) {
    // 在程序根目录下创建VideoDownload文件夹用于存放下载的文件
    QString downloadPath = QCoreApplication::applicationDirPath() + "/VideoDownload";
    QDir().mkpath(downloadPath);

    QStringList arguments;
    arguments << "-o" << downloadPath + "/%(title)s.%(ext)s" << "-f" << "bv[ext=mp4]+ba[ext=m4a]" << "--embed-metadata"
              << "--merge-output-format" << "mp4" << videoUrl;

    auto *outputWindow = new OutputWindow();
    outputWindow->setAttribute(Qt::WA_DeleteOnClose); // 窗口关闭时释放资源
    outputWindow->show();

    outputWindow->startProcess("third/yt-dlp", arguments);
    emit downloadStarted();
}

// 处理下载完成信息
void VideoDownloader::processFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    if (exitStatus == QProcess::CrashExit || exitCode != 0) {
        // 下载错误退出，给出错误信息
        emit downloadError("下载失败！");
    } else {
        emit downloadFinished(downloadedFilePath);
    }
}

// 处理下载中途错误信息
void VideoDownloader::processError(QProcess::ProcessError error) {
    Q_UNUSED(error)
    emit downloadError(downloadProcess->errorString());
}