#include "core/RegistryStyleProvider.h"
#include "core/TemplateRegistry.h"
#include "core/ThemeRegistry.h"
#include "layout/LayoutAlgorithmRegistry.h"
#include "scene/MindMapScene.h"
#include "scene/NodeItem.h"

#include <QKeyEvent>
#include <QTest>

class tst_MindMapSceneKeys : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();

    void bareBackspaceDoesNotDeleteSelectedNode();
    void deleteKeyRemovesSelectedNode();
    void ctrlBackspaceRemovesSelectedNode();

private:
    // Build a scene with the root + one child node, with the child selected.
    // Returns the child so callers can verify it was (or was not) removed.
    NodeItem* makeSceneWithSelectedChild(MindMapScene& scene);
    void sendSceneKeyPress(MindMapScene& scene, Qt::Key key, Qt::KeyboardModifiers mods);
};

void tst_MindMapSceneKeys::initTestCase() {
    TemplateRegistry::instance().loadBuiltins();
    ThemeRegistry::instance().loadBuiltins();
    LayoutAlgorithmRegistry::instance().registerBuiltins();
    MindMapScene::setDefaultStyleProvider(&RegistryStyleProvider::instance());
}

NodeItem* tst_MindMapSceneKeys::makeSceneWithSelectedChild(MindMapScene& scene) {
    auto* child = scene.addNode("Child", scene.rootNode());
    scene.clearSelection();
    child->setSelected(true);
    return child;
}

void tst_MindMapSceneKeys::sendSceneKeyPress(MindMapScene& scene, Qt::Key key,
                                             Qt::KeyboardModifiers mods) {
    QKeyEvent ev(QEvent::KeyPress, key, mods);
    QCoreApplication::sendEvent(&scene, &ev);
}

void tst_MindMapSceneKeys::bareBackspaceDoesNotDeleteSelectedNode() {
    MindMapScene scene;
    NodeItem* child = makeSceneWithSelectedChild(scene);
    QCOMPARE(scene.rootNode()->childNodes().size(), 1);

    sendSceneKeyPress(scene, Qt::Key_Backspace, Qt::NoModifier);

    QCOMPARE(scene.rootNode()->childNodes().size(), 1);
    QVERIFY(scene.rootNode()->childNodes().contains(child));
}

void tst_MindMapSceneKeys::deleteKeyRemovesSelectedNode() {
    MindMapScene scene;
    makeSceneWithSelectedChild(scene);
    QCOMPARE(scene.rootNode()->childNodes().size(), 1);

    sendSceneKeyPress(scene, Qt::Key_Delete, Qt::NoModifier);

    QCOMPARE(scene.rootNode()->childNodes().size(), 0);
}

void tst_MindMapSceneKeys::ctrlBackspaceRemovesSelectedNode() {
    MindMapScene scene;
    makeSceneWithSelectedChild(scene);
    QCOMPARE(scene.rootNode()->childNodes().size(), 1);

    sendSceneKeyPress(scene, Qt::Key_Backspace, Qt::ControlModifier);

    QCOMPARE(scene.rootNode()->childNodes().size(), 0);
}

QTEST_MAIN(tst_MindMapSceneKeys)
#include "tst_MindMapSceneKeys.moc"
