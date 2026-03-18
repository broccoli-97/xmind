#pragma once

#include <QGraphicsItem>
#include <QPainterPath>

class NodeItem;
class MindMapScene;

class EdgeItem : public QGraphicsItem {
public:
    EdgeItem(NodeItem* source, NodeItem* target, QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    void updatePath();

    NodeItem* sourceNode() const;
    NodeItem* targetNode() const;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

private:
    NodeItem* m_source;
    NodeItem* m_target;
    MindMapScene* m_mindMapScene = nullptr;
    QPainterPath m_path;
    QRectF m_boundingRect;
    QPointF m_startPoint;
    bool m_sourceHoverActive = false;

    static constexpr qreal kHitWidth = 20.0;
    static constexpr qreal kEdgeHoverProximity = 30.0;
};
