#include "EdgeItem.h"
#include "AppSettings.h"
#include "Commands.h"
#include "MindMapScene.h"
#include "NodeItem.h"

#include <QCursor>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QUndoStack>
#include <QtMath>

EdgeItem::EdgeItem(NodeItem* source, NodeItem* target, QGraphicsItem* parent)
    : QGraphicsItem(parent), m_source(source), m_target(target) {
    setZValue(-1);
    setFlag(ItemIsSelectable, false);
    setFlag(ItemIsMovable, false);
    setAcceptHoverEvents(true);
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

    // Draw lock icon when locked or hovered
    if (m_locked || m_hovered) {
        drawLockIcon(painter, m_pathMidpoint, m_locked);
    }
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

    // Cache midpoint for lock icon
    m_pathMidpoint = m_path.pointAtPercent(0.5);

    // Expand bounding rect to include lock icon area
    m_boundingRect = m_path.boundingRect().adjusted(-5, -5, 5, 5);
    QRectF iconArea = lockIconRect();
    m_boundingRect = m_boundingRect.united(iconArea);
}

NodeItem* EdgeItem::sourceNode() const {
    return m_source;
}
NodeItem* EdgeItem::targetNode() const {
    return m_target;
}

bool EdgeItem::isLocked() const {
    return m_locked;
}

void EdgeItem::setLocked(bool locked) {
    if (m_locked != locked) {
        m_locked = locked;
        update();
    }
}

QRectF EdgeItem::lockIconRect() const {
    return QRectF(m_pathMidpoint.x() - 10, m_pathMidpoint.y() - 10, 20, 20);
}

void EdgeItem::drawLockIcon(QPainter* painter, const QPointF& center, bool locked) const {
    painter->save();

    // White circle background
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(255, 255, 255, 220));
    painter->drawEllipse(center, 10, 10);

    // Lock body
    QColor bodyColor = locked ? QColor(255, 152, 0) : QColor(158, 158, 158);
    QRectF body(center.x() - 5, center.y() - 1, 10, 8);
    painter->setBrush(bodyColor);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(body, 1.5, 1.5);

    // Shackle (U-shape above the body)
    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(bodyColor, 1.8, Qt::SolidLine, Qt::RoundCap));
    if (locked) {
        // Closed shackle
        QPainterPath shackle;
        shackle.moveTo(center.x() - 3, center.y() - 1);
        shackle.lineTo(center.x() - 3, center.y() - 5);
        shackle.cubicTo(center.x() - 3, center.y() - 8, center.x() + 3, center.y() - 8,
                        center.x() + 3, center.y() - 5);
        shackle.lineTo(center.x() + 3, center.y() - 1);
        painter->drawPath(shackle);
    } else {
        // Open shackle (right side lifted)
        QPainterPath shackle;
        shackle.moveTo(center.x() - 3, center.y() - 1);
        shackle.lineTo(center.x() - 3, center.y() - 5);
        shackle.cubicTo(center.x() - 3, center.y() - 8, center.x() + 3, center.y() - 8,
                        center.x() + 3, center.y() - 6);
        painter->drawPath(shackle);
    }

    // Keyhole dot
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::white);
    painter->drawEllipse(QPointF(center.x(), center.y() + 2), 1.2, 1.2);

    painter->restore();
}

void EdgeItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton && lockIconRect().contains(event->pos())) {
        auto* mindMapScene = dynamic_cast<MindMapScene*>(scene());
        if (mindMapScene) {
            mindMapScene->undoStack()->push(new ToggleEdgeLockCommand(this));
        }
        event->accept();
        return;
    }
    QGraphicsItem::mousePressEvent(event);
}

void EdgeItem::hoverEnterEvent(QGraphicsSceneHoverEvent* /*event*/) {
    m_hovered = true;
    update();
}

void EdgeItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event) {
    if (lockIconRect().contains(event->pos())) {
        setCursor(Qt::PointingHandCursor);
    } else {
        unsetCursor();
    }
}

void EdgeItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* /*event*/) {
    m_hovered = false;
    unsetCursor();
    update();
}
