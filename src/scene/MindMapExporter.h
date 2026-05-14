#pragma once

#include <QList>
#include <QString>

class MindMapScene;
class NodeItem;

// One problem found while validating a markdown import. `line` is 1-based;
// 0 means the issue applies to the whole file.
struct MarkdownImportIssue {
    int line;
    QString message;
};

struct MarkdownImportReport {
    QList<MarkdownImportIssue> errors;
    bool ok() const { return errors.isEmpty(); }
};

class MindMapExporter {
public:
    explicit MindMapExporter(MindMapScene* scene);

    QString exportToText() const;
    QString exportToMarkdown() const;
    bool exportToPng(const QString& filePath, int scaleFactor = 2);
    bool exportToSvg(const QString& filePath);
    bool exportToPdf(const QString& filePath);
    bool importFromText(const QString& text);
    // Parses CommonMark-flavored Markdown (ATX headings + nested lists) into a
    // mindmap tree. The first encountered node becomes the root. Subsequent
    // nodes are placed by relative depth: `##` is one level deeper than `#`,
    // and a 2-space-indented list item is one level deeper than the surrounding
    // list. When `animate` is true the scene's auto-layout is triggered; CLI
    // callers should pass false and invoke layoutWithoutAnimation() directly.
    bool importFromMarkdown(const QString& text, bool animate = true);

    // Strict importer for user-facing GUI imports. Accepts a small, clean
    // subset of Markdown: optional single `# Title` root followed by an
    // unordered list (`-`/`*`/`+`) with 2-space indentation per nesting level.
    // Anything outside this subset (other headings, tables, blockquotes,
    // code fences, ordered lists, HTML, tab indentation, level skips, stray
    // paragraphs) is rejected with a line-numbered message in `report`. If
    // any errors are recorded the scene is left untouched and the function
    // returns false. See docs/markdown-import-format.md for the full spec.
    bool importFromMarkdownStrict(const QString& text,
                                  MarkdownImportReport* report,
                                  bool animate = true);

private:
    void exportNodeToText(NodeItem* node, int indent, QString& output) const;
    void exportNodeToMarkdown(NodeItem* node, int level, QString& output) const;

    MindMapScene* m_scene;
};
