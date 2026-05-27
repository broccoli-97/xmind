#include "scene/AddButtonOverlay.h"
#include "core/ThemeDescriptor.h"
#include "scene/MindMapScene.h"
#include "scene/NodeItem.h"
#include "ui/ThemeManager.h"

#include <QCursor>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QMetaObject>
#include <QPainter>
#include <QTimer>

AddButtonOverlay::AddButtonOverlay(NodeItem* parentNode)
    : QGraphicsItem(parentNode), m_node(parentNode) {
    setAcceptHoverEvents(true);
    setVisible(false);
}

void AddButtonOverlay::setButtonOpacity(qreal opacity) {
    m_opacity = opacity;
    setVisible(opacity > 0.0);
    update();
}

QRectF AddButtonOverlay::boundingRect() const {
    QRectF btn = m_node->addButtonRect();
    constexpr qreal m = NodeItem::kHoverZoneMargin;
    QRectF area = btn.adjusted(-m, -m, m, m);
    return area.united(bridgeRect());
}

QPainterPath AddButtonOverlay::shape() const {
    QPainterPath path;
    QRectF btn = m_node->addButtonRect();
    constexpr qreal m = NodeItem::kHoverZoneMargin;
    path.addEllipse(btn.adjusted(-m, -m, m, m));
    path.addRect(bridgeRect());
    return path;
}

void AddButtonOverlay::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    if (m_opacity < 0.01)
        return;

    auto* mindMapScene = m_node->mindMapScene();
    if (mindMapScene && mindMapScene->isEditing())
        return;

    painter->setRenderHint(QPainter::Antialiasing);
    painter->save();
    painter->setOpacity(m_opacity);

    QRectF btnRect = m_node->addButtonRect();

    // Resolve selection border color from the active theme
    const ThemeColors& globalTC = ThemeManager::colors();
    QColor selectionBorder = globalTC.nodeSelectionBorder;
    if (mindMapScene) {
        const auto* th = mindMapScene->themeDescriptor();
        if (th)
            selectionBorder = th->activeColors().nodeSelectionBorder;
    }

    QColor btnBg;
    if (m_hovered) {
        btnBg = selectionBorder;
    } else {
        btnBg = ThemeManager::isDark() ? QColor(255, 255, 255, 60) : QColor(0, 0, 0, 60);
    }
    painter->setPen(Qt::NoPen);
    painter->setBrush(btnBg);
    painter->drawEllipse(btnRect);

    QColor plusColor = m_hovered           ? Qt::white
                       : ThemeManager::isDark() ? QColor(255, 255, 255, 200)
                                                : QColor(0, 0, 0, 180);
    QPen plusPen(plusColor, 2, Qt::SolidLine, Qt::RoundCap);
    painter->setPen(plusPen);
    QPointF center = btnRect.center();
    constexpr qreal arm = NodeItem::kAddButtonRadius * 0.45;
    painter->drawLine(QPointF(center.x() - arm, center.y()),
                      QPointF(center.x() + arm, center.y()));
    painter->drawLine(QPointF(center.x(), center.y() - arm),
                      QPointF(center.x(), center.y() + arm));

    painter->restore();
}

void AddButtonOverlay::hoverEnterEvent(QGraphicsSceneHoverEvent*) {
    auto* mindMapScene = m_node->mindMapScene();
    if (mindMapScene && mindMapScene->isEditing())
        return;

    m_hovered = true;
    setCursor(Qt::PointingHandCursor);
    update();
    // Cancel the parent node's pending leave timer
    if (m_node->m_hoverLeaveTimer) {
        m_node->m_hoverLeaveTimer->stop();
        delete m_node->m_hoverLeaveTimer;
        m_node->m_hoverLeaveTimer = nullptr;
    }
}

void AddButtonOverlay::hoverLeaveEvent(QGraphicsSceneHoverEvent*) {
    m_hovered = false;
    unsetCursor();
    update();
    // Trigger fade-out on the parent node
    m_node->m_hovered = false;
    m_node->startAddButtonAnimation(false);
}

void AddButtonOverlay::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton && m_opacity > 0.5) {
        auto* mindMapScene = m_node->mindMapScene();
        if (mindMapScene && mindMapScene->isEditing()) {
            event->ignore();
            return;
        }
        event->accept();
        if (mindMapScene) {
            mindMapScene->clearSelection();
            m_node->setSelected(true);
            QMetaObject::invokeMethod(
                mindMapScene, [mindMapScene]() { mindMapScene->addChildToSelected(); },
                Qt::QueuedConnection);
        }
        return;
    }
    QGraphicsItem::mousePressEvent(event);
}

void AddButtonOverlay::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) {
    event->accept(); // Eat double-clicks so they don't trigger text editing
}

QRectF AddButtonOverlay::bridgeRect() const {
    QRectF btn = m_node->addButtonRect();
    QRectF nodeRect = m_node->m_rect;
    constexpr qreal m = NodeItem::kHoverZoneMargin;

    switch (m_node->m_addButtonDir) {
    case NodeItem::ButtonDirection::Right:
        return QRectF(nodeRect.right() - 1, btn.top() - m, btn.left() - nodeRect.right() + 2,
                      btn.height() + m * 2);
    case NodeItem::ButtonDirection::Left:
        return QRectF(btn.right() - 1, btn.top() - m, nodeRect.left() - btn.right() + 2,
                      btn.height() + m * 2);
    case NodeItem::ButtonDirection::Bottom:
        return QRectF(btn.left() - m, nodeRect.bottom() - 1, btn.width() + m * 2,
                      btn.top() - nodeRect.bottom() + 2);
    }
    return {};
}
