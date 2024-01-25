#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QWidget>
#include <QVariant>
#include <QTimer>
#include <QTime>

#include "mpv/client.h"
#include "mpv/qthelper.hpp"

class Application;

class Controller : public QObject {
Q_OBJECT

public:
    explicit Controller(Application *app, QObject *parent = nullptr);

    ~Controller() override;

    [[nodiscard]] mpv_handle *getMpvInstance() const;

    void setPlayerWidget(QWidget *widget);

    void openFile(const QString &filename);

    void togglePlayPause();

    void seek(int seconds);

    void seekRelative(int seconds);

    void setVolume(int volume);

    void toggleMute();

    void setSpeed(double speed);

    void updateSliderPosition();

    void updateSliderDuration();

    void initializeSliderDuration();

    void sliderDragStarted();

    void sliderDragStopped();

    void handleUrl(const QString &url);

    void zoomIn();

    void zoomOut();

    void zoomReset();

    void moveLeft();

    void moveRight();

    void moveUp();

    void moveDown();

    void moveReset();

    void adjustAudio(double sec);

    void resetAudioSync();

    void command(const QStringList &args);

    void setProperty(const QString &name, const QVariant &value);

    [[nodiscard]] QVariant getProperty(const QString &name) const;

private:
    mpv_handle *mpv;

    Application *application;

    bool sliderBeingDragged;

    bool sliderInitialized;

    double duration;

    double zoomFactor;

    double panX;

    double panY;

    QTime totalTime;
};

#endif // CONTROLLER_H
