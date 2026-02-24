#pragma once

#include <QGraphicsItem>
#include <QPainterPath>

class NodeItem;

class EdgeItem : public QGraphicsItem {
public:
    EdgeItem(NodeItem* source, NodeItem* target, QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    void updatePath();

    NodeItem* sourceNode() const;
    NodeItem* targetNode() const;

private:
    NodeItem* m_source;
    NodeItem* m_target;
    QPainterPath m_path;
    QRectF m_boundingRect;
};
