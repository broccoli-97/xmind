#include "core/RegistryStyleProvider.h"
#include "core/TemplateRegistry.h"
#include "core/ThemeRegistry.h"
#include "layout/LayoutAlgorithmRegistry.h"
#include "scene/EdgeItem.h"
#include "scene/MindMapScene.h"
#include "scene/NodeItem.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QKeyEvent>
#include <QTest>

class tst_NodeCollapse : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();

    void collapseHidesDescendantsAndEdges();
    void expandReshowsImmediateDescendants();
    void nestedCollapseStaysCollapsedAfterParentExpand();
    void jsonRoundTripPreservesCollapsedFlag();
    void v3FileLoadsAsAllExpanded();
    void spaceKeyTogglesSelectedNodeCollapse();
    void arrowDoesNotEnterCollapsedSubtree();
};

void tst_NodeCollapse::initTestCase() {
    TemplateRegistry::instance().loadBuiltins();
    ThemeRegistry::instance().loadBuiltins();
    LayoutAlgorithmRegistry::instance().registerBuiltins();
    MindMapScene::setDefaultStyleProvider(&RegistryStyleProvider::instance());
}

void tst_NodeCollapse::collapseHidesDescendantsAndEdges() {
    MindMapScene scene;
    auto* a = scene.addNode("A", scene.rootNode());
    auto* a1 = scene.addNode("A1", a);
    auto* a2 = scene.addNode("A2", a);

    QVERIFY(a1->isVisible());
    QVERIFY(a2->isVisible());

    a->setCollapsed(true);

    QVERIFY(!a1->isVisible());
    QVERIFY(!a2->isVisible());

    // The edges A->A1 and A->A2 should also hide.
    auto* edge1 = scene.findEdge(a, a1);
    auto* edge2 = scene.findEdge(a, a2);
    QVERIFY(edge1 && !edge1->isVisible());
    QVERIFY(edge2 && !edge2->isVisible());
}

void tst_NodeCollapse::expandReshowsImmediateDescendants() {
    MindMapScene scene;
    auto* a = scene.addNode("A", scene.rootNode());
    auto* a1 = scene.addNode("A1", a);

    a->setCollapsed(true);
    QVERIFY(!a1->isVisible());

    a->setCollapsed(false);
    QVERIFY(a1->isVisible());
}

void tst_NodeCollapse::nestedCollapseStaysCollapsedAfterParentExpand() {
    // A -> B (collapsed) -> C. Collapse A as well. Expanding A should reveal
    // B but NOT C, because B is still independently collapsed.
    MindMapScene scene;
    auto* a = scene.addNode("A", scene.rootNode());
    auto* b = scene.addNode("B", a);
    auto* c = scene.addNode("C", b);

    b->setCollapsed(true);
    QVERIFY(!c->isVisible());

    a->setCollapsed(true);
    QVERIFY(!b->isVisible());
    QVERIFY(!c->isVisible());

    a->setCollapsed(false);
    QVERIFY(b->isVisible());
    QVERIFY(!c->isVisible()); // still hidden under B
}

void tst_NodeCollapse::jsonRoundTripPreservesCollapsedFlag() {
    MindMapScene scene1;
    auto* a = scene1.addNode("A", scene1.rootNode());
    scene1.addNode("A1", a);
    a->setCollapsed(true);

    const QJsonObject json = scene1.toJson();

    MindMapScene scene2;
    QVERIFY(scene2.fromJson(json));

    auto rootChildren = scene2.rootNode()->childNodes();
    QCOMPARE(rootChildren.size(), 1);
    QVERIFY(rootChildren[0]->isCollapsed());

    // Descendants should be hidden after the round trip.
    auto leafChildren = rootChildren[0]->childNodes();
    QCOMPARE(leafChildren.size(), 1);
    QVERIFY(!leafChildren[0]->isVisible());
}

void tst_NodeCollapse::v3FileLoadsAsAllExpanded() {
    // A handcrafted v3 file has no `collapsed` field; the migrator runs but
    // the nodes should end up expanded.
    QJsonObject json;
    json["format"] = "ymind";
    json["version"] = 3;

    QJsonObject child;
    child["text"] = "Child";
    child["children"] = QJsonArray{};

    QJsonObject root;
    root["text"] = "Root";
    root["children"] = QJsonArray{child};
    json["root"] = root;

    MindMapScene scene;
    QVERIFY(scene.fromJson(json));
    QCOMPARE(scene.rootNode()->childNodes().size(), 1);
    QVERIFY(!scene.rootNode()->isCollapsed());
    QVERIFY(scene.rootNode()->childNodes()[0]->isVisible());
}

void tst_NodeCollapse::spaceKeyTogglesSelectedNodeCollapse() {
    MindMapScene scene;
    auto* a = scene.addNode("A", scene.rootNode());
    scene.addNode("A1", a);
    scene.clearSelection();
    a->setSelected(true);

    QVERIFY(!a->isCollapsed());

    QKeyEvent down(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
    QCoreApplication::sendEvent(&scene, &down);
    QVERIFY(a->isCollapsed());

    QCoreApplication::sendEvent(&scene, &down);
    QVERIFY(!a->isCollapsed());
}

void tst_NodeCollapse::arrowDoesNotEnterCollapsedSubtree() {
    MindMapScene scene;
    scene.setLayoutStyle(LayoutStyle::RightTree);
    auto* a = scene.addNode("A", scene.rootNode());
    scene.addNode("A1", a);

    a->setCollapsed(true);
    scene.clearSelection();
    a->setSelected(true);

    QKeyEvent right(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    QCoreApplication::sendEvent(&scene, &right);

    // Selection should not have moved into the hidden child.
    QCOMPARE(scene.selectedNode(), a);
}

QTEST_MAIN(tst_NodeCollapse)
#include "tst_NodeCollapse.moc"
