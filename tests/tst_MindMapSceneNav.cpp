#include "core/RegistryStyleProvider.h"
#include "core/TemplateRegistry.h"
#include "core/ThemeRegistry.h"
#include "layout/LayoutAlgorithmRegistry.h"
#include "scene/MindMapScene.h"
#include "scene/NodeItem.h"

#include <QKeyEvent>
#include <QTest>

class tst_MindMapSceneNav : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();

    void rightTreeArrowRightDescendsToFirstChild();
    void rightTreeArrowLeftAscendsToParent();
    void rightTreeArrowDownMovesToNextSibling();
    void rightTreeArrowUpMovesToPreviousSibling();
    void topDownArrowDownDescendsToFirstChild();
    void topDownArrowRightMovesToNextSibling();
    void bilateralRightSideArrowRightDescends();
    void bilateralLeftSideArrowLeftDescends();
    void tabCyclesThroughSiblings();
    void shiftTabReversesCycle();
    void arrowKeyEventSelectsTarget();
    void noSelectionFallsBackToRoot();

private:
    void sendKey(MindMapScene& scene, Qt::Key key, Qt::KeyboardModifiers mods = Qt::NoModifier);
};

void tst_MindMapSceneNav::initTestCase() {
    TemplateRegistry::instance().loadBuiltins();
    ThemeRegistry::instance().loadBuiltins();
    LayoutAlgorithmRegistry::instance().registerBuiltins();
    MindMapScene::setDefaultStyleProvider(&RegistryStyleProvider::instance());
}

void tst_MindMapSceneNav::sendKey(MindMapScene& scene, Qt::Key key, Qt::KeyboardModifiers mods) {
    QKeyEvent ev(QEvent::KeyPress, key, mods);
    QCoreApplication::sendEvent(&scene, &ev);
}

void tst_MindMapSceneNav::rightTreeArrowRightDescendsToFirstChild() {
    MindMapScene scene;
    scene.setLayoutStyle(LayoutStyle::RightTree);
    auto* a = scene.addNode("A", scene.rootNode());
    auto* a1 = scene.addNode("A1", a);

    QCOMPARE(scene.findNeighbor(a, MindMapScene::NavDirection::Right), a1);
}

void tst_MindMapSceneNav::rightTreeArrowLeftAscendsToParent() {
    MindMapScene scene;
    scene.setLayoutStyle(LayoutStyle::RightTree);
    auto* a = scene.addNode("A", scene.rootNode());

    QCOMPARE(scene.findNeighbor(a, MindMapScene::NavDirection::Left), scene.rootNode());
}

void tst_MindMapSceneNav::rightTreeArrowDownMovesToNextSibling() {
    MindMapScene scene;
    scene.setLayoutStyle(LayoutStyle::RightTree);
    auto* a = scene.addNode("A", scene.rootNode());
    auto* b = scene.addNode("B", scene.rootNode());

    QCOMPARE(scene.findNeighbor(a, MindMapScene::NavDirection::Down), b);
    QCOMPARE(scene.findNeighbor(b, MindMapScene::NavDirection::Down),
             static_cast<NodeItem*>(nullptr));
}

void tst_MindMapSceneNav::rightTreeArrowUpMovesToPreviousSibling() {
    MindMapScene scene;
    scene.setLayoutStyle(LayoutStyle::RightTree);
    auto* a = scene.addNode("A", scene.rootNode());
    auto* b = scene.addNode("B", scene.rootNode());

    QCOMPARE(scene.findNeighbor(b, MindMapScene::NavDirection::Up), a);
    QCOMPARE(scene.findNeighbor(a, MindMapScene::NavDirection::Up),
             static_cast<NodeItem*>(nullptr));
}

void tst_MindMapSceneNav::topDownArrowDownDescendsToFirstChild() {
    MindMapScene scene;
    scene.setLayoutStyle(LayoutStyle::TopDown);
    auto* a = scene.addNode("A", scene.rootNode());
    auto* a1 = scene.addNode("A1", a);

    QCOMPARE(scene.findNeighbor(a, MindMapScene::NavDirection::Down), a1);
    QCOMPARE(scene.findNeighbor(a1, MindMapScene::NavDirection::Up), a);
}

