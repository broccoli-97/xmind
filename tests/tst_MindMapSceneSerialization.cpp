#include "core/TemplateRegistry.h"
#include "layout/LayoutAlgorithmRegistry.h"
#include "scene/MindMapScene.h"
#include "scene/NodeItem.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QTest>

class tst_MindMapSceneSerialization : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();

    void toJsonBasicStructure();
    void jsonRoundTrip();
    void fromJsonInvalidFormat();
    void fromJsonMissingRoot();
    void fromJsonV1LayoutStyleMigration();
    void exportToText();
    void exportToMarkdown();
    void importFromText();
    void importFromTextEmpty();
};

void tst_MindMapSceneSerialization::initTestCase() {
    TemplateRegistry::instance().loadBuiltins();
    LayoutAlgorithmRegistry::instance().registerBuiltins();
}

void tst_MindMapSceneSerialization::toJsonBasicStructure() {
    MindMapScene scene;
    scene.rootNode()->setText("Root");
    scene.addNode("Child 1", scene.rootNode());
    scene.addNode("Child 2", scene.rootNode());

    QJsonObject json = scene.toJson();

    QCOMPARE(json["format"].toString(), QString("xmind"));
    QCOMPARE(json["version"].toInt(), 2);
    QVERIFY(json.contains("root"));
    QCOMPARE(json["root"].toObject()["text"].toString(), QString("Root"));

    QJsonArray children = json["root"].toObject()["children"].toArray();
    QCOMPARE(children.size(), 2);
    QCOMPARE(children[0].toObject()["text"].toString(), QString("Child 1"));
    QCOMPARE(children[1].toObject()["text"].toString(), QString("Child 2"));
}

void tst_MindMapSceneSerialization::jsonRoundTrip() {
    // Build a scene
    MindMapScene scene1;
    scene1.rootNode()->setText("Central");
    scene1.setTemplateId("builtin.mindmap");
    auto* a = scene1.addNode("A", scene1.rootNode());
    scene1.addNode("A1", a);
    scene1.addNode("B", scene1.rootNode());

    // Serialize
    QJsonObject json = scene1.toJson();

    // Deserialize into a new scene
    MindMapScene scene2;
    QVERIFY(scene2.fromJson(json));

    // Compare structure
    QCOMPARE(scene2.rootNode()->text(), QString("Central"));
    QCOMPARE(scene2.templateId(), QString("builtin.mindmap"));

    auto rootChildren = scene2.rootNode()->childNodes();
    QCOMPARE(rootChildren.size(), 2);
    QCOMPARE(rootChildren[0]->text(), QString("A"));
    QCOMPARE(rootChildren[1]->text(), QString("B"));

    auto aChildren = rootChildren[0]->childNodes();
    QCOMPARE(aChildren.size(), 1);
    QCOMPARE(aChildren[0]->text(), QString("A1"));
}

void tst_MindMapSceneSerialization::fromJsonInvalidFormat() {
    MindMapScene scene;
    QJsonObject json;
    json["format"] = "not-xmind";
    QVERIFY(!scene.fromJson(json));
}

void tst_MindMapSceneSerialization::fromJsonMissingRoot() {
    MindMapScene scene;
    QJsonObject json;
    json["format"] = "xmind";
    json["version"] = 2;
    // No "root" key — should create a fallback root
    QVERIFY(scene.fromJson(json));
    QVERIFY(scene.rootNode() != nullptr);
    QCOMPARE(scene.rootNode()->text(), QString("Topic"));
}

void tst_MindMapSceneSerialization::fromJsonV1LayoutStyleMigration() {
    // v1 files have layoutStyle but no templateId
    QJsonObject json;
    json["format"] = "xmind";
    json["version"] = 1;
    json["layoutStyle"] = 1; // TopDown

    QJsonObject root;
    root["text"] = "Test";
    json["root"] = root;

    MindMapScene scene;
    QVERIFY(scene.fromJson(json));
    // Should have auto-mapped layoutStyle 1 to builtin.orgchart
    QCOMPARE(scene.templateId(), QString("builtin.orgchart"));
    QCOMPARE(scene.layoutStyle(), LayoutStyle::TopDown);
}

void tst_MindMapSceneSerialization::exportToText() {
    MindMapScene scene;
    scene.rootNode()->setText("Root");
    auto* a = scene.addNode("A", scene.rootNode());
    scene.addNode("A1", a);
    scene.addNode("B", scene.rootNode());

    QString text = scene.exportToText();

    QVERIFY(text.contains("Root"));
    QVERIFY(text.contains("\tA"));
    QVERIFY(text.contains("\t\tA1"));
    QVERIFY(text.contains("\tB"));
}

void tst_MindMapSceneSerialization::exportToMarkdown() {
    MindMapScene scene;
    scene.rootNode()->setText("Root");
    scene.addNode("Child", scene.rootNode());

    QString md = scene.exportToMarkdown();

    QVERIFY(md.contains("# Root"));
    QVERIFY(md.contains("## Child"));
}

void tst_MindMapSceneSerialization::importFromText() {
    MindMapScene scene;
    QString input = "Root\n\tA\n\t\tA1\n\tB\n";
    QVERIFY(scene.importFromText(input));

    QVERIFY(scene.rootNode() != nullptr);
    QCOMPARE(scene.rootNode()->text(), QString("Root"));

    auto children = scene.rootNode()->childNodes();
    QCOMPARE(children.size(), 2);
    QCOMPARE(children[0]->text(), QString("A"));
    QCOMPARE(children[1]->text(), QString("B"));

    auto aChildren = children[0]->childNodes();
    QCOMPARE(aChildren.size(), 1);
    QCOMPARE(aChildren[0]->text(), QString("A1"));
}

void tst_MindMapSceneSerialization::importFromTextEmpty() {
    MindMapScene scene;
    QVERIFY(!scene.importFromText(""));
}

QTEST_MAIN(tst_MindMapSceneSerialization)
#include "tst_MindMapSceneSerialization.moc"
