#include "core/RegistryStyleProvider.h"
#include "core/TemplateRegistry.h"
#include "core/ThemeRegistry.h"
#include "layout/LayoutAlgorithmRegistry.h"
#include "scene/MindMapScene.h"
#include "scene/NodeItem.h"

#include <QTest>

class tst_MarkdownImport : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();

    void empty();
    void rootOnly();
    void headingsBuildHierarchy();
    void listsNestUnderHeading();
    void siblingHeadingsBecomeChildrenOfRoot();
    void mixedHeadingsAndLists();
    void codeFenceIgnored();
    void stripsInlineEmphasis();
    void multipleTopLevelLists();
};

void tst_MarkdownImport::initTestCase() {
    TemplateRegistry::instance().loadBuiltins();
    ThemeRegistry::instance().loadBuiltins();
    LayoutAlgorithmRegistry::instance().registerBuiltins();
    MindMapScene::setDefaultStyleProvider(&RegistryStyleProvider::instance());
}

void tst_MarkdownImport::empty() {
    MindMapScene scene;
    QVERIFY(!scene.importFromMarkdown("", false));
    QVERIFY(!scene.importFromMarkdown("   \n  \n", false));
}

void tst_MarkdownImport::rootOnly() {
    MindMapScene scene;
    QVERIFY(scene.importFromMarkdown("# Just a root", false));
    QCOMPARE(scene.rootNode()->text(), QString("Just a root"));
    QCOMPARE(scene.rootNode()->childNodes().size(), 0);
}

void tst_MarkdownImport::headingsBuildHierarchy() {
    MindMapScene scene;
    const QString md =
        "# Root\n"
        "## A\n"
        "### A1\n"
        "## B\n";
    QVERIFY(scene.importFromMarkdown(md, false));
    QCOMPARE(scene.rootNode()->text(), QString("Root"));
    auto rc = scene.rootNode()->childNodes();
    QCOMPARE(rc.size(), 2);
    QCOMPARE(rc[0]->text(), QString("A"));
    QCOMPARE(rc[1]->text(), QString("B"));
    auto ac = rc[0]->childNodes();
    QCOMPARE(ac.size(), 1);
    QCOMPARE(ac[0]->text(), QString("A1"));
}

void tst_MarkdownImport::listsNestUnderHeading() {
    MindMapScene scene;
    const QString md =
        "# Root\n"
        "## Branch\n"
        "- leaf1\n"
        "- leaf2\n"
        "  - nested\n";
    QVERIFY(scene.importFromMarkdown(md, false));
    auto rc = scene.rootNode()->childNodes();
    QCOMPARE(rc.size(), 1);
    QCOMPARE(rc[0]->text(), QString("Branch"));
    auto bc = rc[0]->childNodes();
    QCOMPARE(bc.size(), 2);
    QCOMPARE(bc[0]->text(), QString("leaf1"));
    QCOMPARE(bc[1]->text(), QString("leaf2"));
    QCOMPARE(bc[1]->childNodes().size(), 1);
    QCOMPARE(bc[1]->childNodes()[0]->text(), QString("nested"));
}

void tst_MarkdownImport::siblingHeadingsBecomeChildrenOfRoot() {
    // Two H1s should collapse to "second is a child of the first" so that the
    // mindmap remains single-rooted.
    MindMapScene scene;
    const QString md = "# First\n# Second\n# Third\n";
    QVERIFY(scene.importFromMarkdown(md, false));
    QCOMPARE(scene.rootNode()->text(), QString("First"));
    auto rc = scene.rootNode()->childNodes();
    QCOMPARE(rc.size(), 2);
    QCOMPARE(rc[0]->text(), QString("Second"));
    QCOMPARE(rc[1]->text(), QString("Third"));
}

void tst_MarkdownImport::mixedHeadingsAndLists() {
    MindMapScene scene;
    const QString md =
        "# Trip\n"
        "## Day 1\n"
        "- Pack\n"
        "- Drive\n"
        "## Day 2\n"
        "- Hike\n"
        "  - Trail map\n";
    QVERIFY(scene.importFromMarkdown(md, false));
    auto rc = scene.rootNode()->childNodes();
    QCOMPARE(rc.size(), 2);
    QCOMPARE(rc[0]->text(), QString("Day 1"));
    QCOMPARE(rc[0]->childNodes().size(), 2);
    QCOMPARE(rc[1]->text(), QString("Day 2"));
    auto d2 = rc[1]->childNodes();
    QCOMPARE(d2.size(), 1);
    QCOMPARE(d2[0]->text(), QString("Hike"));
    QCOMPARE(d2[0]->childNodes().size(), 1);
    QCOMPARE(d2[0]->childNodes()[0]->text(), QString("Trail map"));
}

void tst_MarkdownImport::codeFenceIgnored() {
    MindMapScene scene;
    const QString md =
        "# Title\n"
        "```\n"
        "## not a heading\n"
        "- not a leaf\n"
        "```\n"
        "## Real heading\n";
    QVERIFY(scene.importFromMarkdown(md, false));
    auto rc = scene.rootNode()->childNodes();
    QCOMPARE(rc.size(), 1);
    QCOMPARE(rc[0]->text(), QString("Real heading"));
}

void tst_MarkdownImport::stripsInlineEmphasis() {
    MindMapScene scene;
    const QString md =
        "# **Bold root**\n"
        "## *Italic* branch\n"
        "- a `code` leaf\n"
        "- [a link](http://x.com)\n";
    QVERIFY(scene.importFromMarkdown(md, false));
    QCOMPARE(scene.rootNode()->text(), QString("Bold root"));
    auto rc = scene.rootNode()->childNodes();
    QCOMPARE(rc[0]->text(), QString("Italic branch"));
    auto bc = rc[0]->childNodes();
    QCOMPARE(bc[0]->text(), QString("a code leaf"));
    QCOMPARE(bc[1]->text(), QString("a link"));
}

void tst_MarkdownImport::multipleTopLevelLists() {
    // No heading at all — first list item becomes root.
    MindMapScene scene;
    const QString md =
        "- root\n"
        "  - a\n"
        "  - b\n";
    QVERIFY(scene.importFromMarkdown(md, false));
    QCOMPARE(scene.rootNode()->text(), QString("root"));
    auto rc = scene.rootNode()->childNodes();
    QCOMPARE(rc.size(), 2);
    QCOMPARE(rc[0]->text(), QString("a"));
    QCOMPARE(rc[1]->text(), QString("b"));
}

QTEST_MAIN(tst_MarkdownImport)
#include "tst_MarkdownImport.moc"
