#include <QApplication>
#include "restartpopup.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("RestartPopup");
    app.setQuitOnLastWindowClosed(false);  // keep running as background daemon

    RestartPopup popup;
    popup.start();

    return app.exec();
}
