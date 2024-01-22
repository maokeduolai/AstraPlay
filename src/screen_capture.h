#ifndef SCREEN_CAPTURE_H
#define SCREEN_CAPTURE_H

#include <QDialog>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QPushButton>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDateTime>
#include <QInputDialog>

#include "controller.h"

class ScreenCapture : public QDialog {
Q_OBJECT

public:
    explicit ScreenCapture(mpv_handle *mpv, QWidget *parent = nullptr);

private slots:

    void on_captureCurrentFrameButton_clicked();

    void on_capturePreviewButton_clicked();

private:
    int captureCount;

    QSpinBox *captureCountSpinBox;

    mpv_handle *mpv;

    [[nodiscard]] QVariant getProperty(const QString &name) const;

};


#endif //SCREEN_CAPTURE_H
