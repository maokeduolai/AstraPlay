#include <QWidget>
#include <QVariant>
#include <QDebug>
#include <QTimer>

#include "controller.h"
#include "application.h"

Controller::Controller(Application *app, QObject *parent)
        : QObject(parent), mpv(nullptr), application(app) {
    // 初始化mpv实例
    mpv = mpv_create();
    if (mpv) {
        // 设置视频输出驱动为OpenGL
        mpv_set_option_string(mpv, "vo", "opengl");

        // 设置音频输出驱动为WASAPI
        mpv_set_option_string(mpv, "ao", "wasapi");

        // 启用硬件解码
        mpv_set_option_string(mpv, "hwdec", "auto");

        // 设置视频同步模式
        mpv_set_option_string(mpv, "video-sync", "display-resample");

        // 初始化mpv完成后启动它
        if (mpv_initialize(mpv) < 0) {
            // 错误处理
        }
    }

    // 设置定时器
    auto *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Controller::updateSliderPosition);
    timer->start(1000);  // 每秒更新一次
}

Controller::~Controller() {
    if (mpv) {
        // 清理mpv资源
        mpv_terminate_destroy(mpv);
    }
}

// 初始化滑块的总时长
void Controller::initializeSliderDuration() {
    if (mpv) {
        double duration;
        if (mpv_get_property(mpv, "duration", MPV_FORMAT_DOUBLE, &duration) == 0 && duration >= 0) {
            // 设置滑块的最大值为视频总时长（秒）
            QSlider *slider = application->getSlider();
            slider->setMaximum(static_cast<int>(duration));
        }
    }
}

// 播放时更新滑块位置
void Controller::updateSliderPosition() {
    if (mpv) {
        // 获取当前播放时间
        int64_t time;
        mpv_get_property(mpv, "time-pos", MPV_FORMAT_INT64, &time);

        // 设置滑块的位置
        QSlider *slider = application->getSlider();
        if (slider) { slider->setValue(static_cast<int>(time)); }
    }
}

// 将mpv的视频输出绑定到QWidget上
void Controller::setPlayerWidget(QWidget *widget) {
    mpv_set_option_string(mpv, "wid", QString::number(widget->winId()).toUtf8().constData());
}

// 打开文件
void Controller::openFile(const QString &filename) {
    QStringList args = {"loadfile", filename};
    command(args);

    // 初始化滑块
    initializeSliderDuration();
}

// 切换播放暂停
void Controller::togglePlayPause() {
    // 获取当前的暂停状态
    QVariant pauseValue = getProperty("pause");

    // 切换播放/暂停状态
    const bool isPaused = pauseValue.toBool();
    setProperty("pause", !isPaused);
}

// 跳转到指定播放位置
void Controller::seek(int seconds) {
    QStringList args = {"seek", QString::number(seconds), "absolute"};
    command(args);
}

void Controller::setVolume(int volume) {
    setProperty("volume", volume);
}

void Controller::setMute(bool mute) {
    setProperty("mute", mute);
}

void Controller::setSpeed(double speed) {
    setProperty("speed", speed);
}

// 发送命令到mpv
void Controller::command(const QStringList &args) {
    mpv::qt::command(mpv, args);  // 后期需添加异常处理
}

// mpv属性设置函数
void Controller::setProperty(const QString &name, const QVariant &value) {
    mpv::qt::set_property(mpv, name, value);
}

// 获取mpv属性函数
QVariant Controller::getProperty(const QString &name) const {
    return mpv::qt::get_property_variant(mpv, name);
}
