#include <QSlider>
#include <QEvent>
#include "application.h"
#include "../resources/ui_application.h"
#include "controller.h"

Application::Application(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::Application), slider(new QSlider(Qt::Horizontal, this)), toolBar(nullptr) {
    ui->setupUi(this);

    // 创建一个滑块
    slider->setMinimum(0);
    slider->setMaximum(100);

    // 获取工具栏
    toolBar = findChild<QToolBar *>("toolBar");

    // 创建时隐藏工具栏
    toolBar->hide();

    // 插入滑块
    toolBar->addWidget(slider);

    // 连接滑块的信号到槽
    connect(slider, &QSlider::valueChanged, this, &Application::setPlaybackPosition);

    // 创建并添加音量动作
    auto *volumeAction = new VolumeAction(this);
    connect(volumeAction, &VolumeAction::volumeChanged, this, &Application::setVolume);
    toolBar->addAction(volumeAction);
}

Application::~Application() {
    delete ui;
}

void Application::setPlaybackPosition(int position) {
    if (!slider->isSliderDown()) { // 防止在自动更新进度时触发
    }
}

void Application::setVolume(int volume) {
    // 接收来自controller.h的新设定的音量值
    // 与libmpv连接，设置视频音量
}

void Application::enterEvent(QEvent *event) {
    // 当鼠标进入窗口时显示工具栏
    toolBar->show();
    QMainWindow::enterEvent(event); // 调用基类的事件处理，以便正常处理其他事件
}

void Application::leaveEvent(QEvent *event) {
    // 当鼠标离开窗口时隐藏工具栏
    toolBar->hide();
    QMainWindow::leaveEvent(event);
}
