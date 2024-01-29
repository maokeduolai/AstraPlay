#include "output_window.h"

OutputWindow::OutputWindow(QWidget *parent) : QWidget(parent), process(new QProcess(this)) {
    /*!
     * @brief 设置窗口属性
     */
    setWindowTitle("Process Output");
    resize(600, 400);

    /*!
     * @brief 创建输出框
     */
    textEdit = new QPlainTextEdit(this);
    textEdit->setReadOnly(true);

    /*!
     * @brief 创建布局
     */
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(textEdit);

    /*!
     * @brief 连接信号和槽
     */
    connect(process, &QProcess::readyReadStandardOutput, this, &OutputWindow::onReadyReadStandardOutput);
    connect(process, &QProcess::readyReadStandardError, this, &OutputWindow::onReadyReadStandardError);
    connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this,
            &OutputWindow::processFinished);
    connect(process, &QProcess::errorOccurred, this, &OutputWindow::errorOccurred);
}

/*!
 * @brief 开始进程
 */
void OutputWindow::startProcess(const QString &program, const QStringList &arguments) {
    /*!
     * @brief 设置Qt执行命令时使用系统的环境配置
     */
    process->setEnvironment(QProcess::systemEnvironment());

    process->start(program, arguments);

    /*!
     * @brief 在输出框中显示执行的命令参数
     */
    QString singleArg = "third/yt-dlp " + arguments.join(" ");
    QByteArray stdStr = singleArg.toUtf8();
    textEdit->appendPlainText(stdStr);
}

/*!
 * @brief 输出正常信息
 */
void OutputWindow::onReadyReadStandardOutput() {
    textEdit->appendPlainText(process->readAllStandardOutput());
}

/*!
 * @brief 输出错误信息
 */
void OutputWindow::onReadyReadStandardError() {
    QString errorMessage = process->readAllStandardError();
    textEdit->appendPlainText(errorMessage);
    emit errorMessageEmit(errorMessage);
}