#include "core/TemplateRegistry.h"
#include "layout/LayoutAlgorithmRegistry.h"
#include "scene/MindMapExporter.h"
#include "scene/MindMapScene.h"
#include "scene/NodeItem.h"

#include <QTest>

class tst_MarkdownImportStrict : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();

    void emptyFile();
    void rootHeadingOnly();
    void simpleOutline();
    void multipleLevels();
    void noHeadingFirstItemBecomesRoot();
    void rejectsH2();
    void rejectsSetextHeading();
    void rejectsHorizontalRule();
    void rejectsTabIndentation();
    void rejectsOddSpaceIndent();
    void rejectsLevelSkip();
    void rejectsOrderedList();
    void rejectsBlockquote();
    void rejectsCodeFence();
    void rejectsTable();
    void rejectsHtmlTag();
    void rejectsStrayText();
    void rejectsSecondH1();
    void ignoresHtmlComment();
    void ignoresBlankLines();
    void stripsInlineMarkup();
    void sceneUntouchedOnError();
};

void tst_MarkdownImportStrict::initTestCase() {
    TemplateRegistry::instance().loadBuiltins();
    LayoutAlgorithmRegistry::instance().registerBuiltins();
}

void tst_MarkdownImportStrict::emptyFile() {
    MindMapScene scene;
    MarkdownImportReport report;
    QVERIFY(!scene.importFromMarkdownStrict("", &report, false));
    QVERIFY(!report.ok());
    QCOMPARE(report.errors.size(), 1);
    QCOMPARE(report.errors[0].line, 0);
}

void tst_MarkdownImportStrict::rootHeadingOnly() {
    MindMapScene scene;
    MarkdownImportReport report;
    QVERIFY(scene.importFromMarkdownStrict("# Just a root", &report, false));
    QVERIFY(report.ok());
    QCOMPARE(scene.rootNode()->text(), QString("Just a root"));
    QCOMPARE(scene.rootNode()->childNodes().size(), 0);
}

void tst_MarkdownImportStrict::simpleOutline() {
    MindMapScene scene;
    MarkdownImportReport report;
    const QString md = "# Root\n- a\n- b\n  - b1\n- c\n";
    QVERIFY(scene.importFromMarkdownStrict(md, &report, false));
    QVERIFY(report.ok());
    auto rc = scene.rootNode()->childNodes();
    QCOMPARE(rc.size(), 3);
    QCOMPARE(rc[0]->text(), QString("a"));
    QCOMPARE(rc[1]->text(), QString("b"));
    QCOMPARE(rc[1]->childNodes().size(), 1);
    QCOMPARE(rc[1]->childNodes()[0]->text(), QString("b1"));
    QCOMPARE(rc[2]->text(), QString("c"));
}

void tst_MarkdownImportStrict::multipleLevels() {
    MindMapScene scene;
    MarkdownImportReport report;
    const QString md = "# Root\n"
                       "- a\n"
                       "  - a1\n"
                       "    - a1a\n"
                       "      - a1a1\n";
    QVERIFY(scene.importFromMarkdownStrict(md, &report, false));
    QVERIFY(report.ok());
    auto* n = scene.rootNode()->childNodes()[0];
    QCOMPARE(n->text(), QString("a"));
    n = n->childNodes()[0];
    QCOMPARE(n->text(), QString("a1"));
    n = n->childNodes()[0];
    QCOMPARE(n->text(), QString("a1a"));
    n = n->childNodes()[0];
    QCOMPARE(n->text(), QString("a1a1"));
}

void tst_MarkdownImportStrict::noHeadingFirstItemBecomesRoot() {
    MindMapScene scene;
    MarkdownImportReport report;
    const QString md = "- root\n  - a\n  - b\n";
    QVERIFY(scene.importFromMarkdownStrict(md, &report, false));
    QVERIFY(report.ok());
    QCOMPARE(scene.rootNode()->text(), QString("root"));
    QCOMPARE(scene.rootNode()->childNodes().size(), 2);
}

void tst_MarkdownImportStrict::rejectsH2() {
    MindMapScene scene;
    MarkdownImportReport report;
    QVERIFY(!scene.importFromMarkdownStrict("# Root\n## Branch\n", &report, false));
    QVERIFY(!report.ok());
    QCOMPARE(report.errors[0].line, 2);
    QVERIFY(report.errors[0].message.contains("H1"));
}

void tst_MarkdownImportStrict::rejectsSetextHeading() {
    MindMapScene scene;
    MarkdownImportReport report;
    QVERIFY(!scene.importFromMarkdownStrict("Title\n=====\n", &report, false));
    QVERIFY(!report.ok());
    QCOMPARE(report.errors[0].line, 2);
}

void tst_MarkdownImportStrict::rejectsHorizontalRule() {
    MindMapScene scene;
    MarkdownImportReport report;
    QVERIFY(!scene.importFromMarkdownStrict("# Root\n\n---\n- a\n", &report, false));
    QVERIFY(!report.ok());
    // Line 3 is the rule.
    QCOMPARE(report.errors[0].line, 3);
}

void tst_MarkdownImportStrict::rejectsTabIndentation() {
    MindMapScene scene;
    MarkdownImportReport report;
    const QString md = "# Root\n- a\n\t- a1\n";
    QVERIFY(!scene.importFromMarkdownStrict(md, &report, false));
    QVERIFY(!report.ok());
    QCOMPARE(report.errors[0].line, 3);
    QVERIFY(report.errors[0].message.contains("tabs"));
}

