#include "application.h"

// 创建主窗口
Application::Application(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::Application), slider(new QSlider(Qt::Horizontal, this)), toolBar(nullptr),
          controller(new Controller(this)), volumeAction(new VolumeAction(this)),
          settings("history.ini", QSettings::IniFormat), isFullScreen(false),
          videoDownloader(new VideoDownloader(this)), mediaInfo(new MediaInfo(this)), subtitle(nullptr) {
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

    // 插入时间显示
    timeLabel = new QLabel("00:00:00/00:00:00", this);
    toolBar->addWidget(timeLabel);

    // 添加音量组件
    toolBar->addAction(volumeAction);

    // 调用controller.cpp中的函数，将MPV的视频输出绑定到playerWidget上
    controller->setPlayerWidget(ui->playerWidget);

    // 在playerWidget上安装事件过滤器
    ui->playerWidget->installEventFilter(this);


    // 传递mpv实例给subtitle
    mpv_handle *mpv = controller->getMpvInstance();
    subtitle = new Subtitle(mpv);


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

    // 清除历史记录
    connect(ui->clearHistory, &QAction::triggered, this, &Application::on_actionClearHistory_triggered);

    // 全屏播放
    connect(ui->fullScreen, &QAction::triggered, this, &Application::on_actionFullScreen_triggered);


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


    // 视频截图
    connect(ui->captureScreen, &QAction::triggered, this, &Application::on_actionCaptureScreen_triggered);


    // 视频下载
    connect(ui->videoDownload, &QAction::triggered, this, &Application::on_actionVideoDownload_triggered);

    // 下载完成对应函数
    connect(videoDownloader, &VideoDownloader::downloadFinished, this, &Application::on_DownloadFinished);

    // 下载出现错误，显示错误信息
    connect(videoDownloader, &VideoDownloader::downloadError, this, &Application::on_DownloadError);


    // 音频同步调节
    // 提前0.1s
    connect(ui->auAdvance, &QAction::triggered, this, &Application::on_actionAuAdvance_triggered);

    // 延后0.1s
    connect(ui->auDelay, &QAction::triggered, this, &Application::on_actionAuDelay_triggered);

    // 重置同步
    connect(ui->auSyncReset, &QAction::triggered, this, &Application::on_actionAuSyncReset_triggered);

    // 读取视频元数据
    connect(ui->readRaw, &QAction::triggered, this, &Application::on_actionReadRaw_triggered);

    // 字幕处理
    // 加载外挂字幕
    connect(ui->addSubtitle, &QAction::triggered, this, &Application::on_actionAddSubtitle_triggered);

    // 字幕列表
    connect(ui->subtitleList, &QAction::triggered, this, &Application::on_actionSubtitleList_triggered);

    // 字幕控制
    connect(ui->subtitleControl, &QAction::triggered, this, &Application::on_subtitleControl_clicked);
}

