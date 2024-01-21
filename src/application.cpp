#include "application.h"

// 创建主窗口
Application::Application(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::Application), slider(new QSlider(Qt::Horizontal, this)), toolBar(nullptr),
          controller(new Controller(this)), volumeAction(new VolumeAction(this)),
          settings("history.ini", QSettings::IniFormat), isFullScreen(false) {
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

    // 添加音量组件
    toolBar->addAction(volumeAction);

    // 调用controller.cpp中的函数，将MPV的视频输出绑定到playerWidget上
    controller->setPlayerWidget(ui->playerWidget);

    // 在playerWidget上安装事件过滤器
    ui->playerWidget->installEventFilter(this);

    // 加载播放历史记录
    loadHistory();

    // 连接切换静音的函数
    connect(volumeAction, &VolumeAction::toggleMute, this, &Application::toggleMute);

    // 连接音量变更信号与音量设置函数，同时传递改变后的值
    connect(volumeAction, &VolumeAction::volumeChanged, this, &Application::setVolume);

    // 打开视频文件
    connect(ui->openFile, &QAction::triggered, this, &Application::on_actionOpenFile_triggered);

    // 打开URL
    connect(ui->openURL, &QAction::triggered, this, &Application::on_actionOpenURL_triggered);

    // 退出软件
    connect(ui->exitProgram, &QAction::triggered, this, &Application::on_actionExitProgram_triggered);

    // 播放暂停视频
    connect(ui->toolPlay, &QAction::triggered, this, &Application::on_actionTogglePlayPause_triggered);

    // 跳转到指定播放位置
    connect(slider, &QSlider::sliderReleased, this, &Application::on_slider_Released);

    // 增加播放速度（0.5x）
    connect(ui->speedUp, &QAction::triggered, this, &Application::on_actionSpeedUp_triggered);

    // 减少播放速度（0.5x）
    connect(ui->speedDown, &QAction::triggered, this, &Application::on_actionSpeedDown_triggered);

    // 重置播放速度
    connect(ui->speedReset, &QAction::triggered, this, &Application::on_actionSpeedReset_triggered);

    // 后退3秒
    connect(ui->toolBack, &QAction::triggered, this, &Application::on_actionToolBack_triggered);

    // 前进3秒
    connect(ui->toolForward, &QAction::triggered, this, &Application::on_actionToolForward_triggered);

    // 清除历史记录
    connect(ui->clearHistory, &QAction::triggered, this, &Application::on_actionClearHistory_triggered);

    // 全屏播放
    connect(ui->fullScreen, &QAction::triggered, this, &Application::on_actionFullScreen_triggered);

    // 放大视频10%
    connect(ui->zoomIn, &QAction::triggered, this, &Application::on_actionZoomIn_triggered);

    // 缩小视频10%
    connect(ui->zoomOut, &QAction::triggered, this, &Application::on_actionZoomOut_triggered);

    // 重置视频缩放
    connect(ui->zoomReset, &QAction::triggered, this, &Application::on_actionZoomReset_triggered);

    // 向左移动视频画面
    connect(ui->moveLeft, &QAction::triggered, this, &Application::on_actionMoveLeft_triggered);

    // 向右移动视频画面
    connect(ui->moveRight, &QAction::triggered, this, &Application::on_actionMoveRight_triggered);

    // 向上移动视频画面
    connect(ui->moveUp, &QAction::triggered, this, &Application::on_actionMoveUp_triggered);

    // 向下移动视频画面
    connect(ui->moveDown, &QAction::triggered, this, &Application::on_actionMoveDown_triggered);

    // 重置视频画面移动
    connect(ui->moveReset, &QAction::triggered, this, &Application::on_actionMoveReset_triggered);
}

Application::~Application() {
    delete ui;
}

