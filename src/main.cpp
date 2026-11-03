#include "core/AppSettings.h"
#include "core/Application.h"
#include "core/MainWindow.h"
#include "core/Services.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QFontDatabase>
#include <QIcon>
#include <QLibraryInfo>
#include <QStyleFactory>
#include <QTranslator>

int main(int argc, char* argv[]) {
    Application app(argc, argv);
    app.setStyle(QStyleFactory::create("Fusion"));
    app.setOrganizationName("YMind");
    app.setApplicationName("YMind");
    app.setApplicationVersion(YMIND_VERSION);
    app.setWindowIcon(QIcon(":/icons/ymind.png"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("YMind mind map editor"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("files"), QStringLiteral("Mind maps to open"),
                                 QStringLiteral("[files...]"));
    parser.process(app);

    QFontDatabase::addApplicationFont(":/fonts/Caveat.ttf");

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

    Services services = Services::productionDefaults();
    MainWindow window(services);
    QObject::connect(&app, &Application::fileOpenRequested, &window, &MainWindow::openFile);
    app.startFileOpenHandling();
    window.show();
    for (const QString& path : parser.positionalArguments())
        window.openFile(path);

    return app.exec();
}
