#pragma once

#include "layout/LayoutEngine.h"

#include <QGraphicsScene>
#include <QMap>

class NodeItem;
class EdgeItem;
class QJsonObject;
class QJsonArray;
class QUndoStack;
class TemplateDescriptor;
class InlineEditController;

class MindMapScene : public QGraphicsScene {
    Q_OBJECT

public:
    explicit MindMapScene(QObject* parent = nullptr);

    NodeItem* rootNode() const;
    NodeItem* addNode(const QString& text, NodeItem* parent);
    void removeNode(NodeItem* node);
    void autoLayout();

    NodeItem* selectedNode() const;

    QUndoStack* undoStack() const;
    bool isEditing() const;

    LayoutStyle layoutStyle() const;
    void setLayoutStyle(LayoutStyle style);

    // Template system
    QString templateId() const;
    void setTemplateId(const QString& id);
    const TemplateDescriptor* templateDescriptor() const;

    EdgeItem* findEdge(NodeItem* parent, NodeItem* child) const;

    // Serialization
    QJsonObject toJson() const;
    bool fromJson(const QJsonObject& json);
    bool saveToFile(const QString& filePath);
    bool loadFromFile(const QString& filePath);

    // Export/Import
    QString exportToText() const;
    QString exportToMarkdown() const;
    bool exportToPng(const QString& filePath, int scaleFactor = 2);
    bool exportToSvg(const QString& filePath);
    bool exportToPdf(const QString& filePath);
    bool importFromText(const QString& text);

    // Scene management
    void clearScene();
    bool isModified() const;
    void setModified(bool modified);

    // Root node creation (consolidates 4 duplicated patterns)
    NodeItem* createRootNode(const QString& text);

    // Edge registration (public API for Commands)
    void registerEdge(EdgeItem* edge);
    void unregisterEdge(EdgeItem* edge);

signals:
    void modifiedChanged(bool modified);
    void fileLoaded(const QString& filePath);
    void layoutStyleChanged();

public slots:
    void addChildToSelected();
    void addSiblingToSelected();
    void deleteSelected();
    void startEditing(NodeItem* node);

    void cancelEditing();

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    friend class MindMapSerializer;
    friend class MindMapExporter;

    void finishEditing();
    void markModified();

    NodeItem* m_rootNode = nullptr;
    QList<EdgeItem*> m_edges;
    QUndoStack* m_undoStack;
    bool m_modified = false;
    bool m_batchLoading = false;
    LayoutStyle m_layoutStyle = LayoutStyle::Bilateral;
    QString m_templateId;

    // Editing
    InlineEditController* m_editController;
};
