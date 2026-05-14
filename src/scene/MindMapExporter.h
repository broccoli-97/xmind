#pragma once

#include <QString>

class MindMapScene;
class NodeItem;

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

private:
    void exportNodeToText(NodeItem* node, int indent, QString& output) const;
    void exportNodeToMarkdown(NodeItem* node, int level, QString& output) const;

    MindMapScene* m_scene;
};
