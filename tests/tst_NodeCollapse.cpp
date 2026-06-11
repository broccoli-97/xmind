#include "core/RegistryStyleProvider.h"
#include "core/TemplateRegistry.h"
#include "core/ThemeRegistry.h"
#include "layout/LayoutAlgorithmRegistry.h"
#include "scene/EdgeItem.h"
#include "scene/MindMapScene.h"
#include "scene/NodeItem.h"

#include <QGraphicsSceneMouseEvent>
#include <QJsonArray>
#include <QJsonObject>
#include <QKeyEvent>
#include <QSignalSpy>
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

    // Collapse-aware layout + the scene-level toggle gesture.
    void collapsedSubtreeTakesLeafFootprint();
    void hiddenChildrenStackOntoCollapsedParent();
    void expandRestoresSpreadLayout();
    void toggleNodeCollapsedRelayoutsAndDirtiesScene();
    void toggleNodeCollapsedIgnoresLeaves();
    void addChildToCollapsedNodeAutoExpands();

    // The macOS-style count badge.
    void collapseBadgeGeometry();
    void badgeClickExpandsWithoutSelecting();
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

namespace {

// Root with two branches in a RightTree layout (spread axis = y): "A" carries
// a fat subtree, "B" is a leaf sibling. The vertical gap between A and B is
// the footprint A's subtree claims.
struct FootprintFixture {
    NodeItem* a = nullptr;
    NodeItem* b = nullptr;

    explicit FootprintFixture(MindMapScene& scene) {
        scene.setLayoutStyle(LayoutStyle::RightTree);
        a = scene.addNode("A", scene.rootNode());
        b = scene.addNode("B", scene.rootNode());
        for (int i = 0; i < 4; ++i)
            scene.addNode(QStringLiteral("A child %1").arg(i), a);
    }

    qreal gap() const { return qAbs(a->pos().y() - b->pos().y()); }
};

} // namespace

void tst_NodeCollapse::collapsedSubtreeTakesLeafFootprint() {
    MindMapScene scene;
    FootprintFixture f(scene);

    scene.layoutWithoutAnimation();
    const qreal expandedGap = f.gap();

    f.a->setCollapsed(true);
    scene.layoutWithoutAnimation();

    // The folded branch must stop reserving its children's spread: siblings
    // pack around A as if it were a leaf, instead of leaving a hole.
    QVERIFY2(f.gap() < expandedGap,
             qPrintable(QStringLiteral("collapsed gap %1 not tighter than expanded gap %2")
                            .arg(f.gap())
                            .arg(expandedGap)));
}

void tst_NodeCollapse::hiddenChildrenStackOntoCollapsedParent() {
    MindMapScene scene;
    FootprintFixture f(scene);

    f.a->setCollapsed(true);
    scene.layoutWithoutAnimation();

    // Hidden descendants ride on their folded ancestor so invisible strays
    // can't inflate itemsBoundingRect (zoom-to-fit, exports), and expanding
    // later unfolds them outward from the parent.
    for (auto* child : f.a->childNodes())
        QCOMPARE(child->pos(), f.a->pos());
}

void tst_NodeCollapse::expandRestoresSpreadLayout() {
    MindMapScene scene;
    FootprintFixture f(scene);

    f.a->setCollapsed(true);
    scene.layoutWithoutAnimation();
    f.a->setCollapsed(false);
    scene.layoutWithoutAnimation();

    // Children spread back out: all visible, distinct spread positions, none
    // left sitting on the parent.
    QSet<qreal> spreads;
    for (auto* child : f.a->childNodes()) {
        QVERIFY(child->isVisible());
        QVERIFY(child->pos() != f.a->pos());
        spreads.insert(child->pos().y());
    }
    QCOMPARE(spreads.size(), f.a->childNodes().size());
}

