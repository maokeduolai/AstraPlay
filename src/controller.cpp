#include "controller.h"
#include "application.h"

Controller::Controller(Application *app, QObject *parent)
        : QObject(parent), mpv(mpv_create()), application(app), sliderBeingDragged(false), sliderInitialized(false),
          duration(0.0), zoomFactor(0.0), panX(0.0), panY(0.0), frameRate(0.0) {
    /*!
     * @brief 根据滑块是否被按下，来判断是否处于拖动滑块状态
     */
    QSlider *slider = application->getSlider();
    if (slider) {
        connect(slider, &QSlider::sliderPressed, this, &Controller::sliderDragStarted);
        connect(slider, &QSlider::sliderReleased, this, &Controller::sliderDragStopped);
    }

    /*!
     * @brief 初始化MPV实例
     */
    if (mpv) {
        /*!
         * @brief 设置视频输出驱动为OpenGL
         */
        mpv_set_option_string(mpv, "vo", "opengl");

        /*!
         * @brief 设置音频输出驱动为WASAPI
         */
        mpv_set_option_string(mpv, "ao", "wasapi");

        /*!
         * @brief 启用硬件解码
         */
        mpv_set_option_string(mpv, "hwdec", "auto");

        /*!
         * @brief 设置视频同步模式
         */
        mpv_set_option_string(mpv, "video-sync", "display-resample");

        /*!
         * @brief 设置视频循环播放
         */
        mpv_set_option_string(mpv, "loop-file", "inf");

        /*!
         * @brief 设置初始音量为80
         */
        setProperty("volume", 80);

        /*!
         * @brief 初始化MPV完成后启动它
         */
        if (mpv_initialize(mpv) < 0) {
            /*!
             * @brief 错误处理
             */
            QMessageBox::critical(reinterpret_cast<QWidget *>(app), tr("错误"), tr("MPV初始化失败"));
        }
    }

    /*!
     * @brief 设置定时器
     */
    auto *timer = new QTimer(this);

    /*!
     * @brief 每秒更新一次播放进度
     */
    connect(timer, &QTimer::timeout, this, &Controller::updateSliderPosition);

    /*!
     * @brief 检测视频时长变化，应对在线视频在开始播放时，可能并未完全加载，其总时长未知的情形
     */
    connect(timer, &QTimer::timeout, this, &Controller::updateSliderDuration);
    timer->start(1000);
}

Controller::~Controller() {
    if (mpv) {
        /*!
         * @brief 清理MPV资源
         */
        mpv_terminate_destroy(mpv);
    }
}

/*!
 * @brief 将MPV的视频输出绑定到QWidget上
 */
void Controller::setPlayerWidget(QWidget *widget) {
    if (widget == nullptr) {  // 检查widget是否为空
        QMessageBox::critical(reinterpret_cast<QWidget *>(application), tr("错误"), tr("Widget为空无法绑定！"));
        return;
    }
    mpv_set_option_string(mpv, "wid", QString::number(widget->winId()).toUtf8().constData());
}

/*!
 * @brief 打开文件
 */
void Controller::openFile(const QString &filename) {
    QStringList args = {"loadfile", filename};
    command(args);

    /*!
     * @brief 确保播放状态正确
     */
    QVariant pauseValue = getProperty("pause");
    const bool isPaused = pauseValue.toBool();
    if (isPaused) {
        Controller::togglePlayPause();
    }

    /*!
     * @brief 初始化滑块
     */
    initializeSliderDuration();

    /*!
     * @brief 更新进度条初始化状态
     */
    sliderInitialized = true;

    /*!
     * @brief 切换播放图标到正在播放状态
     */
    application->updatePlayIcon(true);
}

/*!
 * @brief 打开URL
 */
