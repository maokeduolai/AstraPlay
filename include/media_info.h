#ifndef MEDIA_INFO_H
#define MEDIA_INFO_H

#include <QProcess>
#include <QPlainTextEdit>
#include <QMessageBox>

// 创建新的QPlainTextEdit子类，并重写closeEvent方法，实现关闭窗口清空内容
class MyTextEdit : public QPlainTextEdit {
Q_OBJECT
public:
    explicit MyTextEdit(QWidget *parent = nullptr) : QPlainTextEdit(parent) {}

protected:
    void closeEvent(QCloseEvent *event) override {
        this->clear();
        QPlainTextEdit::closeEvent(event);
    }
};

class MediaInfo : public QObject {
Q_OBJECT

public:

    explicit MediaInfo(QObject *parent = nullptr);

    void readRawAttribute(const QString &filename);

    void onReadyReadStandardOutput();

private:

    QProcess *mediaInfoProcess;

    MyTextEdit *textEdit;
};

#endif //MEDIA_INFO_H