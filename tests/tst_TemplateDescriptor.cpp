#include "core/TemplateDescriptor.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QTest>

// TemplateDescriptor now covers only the *structural* concern (layout +
// starter content). Visual-style fields moved to ThemeDescriptor and have
// their own test suite.
class tst_TemplateDescriptor : public QObject {
    Q_OBJECT

private slots:
    void layoutConfigRoundTrip();
    void layoutConfigDefaults();
    void layoutConfigInheritsFromBase();
    void contentNodeRoundTrip();
    void contentNodeEmpty();
    void contentNodeNested();
    void descriptorRoundTrip();
};

void tst_TemplateDescriptor::layoutConfigRoundTrip() {
    TemplateLayoutConfig c;
    c.algorithm = "topdown";
    c.depthSpacing = 80.0;
    c.spreadSpacing = 20.0;

    QJsonObject json = c.toJson();
    TemplateLayoutConfig c2 = TemplateLayoutConfig::fromJson(json);

    QCOMPARE(c2.algorithm, QString("topdown"));
    QCOMPARE(c2.depthSpacing, 80.0);
    QCOMPARE(c2.spreadSpacing, 20.0);
}

void tst_TemplateDescriptor::layoutConfigDefaults() {
    TemplateLayoutConfig c = TemplateLayoutConfig::fromJson(QJsonObject());
    QCOMPARE(c.algorithm, QString("bilateral"));
    QCOMPARE(c.depthSpacing, 100.0);
    QCOMPARE(c.spreadSpacing, 16.0);
}

void tst_TemplateDescriptor::layoutConfigInheritsFromBase() {
    TemplateLayoutConfig base;
    base.algorithm = "righttree";
    base.depthSpacing = 140.0;
    base.spreadSpacing = 24.0;

    TemplateLayoutConfig c = TemplateLayoutConfig::fromJson(QJsonObject(), base);
    QCOMPARE(c.algorithm, QString("righttree"));
    QCOMPARE(c.depthSpacing, 140.0);
    QCOMPARE(c.spreadSpacing, 24.0);

    QJsonObject partial;
    partial["depthSpacing"] = 200.0;
    TemplateLayoutConfig c2 = TemplateLayoutConfig::fromJson(partial, base);
    QCOMPARE(c2.algorithm, QString("righttree"));
    QCOMPARE(c2.depthSpacing, 200.0);
    QCOMPARE(c2.spreadSpacing, 24.0);
}

void tst_TemplateDescriptor::contentNodeRoundTrip() {
    TemplateContentNode n;
    n.text = "Root";
    n.children = {{"Child 1", {}}, {"Child 2", {}}};

    QJsonObject json = n.toJson();
    TemplateContentNode n2 = TemplateContentNode::fromJson(json);

    QCOMPARE(n2.text, QString("Root"));
    QCOMPARE(n2.children.size(), 2);
    QCOMPARE(n2.children[0].text, QString("Child 1"));
    QCOMPARE(n2.children[1].text, QString("Child 2"));
}

void tst_TemplateDescriptor::contentNodeEmpty() {
    QJsonObject empty;
    TemplateContentNode n = TemplateContentNode::fromJson(empty);
    QCOMPARE(n.text, QString("Topic"));
    QVERIFY(n.children.isEmpty());
}

void tst_TemplateDescriptor::contentNodeNested() {
    TemplateContentNode root;
    root.text = "A";
    TemplateContentNode child;
    child.text = "B";
    TemplateContentNode grandchild{ "C", {} };
    child.children = { grandchild };
    root.children = { child };

    QJsonObject json = root.toJson();
    TemplateContentNode r2 = TemplateContentNode::fromJson(json);

    QCOMPARE(r2.text, QString("A"));
    QCOMPARE(r2.children.size(), 1);
    QCOMPARE(r2.children[0].text, QString("B"));
    QCOMPARE(r2.children[0].children.size(), 1);
    QCOMPARE(r2.children[0].children[0].text, QString("C"));
}

void tst_TemplateDescriptor::descriptorRoundTrip() {
    TemplateDescriptor td;
    td.id = "test.example";
    td.name = "Example";
    td.description = "Test template";
    td.layout.algorithm = "righttree";
    td.layout.depthSpacing = 90.0;
    td.content.text = "Root";
    td.content.children = {{"Child A", {}}, {"Child B", {}}};

    QJsonObject json = td.toJson();
    TemplateDescriptor td2 = TemplateDescriptor::fromJson(json);

    QCOMPARE(td2.id, td.id);
    QCOMPARE(td2.name, td.name);
    QCOMPARE(td2.description, td.description);
    QCOMPARE(td2.layout.algorithm, QString("righttree"));
    QCOMPARE(td2.layout.depthSpacing, 90.0);
    QCOMPARE(td2.content.text, QString("Root"));
    QCOMPARE(td2.content.children.size(), 2);
}

QTEST_APPLESS_MAIN(tst_TemplateDescriptor)
#include "tst_TemplateDescriptor.moc"