void Controller::handleUrl(const QString &url) {
    QStringList args = {"loadfile", url};
    command(args);

    /*!
     * @brief 确保播放状态正确
     */
    QVariant pauseValue = getProperty("pause");
    const bool isPaused = pauseValue.toBool();
    if (isPaused) {
        Controller::togglePlayPause();
    }

    /*!
     * @brief 初始化滑块
     */
    initializeSliderDuration();

    /*!
     * @brief 更新进度条初始化状态
     */
    sliderInitialized = true;

    /*!
     * @brief 切换播放图标到正在播放状态
     */
    application->updatePlayIcon(true);
}

/*!
 * @brief 初始化滑块的总时长
 */
void Controller::initializeSliderDuration() {
    duration = Controller::getProperty("duration").toDouble();
    if (mpv) {
        if (duration > 0) {
            /*!
             * @brief 设置滑块的最大值为视频总时长（秒）
             */
            QSlider *slider = application->getSlider();
            slider->setMaximum(static_cast<int>(duration));
        }
    }

    /*!
     * @brief 设置时间显示
     */
    totalTime = QTime((int) (duration / 3600) % 60, (int) (duration / 60) % 60, (int) duration % 60);
    QString timeString = "00:00:00/" + totalTime.toString("hh:mm:ss");
    application->timeLabel->setText(timeString);
}

/*!
 * @brief 应对在线视频在开始播放时，可能并未完全加载，其总时长未知需要缓冲后更新的情况
 */
void Controller::updateSliderDuration() {
    double newDuration = Controller::getProperty("duration").toDouble();
    if (newDuration != duration) {
        /*!
         * @brief 更新视频总时长值
         */
        duration = newDuration;

        /*!
         * @brief 设置滑块的最大值为视频总时长（秒）
         */
        QSlider *slider = application->getSlider();
        slider->setMaximum(static_cast<int>(duration));

        /*!
         * @brief 更新时间显示
         */
        int64_t time;
        mpv_get_property(mpv, "time-pos", MPV_FORMAT_INT64, &time);
        QTime currentTime((int) (time / 3600) % 60, (int) (time / 60) % 60, (int) time % 60);
        totalTime = QTime((int) (newDuration / 3600) % 60, (int) (newDuration / 60) % 60, (int) newDuration % 60);
        QString timeString = currentTime.toString("hh:mm:ss") + "/" + totalTime.toString("hh:mm:ss");
        application->timeLabel->setText(timeString);
    }
}

/*!
 * @brief 播放时更新滑块位置
 */
void Controller::updateSliderPosition() {
    /*!
     * @brief 当MPV实例已完成初始化，播放进度滑块不处于拖动状态，滑块已初始化的情况下才更新滑块位置
     */
    if (mpv && !sliderBeingDragged && sliderInitialized) {
        /*!
         * @brief 获取当前播放时间
         */
        int64_t time;
        mpv_get_property(mpv, "time-pos", MPV_FORMAT_INT64, &time);

        /*!
         * @brief 设置滑块的位置
         */
        QSlider *slider = application->getSlider();
        if (slider) { slider->setValue(static_cast<int>(time)); }

        /*!
         * @brief 更新时间显示
         */
        QTime currentTime((int) (time / 3600) % 60, (int) (time / 60) % 60, (int) time % 60);
        QString timeString = currentTime.toString("hh:mm:ss") + "/" + totalTime.toString("hh:mm:ss");
        application->timeLabel->setText(timeString);
    } else if (mpv && sliderInitialized) {// 当MPV实例已完成初始化，滑块已初始化的情况下继续更新时间显示
        /*!
         * @brief 更新时间显示
         */
        int64_t time;
        mpv_get_property(mpv, "time-pos", MPV_FORMAT_INT64, &time);
        QTime currentTime((int) (time / 3600) % 60, (int) (time / 60) % 60, (int) time % 60);
        QString timeString = currentTime.toString("hh:mm:ss") + "/" + totalTime.toString("hh:mm:ss");
        application->timeLabel->setText(timeString);
    }
}

/*!
 * @brief 开始拖动播放进度滑块，更新状态
 */
void Controller::sliderDragStarted() {
    sliderBeingDragged = true;
}

/*!
 * @brief 结束拖动播放进度滑块，更新状态
 */
void Controller::sliderDragStopped() {
    sliderBeingDragged = false;
}

