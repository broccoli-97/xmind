#pragma once

#include <QGraphicsScene>
#include <QMap>

class NodeItem;
class EdgeItem;
class QLineEdit;
class QGraphicsProxyWidget;
class QJsonObject;
class QJsonArray;
class QUndoStack;
class AddNodeCommand;
class RemoveNodeCommand;

enum class LayoutStyle { Bilateral = 0, TopDown = 1, RightTree = 2 };

class MindMapScene : public QGraphicsScene {
    Q_OBJECT

    friend class AddNodeCommand;
    friend class RemoveNodeCommand;

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

signals:
    void modifiedChanged(bool modified);
    void fileLoaded(const QString& filePath);

public slots:
    void addChildToSelected();
    void addSiblingToSelected();
    void deleteSelected();
    void startEditing(NodeItem* node);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    void finishEditing();
    void cancelEditing();
    void markModified();

    QJsonObject nodeToJson(NodeItem* node) const;
    NodeItem* nodeFromJson(const QJsonObject& json, NodeItem* parent);
    void exportNodeToText(NodeItem* node, int indent, QString& output) const;
    void exportNodeToMarkdown(NodeItem* node, int level, QString& output) const;

    qreal subtreeHeight(NodeItem* node) const;
    void calculatePositions(NodeItem* node, qreal x, qreal y, int direction,
                            QMap<NodeItem*, QPointF>& positions);

    // Layout strategies
    void layoutBilateral(QMap<NodeItem*, QPointF>& positions);
    void layoutTopDown(QMap<NodeItem*, QPointF>& positions);
    void layoutRightTree(QMap<NodeItem*, QPointF>& positions);

    // Top-down helpers
    qreal subtreeWidth(NodeItem* node) const;
    void calculatePositionsTopDown(NodeItem* node, qreal x, qreal y,
                                   QMap<NodeItem*, QPointF>& positions);

    NodeItem* m_rootNode = nullptr;
    QList<EdgeItem*> m_edges;
    QUndoStack* m_undoStack;
    bool m_modified = false;
    LayoutStyle m_layoutStyle = LayoutStyle::Bilateral;

    // Editing state
    NodeItem* m_editingNode = nullptr;
    QGraphicsProxyWidget* m_editProxy = nullptr;
    QLineEdit* m_editLineEdit = nullptr;

    static constexpr qreal kHSpacing = 220.0;
    static constexpr qreal kVSpacing = 16.0;
    static constexpr qreal kNodeHeight = 44.0;
    static constexpr qreal kTopDownLevelSpacing = 100.0;
};
