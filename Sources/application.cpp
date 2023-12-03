#include "Headers/application.h"
#include "Forms/ui_application.h"

Application::Application(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::Application) {
    ui->setupUi(this);
}

Application::~Application() {
    delete ui;
}
