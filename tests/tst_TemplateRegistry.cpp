#include "core/TemplateRegistry.h"

#include <QTest>

class tst_TemplateRegistry : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();

    void builtinsAreLoaded();
    void templateByIdFindsBuiltins();
    void templateByIdReturnsNullForUnknown();
    void allTemplatesReturnsThreeBuiltins();
    void registerCustomTemplate();
    void registerEmptyIdIsIgnored();
    void builtinIdForLayoutStyleMapping();
    void builtinIdForLayoutStyleUnknown();
};

void tst_TemplateRegistry::initTestCase() {
    TemplateRegistry::instance().loadBuiltins();
}

void tst_TemplateRegistry::builtinsAreLoaded() {
    QVERIFY(TemplateRegistry::instance().templateById("builtin.mindmap") != nullptr);
    QVERIFY(TemplateRegistry::instance().templateById("builtin.orgchart") != nullptr);
    QVERIFY(TemplateRegistry::instance().templateById("builtin.projectplan") != nullptr);
}

void tst_TemplateRegistry::templateByIdFindsBuiltins() {
    const auto* td = TemplateRegistry::instance().templateById("builtin.mindmap");
    QVERIFY(td != nullptr);
    QCOMPARE(td->id, QString("builtin.mindmap"));
    QCOMPARE(td->name, QString("Mind Map"));
    QCOMPARE(td->layout.algorithm, QString("bilateral"));
}

void tst_TemplateRegistry::templateByIdReturnsNullForUnknown() {
    QVERIFY(TemplateRegistry::instance().templateById("nonexistent") == nullptr);
}

void tst_TemplateRegistry::allTemplatesReturnsThreeBuiltins() {
    auto all = TemplateRegistry::instance().allTemplates();
    // At least 3 builtins (may include custom from other tests)
    QVERIFY(all.size() >= 3);
}

void tst_TemplateRegistry::registerCustomTemplate() {
    TemplateDescriptor td;
    td.id = "test.custom";
    td.name = "Custom";

    TemplateRegistry::instance().registerTemplate(td);

    const auto* found = TemplateRegistry::instance().templateById("test.custom");
    QVERIFY(found != nullptr);
    QCOMPARE(found->name, QString("Custom"));
}

void tst_TemplateRegistry::registerEmptyIdIsIgnored() {
    int before = TemplateRegistry::instance().allTemplates().size();

    TemplateDescriptor td;
    td.id = "";
    TemplateRegistry::instance().registerTemplate(td);

    int after = TemplateRegistry::instance().allTemplates().size();
    QCOMPARE(after, before);
}

void tst_TemplateRegistry::builtinIdForLayoutStyleMapping() {
    QCOMPARE(TemplateRegistry::builtinIdForLayoutStyle(0), QString("builtin.mindmap"));
    QCOMPARE(TemplateRegistry::builtinIdForLayoutStyle(1), QString("builtin.orgchart"));
    QCOMPARE(TemplateRegistry::builtinIdForLayoutStyle(2), QString("builtin.projectplan"));
}

void tst_TemplateRegistry::builtinIdForLayoutStyleUnknown() {
    QCOMPARE(TemplateRegistry::builtinIdForLayoutStyle(99), QString("builtin.mindmap"));
    QCOMPARE(TemplateRegistry::builtinIdForLayoutStyle(-1), QString("builtin.mindmap"));
}

QTEST_APPLESS_MAIN(tst_TemplateRegistry)
#include "tst_TemplateRegistry.moc"
