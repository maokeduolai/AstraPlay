#include "screen_capture.h"

ScreenCapture::ScreenCapture(mpv_handle *mpv, QWidget *parent) : QDialog(parent), mpv(mpv), captureCount(0) {
    /*!
     * @brief 创建垂直布局
     */
    auto *mainLayout = new QVBoxLayout;

    /*!
     * @brief 创建“截取当前帧”按钮
     */
    auto *captureCurrentFrameButton = new QPushButton(tr("截取当前帧"), this);
    connect(captureCurrentFrameButton, &QPushButton::clicked, this,
            &ScreenCapture::on_captureCurrentFrameButton_clicked);
    mainLayout->addWidget(captureCurrentFrameButton);  // 将按钮添加到布局中

    /*!
     * @brief 创建水平布局，放置数字输入框和“截取预览图”按钮
     */
    auto *secondRowLayout = new QHBoxLayout;

    /*!
     * @brief 创建数字输入框
     */
    captureCountSpinBox = new QSpinBox(this);
    secondRowLayout->addWidget(captureCountSpinBox);  // 将数字输入框添加到布局中

    /*!
     * @brief 创建“截取预览图”按钮
     */
    auto *capturePreviewButton = new QPushButton(tr("截取预览图"), this);
    connect(capturePreviewButton, &QPushButton::clicked, this, &ScreenCapture::on_capturePreviewButton_clicked);
    secondRowLayout->addWidget(capturePreviewButton);  // 将按钮添加到布局中

    mainLayout->addLayout(secondRowLayout);  // 将第二行的布局添加到主布局中

    setLayout(mainLayout);  // 将主布局设置为这个窗口的布局
}

void ScreenCapture::on_captureCurrentFrameButton_clicked() {
    /*!
     * @brief 获取当前的日期和时间
     */
    QDateTime now = QDateTime::currentDateTime();

    /*!
     * @brief 格式化文件名
     */
    QString tempFileName = QString("%1/AstraPlay_ScreenShot_%2.png")
            .arg(QStandardPaths::writableLocation(QStandardPaths::TempLocation), now.toString("yyyyMMddHHmm"));

    /*!
     * @brief 截图并保存到临时文件中
     */
    QStringList args = {"screenshot-to-file", tempFileName.toUtf8().constData(), "video"};
    mpv::qt::command(mpv, args);

    /*!
     * @brief 弹出保存文件对话框
     */
    QString fileName = QFileDialog::getSaveFileName(this, tr("保存截图"), "", tr("图片文件 (*.png *.jpg)"));
    if (!fileName.isEmpty()) {
        /*!
         * @brief 将临时文件移动到用户选择的位置
         */
        QFile::rename(tempFileName, fileName);

        /*!
         * @brief 关闭窗口
         */
        this->close();
    }
}

void ScreenCapture::on_capturePreviewButton_clicked() {
    /*!
     * @brief 获取用户需要的预览图数量
     */
    captureCount = captureCountSpinBox->value();

    /*!
     * @brief 获取视频的总时长
     */
    double duration;
    duration = getProperty("duration").toDouble();

    /*!
     * @brief 计算截图的时间点
     */
    double step = duration / (captureCount + 1);

    /*!
     * @brief 获取用户输入的基础文件名
     */
    bool ok;
    QString baseFileName = QInputDialog::getText(this, tr("输入文件名"), tr("文件名："), QLineEdit::Normal,
                                                 QDir::home().dirName(), &ok);

    if (!ok || baseFileName.isEmpty()) {
        /*!
         * @brief 用户取消输入或输入的文件名为空，退出函数
         */
        return;
    }

    /*!
     * @brief 获取保存文件的目录
     */
    QString dirName = QFileDialog::getExistingDirectory(this, tr("选择保存目录"), "",
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dirName.isEmpty()) {
        /*!
         * @brief 用户取消选择目录，退出函数
         */
        return;
    }

    /*!
     * @brief 进行截图
     */
    for (int i = 0; i < captureCount; ++i) {
        double time = step * (i + 1);
        mpv::qt::set_property(mpv, "time-pos", time);

        /*!
         * @brief 创建文件名
         */
        QString fileName = QString("%1/%2_%3.png")
                .arg(dirName, baseFileName)
                .arg(i + 1, 3, 10, QChar('0'));  // 使用填充字符'0'和字段宽度3来生成序号

        /*!
         * @brief 截图并保存
         */
        QStringList args = {"screenshot-to-file", fileName.toUtf8().constData(), "video"};
        mpv::qt::command(mpv, args);
    }

    /*!
     * @brief 关闭窗口
     */
    this->close();
}

QVariant ScreenCapture::getProperty(const QString &name) const {
    return mpv::qt::get_property_variant(mpv, name);
}


