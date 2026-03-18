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

private:
    void exportNodeToText(NodeItem* node, int indent, QString& output) const;
    void exportNodeToMarkdown(NodeItem* node, int level, QString& output) const;

    MindMapScene* m_scene;
};
