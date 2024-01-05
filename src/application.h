#ifndef APPLICATION_H
#define APPLICATION_H

#include <QMainWindow>
#include <QSlider>
#include <QToolBar>

QT_BEGIN_NAMESPACE
namespace Ui {
    class Application;
}
QT_END_NAMESPACE

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

private:
    Ui::Application *ui;

    QSlider *slider;

    QToolBar *toolBar;
};

#endif // APPLICATION_H
