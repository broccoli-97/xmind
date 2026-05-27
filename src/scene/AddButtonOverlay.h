#pragma once

#include <QGraphicsItem>

class NodeItem;
class QGraphicsSceneHoverEvent;
class QGraphicsSceneMouseEvent;

// Separate child item so the hover-add button never inflates NodeItem's
// boundingRect — that's what kept us from disturbing the scene rect when
// the button extended past the node's edge.
class AddButtonOverlay : public QGraphicsItem {
public:
    explicit AddButtonOverlay(NodeItem* parentNode);

    void setButtonOpacity(qreal opacity);
    qreal buttonOpacity() const { return m_opacity; }
    bool isButtonHovered() const { return m_hovered; }

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

private:
    QRectF bridgeRect() const;

    NodeItem* m_node;
    qreal m_opacity = 0.0;
    bool m_hovered = false;
};
