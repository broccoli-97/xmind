#include "core/MainWindow.h"

#include <QApplication>
#include <QStyleFactory>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setStyle(QStyleFactory::create("Fusion"));
    app.setOrganizationName("YMind");
    app.setApplicationName("YMind");
    app.setApplicationVersion(YMIND_VERSION);

    MainWindow window;
    window.show();

    return app.exec();
}