/*!
 * @brief 放大视频10%
 */
void Controller::zoomIn() {
    zoomFactor += 0.1;
    if (zoomFactor < 3.0) {
        setProperty("video-zoom", zoomFactor);
    }
}

/*!
 * @brief 缩小视频10%
 */
void Controller::zoomOut() {
    zoomFactor -= 0.1;
    if (zoomFactor > -3.0) {
        setProperty("video-zoom", zoomFactor);
    }
}

/*!
 * @brief 重置视频缩放
 */
void Controller::zoomReset() {
    zoomFactor = 0.0;
    setProperty("video-zoom", 0.0);
}

/*!
 * @brief 视频位置控制
 */
void Controller::moveLeft() {
    panX -= 0.1;
    setProperty("video-pan-x", panX);
}

void Controller::moveRight() {
    panX += 0.1;
    setProperty("video-pan-x", panX);
}

void Controller::moveUp() {
    panY -= 0.1;
    setProperty("video-pan-y", panY);
}

void Controller::moveDown() {
    panY += 0.1;
    setProperty("video-pan-y", panY);
}

void Controller::moveReset() {
    panX = 0.0;
    panY = 0.0;
    setProperty("video-pan-x", panX);
    setProperty("video-pan-y", panY);
}

/*!
 * @brief 跳转到指定播放位置
 */
void Controller::seek(int seconds) {
    QStringList args = {"seek", QString::number(seconds), "absolute"};
    command(args);
}

/*!
 * @brief 跳转到相对播放位置
 */
void Controller::seekRelative(int seconds) {
    /*!
     * @brief 添加判断防止跳转越界
     */
    QVariant QTime = getProperty("time-pos");
    const double time = QTime.toDouble();

    if ((time + seconds <= duration) && (time + seconds >= 0.0)) {
        QStringList args = {"seek", QString::number(seconds), "relative"};
        command(args);
    }
}

/*!
 * @brief 切换播放暂停
 */
void Controller::togglePlayPause() {
    /*!
     * @brief 获取当前的暂停状态
     */
    QVariant pauseValue = getProperty("pause");

    /*!
     * @brief 切换播放/暂停状态
     */
    const bool isPaused = pauseValue.toBool();
    setProperty("pause", !isPaused);

    /*!
     * @brief 切换对应状态图标
     */
    application->updatePlayIcon(isPaused);
}

/*!
 * @brief 播放视频
 */
void Controller::playVideo() {
    setProperty("pause", false);

    /*!
     * @brief 切换播放图标
     */
    application->updatePlayIcon(true);
}

/*!
 * @brief 设置播放音量
 */
void Controller::setVolume(int volume, bool flag) {
    if (flag) {
        /*!
         * @brief 获取当前音量
         */
        QVariant QCurrentVolumeValue = getProperty("volume");
        const int currentVolumeValue = QCurrentVolumeValue.toInt();

        /*!
         * @brief 设置相对音量
         */
        setProperty("volume", currentVolumeValue + volume);

        application->volumeAction->updateVolumeSlider(currentVolumeValue + volume);
    } else {
        /*!
         * @brief 设置绝对音量
         */
        setProperty("volume", volume);
    }
}

/*!
 * @brief 切换静音状态
 */
void Controller::toggleMute() {
    /*!
     * @brief 获取当前的静音状态
     */
    QVariant muteValue = getProperty("mute");

    /*!
     * @brief 切换静音状态
     */
    const bool isMute = muteValue.toBool();
    setProperty("mute", !isMute);

    /*!
     * @brief 切换对应状态图标
     */
    application->updateVolumeIcon(!isMute);

    /*!
     * @brief 切换对应勾选状态
     */
    application->ui->muteAudio->setChecked(!isMute);
}

/*!
 * @brief 设置播放速度
 */
void Controller::setSpeed(double speed) {
    QVariant qCurrentSpeed = getProperty("speed");
    double const currentSpeed = qCurrentSpeed.toDouble();

    if (currentSpeed + speed <= 10 && currentSpeed + speed >= 0 && speed != 0) {
        setProperty("speed", currentSpeed + speed);
    } else if (speed == 0) {
        setProperty("speed", 1.0);
    }
}

