#pragma once

#include <QColor>
#include <QFont>
#include <QGraphicsObject>
#include <QList>

class EdgeItem;

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
    void removeChild(NodeItem* child);

    int level() const;
    QColor nodeColor() const;

    void addEdge(EdgeItem* edge);
    void removeEdge(EdgeItem* edge);

    QRectF nodeRect() const;
    void moveSubtree(const QPointF& delta);

signals:
    void doubleClicked(NodeItem* node);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    void updateGeometry();

    QString m_text;
    QFont m_font;
    QRectF m_rect;
    NodeItem* m_parentNode = nullptr;
    QList<NodeItem*> m_children;
    QList<EdgeItem*> m_edges;
    QPointF m_dragStartPos;
    bool m_dragging = false;

    static constexpr qreal kMinWidth = 120.0;
    static constexpr qreal kPadding = 16.0;
    static constexpr qreal kRadius = 10.0;
};
