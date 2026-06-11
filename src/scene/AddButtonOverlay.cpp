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

bool AddButtonOverlay::chevronVisible() const {
    // The fold chevron appears at the junction of any expanded branch. The
    // root is excluded: in bilateral layouts its children grow both ways, so
    // a single junction side would be arbitrary (Space also skips the root).
    return m_node->parentNode() && !m_node->childNodes().isEmpty() && !m_node->isCollapsed();
}

QRectF AddButtonOverlay::boundingRect() const {
    QRectF btn = m_node->addButtonRect();
    constexpr qreal m = NodeItem::kHoverZoneMargin;
    QRectF area = btn.adjusted(-m, -m, m, m).united(bridgeRect());
    if (chevronVisible())
        area = area.united(m_node->collapseControlRect().adjusted(-m, -m, m, m));
    return area;
}

QPainterPath AddButtonOverlay::shape() const {
    QPainterPath path;
    QRectF btn = m_node->addButtonRect();
    constexpr qreal m = NodeItem::kHoverZoneMargin;
    path.addEllipse(btn.adjusted(-m, -m, m, m));
    path.addRect(bridgeRect());
    if (chevronVisible())
        path.addEllipse(m_node->collapseControlRect().adjusted(-m, -m, m, m));
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

    const QColor idleBg = ThemeManager::isDark() ? QColor(255, 255, 255, 60) : QColor(0, 0, 0, 60);
    const QColor idleGlyph =
        ThemeManager::isDark() ? QColor(255, 255, 255, 200) : QColor(0, 0, 0, 180);

    // ----- Add button ("+") --------------------------------------------------
    const bool addHovered = (m_hoverPart == Part::Add);
    painter->setPen(Qt::NoPen);
    painter->setBrush(addHovered ? selectionBorder : idleBg);
    painter->drawEllipse(btnRect);

    QPen plusPen(addHovered ? QColor(Qt::white) : idleGlyph, 2, Qt::SolidLine, Qt::RoundCap);
    painter->setPen(plusPen);
    QPointF center = btnRect.center();
    constexpr qreal arm = NodeItem::kAddButtonRadius * 0.45;
    painter->drawLine(QPointF(center.x() - arm, center.y()), QPointF(center.x() + arm, center.y()));
    painter->drawLine(QPointF(center.x(), center.y() - arm), QPointF(center.x(), center.y() + arm));

    // ----- Collapse chevron --------------------------------------------------
    // A smaller circle at the branch junction; the chevron points back toward
    // the node — "fold the branch into it" — mirroring how a macOS disclosure
    // chevron signals the direction content will tuck away.
    if (chevronVisible()) {
        const QRectF foldRect = m_node->collapseControlRect();
        const bool foldHovered = (m_hoverPart == Part::Collapse);
        painter->setPen(Qt::NoPen);
        painter->setBrush(foldHovered ? selectionBorder : idleBg);
        painter->drawEllipse(foldRect);

        QPen chevPen(foldHovered ? QColor(Qt::white) : idleGlyph, 1.8, Qt::SolidLine, Qt::RoundCap,
                     Qt::RoundJoin);
        painter->setPen(chevPen);
        painter->setBrush(Qt::NoBrush);
        const QPointF c = foldRect.center();
        constexpr qreal a = 3.2;
        QPainterPath chev;
        switch (m_node->m_addButtonDir) {
        case NodeItem::ButtonDirection::Left: // children grow left → fold right
            chev.moveTo(c.x() - a * 0.5, c.y() - a);
            chev.lineTo(c.x() + a * 0.5, c.y());
            chev.lineTo(c.x() - a * 0.5, c.y() + a);
            break;
        case NodeItem::ButtonDirection::Bottom: // children grow down → fold up
            chev.moveTo(c.x() - a, c.y() + a * 0.5);
            chev.lineTo(c.x(), c.y() - a * 0.5);
            chev.lineTo(c.x() + a, c.y() + a * 0.5);
            break;
        case NodeItem::ButtonDirection::Right: // children grow right → fold left
        default:
            chev.moveTo(c.x() + a * 0.5, c.y() - a);
            chev.lineTo(c.x() - a * 0.5, c.y());
            chev.lineTo(c.x() + a * 0.5, c.y() + a);
            break;
        }
        painter->drawPath(chev);
    }

    painter->restore();
}

AddButtonOverlay::Part AddButtonOverlay::partAt(const QPointF& pos) const {
    if (chevronVisible() && m_node->collapseControlRect().adjusted(-2, -2, 2, 2).contains(pos))
        return Part::Collapse;
    if (m_node->addButtonRect().adjusted(-4, -4, 4, 4).contains(pos))
        return Part::Add;
    return Part::None;
}

void AddButtonOverlay::setHoverPart(Part part) {
    if (m_hoverPart == part)
        return;
    m_hoverPart = part;
    if (part != Part::None)
        setCursor(Qt::PointingHandCursor);
    else
        unsetCursor();
    update();
}

void AddButtonOverlay::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    auto* mindMapScene = m_node->mindMapScene();
    if (mindMapScene && mindMapScene->isEditing())
        return;

    setHoverPart(partAt(event->pos()));
    // Cancel the parent node's pending leave timer
    if (m_node->m_hoverLeaveTimer) {
        m_node->m_hoverLeaveTimer->stop();
        delete m_node->m_hoverLeaveTimer;
        m_node->m_hoverLeaveTimer = nullptr;
    }
}

void AddButtonOverlay::hoverMoveEvent(QGraphicsSceneHoverEvent* event) {
    setHoverPart(partAt(event->pos()));
}

void AddButtonOverlay::hoverLeaveEvent(QGraphicsSceneHoverEvent*) {
    setHoverPart(Part::None);
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
        switch (partAt(event->pos())) {
        case Part::Collapse:
            event->accept();
            if (mindMapScene) {
                // Queued like the add path: the toggle hides this overlay
                // (NodeItem::setCollapsed → cancelAddButton), so don't tear
                // ourselves down while still handling our own press.
                QMetaObject::invokeMethod(
                    mindMapScene,
                    [mindMapScene, node = m_node]() { mindMapScene->toggleNodeCollapsed(node); },
                    Qt::QueuedConnection);
            }
            return;
        case Part::Add:
            event->accept();
            if (mindMapScene) {
                mindMapScene->clearSelection();
                m_node->setSelected(true);
                QMetaObject::invokeMethod(
                    mindMapScene, [mindMapScene]() { mindMapScene->addChildToSelected(); },
                    Qt::QueuedConnection);
            }
            return;
        case Part::None:
            // The bridge strip between the controls is hover keep-alive only;
            // let the press fall through to whatever is underneath.
            break;
        }
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
