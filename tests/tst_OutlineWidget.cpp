#include "core/RegistryStyleProvider.h"
#include "core/TemplateRegistry.h"
#include "core/ThemeRegistry.h"
#include "layout/LayoutAlgorithmRegistry.h"
#include "scene/MindMapScene.h"
#include "scene/NodeItem.h"
#include "ui/OutlineWidget.h"

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

QTEST_MAIN(tst_OutlineWidget)
#include "tst_OutlineWidget.moc"
