#include "scene/EdgeItem.h"
#include "core/TemplateDescriptor.h"
#include "core/ThemeDescriptor.h"
#include "scene/MindMapScene.h"
#include "scene/NodeItem.h"
#include "ui/ThemeManager.h"

#include <QGraphicsSceneHoverEvent>
#include <QPainter>
#include <QPainterPathStroker>
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

namespace {

Qt::PenStyle penStyleFor(const QString& dash) {
    if (dash == QLatin1String("dashed"))
        return Qt::DashLine;
    if (dash == QLatin1String("dotted"))
        return Qt::DotLine;
    return Qt::SolidLine;
}

Qt::PenCapStyle capStyleFor(const QString& cap) {
    if (cap == QLatin1String("flat"))
        return Qt::FlatCap;
    if (cap == QLatin1String("square"))
        return Qt::SquareCap;
    return Qt::RoundCap;
}

QColor modifiedEdgeColor(const QColor& base, int lighten, const QString& modifier) {
    if (modifier == QLatin1String("same"))
        return base;
    if (modifier == QLatin1String("darken")) {
        // Inverse of "lighten": if lighten is 140 (40% brighter), darken by ~40%.
        int factor = qMax(101, lighten);  // factor=140 means 1.4× brightness
        return base.darker(factor);
    }
    // "lighten" (default)
    return base.lighter(lighten);
}

} // namespace

void EdgeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/,
                     QWidget* /*widget*/) {
    painter->setRenderHint(QPainter::Antialiasing);

    int lighten = ThemeManager::colors().edgeLightenFactor;
    qreal edgeWidth = 2.5;
    QString colorSource = QStringLiteral("target");
    QString dashStyle = QStringLiteral("solid");
    QString colorModifier = QStringLiteral("lighten");
    QString lineCap = QStringLiteral("round");

    if (m_mindMapScene) {
        const auto* th = m_mindMapScene->themeDescriptor();
        if (th) {
            lighten = th->activeColors().edgeLightenFactor;
            edgeWidth = th->edgeStyle.width;
            colorSource = th->edgeStyle.colorSource;
            dashStyle = th->edgeStyle.dashStyle;
            colorModifier = th->edgeStyle.colorModifier;
            lineCap = th->edgeStyle.lineCap;
        }
        // Template override (e.g. Lined forces branch-colored edges).
        if (const auto* td = m_mindMapScene->templateDescriptor()) {
            if (!td->edgeColorSourceOverride.isEmpty())
                colorSource = td->edgeColorSourceOverride;
        }
    }

    QColor base = (colorSource == QLatin1String("branch")) ? m_target->branchColor()
                                                           : m_target->nodeColor();
    QColor color = modifiedEdgeColor(base, lighten, colorModifier);
    QPen pen(color, edgeWidth, penStyleFor(dashStyle), capStyleFor(lineCap));
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(m_path);
}

void EdgeItem::updatePath() {
    prepareGeometryChange();

    QPointF srcPos = m_source->pos();
    QPointF tgtPos = m_target->pos();
    QRectF srcRect = m_source->nodeRect();
    QRectF tgtRect = m_target->nodeRect();

    // Resolve anchor and curvature: theme provides the base; template may
    // override the anchor (Lined forces baseline).
    QString anchor = QStringLiteral("center");
    qreal curvature = 0.5;
    if (m_mindMapScene) {
        if (const auto* th = m_mindMapScene->themeDescriptor()) {
            anchor = th->edgeStyle.anchor;
            curvature = qBound<qreal>(0.0, th->edgeStyle.curvature, 0.95);
        }
        if (const auto* td = m_mindMapScene->templateDescriptor()) {
            if (!td->edgeAnchorOverride.isEmpty())
                anchor = td->edgeAnchorOverride;
        }
    }
    const bool baseline = (anchor == QLatin1String("baseline"));

    QPointF start, end;
    qreal dx = tgtPos.x() - srcPos.x();

    // Vertical offset for "baseline" anchor: line meets the bottom of the
    // node's text rect rather than the vertical centre. Yields the
    // "text floats above the line" look.
    auto vOffset = [baseline](const QRectF& r) {
        return baseline ? r.bottom() : 0.0;
    };

    if (qAbs(dx) > 10) {
        if (dx > 0) {
            start = QPointF(srcPos.x() + srcRect.right(), srcPos.y() + vOffset(srcRect));
            end = QPointF(tgtPos.x() + tgtRect.left(), tgtPos.y() + vOffset(tgtRect));
        } else {
            start = QPointF(srcPos.x() + srcRect.left(), srcPos.y() + vOffset(srcRect));
            end = QPointF(tgtPos.x() + tgtRect.right(), tgtPos.y() + vOffset(tgtRect));
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

    qreal cdx = (end.x() - start.x()) * curvature;
    qreal cdy = (end.y() - start.y()) * curvature;

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

    m_startPoint = start;
    m_boundingRect = m_path.boundingRect().adjusted(-5, -5, 5, 5);
}

NodeItem* EdgeItem::sourceNode() const {
    return m_source;
}
NodeItem* EdgeItem::targetNode() const {
    return m_target;
}

QPainterPath EdgeItem::shape() const {
    QPainterPathStroker stroker;
    stroker.setWidth(kHitWidth);
    return stroker.createStroke(m_path);
}

void EdgeItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event) {
    qreal dist = QLineF(event->pos(), m_startPoint).length();
    if (dist < kEdgeHoverProximity) {
        if (!m_sourceHoverActive) {
            m_sourceHoverActive = true;
            m_source->showAddButton();
        }
    } else {
        if (m_sourceHoverActive) {
            m_sourceHoverActive = false;
            m_source->hideAddButton();
        }
    }
    QGraphicsItem::hoverMoveEvent(event);
}

void EdgeItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    if (m_sourceHoverActive) {
        m_sourceHoverActive = false;
        m_source->hideAddButton();
    }
    QGraphicsItem::hoverLeaveEvent(event);
}

QVariant EdgeItem::itemChange(GraphicsItemChange change, const QVariant& value) {
    if (change == ItemSceneHasChanged) {
        m_mindMapScene = dynamic_cast<MindMapScene*>(scene());
    }
    return QGraphicsItem::itemChange(change, value);
}
