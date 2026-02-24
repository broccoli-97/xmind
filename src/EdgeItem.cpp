#include "EdgeItem.h"
#include "AppSettings.h"
#include "NodeItem.h"

#include <QPainter>
#include <QtMath>

EdgeItem::EdgeItem(NodeItem* source, NodeItem* target, QGraphicsItem* parent)
    : QGraphicsItem(parent), m_source(source), m_target(target) {
    setZValue(-1);
    setFlag(ItemIsSelectable, false);
    setFlag(ItemIsMovable, false);
    updatePath();
}

QRectF EdgeItem::boundingRect() const {
    return m_boundingRect;
}

void EdgeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/,
                     QWidget* /*widget*/) {
    painter->setRenderHint(QPainter::Antialiasing);
    int lighten = (AppSettings::instance().theme() == AppTheme::Dark) ? 120 : 140;
    QColor color = m_target->nodeColor().lighter(lighten);
    painter->setPen(QPen(color, 2.5, Qt::SolidLine, Qt::RoundCap));
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(m_path);
}

void EdgeItem::updatePath() {
    prepareGeometryChange();

    QPointF srcPos = m_source->pos();
    QPointF tgtPos = m_target->pos();
    QRectF srcRect = m_source->nodeRect();
    QRectF tgtRect = m_target->nodeRect();

    QPointF start, end;
    qreal dx = tgtPos.x() - srcPos.x();

    if (qAbs(dx) > 10) {
        if (dx > 0) {
            start = QPointF(srcPos.x() + srcRect.right(), srcPos.y());
            end = QPointF(tgtPos.x() + tgtRect.left(), tgtPos.y());
        } else {
            start = QPointF(srcPos.x() + srcRect.left(), srcPos.y());
            end = QPointF(tgtPos.x() + tgtRect.right(), tgtPos.y());
        }
    } else {
        qreal dy = tgtPos.y() - srcPos.y();
        if (dy > 0) {
            start = QPointF(srcPos.x(), srcPos.y() + srcRect.bottom());
            end = QPointF(tgtPos.x(), tgtPos.y() + tgtRect.top());
        } else {
            start = QPointF(srcPos.x(), srcPos.y() + srcRect.top());
            end = QPointF(tgtPos.x(), tgtPos.y() + tgtRect.bottom());
        }
    }

    qreal cdx = (end.x() - start.x()) * 0.5;
    qreal cdy = (end.y() - start.y()) * 0.5;

    QPointF cp1, cp2;
    if (qAbs(dx) > 10) {
        cp1 = QPointF(start.x() + cdx, start.y());
        cp2 = QPointF(end.x() - cdx, end.y());
    } else {
        cp1 = QPointF(start.x(), start.y() + cdy);
        cp2 = QPointF(end.x(), end.y() - cdy);
    }

    m_path = QPainterPath();
    m_path.moveTo(start);
    m_path.cubicTo(cp1, cp2, end);

    m_boundingRect = m_path.boundingRect().adjusted(-5, -5, 5, 5);
}

NodeItem* EdgeItem::sourceNode() const {
    return m_source;
}
NodeItem* EdgeItem::targetNode() const {
    return m_target;
}
