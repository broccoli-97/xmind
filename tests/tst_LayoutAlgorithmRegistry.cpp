#include "layout/LayoutAlgorithmRegistry.h"

#include <QTest>

class tst_LayoutAlgorithmRegistry : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();

    void builtinsRegistered();
    void algorithmNamesContainsAll();
    void lookupByName();
    void unknownReturnsNull();
};

void tst_LayoutAlgorithmRegistry::initTestCase() {
    LayoutAlgorithmRegistry::instance().registerBuiltins();
}

void tst_LayoutAlgorithmRegistry::builtinsRegistered() {
    QVERIFY(LayoutAlgorithmRegistry::instance().algorithm("bilateral") != nullptr);
    QVERIFY(LayoutAlgorithmRegistry::instance().algorithm("topdown") != nullptr);
    QVERIFY(LayoutAlgorithmRegistry::instance().algorithm("righttree") != nullptr);
}

void tst_LayoutAlgorithmRegistry::algorithmNamesContainsAll() {
    QStringList names = LayoutAlgorithmRegistry::instance().algorithmNames();
    QVERIFY(names.contains("bilateral"));
    QVERIFY(names.contains("topdown"));
    QVERIFY(names.contains("righttree"));
    QCOMPARE(names.size(), 3);
}

void tst_LayoutAlgorithmRegistry::lookupByName() {
    const auto* algo = LayoutAlgorithmRegistry::instance().algorithm("bilateral");
    QVERIFY(algo != nullptr);
    QCOMPARE(algo->name(), QString("bilateral"));
}

void tst_LayoutAlgorithmRegistry::unknownReturnsNull() {
    QVERIFY(LayoutAlgorithmRegistry::instance().algorithm("nonexistent") == nullptr);
    QVERIFY(LayoutAlgorithmRegistry::instance().algorithm("") == nullptr);
}

QTEST_APPLESS_MAIN(tst_LayoutAlgorithmRegistry)
#include "tst_LayoutAlgorithmRegistry.moc"
