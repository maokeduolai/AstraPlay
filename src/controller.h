#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>

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

    void setVolume(int volume);

    void setMute(bool mute);

    void setSpeed(double speed);

    void updateSliderPosition();

    void initializeSliderDuration();

private:
    mpv_handle *mpv;
    Application *application;

    void command(const QStringList &args);

    void setProperty(const QString &name, const QVariant &value);

    [[nodiscard]] QVariant getProperty(const QString &name) const;
};

#endif // CONTROLLER_H
