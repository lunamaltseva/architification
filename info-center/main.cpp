#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("InfoCenter");
    app.setApplicationDisplayName("About this System — Info Center");

    MainWindow w;
    w.resize(960, 720);
    w.show();

    return app.exec();
}
