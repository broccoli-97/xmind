#include "core/Commands.h"
#include "core/RegistryStyleProvider.h"
#include "core/TemplateRegistry.h"
#include "core/ThemeRegistry.h"
#include "layout/LayoutAlgorithmRegistry.h"
#include "scene/MindMapScene.h"
#include "scene/NodeItem.h"

#include <QLineF>
#include <QTest>
#include <QUndoStack>

// Guards the auto-layout trigger rules: a full re-layout runs only on
// explicit gestures (Ctrl+L, template switch, initial content), never as a
// side effect of editing. See MindMapScene::autoLayout() for the contract.
class tst_AutoLayoutTriggers : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();

    void editConfirmKeepsAllNodesInPlace();
    void editUndoRedoKeepsAllNodesInPlace();
    void manualAutoLayoutStillRepositionsNodes();
    void templateSwitchMarksSceneModified();
    void sameTemplateIdKeepsCleanState();
    void themeSwitchMarksSceneModified();
};

void tst_AutoLayoutTriggers::initTestCase() {
    TemplateRegistry::instance().loadBuiltins();
    ThemeRegistry::instance().loadBuiltins();
    LayoutAlgorithmRegistry::instance().registerBuiltins();
    MindMapScene::setDefaultStyleProvider(&RegistryStyleProvider::instance());
}

void tst_AutoLayoutTriggers::editConfirmKeepsAllNodesInPlace() {
    MindMapScene scene;
    auto* alpha = scene.addNode("Alpha", scene.rootNode());
    auto* beta = scene.addNode("Beta", scene.rootNode());
    auto* gamma = scene.addNode("Gamma", alpha);
    scene.layoutWithoutAnimation();

    const QPointF rootPos = scene.rootNode()->pos();
    const QPointF betaPos = beta->pos();
    const QPointF gammaPos = gamma->pos();
    const QPointF alphaPos = alpha->pos();

    // Exactly what InlineEditController::finishEditing pushes on confirm.
    scene.undoStack()->push(new EditTextCommand(
        alpha, alpha->text(), "Alpha with a considerably longer label that widens the node"));

    // A regressed re-layout would animate over 400ms (positions only diverge
    // once the animation runs), so wait past it before comparing.
    QTest::qWait(500);

    QCOMPARE(scene.rootNode()->pos(), rootPos);
    QCOMPARE(alpha->pos(), alphaPos);
    QCOMPARE(beta->pos(), betaPos);
    QCOMPARE(gamma->pos(), gammaPos);
}

void tst_AutoLayoutTriggers::editUndoRedoKeepsAllNodesInPlace() {
    MindMapScene scene;
    auto* alpha = scene.addNode("Alpha", scene.rootNode());
    auto* beta = scene.addNode("Beta", scene.rootNode());
    scene.layoutWithoutAnimation();

    const QPointF alphaPos = alpha->pos();
    const QPointF betaPos = beta->pos();

    scene.undoStack()->push(new EditTextCommand(alpha, "Alpha", "Renamed alpha topic"));
    QCOMPARE(alpha->text(), QStringLiteral("Renamed alpha topic"));

    scene.undoStack()->undo();
    QCOMPARE(alpha->text(), QStringLiteral("Alpha"));
    scene.undoStack()->redo();
    QCOMPARE(alpha->text(), QStringLiteral("Renamed alpha topic"));

    QTest::qWait(500);
    QCOMPARE(alpha->pos(), alphaPos);
    QCOMPARE(beta->pos(), betaPos);
}

void tst_AutoLayoutTriggers::manualAutoLayoutStillRepositionsNodes() {
    MindMapScene scene;
    auto* alpha = scene.addNode("Alpha", scene.rootNode());
    scene.addNode("Beta", scene.rootNode());

    // Scatter a node far away, then ask for an explicit re-layout.
    const QPointF scattered(4000.0, 4000.0);
    alpha->setPos(scattered);

    scene.autoLayout();

    // The 400ms animation pulls it back near the root; poll until it has
    // clearly left the scattered position.
    QTRY_VERIFY_WITH_TIMEOUT(QLineF(alpha->pos(), scattered).length() > 500.0, 2000);
}

void tst_AutoLayoutTriggers::templateSwitchMarksSceneModified() {
    MindMapScene scene;
    scene.addNode("Child", scene.rootNode());
    scene.setModified(false);

    const auto templates = TemplateRegistry::instance().allTemplates();
    QVERIFY(!templates.isEmpty());
    scene.setTemplateId(TemplateId(templates.first()->id));

    // templateId is persisted in the file, so switching must dirty the
    // document — otherwise closing loses the change without a save prompt.
    QVERIFY(scene.isModified());
}

void tst_AutoLayoutTriggers::sameTemplateIdKeepsCleanState() {
    MindMapScene scene;
    const auto templates = TemplateRegistry::instance().allTemplates();
    QVERIFY(!templates.isEmpty());
    const TemplateId id(templates.first()->id);

    scene.setTemplateId(id);
    scene.setModified(false);

    scene.setTemplateId(id);
    QVERIFY(!scene.isModified());
}

void tst_AutoLayoutTriggers::themeSwitchMarksSceneModified() {
    MindMapScene scene;
    scene.setModified(false);

    const auto themes = ThemeRegistry::instance().allThemes();
    QVERIFY(!themes.isEmpty());
    const ThemeId id(themes.first()->id);

    scene.setThemeId(id);
    QVERIFY(scene.isModified());

    // Re-applying the active theme is a no-op and must stay clean.
    scene.setModified(false);
    scene.setThemeId(id);
    QVERIFY(!scene.isModified());
}

QTEST_MAIN(tst_AutoLayoutTriggers)
#include "tst_AutoLayoutTriggers.moc"
