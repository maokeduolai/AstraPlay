#ifndef APPLICATION_H
#define APPLICATION_H

#include <QMainWindow>
#include <QSlider>
#include <QToolBar>
#include <QWidgetAction>
#include <QToolButton>
#include <QHBoxLayout>
#include <QApplication>

#include "controller.h"

QT_BEGIN_NAMESPACE
namespace Ui {
    class Application;
}
QT_END_NAMESPACE

// 声明主窗口相关行为函数
class Application : public QMainWindow {
Q_OBJECT

public:
    explicit Application(QWidget *parent = nullptr);

    ~Application() override;

protected:
    void enterEvent(QEvent *event) override;

    void leaveEvent(QEvent *event) override;

private slots:

    void setPlaybackPosition(int position);

    static void setVolume(int volume);

    void on_actionOpenFile_triggered();

    static void on_actionExitProgram_triggered();

    void on_actionTogglePlayPause_triggered();

private:
    Ui::Application *ui;

    QSlider *slider;

    QToolBar *toolBar;

    Controller *controller;
};

// 音量调整组件
class VolumeControlWidget : public QWidget {
Q_OBJECT
public:
    explicit VolumeControlWidget(QWidget *parent = nullptr) : QWidget(parent) {
        auto *layout = new QHBoxLayout(this);

        // 创建按钮
        button = new QToolButton(this);
        button->setIcon(QIcon(":/icons/icons/volume.png")); // 设置音量按钮图标
        layout->addWidget(button, 0, Qt::AlignCenter);

        // 创建滑块
        slider = new QSlider(Qt::Horizontal, this);
        slider->setRange(0, 100); // 音量范围是0到100
        slider->setValue(80); // 初始音量是80
        layout->addWidget(slider, 0, Qt::AlignCenter);

        setLayout(layout);

        // 连接滑块的信号
        connect(slider, &QSlider::valueChanged, this, &VolumeControlWidget::volumeChanged);
    }

    QToolButton *button;
    QSlider *slider;

signals:

    // 音量滑块的值改变时发射信号到VolumeAction
    void volumeChanged(int volume);
};

// 音量调整滑块
class VolumeAction : public QWidgetAction {
Q_OBJECT
public:
    explicit VolumeAction(QObject *parent = nullptr) : QWidgetAction(parent) {}

protected:
    QWidget *createWidget(QWidget *parent) override {
        auto *volumeControlWidget = new VolumeControlWidget(parent);
        connect(volumeControlWidget, &VolumeControlWidget::volumeChanged, this, &VolumeAction::volumeChanged);
        return volumeControlWidget;
    }

signals:

    // 继续传递值改变的信号到application.cpp设置音量
    void volumeChanged(int volume);
};

#endif // APPLICATION_H
