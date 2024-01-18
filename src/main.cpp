#include <QApplication>

#include "application.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // 设置全局字体
    QFont font("Source Han Sans CN", 10);
    QApplication::setFont(font);

    Application w;
    w.show();
    return QApplication::exec();
}
