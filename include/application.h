#ifndef APPLICATION_H
#define APPLICATION_H

#include <QMainWindow>
#include <QSlider>
#include <QToolBar>
#include <QWidgetAction>
#include <QToolButton>
#include <QHBoxLayout>
#include <QApplication>
#include <QPushButton>
#include <QDebug>
#include <QEvent>
#include <QFileDialog>
#include <QCoreApplication>
#include <QLineEdit>
#include <QSettings>
#include <QDesktopServices>
#include <QLabel>

#include "../resources/ui_application.h"
#include "controller.h"
#include "screen_capture.h"
#include "video_downloader.h"
#include "output_window.h"

QT_BEGIN_NAMESPACE
namespace Ui {
    class Application;
}
QT_END_NAMESPACE

// 音量控制组件
class VolumeControlWidget : public QWidget {
Q_OBJECT
public:
    explicit VolumeControlWidget(QWidget *parent = nullptr) : QWidget(parent) {
        auto *layout = new QHBoxLayout(this);

        // 创建按钮
        button = new QToolButton(this);
        button->setIcon(QIcon(":/icons/icons/volume.png")); // 设置音量按钮图标
        layout->addWidget(button, 0, Qt::AlignCenter);

        // 连接按钮的信号
        connect(button, &QPushButton::clicked, this, &VolumeControlWidget::toggleMute);

        // 创建滑块
        slider = new QSlider(Qt::Horizontal, this);
        slider->setRange(0, 100); // 音量范围是0到100
        slider->setValue(80); // 初始音量是80，设置对应滑块位置
        layout->addWidget(slider, 0, Qt::AlignCenter);

        // 创建按钮与滑块的布局
        setLayout(layout);

        // 连接滑块的信号
        connect(slider, &QSlider::valueChanged, this, &VolumeControlWidget::volumeChanged);
    }

    QToolButton *button;
    QSlider *slider;

signals:

    // 传递切换静音的信号到VolumeAction类
    void toggleMute();

    // 传递音量改变的信号与改变后的值到VolumeAction类
    void volumeChanged(int newVolume);
};

// 音量控制控件
class VolumeAction : public QWidgetAction {
Q_OBJECT
public:
    explicit VolumeAction(QObject *parent = nullptr) : QWidgetAction(parent), volumeWidget(nullptr) {}

    ~VolumeAction() override {
        delete volumeWidget;  // 销毁由VolumeAction管理的volumeWidget实例，防止内存泄漏
    }

protected:
    QWidget *createWidget(QWidget *parent) override {
        volumeWidget = new VolumeControlWidget(parent);
        connect(volumeWidget, &VolumeControlWidget::toggleMute, this, &VolumeAction::toggleMute);
        connect(volumeWidget, &VolumeControlWidget::volumeChanged, this, &VolumeAction::volumeChanged);
        return volumeWidget;
    }

public slots:

    // 设置音量图标
    void setVolumeIcon(const QIcon &icon) {
        if (volumeWidget && volumeWidget->button) {
            volumeWidget->button->setIcon(icon);
        }
    }

signals:

    // 传递切换静音的信号到controller.cpp
    void toggleMute();

    // 继续传递值改变的信号到application.cpp
    void volumeChanged(int newVolume);

private:
    VolumeControlWidget *volumeWidget;
};

// 声明主窗口相关行为函数
class Application : public QMainWindow {
Q_OBJECT

public:
    explicit Application(QWidget *parent = nullptr);

    ~Application() override;

    [[nodiscard]] QSlider *getSlider() const;

    void updatePlayIcon(bool isPlay);

    void updateVolumeIcon(bool isMute);

    static void on_DownloadError(const QString &error);

    QLabel *timeLabel;

protected:
    void enterEvent(QEvent *event) override;

    void leaveEvent(QEvent *event) override;

private slots:

    void toggleMute();

    void setVolume(int newVolume);

    void loadHistory();

    void saveHistory();

    void addHistory(const QString &filename);

    void on_actionOpenFile_triggered();

    void on_actionExitProgram_triggered();

    void on_actionTogglePlayPause_triggered();

    void on_slider_Released();

    void on_actionSpeedUp_triggered();

    void on_actionSpeedDown_triggered();

    void on_actionSpeedReset_triggered();

    void on_actionToolBack_triggered();

    void on_actionToolForward_triggered();

    void on_actionOpenURL_triggered();

    void on_actionClearHistory_triggered();

    void on_actionFullScreen_triggered();

    void on_actionZoomIn_triggered();

    void on_actionZoomOut_triggered();

    void on_actionZoomReset_triggered();

    void on_actionMoveLeft_triggered();

    void on_actionMoveRight_triggered();

    void on_actionMoveUp_triggered();

    void on_actionMoveDown_triggered();

    void on_actionMoveReset_triggered();

    void on_actionCaptureScreen_triggered();

    void on_actionVideoDownload_triggered();

    void on_DownloadFinished(const QString &filePath);

    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Ui::Application *ui;

    QSlider *slider;

    QToolBar *toolBar;

    Controller *controller;

    VolumeAction *volumeAction;

    VideoDownloader *videoDownloader;

    QList<QAction *> historyActions;

    QSettings settings;

    const int maxHistorySize = 5;  // 最大可记录历史数为5条

    bool isFullScreen;

    Qt::WindowFlags originalFlags;
};

#endif // APPLICATION_H