void tst_MarkdownImportStrict::rejectsOddSpaceIndent() {
    MindMapScene scene;
    MarkdownImportReport report;
    const QString md = "# Root\n- a\n   - a1\n";
    QVERIFY(!scene.importFromMarkdownStrict(md, &report, false));
    QVERIFY(!report.ok());
    QCOMPARE(report.errors[0].line, 3);
    QVERIFY(report.errors[0].message.contains("multiple of 2"));
}

void tst_MarkdownImportStrict::rejectsLevelSkip() {
    MindMapScene scene;
    MarkdownImportReport report;
    const QString md = "# Root\n- a\n    - skipped\n";
    QVERIFY(!scene.importFromMarkdownStrict(md, &report, false));
    QVERIFY(!report.ok());
    QCOMPARE(report.errors[0].line, 3);
    QVERIFY(report.errors[0].message.contains("skipped"));
}

void tst_MarkdownImportStrict::rejectsOrderedList() {
    MindMapScene scene;
    MarkdownImportReport report;
    QVERIFY(!scene.importFromMarkdownStrict("# Root\n1. a\n", &report, false));
    QVERIFY(!report.ok());
    QCOMPARE(report.errors[0].line, 2);
    QVERIFY(report.errors[0].message.contains("Ordered"));
}

void tst_MarkdownImportStrict::rejectsBlockquote() {
    MindMapScene scene;
    MarkdownImportReport report;
    QVERIFY(!scene.importFromMarkdownStrict("# Root\n> quote\n", &report, false));
    QVERIFY(!report.ok());
    QCOMPARE(report.errors[0].line, 2);
}

void tst_MarkdownImportStrict::rejectsCodeFence() {
    MindMapScene scene;
    MarkdownImportReport report;
    QVERIFY(!scene.importFromMarkdownStrict("# Root\n```\ncode\n```\n", &report, false));
    QVERIFY(!report.ok());
    QCOMPARE(report.errors[0].line, 2);
}

void tst_MarkdownImportStrict::rejectsTable() {
    MindMapScene scene;
    MarkdownImportReport report;
    QVERIFY(!scene.importFromMarkdownStrict("# Root\n| a | b |\n", &report, false));
    QVERIFY(!report.ok());
    QCOMPARE(report.errors[0].line, 2);
}

void tst_MarkdownImportStrict::rejectsHtmlTag() {
    MindMapScene scene;
    MarkdownImportReport report;
    QVERIFY(!scene.importFromMarkdownStrict("# Root\n<div>x</div>\n", &report, false));
    QVERIFY(!report.ok());
    QCOMPARE(report.errors[0].line, 2);
}

void tst_MarkdownImportStrict::rejectsStrayText() {
    MindMapScene scene;
    MarkdownImportReport report;
    const QString md = "# Root\n- a\nstray paragraph\n";
    QVERIFY(!scene.importFromMarkdownStrict(md, &report, false));
    QVERIFY(!report.ok());
    QCOMPARE(report.errors[0].line, 3);
    QVERIFY(report.errors[0].message.contains("Stray"));
}

void tst_MarkdownImportStrict::rejectsSecondH1() {
    MindMapScene scene;
    MarkdownImportReport report;
    QVERIFY(!scene.importFromMarkdownStrict("# First\n# Second\n", &report, false));
    QVERIFY(!report.ok());
    QCOMPARE(report.errors[0].line, 2);
}

void tst_MarkdownImportStrict::ignoresHtmlComment() {
    MindMapScene scene;
    MarkdownImportReport report;
    const QString md = "<!-- this is metadata -->\n# Root\n- a\n";
    QVERIFY(scene.importFromMarkdownStrict(md, &report, false));
    QVERIFY(report.ok());
    QCOMPARE(scene.rootNode()->text(), QString("Root"));
}

void tst_MarkdownImportStrict::ignoresBlankLines() {
    MindMapScene scene;
    MarkdownImportReport report;
    const QString md = "\n\n# Root\n\n- a\n\n  - a1\n\n";
    QVERIFY(scene.importFromMarkdownStrict(md, &report, false));
    QVERIFY(report.ok());
    QCOMPARE(scene.rootNode()->childNodes().size(), 1);
    QCOMPARE(scene.rootNode()->childNodes()[0]->text(), QString("a"));
}

void tst_MarkdownImportStrict::stripsInlineMarkup() {
    MindMapScene scene;
    MarkdownImportReport report;
    const QString md = "# **Bold root**\n- *Italic item*\n- `code` and [link](u)\n";
    QVERIFY(scene.importFromMarkdownStrict(md, &report, false));
    QVERIFY(report.ok());
    QCOMPARE(scene.rootNode()->text(), QString("Bold root"));
    auto rc = scene.rootNode()->childNodes();
    QCOMPARE(rc[0]->text(), QString("Italic item"));
    QCOMPARE(rc[1]->text(), QString("code and link"));
}

void tst_MarkdownImportStrict::sceneUntouchedOnError() {
    // Build a baseline scene from a valid import, then attempt a bad one and
    // verify the scene is left at the baseline (strict importer must not
    // partially mutate when validation fails).
    MindMapScene scene;
    MarkdownImportReport report;
    QVERIFY(scene.importFromMarkdownStrict("# Baseline\n- a\n", &report, false));
    const QString baselineRootText = scene.rootNode()->text();
    const int baselineChildCount = scene.rootNode()->childNodes().size();

    const QString bad = "# Other\n- ok\n## badness\n- never\n";
    MarkdownImportReport badReport;
    QVERIFY(!scene.importFromMarkdownStrict(bad, &badReport, false));
    QVERIFY(!badReport.ok());

    QCOMPARE(scene.rootNode()->text(), baselineRootText);
    QCOMPARE(scene.rootNode()->childNodes().size(), baselineChildCount);
}

QTEST_MAIN(tst_MarkdownImportStrict)
#include "tst_MarkdownImportStrict.moc"
