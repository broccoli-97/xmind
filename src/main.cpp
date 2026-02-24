#include "MainWindow.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setOrganizationName("XMind");
    app.setApplicationName("XMind");
    app.setApplicationVersion("1.0.0");

    MainWindow window;
    window.show();

    return app.exec();
}