Application::~Application() {
    delete ui;
    delete subtitle;
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
void Application::addHistory(const QString &filepath) {
    // 检查历史记录中是否已存在该文件路径
    bool alreadyExists = std::any_of(historyActions.begin(), historyActions.end(),
                                     [&filepath](const QAction *action) {
                                         return filepath == action->data().toString();
                                     });

    // 如果文件路径已存在，则不添加新记录
    if (alreadyExists) {
        return;
    }

    // 在列表中仅显示文件名
    auto *action = new QAction(QFileInfo(filepath).fileName(), this);
    action->setData(filepath);
    connect(action, &QAction::triggered, [this, filepath]() {
        controller->openFile(filepath);  // 点击记录时打开对应文件
        filename = filepath;
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
    filename = QFileDialog::getOpenFileName(
            this,
            tr("打开媒体文件"),
            "",
            tr("视频文件 (*.mpg *.mpeg *.avi *.mp4 *.mkv *.webm *.wmv *.mov *.flv *.m4v *.ogv *.3gp *.3g2);;"
               "音频文件 (*.mp3 *.aac *.ogg *.flac *.alac *.wav *.wv);;"
               "所有文件 (*)")
    );

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
        filename = url;
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

// 视频缩放控制
void Application::on_actionZoomIn_triggered() {
    controller->zoomIn();
}

void Application::on_actionZoomOut_triggered() {
    controller->zoomOut();
}

void Application::on_actionZoomReset_triggered() {
    controller->zoomReset();
}

// 视频画面移动控制
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

// 视频截图窗口
void Application::on_actionCaptureScreen_triggered() {
    // 获取mpv实例
    mpv_handle *mpv = controller->getMpvInstance();

    // 创建截图窗口实例
    auto *dialog = new ScreenCapture(mpv, this);

    // 显示截图窗口
    dialog->show();
}

// 音频提前0.1s
void Application::on_actionAuAdvance_triggered() {
    controller->adjustAudio(-0.1);
}

// 音频延后0.1s
void Application::on_actionAuDelay_triggered() {
    controller->adjustAudio(0.1);
}

// 音频同步重置
void Application::on_actionAuSyncReset_triggered() {
    controller->resetAudioSync();
}

// 视频下载
void Application::on_actionVideoDownload_triggered() {
    QDialog dialog(this);
    QGridLayout layout(&dialog);

    // 创建布局
    QLineEdit lineEdit;
    layout.addWidget(&lineEdit, 0, 0, 1, 2);

    // 创建按钮
    QPushButton cancelButton("取消", &dialog);
    QPushButton confirmButton("下载", &dialog);
    layout.addWidget(&cancelButton, 1, 0);
    layout.addWidget(&confirmButton, 1, 1);

    connect(&cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(&confirmButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    if (dialog.exec() == QDialog::Accepted) {
        QString videoUrl = lineEdit.text();
        videoDownloader->downloadVideo(videoUrl);
    }
}

// 下载过程中的错误处理
void Application::on_DownloadError(const QString &error) {
    // 创建一个消息框来显示错误
    auto *errorMessageBox = new QMessageBox;
    errorMessageBox->setAttribute(Qt::WA_DeleteOnClose); // 对话框关闭时自动释放
    errorMessageBox->setWindowTitle(tr("下载错误"));
    errorMessageBox->setText(tr("在下载过程中发生错误:"));
    errorMessageBox->setInformativeText(error);
    errorMessageBox->setStandardButtons(QMessageBox::Ok);
    errorMessageBox->setDefaultButton(QMessageBox::Ok);
    errorMessageBox->setIcon(QMessageBox::Warning);

    // 显示消息框
    errorMessageBox->show();
}

// 下载完成处理
void Application::on_DownloadFinished(const QString &folderPath) {
    auto *finishedMessageBox = new QMessageBox;
    finishedMessageBox->setWindowTitle(tr("下载完成"));
    finishedMessageBox->setText(tr("视频已成功下载！"));
    finishedMessageBox->setInformativeText(tr("您想要做什么？"));
    finishedMessageBox->setIcon(QMessageBox::Information);

    // 创建按钮
    QAbstractButton *openFileButton = finishedMessageBox->addButton(tr("打开视频"), QMessageBox::AcceptRole);
    QAbstractButton *openFolderButton = finishedMessageBox->addButton(tr("打开文件夹"), QMessageBox::HelpRole);
    finishedMessageBox->addButton(QMessageBox::Close);

    // 显示消息框
    finishedMessageBox->exec();

    // 检查用户点击了哪个按钮
    if (finishedMessageBox->clickedButton() == openFileButton) {
        // 获取最近修改的文件的文件名
        QDir directory(folderPath);

        // 获取根据修改日期从晚到早的文件名排序
        QFileInfoList fileList = directory.entryInfoList(QDir::Files, QDir::Time);

        // 得到最近修改的文件及其完整路径
        QFileInfo mostRecentFile = fileList.first();
        QString filePath = mostRecentFile.absoluteFilePath();

        // 打开视频文件
        controller->openFile(filePath);
    } else if (finishedMessageBox->clickedButton() == openFolderButton) {
        // 打开视频文件所在的文件夹
        QDesktopServices::openUrl(QUrl::fromLocalFile(folderPath));
    }
}

// 读取视频元数据
void Application::on_actionReadRaw_triggered() {
    mediaInfo->readRawAttribute(filename);
}

// 加载外挂字幕
void Application::on_actionAddSubtitle_triggered() {
    QString subFilename = QFileDialog::getOpenFileName(this, tr("打开字幕文件"), "",
                                                       tr("字幕文件 (*.srt *.ass *.ssa *.sub)"));
    if (!subFilename.isEmpty()) {
        subtitle->loadSubtitle(subFilename);
    }
}

// 字幕列表
void Application::on_actionSubtitleList_triggered() {
    QDialog dialog(this);
    dialog.setWindowTitle(tr("字幕列表"));

    QVBoxLayout layout(&dialog);

    QListWidget listWidget;
    layout.addWidget(&listWidget);

    // 获取当前视频的字幕列表
    QMap<QString, SubtitleInfo> allSubtitle = subtitle->getSubtitleList();
    for (auto it = allSubtitle.constBegin(); it != allSubtitle.constEnd(); ++it) {
        const QString &title = it.key();
        const SubtitleInfo &info = it.value();
        QString itemText = QString("%1 [%2]").arg(title, info.lang);
        listWidget.addItem(itemText);
    }

    QPushButton closeButton(tr("应用"), &dialog);
    connect(&closeButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    layout.addWidget(&closeButton);

    dialog.setLayout(&layout);
    dialog.exec();

    if (listWidget.currentItem()) {
        // 当用户选择一个字幕时
        QString selectedSubtitleText = listWidget.currentItem()->text();

        // 提取出字幕标题
        int bracketIndex = selectedSubtitleText.indexOf(" ["); // 找到方括号的位置
        QString selectedSubtitleTitle = selectedSubtitleText.left(bracketIndex);

        // 根据字幕标题获取字幕ID
        SubtitleInfo selectedSubtitleInfo = allSubtitle.value(selectedSubtitleTitle);
        int selectedSubtitleId = selectedSubtitleInfo.id; // 获取ID
        subtitle->setSubtitleTrack(selectedSubtitleId);
    }
}

// 字幕控制
void Application::on_subtitleControl_clicked() {
    QDialog dialog(this);
    dialog.setWindowTitle(tr("字幕控制"));

    QVBoxLayout layout(&dialog);

    // 字幕字体和字号选择
    QHBoxLayout fontLayout;
    QLabel fontLabel(tr("字体："), &dialog);
    QComboBox fontComboBox(&dialog);

    // 获取系统字体
    QFontDatabase fontDatabase;
    QStringList fontList = fontDatabase.families();

    fontComboBox.addItems(fontList);
    fontLayout.addWidget(&fontLabel);
    fontLayout.addWidget(&fontComboBox);

    QLabel fontSizeLabel(tr("字号："), &dialog);
    QSpinBox fontSizeSpinBox(&dialog);
    fontSizeSpinBox.setRange(1, 72); // 假定的字号范围
    fontLayout.addWidget(&fontSizeLabel);
    fontLayout.addWidget(&fontSizeSpinBox);

    layout.addLayout(&fontLayout);

    // 获取当前使用的字幕字体属性
    QFont subtitleFont = subtitle->getCurrentSubtitleFont();

    // 获取当前字幕的字体和字号
    QString subtitleFontFamily = subtitleFont.family();
    int subtitleFontSize = subtitleFont.pointSize();

    // 设置字体下拉框的当前选中项为当前字幕的字体
    int fontIndex = fontComboBox.findText(subtitleFontFamily);
    if (fontIndex != -1) { // 如果找到了字体
        fontComboBox.setCurrentIndex(fontIndex);
    }

    // 设置字号选择框的当前值为当前字幕的字号
    fontSizeSpinBox.setValue(subtitleFontSize);

    // 应用按钮
    QPushButton applyButton(tr("应用（对ASS字幕无效"), &dialog);
    connect(&applyButton, &QPushButton::clicked, [&dialog, this, &fontComboBox, &fontSizeSpinBox]() {
        QString font = fontComboBox.currentText();
        int fontSize = fontSizeSpinBox.value();
        subtitle->setSubtitleFont(font, fontSize);
        dialog.accept();
    });
    layout.addWidget(&applyButton);

    // 字幕同步控制
    QHBoxLayout syncLayout;
    QPushButton delayMinusButton(tr("-0.1s"), &dialog);
    QPushButton delayPlusButton(tr("+0.1s"), &dialog);
    syncLayout.addWidget(&delayMinusButton);
    syncLayout.addWidget(&delayPlusButton);

    connect(&delayMinusButton, &QPushButton::clicked, [this]() {
        subtitle->setSubtitleDelay(-0.1);
    });
    connect(&delayPlusButton, &QPushButton::clicked, [this]() {
        subtitle->setSubtitleDelay(0.1);
    });

    layout.addLayout(&syncLayout);

    dialog.setLayout(&layout);
    dialog.exec();
}

// 给Controller类提供slider用于对播放进度滑块进行初始化与更新操作
QSlider *Application::getSlider() const {
    return this->slider;
}