#include <QApplication>

#include "application.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // 加载QSS文件
    QFile qss("../resources/Aqua.qss");
    if (qss.open(QFile::ReadOnly)) {
        QString style = QLatin1String(qss.readAll());
        a.setStyleSheet(style);
        qss.close();
    }

    // 设置全局字体
    QFont font("Source Han Sans CN", 10);
    QApplication::setFont(font);

    Application w;
    w.show();
    return QApplication::exec();
}
