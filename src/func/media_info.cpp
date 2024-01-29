#include "media_info.h"

MediaInfo::MediaInfo(QObject *parent) : QObject(parent), textEdit(new MyTextEdit) {
    mediaInfoProcess = new QProcess(this);
    connect(mediaInfoProcess, &QProcess::readyReadStandardOutput, this, &MediaInfo::onReadyReadStandardOutput);
}

void MediaInfo::readRawAttribute(const QString &filename) {
    if (filename.isEmpty()) {
        /*!
         * @brief 创建一个消息框来显示错误
         */
        auto *errorMessageBox = new QMessageBox;
        errorMessageBox->setAttribute(Qt::WA_DeleteOnClose); // 对话框关闭时自动释放
        errorMessageBox->setWindowTitle(tr("读取元数据错误"));
        errorMessageBox->setText(tr("在读取元数据过程中发生错误:"));
        errorMessageBox->setInformativeText("无正在播放的文件！");
        errorMessageBox->setStandardButtons(QMessageBox::Ok);
        errorMessageBox->setDefaultButton(QMessageBox::Ok);
        errorMessageBox->setIcon(QMessageBox::Warning);

        /*!
         * @brief 显示消息框
         */
        errorMessageBox->show();
        return;
    }

    /*!
     * @brief 构建MediaInfo命令参数
     */
    QStringList arguments;
    arguments << filename;

    /*!
     * @brief 启动MediaInfo进程
     */
    mediaInfoProcess->start("third/Mediainfo/mediainfo", arguments);

    /*!
     * @brief 创建一个只读的TextEdit来显示输出
     */
    textEdit->setReadOnly(true);
    textEdit->setWindowTitle("Media Info");
    textEdit->resize(600, 400);
    textEdit->show();
}

void MediaInfo::onReadyReadStandardOutput() {
    textEdit->appendPlainText(mediaInfoProcess->readAllStandardOutput());
}
