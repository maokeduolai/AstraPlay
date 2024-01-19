#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QWidget>
#include <QVariant>
#include <QTimer>

#include <mpv/client.h>
#include <mpv/qthelper.hpp>

class Application;

class Controller : public QObject {
Q_OBJECT

public:
    explicit Controller(Application *app, QObject *parent = nullptr);

    ~Controller() override;

    // 设置播放窗口
    void setPlayerWidget(QWidget *widget);

    void openFile(const QString &filename);

    void togglePlayPause();

    void seek(int seconds);

    void seekRelative(int seconds);

    void setVolume(int volume);

    void toggleMute();

    void setSpeed(double speed);

    void updateSliderPosition();

    void initializeSliderDuration();

    void sliderDragStarted();

    void sliderDragStopped();

private:
    mpv_handle *mpv;

    Application *application;

    void command(const QStringList &args);

    void setProperty(const QString &name, const QVariant &value);

    [[nodiscard]] QVariant getProperty(const QString &name) const;

    bool sliderBeingDragged;

    bool sliderInitialized;
};

#endif // CONTROLLER_H