void tst_NodeCollapse::toggleNodeCollapsedRelayoutsAndDirtiesScene() {
    MindMapScene scene;
    FootprintFixture f(scene);
    scene.layoutWithoutAnimation();
    const qreal expandedGap = f.gap();
    scene.setModified(false);

    QSignalSpy spy(&scene, &MindMapScene::nodeCollapseChanged);
    scene.toggleNodeCollapsed(f.a);

    QVERIFY(f.a->isCollapsed());
    QVERIFY(scene.isModified());
    QCOMPARE(spy.count(), 1);

    // The toggle re-layouts (animated, 400ms): wait for the map to tighten.
    QTRY_VERIFY_WITH_TIMEOUT(f.gap() < expandedGap - 1.0, 2000);
}

void tst_NodeCollapse::toggleNodeCollapsedIgnoresLeaves() {
    MindMapScene scene;
    auto* leaf = scene.addNode("Leaf", scene.rootNode());
    scene.setModified(false);

    QSignalSpy spy(&scene, &MindMapScene::nodeCollapseChanged);
    scene.toggleNodeCollapsed(leaf);

    QVERIFY(!leaf->isCollapsed());
    QVERIFY(!scene.isModified());
    QCOMPARE(spy.count(), 0);
}

void tst_NodeCollapse::addChildToCollapsedNodeAutoExpands() {
    MindMapScene scene;
    auto* a = scene.addNode("A", scene.rootNode());
    scene.addNode("A1", a);
    a->setCollapsed(true);

    scene.clearSelection();
    a->setSelected(true);
    scene.addChildToSelected();

    // The fold must open first — otherwise the new child would be born
    // visible among hidden siblings.
    QVERIFY(!a->isCollapsed());
    QCOMPARE(a->childNodes().size(), 2);
    for (auto* child : a->childNodes())
        QVERIFY(child->isVisible());

    scene.cancelEditing(); // tear down the inline edit addChild started
}

void tst_NodeCollapse::collapseBadgeGeometry() {
    MindMapScene scene;
    scene.setLayoutStyle(LayoutStyle::RightTree);
    auto* a = scene.addNode("A", scene.rootNode());
    scene.addNode("A1", a);
    scene.addNode("A2", a);
    scene.layoutWithoutAnimation();

    // Expanded: no badge, and the node's hit shape stays the body.
    QVERIFY(!a->collapseBadgeVisible());

    a->setCollapsed(true);
    QVERIFY(a->collapseBadgeVisible());
    QCOMPARE(a->descendantCount(), 2);

    const QRectF badge = a->collapseControlRect();
    // RightTree children grow rightward, so the badge sits just past the
    // right edge — the junction where the folded branch would continue.
    QVERIFY(badge.left() >= a->nodeRect().right());
    QVERIFY(a->boundingRect().contains(badge));
    // Clickable: the badge is part of the node's hit-test shape (and wasn't
    // before folding — the same point must not hit the node when expanded).
    QVERIFY(a->shape().contains(badge.center()));
    a->setCollapsed(false);
    QVERIFY(!a->shape().contains(badge.center()));
}

void tst_NodeCollapse::badgeClickExpandsWithoutSelecting() {
    MindMapScene scene;
    scene.setLayoutStyle(LayoutStyle::RightTree);
    auto* a = scene.addNode("A", scene.rootNode());
    scene.addNode("A1", a);
    scene.layoutWithoutAnimation();
    a->setCollapsed(true);
    scene.clearSelection();

    const QPointF scenePos = a->mapToScene(a->collapseControlRect().center());

    QGraphicsSceneMouseEvent press(QEvent::GraphicsSceneMousePress);
    press.setScenePos(scenePos);
    press.setButton(Qt::LeftButton);
    press.setButtons(Qt::LeftButton);
    QCoreApplication::sendEvent(&scene, &press);

    QGraphicsSceneMouseEvent release(QEvent::GraphicsSceneMouseRelease);
    release.setScenePos(scenePos);
    release.setButton(Qt::LeftButton);
    release.setButtons(Qt::NoButton);
    QCoreApplication::sendEvent(&scene, &release);

    // The click-on-release toggles the fold open...
    QVERIFY(!a->isCollapsed());
    // ...and, Finder-disclosure-like, does not change the selection.
    QCOMPARE(scene.selectedNode(), static_cast<NodeItem*>(nullptr));
}

QTEST_MAIN(tst_NodeCollapse)
#include "tst_NodeCollapse.moc"
