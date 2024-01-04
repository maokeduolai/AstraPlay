#include "application.h"

#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    Application w;
    w.show();
    return QApplication::exec();
}
