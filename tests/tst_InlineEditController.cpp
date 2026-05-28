#include "core/RegistryStyleProvider.h"
#include "core/TemplateRegistry.h"
#include "core/ThemeRegistry.h"
#include "layout/LayoutAlgorithmRegistry.h"
#include "scene/InlineEditController.h"
#include "scene/MindMapScene.h"
#include "scene/NodeItem.h"

#include <QSignalSpy>
#include <QTest>

class tst_InlineEditController : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();

    void startEditingEmitsEditingStartedOnce();
    void finishEditingEmitsEditingFinishedOnce();
    void cancelEditingEmitsEditingFinishedOnce();
    void sceneReemitsControllerSignals();
};

void tst_InlineEditController::initTestCase() {
    qRegisterMetaType<NodeItem*>("NodeItem*");

    TemplateRegistry::instance().loadBuiltins();
    ThemeRegistry::instance().loadBuiltins();
    LayoutAlgorithmRegistry::instance().registerBuiltins();
    MindMapScene::setDefaultStyleProvider(&RegistryStyleProvider::instance());
}

void tst_InlineEditController::startEditingEmitsEditingStartedOnce() {
    MindMapScene scene;
    auto* child = scene.addNode("Child", scene.rootNode());

    QSignalSpy spy(&scene, &MindMapScene::editingStarted);
    QVERIFY(spy.isValid());

    scene.startEditing(child);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).value<NodeItem*>(), child);

    QVERIFY(scene.isEditing());
    scene.cancelEditing();
}

void tst_InlineEditController::finishEditingEmitsEditingFinishedOnce() {
    // addChildToSelected is one of the few public scene calls that internally
    // finishes any in-progress edit before doing its work — exercising the
    // commit path on InlineEditController::finishEditing. It then starts a
    // fresh edit on the newly created node, which we tear down afterwards.
    MindMapScene scene;
    auto* child = scene.addNode("Child", scene.rootNode());
    scene.startEditing(child);
    QVERIFY(scene.isEditing());

    QSignalSpy spy(&scene, &MindMapScene::editingFinished);
    QVERIFY(spy.isValid());

    scene.addChildToSelected();

    QCOMPARE(spy.count(), 1);
    scene.cancelEditing(); // tear down the second edit that addChild started.
}

void tst_InlineEditController::cancelEditingEmitsEditingFinishedOnce() {
    MindMapScene scene;
    auto* child = scene.addNode("Child", scene.rootNode());

    scene.startEditing(child);
    QVERIFY(scene.isEditing());

    QSignalSpy spy(&scene, &MindMapScene::editingFinished);
    QVERIFY(spy.isValid());

    scene.cancelEditing();

    QCOMPARE(spy.count(), 1);
    QVERIFY(!scene.isEditing());
}

void tst_InlineEditController::sceneReemitsControllerSignals() {
    // Quick belt-and-braces check that scene.editingStarted carries through
    // the same NodeItem* the controller fired with.
    MindMapScene scene;
    auto* a = scene.addNode("A", scene.rootNode());
    auto* b = scene.addNode("B", scene.rootNode());

    QSignalSpy startedSpy(&scene, &MindMapScene::editingStarted);
    QSignalSpy finishedSpy(&scene, &MindMapScene::editingFinished);

    scene.startEditing(a);
    scene.cancelEditing();
    scene.startEditing(b);
    scene.cancelEditing();

    QCOMPARE(startedSpy.count(), 2);
    QCOMPARE(finishedSpy.count(), 2);
    QCOMPARE(startedSpy.at(0).at(0).value<NodeItem*>(), a);
    QCOMPARE(startedSpy.at(1).at(0).value<NodeItem*>(), b);
}

QTEST_MAIN(tst_InlineEditController)
#include "tst_InlineEditController.moc"
