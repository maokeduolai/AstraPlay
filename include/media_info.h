#ifndef MEDIA_INFO_H
#define MEDIA_INFO_H

#include <QProcess>
#include <QPlainTextEdit>
#include <QMessageBox>

class MediaInfo : public QObject {
Q_OBJECT

public:

    explicit MediaInfo(QObject *parent = nullptr);

    void readRawAttribute(const QString &filename);

    void onReadyReadStandardOutput();

private:

    QProcess *mediaInfoProcess;

    QPlainTextEdit *textEdit;

};


#endif //MEDIA_INFO_H
