#include <QApplication>
#include "assistant.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("ArchAssistant");
    app.setApplicationDisplayName("Arch AI Assistant™");

    Assistant w;
    w.resize(700, 560);
    w.show();

    return app.exec();
}
