#include "video_downloader.h"

VideoDownloader::VideoDownloader(QObject *parent) : QObject(parent), downloadFolderPath(""), outputWindow(new OutputWindow()) {}

void VideoDownloader::downloadVideo(const QString &videoUrl) {
    /*!
     * @brief 在程序根目录下创建VideoDownload文件夹用于存放下载的文件
     */
    downloadFolderPath = QCoreApplication::applicationDirPath() + "/VideoDownload";
    QDir().mkpath(downloadFolderPath);

    QStringList arguments;
    arguments << "-o" << downloadFolderPath + "/%(title)s.%(ext)s" << "-f" << "bv[ext=mp4]+ba[ext=m4a]" << "--embed-metadata"
              << "--merge-output-format" << "mp4" << videoUrl;

    outputWindow->setAttribute(Qt::WA_DeleteOnClose); // 窗口关闭时释放资源
    outputWindow->show();

    /*!
     * @brief 将OutputWindow信号与VideoDownloader连接
     */
    connect(outputWindow, &OutputWindow::errorOccurred, this, &VideoDownloader::processError);
    connect(outputWindow, &OutputWindow::processFinished, this, &VideoDownloader::processFinished);
    connect(outputWindow, &OutputWindow::errorMessageEmit, this, &VideoDownloader::downloadError);

    /*!
     * @brief 启动yt-dlp进行下载
     */
    outputWindow->startProcess("third/yt-dlp", arguments);
}

/*!
 * @brief 处理下载完成信息
 */
void VideoDownloader::processFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    if (exitStatus == QProcess::CrashExit || exitCode != 0) {
        /*!
         * @brief 下载错误退出，给出错误信息
         */
        emit downloadError("下载失败！");
    } else {
        outputWindow->close();
        emit downloadFinished(downloadFolderPath);
    }
}

/*!
 * @brief 处理进程错误信息
 */
void VideoDownloader::processError(QProcess::ProcessError error) {
    QString errorString;

    /*!
     * @brief 根据对应错误代码给出错误信息
     */
    switch (error) {
        case QProcess::FailedToStart:
            errorString = QObject::tr("进程启动失败。可能是调用的程序丢失，也可能是调用程序的权限不足。");
            break;
        case QProcess::Crashed:
            errorString = QObject::tr("进程在成功启动一段时间后崩溃。");
            break;
        case QProcess::Timedout:
            errorString = QObject::tr("最后执行的一个函数超时了。QProcess的状态保持不变，你可以再次尝试调用该函数。");
            break;
        case QProcess::ReadError:
            errorString = QObject::tr("尝试从进程中读取数据时发生错误。例如，进程可能没有运行。");
            break;
        case QProcess::WriteError:
            errorString = QObject::tr("尝试向进程写入时发生错误。例如，进程可能没有运行，或者阻止了输入。");
            break;
        case QProcess::UnknownError:
        default:
            errorString = QObject::tr("发生未知错误。");
            break;
    }

    /*!
     * @brief 显示错误信息
     */
    emit downloadError(errorString);
}