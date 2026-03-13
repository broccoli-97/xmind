#include "core/AppSettings.h"
#include "core/MainWindow.h"

#include <QApplication>
#include <QLibraryInfo>
#include <QStyleFactory>
#include <QTranslator>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setStyle(QStyleFactory::create("Fusion"));
    app.setOrganizationName("YMind");
    app.setApplicationName("YMind");
    app.setApplicationVersion(YMIND_VERSION);

    // Load translations based on language setting
    QString lang = AppSettings::instance().language();
    if (lang != "en") {
        // Load Qt's own translations (standard dialogs, buttons, etc.)
        auto* qtTranslator = new QTranslator(&app);
        if (qtTranslator->load("qt_" + lang,
                               QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
            app.installTranslator(qtTranslator);
        }

        // Load application translations
        auto* appTranslator = new QTranslator(&app);
        if (appTranslator->load(":/translations/ymind_" + lang + ".qm")) {
            app.installTranslator(appTranslator);
        }
    }

    MainWindow window;
    window.show();

    return app.exec();
}
