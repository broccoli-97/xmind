#pragma once

#include <QGraphicsItem>
#include <QPainterPath>

class NodeItem;
class MindMapScene;

class EdgeItem : public QGraphicsItem {
public:
    EdgeItem(NodeItem* source, NodeItem* target, QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    void updatePath();

    NodeItem* sourceNode() const;
    NodeItem* targetNode() const;

    bool isLocked() const;
    void setLocked(bool locked);
    QRectF lockIconRect() const;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

private:
    void drawLockIcon(QPainter* painter, const QPointF& center, bool locked) const;

    NodeItem* m_source;
    NodeItem* m_target;
    QPainterPath m_path;
    QRectF m_boundingRect;
    bool m_locked = false;
    QPointF m_pathMidpoint;
    bool m_hovered = false;
};
