#include "core/RegistryStyleProvider.h"
#include "core/TemplateRegistry.h"
#include "core/ThemeRegistry.h"
#include "layout/LayoutAlgorithmRegistry.h"
#include "scene/MindMapScene.h"
#include "scene/NodeItem.h"
#include "ui/OutlineWidget.h"

#include <QKeyEvent>
#include <QTest>
#include <QTreeWidget>

// Regression coverage for the BACKLOG.md "outline doesn't sync to selection"
// note. The plumbing already exists (OutlineWidget::refresh wires
// QGraphicsScene::selectionChanged -> syncSelection), but it's load-bearing
// for keyboard navigation and easy to lose to an inadvertent disconnect.
class tst_OutlineWidget : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();

    void selectingNodeInSceneUpdatesOutlineCurrentItem();
    void clearingSceneSelectionClearsOutlineCurrentItem();

    void foldingOutlineItemCollapsesSceneSubtree();
    void unfoldingOutlineItemExpandsSceneSubtree();
    void canvasCollapseFoldsMatchingOutlineItem();
    void rebuildReflectsCollapsedNodeAsFolded();

private:
    QTreeWidgetItem* findOutlineItemForNode(QTreeWidget* tree, NodeItem* target);
};

void tst_OutlineWidget::initTestCase() {
    TemplateRegistry::instance().loadBuiltins();
    ThemeRegistry::instance().loadBuiltins();
    LayoutAlgorithmRegistry::instance().registerBuiltins();
    MindMapScene::setDefaultStyleProvider(&RegistryStyleProvider::instance());
}

QTreeWidgetItem* tst_OutlineWidget::findOutlineItemForNode(QTreeWidget* tree, NodeItem* target) {
    const quintptr targetPtr = reinterpret_cast<quintptr>(target);
    QTreeWidgetItemIterator it(tree);
    while (*it) {
        if ((*it)->data(0, Qt::UserRole).value<quintptr>() == targetPtr)
            return *it;
        ++it;
    }
    return nullptr;
}

void tst_OutlineWidget::selectingNodeInSceneUpdatesOutlineCurrentItem() {
    MindMapScene scene;
    auto* child = scene.addNode("Child", scene.rootNode());

    OutlineWidget outline;
    outline.refresh(&scene);

    auto* tree = outline.findChild<QTreeWidget*>();
    QVERIFY(tree);

    // Drive selection on the scene side; the outline must mirror it.
    scene.clearSelection();
    child->setSelected(true);

    auto* expectedItem = findOutlineItemForNode(tree, child);
    QVERIFY2(expectedItem, "outline tree should contain an entry for the child node");
    QCOMPARE(tree->currentItem(), expectedItem);
}

void tst_OutlineWidget::clearingSceneSelectionClearsOutlineCurrentItem() {
    MindMapScene scene;
    auto* child = scene.addNode("Child", scene.rootNode());

    OutlineWidget outline;
    outline.refresh(&scene);

    child->setSelected(true);
    auto* tree = outline.findChild<QTreeWidget*>();
    QVERIFY(tree && tree->currentItem() != nullptr);

    scene.clearSelection();
    QCOMPARE(tree->currentItem(), static_cast<QTreeWidgetItem*>(nullptr));
}

void tst_OutlineWidget::foldingOutlineItemCollapsesSceneSubtree() {
    MindMapScene scene;
    auto* a = scene.addNode("A", scene.rootNode());
    auto* a1 = scene.addNode("A1", a);

    OutlineWidget outline;
    outline.refresh(&scene);
    auto* tree = outline.findChild<QTreeWidget*>();
    QVERIFY(tree);

    auto* itemA = findOutlineItemForNode(tree, a);
    QVERIFY(itemA);
    QVERIFY(!a->isCollapsed());
    QVERIFY(a1->isVisible());

    // Fold the branch in the outline; the canvas node must collapse and hide
    // its descendant.
    itemA->setExpanded(false);
    QVERIFY(a->isCollapsed());
    QVERIFY(!a1->isVisible());
}

void tst_OutlineWidget::unfoldingOutlineItemExpandsSceneSubtree() {
    MindMapScene scene;
    auto* a = scene.addNode("A", scene.rootNode());
    auto* a1 = scene.addNode("A1", a);
    a->setCollapsed(true);

    OutlineWidget outline;
    outline.refresh(&scene);
    auto* tree = outline.findChild<QTreeWidget*>();
    QVERIFY(tree);

    auto* itemA = findOutlineItemForNode(tree, a);
    QVERIFY(itemA);
    // A built-from-collapsed node shows up folded.
    QVERIFY(!itemA->isExpanded());
    QVERIFY(!a1->isVisible());

    itemA->setExpanded(true);
    QVERIFY(!a->isCollapsed());
    QVERIFY(a1->isVisible());
}

void tst_OutlineWidget::canvasCollapseFoldsMatchingOutlineItem() {
    MindMapScene scene;
    auto* a = scene.addNode("A", scene.rootNode());
    scene.addNode("A1", a);

    OutlineWidget outline;
    outline.refresh(&scene);
    auto* tree = outline.findChild<QTreeWidget*>();
    QVERIFY(tree);

    auto* itemA = findOutlineItemForNode(tree, a);
    QVERIFY(itemA && itemA->isExpanded());

    // Collapse on the canvas via the Space shortcut; the outline must mirror it.
    scene.clearSelection();
    a->setSelected(true);
    QKeyEvent space(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
    QCoreApplication::sendEvent(&scene, &space);

    QVERIFY(a->isCollapsed());
    QVERIFY(!itemA->isExpanded());
}

void tst_OutlineWidget::rebuildReflectsCollapsedNodeAsFolded() {
    MindMapScene scene;
    auto* a = scene.addNode("A", scene.rootNode());
    scene.addNode("A1", a);

    OutlineWidget outline;
    outline.refresh(&scene);

    a->setCollapsed(true);
    // A fresh rebuild (e.g. after an undo-stack change) should keep the fold.
    outline.refresh(&scene);

    auto* tree = outline.findChild<QTreeWidget*>();
    QVERIFY(tree);
    auto* itemA = findOutlineItemForNode(tree, a);
    QVERIFY(itemA);
    QVERIFY(!itemA->isExpanded());
}

QTEST_MAIN(tst_OutlineWidget)
#include "tst_OutlineWidget.moc"
