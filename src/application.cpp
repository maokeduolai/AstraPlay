#include "application.h"

// 创建主窗口
Application::Application(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::Application), slider(new QSlider(Qt::Horizontal, this)), toolBar(nullptr),
          controller(new Controller(this)), volumeAction(new VolumeAction(this)) {
    ui->setupUi(this);

    // 创建一个播放进度滑块
    slider->setMinimum(0);
    slider->setMaximum(100);

    // 获取工具栏
    toolBar = findChild<QToolBar *>("toolBar");

    // 创建时隐藏工具栏
    toolBar->hide();

    // 插入滑块
    toolBar->addWidget(slider);

    // 连接切换静音的函数
    connect(volumeAction, &VolumeAction::toggleMute, this, &Application::toggleMute);

    // 连接音量变更信号与音量设置函数，同时传递改变后的值
    connect(volumeAction, &VolumeAction::volumeChanged, this, &Application::setVolume);

    // 添加音量组件
    toolBar->addAction(volumeAction);

    // 调用controller.cpp中的函数，将MPV的视频输出绑定到playerWidget上
    controller->setPlayerWidget(ui->playerWidget);

    // 打开视频文件
    connect(ui->openFile, &QAction::triggered, this, &Application::on_actionOpenFile_triggered);

    // 退出软件
    connect(ui->exitProgram, &QAction::triggered, this, &Application::on_actionExitProgram_triggered);

    // 播放暂停视频
    connect(ui->toolPlay, &QAction::triggered, this, &Application::on_actionTogglePlayPause_triggered);

    // 跳转到指定播放位置
    connect(slider, &QSlider::sliderReleased, this, &Application::on_slider_Released);
}

Application::~Application() {
    delete ui;
}

// 工具栏显示与隐藏行为
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

// 设置播放音量
void Application::setVolume(int newVolume) {
    controller->setVolume(newVolume);
}

// 切换静音
void Application::toggleMute() {
    controller->toggleMute();
}

// 打开视频文件
void Application::on_actionOpenFile_triggered() {
    // 弹出文件选择对话框让用户选择文件
    QString filename = QFileDialog::getOpenFileName(this, tr("打开视频"), "", tr("视频文件 (*.mp4 *.avi *.mkv *.mov)"));

    // 在路径不为空的情况下打开文件
    if (!filename.isEmpty()) {
        controller->openFile(filename);  // 调用Controller的openFile方法
    }
}

// 退出应用程序
void Application::on_actionExitProgram_triggered() {
    QCoreApplication::quit();
}

// 播放暂停视频
void Application::on_actionTogglePlayPause_triggered() {
    controller->togglePlayPause();
}

// 更新播放状态对应的播放图标
void Application::updatePlayIcon(bool isPlay) {
    if (isPlay) {
        ui->toolPlay->setIcon(QIcon(":/icons/icons/pause-button.png"));
    } else {
        ui->toolPlay->setIcon(QIcon(":/icons/icons/play-button.png"));
    }
}

// 更新静音状态对应的声音图标
void Application::updateVolumeIcon(bool isMute) {
    if (isMute) {
        volumeAction->setVolumeIcon(QIcon(":/icons/icons/mute.png"));
    } else {
        volumeAction->setVolumeIcon(QIcon(":/icons/icons/volume.png"));
    }
}

// 跳转到指定播放位置
void Application::on_slider_Released() {
    int seconds = slider->value();
    controller->seek(seconds);
}

// 给Controller类提供slider用于对播放进度滑块进行初始化与更新操作
QSlider *Application::getSlider() const {
    return this->slider;
}