void tst_MindMapSceneNav::topDownArrowRightMovesToNextSibling() {
    MindMapScene scene;
    scene.setLayoutStyle(LayoutStyle::TopDown);
    auto* a = scene.addNode("A", scene.rootNode());
    auto* b = scene.addNode("B", scene.rootNode());

    QCOMPARE(scene.findNeighbor(a, MindMapScene::NavDirection::Right), b);
    QCOMPARE(scene.findNeighbor(b, MindMapScene::NavDirection::Left), a);
}

void tst_MindMapSceneNav::bilateralRightSideArrowRightDescends() {
    MindMapScene scene;
    scene.setLayoutStyle(LayoutStyle::Bilateral);
    auto* right = scene.addNode("R", scene.rootNode());
    right->setPos(200, 0); // explicitly on the right side
    auto* r1 = scene.addNode("R1", right);

    QCOMPARE(scene.findNeighbor(right, MindMapScene::NavDirection::Right), r1);
    QCOMPARE(scene.findNeighbor(right, MindMapScene::NavDirection::Left), scene.rootNode());
}

void tst_MindMapSceneNav::bilateralLeftSideArrowLeftDescends() {
    MindMapScene scene;
    scene.setLayoutStyle(LayoutStyle::Bilateral);
    auto* left = scene.addNode("L", scene.rootNode());
    left->setPos(-200, 0); // on the left side
    auto* l1 = scene.addNode("L1", left);

    // For left-side nodes, "deeper" is leftward (children grow outward from
    // root), so ← descends and → ascends.
    QCOMPARE(scene.findNeighbor(left, MindMapScene::NavDirection::Left), l1);
    QCOMPARE(scene.findNeighbor(left, MindMapScene::NavDirection::Right), scene.rootNode());
}

void tst_MindMapSceneNav::tabCyclesThroughSiblings() {
    MindMapScene scene;
    auto* a = scene.addNode("A", scene.rootNode());
    auto* b = scene.addNode("B", scene.rootNode());
    auto* c = scene.addNode("C", scene.rootNode());

    QCOMPARE(scene.findNeighbor(a, MindMapScene::NavDirection::NextSibling), b);
    QCOMPARE(scene.findNeighbor(b, MindMapScene::NavDirection::NextSibling), c);
    // Wraps from the last sibling back to the first.
    QCOMPARE(scene.findNeighbor(c, MindMapScene::NavDirection::NextSibling), a);
}

void tst_MindMapSceneNav::shiftTabReversesCycle() {
    MindMapScene scene;
    auto* a = scene.addNode("A", scene.rootNode());
    auto* b = scene.addNode("B", scene.rootNode());
    auto* c = scene.addNode("C", scene.rootNode());

    QCOMPARE(scene.findNeighbor(c, MindMapScene::NavDirection::PrevSibling), b);
    QCOMPARE(scene.findNeighbor(a, MindMapScene::NavDirection::PrevSibling), c);
}

void tst_MindMapSceneNav::arrowKeyEventSelectsTarget() {
    // End-to-end: the keyPressEvent path actually moves selection.
    MindMapScene scene;
    scene.setLayoutStyle(LayoutStyle::RightTree);
    auto* a = scene.addNode("A", scene.rootNode());
    auto* a1 = scene.addNode("A1", a);

    scene.clearSelection();
    a->setSelected(true);

    sendKey(scene, Qt::Key_Right);
    QCOMPARE(scene.selectedNode(), a1);
}

void tst_MindMapSceneNav::noSelectionFallsBackToRoot() {
    // Pressing an arrow with nothing selected should not crash; it should
    // start navigating from the root.
    MindMapScene scene;
    scene.setLayoutStyle(LayoutStyle::RightTree);
    auto* a = scene.addNode("A", scene.rootNode());
    scene.clearSelection();

    sendKey(scene, Qt::Key_Right);
    QCOMPARE(scene.selectedNode(), a);
}

QTEST_MAIN(tst_MindMapSceneNav)
#include "tst_MindMapSceneNav.moc"