// 重写eventFilter()方法
bool Application::eventFilter(QObject *watched, QEvent *event) {
    // 双击播放窗口切换全屏播放
    if (watched == ui->playerWidget && event->type() == QEvent::MouseButtonDblClick) {
        Application::on_actionFullScreen_triggered();
        return true;
    }

    // 单击播放窗口切换播放、暂停
    if (watched == ui->playerWidget && event->type() == QEvent::MouseButtonPress) {
        Application::on_actionTogglePlayPause_triggered();
        return true;
    }
    return QMainWindow::eventFilter(watched, event);
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

// 加载历史记录
void Application::loadHistory() {
    QStringList files = settings.value("history").toStringList();

    // 向列表中添加历史记录
    for (const QString &file: files) {
        addHistory(file);
    }
}

// 保存历史记录
void Application::saveHistory() {
    QStringList files;
    for (const QAction *action: historyActions) {
        files.append(action->data().toString());
    }

    settings.setValue("history", files);
}

// 添加历史记录
void Application::addHistory(const QString &filename) {
    // 在列表中仅显示文件名
    auto *action = new QAction(QFileInfo(filename).fileName(), this);
    action->setData(filename);
    connect(action, &QAction::triggered, [this, filename]() {
        controller->openFile(filename);  // 点击记录时打开对应文件
    });

    // 向界面中添加记录
    historyActions.prepend(action);
    ui->menuHistory->insertAction(ui->menuHistory->actions().isEmpty() ? nullptr : ui->menuHistory->actions().first(),
                                  action);

    // 当记录条数超过5条时，清理最早的一条记录
    while (historyActions.size() > maxHistorySize) {
        QAction *lastAction = historyActions.takeLast();
        ui->menuHistory->removeAction(lastAction);
        delete lastAction;
    }

    saveHistory();
}

// 清除历史记录
void Application::on_actionClearHistory_triggered() {
    for (QAction *action: historyActions) {
        ui->menuHistory->removeAction(action);
        delete action;
    }

    historyActions.clear();
    saveHistory();
}

// 打开视频文件
void Application::on_actionOpenFile_triggered() {
    // 弹出文件选择对话框让用户选择文件
    QString filename = QFileDialog::getOpenFileName(this, tr("打开视频"), "", tr("视频文件 (*.mp4 *.avi *.mkv *.mov)"));

    // 在路径不为空的情况下打开文件
    if (!filename.isEmpty()) {
        controller->openFile(filename);  // 调用Controller的openFile方法
        Application::addHistory(filename);
    }
}

// 打开URL
void Application::on_actionOpenURL_triggered() {
    QDialog dialog(this);
    QGridLayout layout(&dialog);

    // 创建布局
    QLineEdit lineEdit;
    layout.addWidget(&lineEdit, 0, 0, 1, 2);

    // 创建按钮
    QPushButton cancelButton("取消", &dialog);
    QPushButton confirmButton("确认", &dialog);
    layout.addWidget(&cancelButton, 1, 0);
    layout.addWidget(&confirmButton, 1, 1);

    connect(&cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(&confirmButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    if (dialog.exec() == QDialog::Accepted) {
        QString url = lineEdit.text();
        controller->handleUrl(url);
        Application::addHistory(url);
    }
}

// 退出应用程序
void Application::on_actionExitProgram_triggered() {
    // 保存播放历史记录
    Application::saveHistory();

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

// 全屏播放
void Application::on_actionFullScreen_triggered() {
    if (isFullScreen) {
        // 退出全屏
        // 恢复原来的窗口标志
        ui->playerWidget->setWindowFlags(originalFlags);

        // 恢复原来的窗口状态
        ui->playerWidget->showNormal();
        isFullScreen = false;
    } else {
        // 进入全屏
        // 保存当前的窗口标志
        originalFlags = ui->playerWidget->windowFlags();

        // 将playerWidget提升为顶级窗口
        ui->playerWidget->setWindowFlags(Qt::Window);
        ui->playerWidget->showFullScreen();
        isFullScreen = true;
    }
}

// 放大视频10%
void Application::on_actionZoomIn_triggered() {
    controller->zoomIn();
}

// 缩小视频10%
void Application::on_actionZoomOut_triggered() {
    controller->zoomOut();
}

// 视频缩放控制
void Application::on_actionZoomReset_triggered() {
    controller->zoomReset();
}

void Application::on_actionMoveLeft_triggered() {
    controller->moveLeft();
}

void Application::on_actionMoveRight_triggered() {
    controller->moveRight();
}

void Application::on_actionMoveUp_triggered() {
    controller->moveUp();
}

void Application::on_actionMoveDown_triggered() {
    controller->moveDown();
}

void Application::on_actionMoveReset_triggered() {
    controller->moveReset();
}

// 跳转到指定播放位置
void Application::on_slider_Released() {
    int seconds = slider->value();
    controller->seek(seconds);
}

// 增加播放速度（0.5x）
void Application::on_actionSpeedUp_triggered() {
    controller->setSpeed(0.5);
}

// 减少播放速度（0.5x）
void Application::on_actionSpeedDown_triggered() {
    controller->setSpeed(-0.5);
}

// 重置播放速度
void Application::on_actionSpeedReset_triggered() {
    controller->setSpeed(0);
}

// 后退3秒
void Application::on_actionToolBack_triggered() {
    controller->seekRelative(-3);
}

// 前进3秒
void Application::on_actionToolForward_triggered() {
    controller->seekRelative(3);
}

// 给Controller类提供slider用于对播放进度滑块进行初始化与更新操作
QSlider *Application::getSlider() const {
    return this->slider;
}