/*!
 * @brief 设置播放速度倍数
 */
void Controller::setSpeedMultiple(double multiple) {
    QVariant qCurrentSpeed = getProperty("speed");
    double const currentSpeed = qCurrentSpeed.toDouble();

    /*!
     * @brief 暂停状态下按L键开始默认速度播放，非暂停状态下倍速播放
     */
    if (multiple == 2) {
        /*!
         * @brief 获取当前的暂停状态
         */
        QVariant pauseValue = getProperty("pause");
        const bool isPaused = pauseValue.toBool();

        if (isPaused) {
            setProperty("speed", 1.0);
            return;
        } else {
            setProperty("speed", currentSpeed * multiple);
            return;
        }
    }

    setProperty("speed", currentSpeed * multiple);
}

/*!
 * @brief 调整音频同步
 */
void Controller::adjustAudio(double sec) {
    /*!
     * @brief 获取当前音频延迟
     */
    QVariant QCurrentDelay = getProperty("audio-delay");
    double currentDelay = QCurrentDelay.toDouble();

    /*!
     * @brief 调整音频延迟
     */
    setProperty("audio-delay", currentDelay + sec);
}

/*!
 * @brief 重置音频同步设置
 */
void Controller::resetAudioSync() {
    setProperty("audio-delay", 0);
}

/*!
 * @brief 获取视频帧率（对某些文件并非完全可靠）
 */
void Controller::getFrameRate() {
    QVariant QFrameRate = getProperty("container-fps");
    frameRate = QFrameRate.toDouble();
}

/*!
 * @brief 跳转到上一帧
 */
void Controller::goToPreviousFrame() {
    /*!
     * @brief 获取当前播放位置
     */
    QVariant QPosition = getProperty("time-pos");
    double position = QPosition.toDouble();

    /*!
     * @brief 获取该视频帧率
     */
    getFrameRate();

    /*!
     * @brief 设置新的播放位置
     */
    setProperty("time-pos", position - 1.0 / frameRate);
}

/*!
 * @brief 跳转到下一帧
 */
void Controller::goToNextFrame() {
    /*!
     * @brief 获取当前播放位置
     */
    QVariant QPosition = getProperty("time-pos");
    double position = QPosition.toDouble();

    /*!
     * @brief 获取该视频帧率
     */
    getFrameRate();

    /*!
     * @brief 设置新的播放位置
     */
    setProperty("time-pos", position + 1.0 / frameRate);
}

/*!
 * @brief 发送命令到MPV
 */
void Controller::command(const QStringList &args) {
    auto result = mpv::qt::command(mpv, args);
    if (mpv::qt::is_error(result)) {
        QMessageBox::critical(reinterpret_cast<QWidget *>(application), tr("错误"),
                              "MPV命令错误：" + QString::number(mpv::qt::get_error(result)));
    }
}

/*!
 * @brief MPV属性设置函数
 */
void Controller::setProperty(const QString &name, const QVariant &value) {
    auto result = mpv::qt::set_property(mpv, name, value);
    if (mpv::qt::is_error(result)) {
        QMessageBox::critical(reinterpret_cast<QWidget *>(application), tr("错误"),
                              "MPV设置参数错误：" + QString::number(mpv::qt::get_error(result)));
    }
}

/*!
 * @brief 获取MPV属性值函数
 */
QVariant Controller::getProperty(const QString &name) const {
    auto result = mpv::qt::get_property_variant(mpv, name);
    if (mpv::qt::is_error(result)) {
        QMessageBox::critical(reinterpret_cast<QWidget *>(application), tr("错误"),
                              "MPV获取参数错误：" + QString::number(mpv::qt::get_error(result)));
        return {};
    }
    return result;
}

/*!
 * @brief 返回mpv实例
 */
mpv_handle *Controller::getMpvInstance() const {
    return this->mpv;
}