#include "core/AppSettings.h"
#include "core/Application.h"
#include "core/MainWindow.h"
#include "core/Services.h"
#include "scene/MindMapScene.h"
#include "scene/NodeItem.h"
#include "ui/TabManager.h"

#include <QFileOpenEvent>
#include <QIcon>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>
#include <QUrl>
#include <QUuid>

class tst_ApplicationFileOpen : public QObject {
    Q_OBJECT

private slots:
    void startupEventsWaitForWindow();
    void localUrlIsDelivered();
    void nonLocalUrlIsIgnored();
    void opensFilesAndReusesExistingTabs();
};

void tst_ApplicationFileOpen::startupEventsWaitForWindow() {
    auto* app = static_cast<Application*>(qApp);
    // Preserve requests even if startup recovery dialogs run an event loop
    // before MainWindow has finished construction and connected its handler.
    QFileOpenEvent first(QStringLiteral("/tmp/first 中文.ymind"));
    QFileOpenEvent second(QStringLiteral("/tmp/second.ymind"));
    QVERIFY(QCoreApplication::sendEvent(app, &first));
    QVERIFY(QCoreApplication::sendEvent(app, &second));
    QCoreApplication::processEvents();

    QSignalSpy spy(app, &Application::fileOpenRequested);
    app->startFileOpenHandling();
    QTRY_COMPARE(spy.count(), 2);
    QCOMPARE(spy.at(0).at(0).toString(), first.file());
    QCOMPARE(spy.at(1).at(0).toString(), second.file());
    QCoreApplication::processEvents();
    QCOMPARE(spy.count(), 2);
}

void tst_ApplicationFileOpen::localUrlIsDelivered() {
    auto* app = static_cast<Application*>(qApp);
    QSignalSpy spy(app, &Application::fileOpenRequested);
    const QString path = QStringLiteral("/tmp/中文 folder/map.ymind");
    QFileOpenEvent event(QUrl::fromLocalFile(path));
    QVERIFY(QCoreApplication::sendEvent(app, &event));
    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), path);
}

void tst_ApplicationFileOpen::nonLocalUrlIsIgnored() {
    QSignalSpy spy(static_cast<Application*>(qApp), &Application::fileOpenRequested);
    QFileOpenEvent event(QUrl(QStringLiteral("https://example.com/map.ymind")));
    QCoreApplication::sendEvent(qApp, &event);
    QCoreApplication::processEvents();
    QCOMPARE(spy.count(), 0);
}

void tst_ApplicationFileOpen::opensFilesAndReusesExistingTabs() {
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    AppSettings::instance().setCheckForUpdatesEnabled(false);
    AppSettings::instance().setAutoSaveEnabled(false);
    Services services = Services::productionDefaults();
    MainWindow window(services);
    auto* tabs = window.findChild<TabManager*>();
    QVERIFY(tabs);

    MindMapScene source;
    source.rootNode()->setText(QStringLiteral("中文 document"));
    source.addNode(QStringLiteral("Saved child"), source.rootNode());
    const QString first = directory.filePath(QStringLiteral("first 中文.ymind"));
    const QString second = directory.filePath(QStringLiteral("second.ymind"));
    QVERIFY(source.saveToFile(first));
    source.rootNode()->setText(QStringLiteral("Second document"));
    QVERIFY(source.saveToFile(second));

    window.openFile(first);
    QCOMPARE(tabs->tabCount(), 1);
    QCOMPARE(tabs->currentScene()->rootNode()->text(), QStringLiteral("中文 document"));
    QCOMPARE(tabs->currentScene()->rootNode()->childNodes().size(), 1);
    window.openFile(second);
    QCOMPARE(tabs->tabCount(), 2);
    QCOMPARE(tabs->currentScene()->rootNode()->text(), QStringLiteral("Second document"));

    window.openFile(directory.filePath(QStringLiteral("./first 中文.ymind")));
    QCOMPARE(tabs->tabCount(), 2);
    QCOMPARE(tabs->currentScene()->rootNode()->text(), QStringLiteral("中文 document"));

    auto* app = static_cast<Application*>(qApp);
    const auto connection =
        connect(app, &Application::fileOpenRequested, &window, &MainWindow::openFile);
    QFileOpenEvent event(QUrl::fromLocalFile(second));
    QVERIFY(QCoreApplication::sendEvent(app, &event));
    QTRY_COMPARE(tabs->currentScene()->rootNode()->text(), QStringLiteral("Second document"));
    QCOMPARE(tabs->tabCount(), 2);
    disconnect(connection);

    QVERIFY(!QIcon(QStringLiteral(":/icons/ymind.png")).pixmap(128, 128).isNull());
    window.close();
}

int main(int argc, char** argv) {
    Application app(argc, argv);
    QStandardPaths::setTestModeEnabled(true);
    app.setOrganizationName(QStringLiteral("YMindTests"));
    app.setApplicationName(QStringLiteral("FileOpen-%1").arg(QUuid::createUuid().toString()));
    tst_ApplicationFileOpen test;
    return QTest::qExec(&test, argc, argv);
}

#include "tst_ApplicationFileOpen.moc"
