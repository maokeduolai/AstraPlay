#include <QWidget>
#include <QVariant>
#include <QByteArray>
#include <QDebug>

#include "controller.h"

Controller::Controller(QObject *parent)
        : QObject(parent), mpv(nullptr), playerWidget(nullptr) {
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
}

Controller::~Controller() {
    if (mpv) {
        // 清理mpv资源
        mpv_terminate_destroy(mpv);
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
}

// 切换播放暂停
void Controller::togglePlayPause() {
    // 获取当前的暂停状态
    QVariant pauseValue = getProperty("pause");

    // 切换播放/暂停状态
    const bool isPaused = pauseValue.toBool();
    setProperty("pause", !isPaused);
}

void Controller::seek(int seconds) {
    QStringList args = {"seek", QString::number(seconds), "relative"};
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

void Controller::command(const QStringList &args) {
    QVector<const char *> argv;
    for (const QString &arg: args)
        argv << arg.toUtf8().constData();
    argv << nullptr;

    mpv_command(mpv, argv.data());
}

// mpv属性设置函数
void Controller::setProperty(const QString &name, const QVariant &value) {
    mpv::qt::set_property(mpv, name, value);
}

// 获取mpv属性函数
QVariant Controller::getProperty(const QString &name) const {
    return mpv::qt::get_property_variant(mpv, name);
}
