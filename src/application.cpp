#include "application.h"
#include "../resources/ui_application.h"

Application::Application(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::Application) {
    ui->setupUi(this);
}

Application::~Application() {
    delete ui;
}
