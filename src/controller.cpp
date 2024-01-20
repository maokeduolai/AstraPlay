#include "controller.h"
#include "application.h"

Controller::Controller(Application *app, QObject *parent)
        : QObject(parent), mpv(mpv_create()), application(app), sliderBeingDragged(false), sliderInitialized(false),
          duration(0.0) {
    // 根据滑块是否被按下，来判断是否处于拖动滑块状态
    QSlider *slider = application->getSlider();
    if (slider) {
        connect(slider, &QSlider::sliderPressed, this, &Controller::sliderDragStarted);
        connect(slider, &QSlider::sliderReleased, this, &Controller::sliderDragStopped);
    }

    // 初始化MPV实例
    if (mpv) {
        // 设置视频输出驱动为OpenGL
        mpv_set_option_string(mpv, "vo", "opengl");

        // 设置音频输出驱动为WASAPI
        mpv_set_option_string(mpv, "ao", "wasapi");

        // 启用硬件解码
        mpv_set_option_string(mpv, "hwdec", "auto");

        // 设置视频同步模式
        mpv_set_option_string(mpv, "video-sync", "display-resample");

        // 设置视频循环播放
        mpv_set_option_string(mpv, "loop-file", "inf");

        // 设置初始音量为80
        int initialVolume = 80;
        mpv_set_option(mpv, "volume", MPV_FORMAT_INT64, &initialVolume);

        // 初始化MPV完成后启动它
        if (mpv_initialize(mpv) < 0) {
            // 错误处理
        }
    }

    // 设置定时器
    auto *timer = new QTimer(this);

    // 每秒更新一次播放进度
    connect(timer, &QTimer::timeout, this, &Controller::updateSliderPosition);

    // 检测视频时长变化，应对在线视频在开始播放时，可能并未完全加载，其总时长未知的情形
    connect(timer, &QTimer::timeout, this, &Controller::updateSliderDuration);
    timer->start(1000);
}

Controller::~Controller() {
    if (mpv) {
        // 清理MPV资源
        mpv_terminate_destroy(mpv);
    }
}

// 将MPV的视频输出绑定到QWidget上
void Controller::setPlayerWidget(QWidget *widget) {
    mpv_set_option_string(mpv, "wid", QString::number(widget->winId()).toUtf8().constData());
}

// 打开文件
void Controller::openFile(const QString &filename) {
    QStringList args = {"loadfile", filename};
    command(args);

    // 初始化滑块
    initializeSliderDuration();

    // 更新进度条初始化状态
    sliderInitialized = true;

    // 切换播放图标到正在播放状态
    application->updatePlayIcon(true);
}

// 打开URL
void Controller::handleUrl(const QString &url) {
    QStringList args = {"loadfile", url};
    command(args);

    // 初始化滑块
    initializeSliderDuration();

    // 更新进度条初始化状态
    sliderInitialized = true;

    // 切换播放图标到正在播放状态
    application->updatePlayIcon(true);
}

// 初始化滑块的总时长
void Controller::initializeSliderDuration() {
    duration = Controller::getProperty("duration").toDouble();
    if (mpv) {
        if (duration > 0) {
            // 设置滑块的最大值为视频总时长（秒）
            QSlider *slider = application->getSlider();
            slider->setMaximum(static_cast<int>(duration));
        }
    }
}

// 应对在线视频在开始播放时，可能并未完全加载，其总时长未知需要缓冲后更新的情况
void Controller::updateSliderDuration() {
    double newDuration = Controller::getProperty("duration").toDouble();
    if (newDuration != duration) {
        // 设置滑块的最大值为视频总时长（秒）
        QSlider *slider = application->getSlider();
        slider->setMaximum(static_cast<int>(newDuration));
    }
}

// 播放时更新滑块位置
void Controller::updateSliderPosition() {
    // 当MPV实例已完成初始化，播放进度滑块不处于拖动状态，滑块已初始化的情况下才更新滑块位置
    if (mpv && !sliderBeingDragged && sliderInitialized) {
        // 获取当前播放时间
        int64_t time;
        mpv_get_property(mpv, "time-pos", MPV_FORMAT_INT64, &time);

        // 设置滑块的位置
        QSlider *slider = application->getSlider();
        if (slider) { slider->setValue(static_cast<int>(time)); }
    }
}

// 开始拖动播放进度滑块，更新状态
void Controller::sliderDragStarted() {
    sliderBeingDragged = true;
}

// 结束拖动播放进度滑块，更新状态
void Controller::sliderDragStopped() {
    sliderBeingDragged = false;
}

// 跳转到指定播放位置
void Controller::seek(int seconds) {
    QStringList args = {"seek", QString::number(seconds), "absolute"};
    command(args);
}

// 跳转到相对播放位置
void Controller::seekRelative(int seconds) {
    // 添加判断防止跳转越界
    QVariant QTime = getProperty("time-pos");
    const double time = QTime.toDouble();

    if ((time + seconds <= duration) && (time + seconds >= 0.0)) {
        QStringList args = {"seek", QString::number(seconds), "relative"};
        command(args);
    }
}

// 切换播放暂停
void Controller::togglePlayPause() {
    // 获取当前的暂停状态
    QVariant pauseValue = getProperty("pause");

    // 切换播放/暂停状态
    const bool isPaused = pauseValue.toBool();
    setProperty("pause", !isPaused);

    // 切换对应状态图标
    application->updatePlayIcon(isPaused);
}

// 设置播放音量
void Controller::setVolume(int volume) {
    setProperty("volume", volume);
}

// 切换静音状态
void Controller::toggleMute() {
    // 获取当前的静音状态
    QVariant muteValue = getProperty("mute");

    // 切换静音状态
    const bool isMute = muteValue.toBool();
    setProperty("mute", !isMute);

    // 切换对应状态图标
    application->updateVolumeIcon(!isMute);
}

// 设置播放速度
void Controller::setSpeed(double speed) {
    QVariant qCurrentSpeed = getProperty("speed");
    double const currentSpeed = qCurrentSpeed.toDouble();

    if (currentSpeed + speed <= 10 && currentSpeed + speed >= 0 && speed != 0) {
        setProperty("speed", currentSpeed + speed);
    } else if (speed == 0) {
        setProperty("speed", 1.0);
    }
}

// 发送命令到MPV
void Controller::command(const QStringList &args) {
    mpv::qt::command(mpv, args);  // 需添加异常处理
}

// MPV属性设置函数
void Controller::setProperty(const QString &name, const QVariant &value) {
    mpv::qt::set_property(mpv, name, value);
}

// 获取MPV属性值函数
QVariant Controller::getProperty(const QString &name) const {
    return mpv::qt::get_property_variant(mpv, name);
}