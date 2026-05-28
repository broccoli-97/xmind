#include "core/RegistryStyleProvider.h"
#include "core/TemplateRegistry.h"
#include "core/ThemeRegistry.h"
#include "layout/LayoutAlgorithmRegistry.h"
#include "scene/MindMapScene.h"
#include "scene/NodeItem.h"

#include <QTest>

class tst_SceneFindMatches : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();

    void emptyNeedleReturnsEmpty();
    void caseInsensitiveByDefault();
    void caseSensitiveSearchRespectsExactCase();
    void returnsAllMatchesPreOrder();
    void substringMatches();
    void noMatchesReturnsEmpty();
    void clearHighlightsResetsBothFlags();
};

void tst_SceneFindMatches::initTestCase() {
    TemplateRegistry::instance().loadBuiltins();
    ThemeRegistry::instance().loadBuiltins();
    LayoutAlgorithmRegistry::instance().registerBuiltins();
    MindMapScene::setDefaultStyleProvider(&RegistryStyleProvider::instance());
}

void tst_SceneFindMatches::emptyNeedleReturnsEmpty() {
    MindMapScene scene;
    scene.addNode("Anything", scene.rootNode());
    QVERIFY(scene.findMatches("").isEmpty());
}

void tst_SceneFindMatches::caseInsensitiveByDefault() {
    MindMapScene scene;
    scene.rootNode()->setText("Root");
    auto* hello = scene.addNode("Hello World", scene.rootNode());

    auto matches = scene.findMatches("hello");
    QCOMPARE(matches.size(), 1);
    QCOMPARE(matches[0], hello);
}

void tst_SceneFindMatches::caseSensitiveSearchRespectsExactCase() {
    MindMapScene scene;
    scene.rootNode()->setText("root");
    auto* upper = scene.addNode("Foo", scene.rootNode());
    scene.addNode("foo", scene.rootNode());

    auto matches = scene.findMatches("Foo", Qt::CaseSensitive);
    QCOMPARE(matches.size(), 1);
    QCOMPARE(matches[0], upper);
}

void tst_SceneFindMatches::returnsAllMatchesPreOrder() {
    // Build root -> [A, B]; A has child A1. The pre-order walk is
    // root, A, A1, B. Querying for "" letter would match everything, but
    // we narrow with "1" / a unique letter.
    MindMapScene scene;
    scene.rootNode()->setText("a-root");
    auto* a = scene.addNode("a-child", scene.rootNode());
    auto* a1 = scene.addNode("a-grandchild", a);
    auto* b = scene.addNode("a-sibling", scene.rootNode());

    auto matches = scene.findMatches("a-");
    QCOMPARE(matches.size(), 4);
    // Pre-order: root, a, a1, b.
    QCOMPARE(matches[0], scene.rootNode());
    QCOMPARE(matches[1], a);
    QCOMPARE(matches[2], a1);
    QCOMPARE(matches[3], b);
}

void tst_SceneFindMatches::substringMatches() {
    MindMapScene scene;
    auto* a = scene.addNode("The quick brown fox", scene.rootNode());
    auto matches = scene.findMatches("brown");
    QCOMPARE(matches.size(), 1);
    QCOMPARE(matches[0], a);
}

void tst_SceneFindMatches::noMatchesReturnsEmpty() {
    MindMapScene scene;
    scene.addNode("Hello", scene.rootNode());
    QVERIFY(scene.findMatches("zzz").isEmpty());
}

void tst_SceneFindMatches::clearHighlightsResetsBothFlags() {
    MindMapScene scene;
    auto* a = scene.addNode("A", scene.rootNode());
    auto* b = scene.addNode("B", scene.rootNode());

    a->setSearchMatch(true);
    a->setSearchCurrent(true);
    b->setSearchMatch(true);

    scene.clearSearchHighlights();

    QVERIFY(!a->isSearchMatch());
    QVERIFY(!b->isSearchMatch());
}

QTEST_MAIN(tst_SceneFindMatches)
#include "tst_SceneFindMatches.moc"
