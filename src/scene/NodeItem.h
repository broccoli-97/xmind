#pragma once

#include <QColor>
#include <QFont>
#include <QGraphicsObject>
#include <QList>

class AddButtonOverlay;
class EdgeItem;
class MindMapScene;
class QTimer;
class QVariantAnimation;

class NodeItem : public QGraphicsObject {
    Q_OBJECT

public:
    explicit NodeItem(const QString& text, QGraphicsItem* parent = nullptr);
    ~NodeItem() override;

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    QString text() const;
    void setText(const QString& text);

    NodeItem* parentNode() const;
    void setParentNode(NodeItem* parent);

    QList<NodeItem*> childNodes() const;
    void addChild(NodeItem* child);
    void insertChild(int index, NodeItem* child);
    void removeChild(NodeItem* child);

    int level() const;
    QColor nodeColor() const;
    QFont font() const;

    void addEdge(EdgeItem* edge);
    void removeEdge(EdgeItem* edge);

    QRectF nodeRect() const;
    void moveSubtree(const QPointF& delta);

    void showAddButton();
    void hideAddButton();

signals:
    void doubleClicked(NodeItem* node);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

private:
    friend class AddButtonOverlay;

    enum class ButtonDirection { Right, Left, Bottom };

    void updateGeometry();
    ButtonDirection addButtonDirection() const;
    QRectF addButtonRect() const;
    void startAddButtonAnimation(bool fadeIn);
    MindMapScene* mindMapScene() const;

    QString m_text;
    QFont m_font;
    QRectF m_rect;
    NodeItem* m_parentNode = nullptr;
    QList<NodeItem*> m_children;
    QList<EdgeItem*> m_edges;
    QPointF m_dragStartPos;
    QPointF m_dragOrigPos;
    bool m_dragging = false;
    bool m_hovered = false;
    MindMapScene* m_mindMapScene = nullptr;
    qreal m_savedZValue = 0.0;
    ButtonDirection m_addButtonDir = ButtonDirection::Right;
    QVariantAnimation* m_addButtonAnimation = nullptr;
    QTimer* m_hoverLeaveTimer = nullptr;
    AddButtonOverlay* m_addButtonOverlay = nullptr;

    static constexpr qreal kMinWidth = 120.0;
    static constexpr qreal kMaxWidth = 300.0;
    static constexpr qreal kPadding = 16.0;
    static constexpr qreal kRadius = 10.0;
    static constexpr qreal kAddButtonRadius = 12.0;
    static constexpr qreal kAddButtonOffset = 6.0;
    static constexpr qreal kHoverZoneMargin = 10.0;
};
