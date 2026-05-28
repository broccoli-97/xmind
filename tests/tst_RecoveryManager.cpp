#include "core/RecoveryManager.h"
#include "core/RegistryStyleProvider.h"
#include "core/TemplateRegistry.h"
#include "core/ThemeRegistry.h"
#include "layout/LayoutAlgorithmRegistry.h"
#include "scene/MindMapScene.h"
#include "scene/NodeItem.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QTemporaryDir>
#include <QTest>

class tst_RecoveryManager : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();

    void initializeCreatesSessionDir();
    void saveSnapshotWritesAtomicFile();
    void saveSnapshotDoesNotTouchSceneModifiedState();
    void discardSnapshotRemovesFile();
    void findOrphanSessionsListsDirsWithDifferentUuid();
    void findOrphanSessionsIgnoresOwnSession();
    void loadSnapshotsReturnsParsedJsonSortedByTabIdx();
    void clearCurrentSessionRemovesDir();
    void discardSessionRemovesArbitraryDir();
};

void tst_RecoveryManager::initTestCase() {
    TemplateRegistry::instance().loadBuiltins();
    ThemeRegistry::instance().loadBuiltins();
    LayoutAlgorithmRegistry::instance().registerBuiltins();
    MindMapScene::setDefaultStyleProvider(&RegistryStyleProvider::instance());
}

void tst_RecoveryManager::initializeCreatesSessionDir() {
    QTemporaryDir root;
    QVERIFY(root.isValid());

    RecoveryManager rm(root.path());
    rm.initialize();
    QVERIFY(!rm.currentSessionId().isEmpty());

    QDir sessionDir(root.path() + "/" + rm.currentSessionId());
    QVERIFY(sessionDir.exists());
}

void tst_RecoveryManager::saveSnapshotWritesAtomicFile() {
    QTemporaryDir root;
    RecoveryManager rm(root.path());
    rm.initialize();

    MindMapScene scene;
    scene.rootNode()->setText("Recovered");
    scene.addNode("Child", scene.rootNode());

    QVERIFY(rm.saveSnapshot(0, &scene));

    QFile f(root.path() + "/" + rm.currentSessionId() + "/tab-0.ymind");
    QVERIFY(f.exists());
    QVERIFY(f.size() > 0);

    // Round-trip the snapshot back through a fresh scene.
    QVERIFY(f.open(QIODevice::ReadOnly));
    const QByteArray bytes = f.readAll();
    f.close();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(bytes, &err);
    QCOMPARE(err.error, QJsonParseError::NoError);
    QVERIFY(doc.isObject());

    MindMapScene restored;
    QVERIFY(restored.fromJson(doc.object()));
    QCOMPARE(restored.rootNode()->text(), QString("Recovered"));
    QCOMPARE(restored.rootNode()->childNodes().size(), 1);
}

void tst_RecoveryManager::saveSnapshotDoesNotTouchSceneModifiedState() {
    QTemporaryDir root;
    RecoveryManager rm(root.path());
    rm.initialize();

    MindMapScene scene;
    scene.addNode("X", scene.rootNode());
    QVERIFY(scene.isModified());

    QVERIFY(rm.saveSnapshot(0, &scene));
    // The scene should still be modified — a recovery snapshot is a parallel
    // safety net, not a real save.
    QVERIFY(scene.isModified());
}

void tst_RecoveryManager::discardSnapshotRemovesFile() {
    QTemporaryDir root;
    RecoveryManager rm(root.path());
    rm.initialize();

    MindMapScene scene;
    QVERIFY(rm.saveSnapshot(0, &scene));

    const QString path = root.path() + "/" + rm.currentSessionId() + "/tab-0.ymind";
    QVERIFY(QFile::exists(path));

    rm.discardSnapshot(0);
    QVERIFY(!QFile::exists(path));
}

void tst_RecoveryManager::findOrphanSessionsListsDirsWithDifferentUuid() {
    QTemporaryDir root;
    QVERIFY(QDir().mkpath(root.path() + "/abandoned-uuid-1234"));
    QVERIFY(QDir().mkpath(root.path() + "/abandoned-uuid-5678"));

    RecoveryManager rm(root.path());
    rm.initialize();

    const auto orphans = rm.findOrphanSessions();
    QCOMPARE(orphans.size(), 2);

    QStringList ids;
    for (const auto& o : orphans)
        ids << o.sessionId;
    std::sort(ids.begin(), ids.end());
    QCOMPARE(ids, (QStringList{"abandoned-uuid-1234", "abandoned-uuid-5678"}));
}

void tst_RecoveryManager::findOrphanSessionsIgnoresOwnSession() {
    QTemporaryDir root;
    RecoveryManager rm(root.path());
    rm.initialize();
    // The session dir from initialize() must NOT show up as an orphan.
    QVERIFY(rm.findOrphanSessions().isEmpty());
}

void tst_RecoveryManager::loadSnapshotsReturnsParsedJsonSortedByTabIdx() {
    QTemporaryDir root;
    RecoveryManager rm(root.path());
    rm.initialize();

    // Save three tabs out of order; loadSnapshots must return them by tabIdx.
    MindMapScene sceneA, sceneB, sceneC;
    sceneA.rootNode()->setText("A");
    sceneB.rootNode()->setText("B");
    sceneC.rootNode()->setText("C");
    QVERIFY(rm.saveSnapshot(2, &sceneC)); // C first at idx 2
    QVERIFY(rm.saveSnapshot(0, &sceneA));
    QVERIFY(rm.saveSnapshot(1, &sceneB));

    const QString dir = root.path() + "/" + rm.currentSessionId();
    const auto loaded = rm.loadSnapshots(dir);
    QCOMPARE(loaded.size(), 3);

    // The "root" object in each JSON carries the text we set, so use it as
    // an order check.
    QCOMPARE(loaded[0]["root"].toObject()["text"].toString(), QString("A"));
    QCOMPARE(loaded[1]["root"].toObject()["text"].toString(), QString("B"));
    QCOMPARE(loaded[2]["root"].toObject()["text"].toString(), QString("C"));
}

void tst_RecoveryManager::clearCurrentSessionRemovesDir() {
    QTemporaryDir root;
    RecoveryManager rm(root.path());
    rm.initialize();

    const QString dir = root.path() + "/" + rm.currentSessionId();
    QVERIFY(QDir(dir).exists());

    rm.clearCurrentSession();
    QVERIFY(!QDir(dir).exists());
}

void tst_RecoveryManager::discardSessionRemovesArbitraryDir() {
    QTemporaryDir root;
    const QString orphan = root.path() + "/uuid-to-blow-away";
    QVERIFY(QDir().mkpath(orphan));
    QFile f(orphan + "/tab-0.ymind");
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("{}");
    f.close();

    RecoveryManager rm(root.path());
    rm.initialize();
    rm.discardSession(orphan);

    QVERIFY(!QDir(orphan).exists());
}

QTEST_MAIN(tst_RecoveryManager)
#include "tst_RecoveryManager.moc"
