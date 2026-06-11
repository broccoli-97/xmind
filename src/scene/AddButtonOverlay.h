#pragma once

#include <QGraphicsItem>

class NodeItem;
class QGraphicsSceneHoverEvent;
class QGraphicsSceneMouseEvent;

// Separate child item so the hover-add button never inflates NodeItem's
// boundingRect — that's what kept us from disturbing the scene rect when
// the button extended past the node's edge.
//
// For nodes with children it paints two controls along the child axis,
// macOS-style revealed on hover: a collapse chevron at the branch junction
// (fold the subtree) and the "+" add button beyond it. Leaf nodes get only
// the "+"; folded nodes get neither (NodeItem's count badge owns the
// junction — see NodeItem::collapseBadgeVisible).
class AddButtonOverlay : public QGraphicsItem {
public:
    explicit AddButtonOverlay(NodeItem* parentNode);

    void setButtonOpacity(qreal opacity);
    qreal buttonOpacity() const { return m_opacity; }
    bool isButtonHovered() const { return m_hoverPart != Part::None; }

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

private:
    enum class Part { None, Add, Collapse };

    QRectF bridgeRect() const;
    bool chevronVisible() const;
    Part partAt(const QPointF& pos) const;
    void setHoverPart(Part part);

    NodeItem* m_node;
    qreal m_opacity = 0.0;
    Part m_hoverPart = Part::None;
};
