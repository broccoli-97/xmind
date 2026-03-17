#include "scene/NodeItem.h"
#include "core/AppSettings.h"
#include "core/Commands.h"
#include "core/TemplateDescriptor.h"
#include "layout/LayoutStyle.h"
#include "scene/EdgeItem.h"
#include "scene/MindMapScene.h"
#include "ui/ThemeManager.h"

#include <QFontMetricsF>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QMetaObject>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QTimer>
#include <QVariantAnimation>

// ===========================================================================
// AddButtonOverlay — separate child item so it never inflates NodeItem's
//                    boundingRect and therefore cannot disturb the scene rect.
// ===========================================================================

class AddButtonOverlay : public QGraphicsItem {
public:
    explicit AddButtonOverlay(NodeItem* parentNode)
        : QGraphicsItem(parentNode), m_node(parentNode) {
        setAcceptHoverEvents(true);
        setVisible(false);
    }

    void setButtonOpacity(qreal opacity) {
        m_opacity = opacity;
        setVisible(opacity > 0.0);
        update();
    }

    qreal buttonOpacity() const { return m_opacity; }
    bool isButtonHovered() const { return m_hovered; }

    QRectF boundingRect() const override {
        QRectF btn = m_node->addButtonRect();
        constexpr qreal m = NodeItem::kHoverZoneMargin;
        QRectF area = btn.adjusted(-m, -m, m, m);
        return area.united(bridgeRect());
    }

    QPainterPath shape() const override {
        QPainterPath path;
        QRectF btn = m_node->addButtonRect();
        constexpr qreal m = NodeItem::kHoverZoneMargin;
        path.addEllipse(btn.adjusted(-m, -m, m, m));
        path.addRect(bridgeRect());
        return path;
    }

    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override {
        if (m_opacity < 0.01)
            return;

        auto* mindMapScene = dynamic_cast<MindMapScene*>(m_node->scene());
        if (mindMapScene && mindMapScene->isEditing())
            return;

        painter->setRenderHint(QPainter::Antialiasing);
        painter->save();
        painter->setOpacity(m_opacity);

        QRectF btnRect = m_node->addButtonRect();

        // Resolve selection border color
        const ThemeColors& globalTC = ThemeManager::colors();
        QColor selectionBorder = globalTC.nodeSelectionBorder;
        if (mindMapScene) {
            const auto* td = mindMapScene->templateDescriptor();
            if (td)
                selectionBorder = td->activeColors().nodeSelectionBorder;
        }

        // Button background
        QColor btnBg;
        if (m_hovered) {
            btnBg = selectionBorder;
        } else {
            btnBg = ThemeManager::isDark() ? QColor(255, 255, 255, 60) : QColor(0, 0, 0, 60);
        }
        painter->setPen(Qt::NoPen);
        painter->setBrush(btnBg);
        painter->drawEllipse(btnRect);

        // "+" icon
        QColor plusColor = m_hovered        ? Qt::white
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

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent*) override {
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

    void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override {
        m_hovered = false;
        unsetCursor();
        update();
        // Trigger fade-out on the parent node
        m_node->m_hovered = false;
        m_node->startAddButtonAnimation(false);
    }

    void mousePressEvent(QGraphicsSceneMouseEvent* event) override {
        if (event->button() == Qt::LeftButton && m_opacity > 0.5) {
            event->accept();
            auto* mindMapScene = dynamic_cast<MindMapScene*>(m_node->scene());
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

    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override {
        event->accept(); // Eat double-clicks so they don't trigger text editing
    }

private:
    QRectF bridgeRect() const {
        QRectF btn = m_node->addButtonRect();
        QRectF nodeRect = m_node->m_rect;
        constexpr qreal m = NodeItem::kHoverZoneMargin;

        switch (m_node->m_addButtonDir) {
        case NodeItem::ButtonDirection::Right:
            return QRectF(nodeRect.right() - 1, btn.top() - m,
                          btn.left() - nodeRect.right() + 2, btn.height() + m * 2);
        case NodeItem::ButtonDirection::Left:
            return QRectF(btn.right() - 1, btn.top() - m, nodeRect.left() - btn.right() + 2,
                          btn.height() + m * 2);
        case NodeItem::ButtonDirection::Bottom:
            return QRectF(btn.left() - m, nodeRect.bottom() - 1, btn.width() + m * 2,
                          btn.top() - nodeRect.bottom() + 2);
        }
        return {};
    }

    NodeItem* m_node;
    qreal m_opacity = 0.0;
    bool m_hovered = false;
};

// ===========================================================================
// NodeItem
// ===========================================================================

NodeItem::NodeItem(const QString& text, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_text(text) {
    setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);
    setCacheMode(DeviceCoordinateCache);
    m_font.setPointSize(AppSettings::instance().defaultFontSize());
    m_font.setFamily(AppSettings::instance().defaultFontFamily());
    updateGeometry();
}

NodeItem::~NodeItem() = default;

QRectF NodeItem::boundingRect() const {
    constexpr qreal kShadowSpread = 10.0;
    constexpr qreal kShadowOffsetY = 4.0;
    constexpr qreal kMargin = 2.0;
    return m_rect.adjusted(-kShadowSpread - kMargin, -kShadowSpread - kMargin,
                           kShadowSpread + kMargin, kShadowSpread + kShadowOffsetY + kMargin);
}

QPainterPath NodeItem::shape() const {
    QPainterPath path;
    path.addRoundedRect(m_rect, kRadius, kRadius);
    return path;
}

void NodeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
                     QWidget* /*widget*/) {
    painter->setRenderHint(QPainter::Antialiasing);

    // Resolve colors: template-specific if available, else global
    const ThemeColors& globalTC = ThemeManager::colors();
    QColor shadowColor = globalTC.nodeShadow;
    QColor selectionBorder = globalTC.nodeSelectionBorder;
    QColor textColor = globalTC.nodeText;

    auto* mindMapScene = dynamic_cast<MindMapScene*>(scene());
    if (mindMapScene) {
        const auto* td = mindMapScene->templateDescriptor();
        if (td) {
            const auto& tc = td->activeColors();
            shadowColor = tc.nodeShadow;
            selectionBorder = tc.nodeSelectionBorder;
            textColor = tc.nodeText;
        }
    }

    QColor bg = nodeColor();

    // Soft multi-layer shadow for floating effect
    painter->setPen(Qt::NoPen);
    constexpr int kShadowLayers = 5;
    constexpr qreal kShadowSpread = 10.0;
    constexpr qreal kShadowOffsetY = 4.0;
    int layerAlpha = qBound(6, shadowColor.alpha() / 3, 20);
    for (int i = kShadowLayers; i >= 1; --i) {
        qreal expand = kShadowSpread * i / kShadowLayers;
        QColor sc = shadowColor;
        sc.setAlpha(layerAlpha);
        painter->setBrush(sc);
        QRectF sr = m_rect.adjusted(-expand, -expand, expand, expand)
                        .translated(0, kShadowOffsetY);
        painter->drawRoundedRect(sr, kRadius + expand, kRadius + expand);
    }

    // Body (borderless, selection highlight only)
    if (option->state & QStyle::State_Selected) {
        painter->setPen(QPen(selectionBorder, 3));
    } else {
        painter->setPen(Qt::NoPen);
    }
    painter->setBrush(bg);
    painter->drawRoundedRect(m_rect, kRadius, kRadius);

    // Text (word-wrapped within the padded area)
    painter->setPen(textColor);
    painter->setFont(m_font);
    QRectF textArea = m_rect.adjusted(kPadding, kPadding, -kPadding, -kPadding);
    painter->drawText(textArea, Qt::AlignCenter | Qt::TextWrapAnywhere, m_text);
}

QString NodeItem::text() const {
    return m_text;
}

void NodeItem::setText(const QString& text) {
    m_text = text;
    updateGeometry();
    update();
}

NodeItem* NodeItem::parentNode() const {
    return m_parentNode;
}

void NodeItem::setParentNode(NodeItem* parent) {
    m_parentNode = parent;
}

QList<NodeItem*> NodeItem::childNodes() const {
    return m_children;
}

void NodeItem::addChild(NodeItem* child) {
    m_children.append(child);
    child->setParentNode(this);
}

void NodeItem::insertChild(int index, NodeItem* child) {
    if (index < 0 || index > m_children.size())
        index = m_children.size();
    m_children.insert(index, child);
    child->setParentNode(this);
}

void NodeItem::removeChild(NodeItem* child) {
    m_children.removeOne(child);
    child->setParentNode(nullptr);
}

int NodeItem::level() const {
    int lvl = 0;
    const NodeItem* p = m_parentNode;
    while (p) {
        lvl++;
        p = p->parentNode();
    }
    return lvl;
}

QColor NodeItem::nodeColor() const {
    auto* mindMapScene = dynamic_cast<MindMapScene*>(scene());
    if (mindMapScene) {
        const auto* td = mindMapScene->templateDescriptor();
        if (td)
            return td->activeColors().nodePalette[level() % 6];
    }
    return ThemeManager::colors().nodePalette[level() % 6];
}

QFont NodeItem::font() const {
    return m_font;
}

void NodeItem::addEdge(EdgeItem* edge) {
    m_edges.append(edge);
}

void NodeItem::removeEdge(EdgeItem* edge) {
    m_edges.removeOne(edge);
}

QRectF NodeItem::nodeRect() const {
    return m_rect;
}

void NodeItem::moveSubtree(const QPointF& delta) {
    moveBy(delta.x(), delta.y());
    for (auto* child : m_children) {
        child->moveSubtree(delta);
    }
}

QVariant NodeItem::itemChange(GraphicsItemChange change, const QVariant& value) {
    if (change == ItemPositionHasChanged) {
        for (auto* edge : m_edges) {
            edge->updatePath();
        }
    }
    return QGraphicsObject::itemChange(change, value);
}

void NodeItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) {
    Q_UNUSED(event);
    emit doubleClicked(this);
}

void NodeItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_dragStartPos = pos();
        m_dragOrigPos = pos();
        m_dragging = true;
    }
    QGraphicsObject::mousePressEvent(event);
}

void NodeItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    // Close any open editing widget when a drag starts — mouseMoveEvent is
    // only delivered while a button is held, so any call here means the user
    // is dragging rather than editing.
    auto* mindMapScene = dynamic_cast<MindMapScene*>(scene());
    if (mindMapScene && mindMapScene->isEditing()) {
        mindMapScene->cancelEditing();
    }

    if (m_dragging) {
        QPointF delta = pos() - m_dragStartPos;
        m_dragStartPos = pos();
        // Move children along with this node
        for (auto* child : m_children) {
            child->moveSubtree(delta);
        }
    }
    QGraphicsObject::mouseMoveEvent(event);
}

void NodeItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    if (m_dragging && pos() != m_dragOrigPos) {
        auto* mindMapScene = dynamic_cast<MindMapScene*>(scene());
        if (mindMapScene) {
            mindMapScene->undoStack()->push(new MoveNodeCommand(this, m_dragOrigPos, pos()));
        }
    }
    m_dragging = false;
    QGraphicsObject::mouseReleaseEvent(event);
}

void NodeItem::updateGeometry() {
    prepareGeometryChange();
    QFontMetricsF fm(m_font);
    qreal textW = fm.horizontalAdvance(m_text);
    qreal w = qMax(kMinWidth, qMin(kMaxWidth, textW + kPadding * 2));

    // When text exceeds available width, wrap to multiple lines
    qreal availableTextW = w - kPadding * 2;
    QRectF textRect =
        fm.boundingRect(QRectF(0, 0, availableTextW, 0), Qt::TextWrapAnywhere, m_text);
    qreal h = textRect.height() + kPadding * 2;

    m_rect = QRectF(-w / 2, -h / 2, w, h);

    // Update connected edges since node geometry changed
    for (auto* edge : m_edges) {
        edge->updatePath();
    }
}

NodeItem::ButtonDirection NodeItem::addButtonDirection() const {
    auto* mindMapScene = dynamic_cast<MindMapScene*>(scene());
    if (!mindMapScene)
        return ButtonDirection::Right;

    // Determine effective layout style (template overrides scene default)
    LayoutStyle style = mindMapScene->layoutStyle();
    const auto* td = mindMapScene->templateDescriptor();
    if (td)
        style = algorithmNameToLayoutStyle(td->layout.algorithm);

    switch (style) {
    case LayoutStyle::TopDown:
        return ButtonDirection::Bottom;
    case LayoutStyle::RightTree:
        return ButtonDirection::Right;
    case LayoutStyle::Bilateral:
    default:
        if (!m_parentNode) {
            // Root node: next child index determines direction
            // Bilateral alternates even=right, odd=left
            return (m_children.size() % 2 == 0) ? ButtonDirection::Right : ButtonDirection::Left;
        }
        // Non-root: inherit side from position relative to root (at origin)
        return (pos().x() >= 0) ? ButtonDirection::Right : ButtonDirection::Left;
    }
}

QRectF NodeItem::addButtonRect() const {
    qreal diameter = kAddButtonRadius * 2;
    switch (m_addButtonDir) {
    case ButtonDirection::Left:
        return QRectF(m_rect.left() - kAddButtonOffset - diameter, -kAddButtonRadius, diameter,
                      diameter);
    case ButtonDirection::Bottom:
        return QRectF(-kAddButtonRadius, m_rect.bottom() + kAddButtonOffset, diameter, diameter);
    case ButtonDirection::Right:
    default:
        return QRectF(m_rect.right() + kAddButtonOffset, -kAddButtonRadius, diameter, diameter);
    }
}

void NodeItem::startAddButtonAnimation(bool fadeIn) {
    if (m_addButtonAnimation) {
        m_addButtonAnimation->stop();
        m_addButtonAnimation->deleteLater();
        m_addButtonAnimation = nullptr;
    }

    if (!m_addButtonOverlay)
        return;

    auto* anim = new QVariantAnimation(this);
    anim->setDuration(200);
    anim->setEasingCurve(QEasingCurve::InOutQuad);
    anim->setStartValue(m_addButtonOverlay->buttonOpacity());
    anim->setEndValue(fadeIn ? 1.0 : 0.0);

    connect(anim, &QVariantAnimation::valueChanged, this, [this](const QVariant& value) {
        if (m_addButtonOverlay)
            m_addButtonOverlay->setButtonOpacity(value.toReal());
    });

    connect(anim, &QVariantAnimation::finished, this, [this, fadeIn, anim]() {
        if (!fadeIn) {
            setZValue(m_savedZValue);
            if (m_addButtonOverlay)
                m_addButtonOverlay->setVisible(false);
        }
        anim->deleteLater();
        m_addButtonAnimation = nullptr;
    });

    m_addButtonAnimation = anim;
    anim->start();
}

void NodeItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    Q_UNUSED(event);

    // Cancel any pending fade-out from a previous brief leave
    if (m_hoverLeaveTimer) {
        m_hoverLeaveTimer->stop();
        delete m_hoverLeaveTimer;
        m_hoverLeaveTimer = nullptr;
    }

    if (!m_hovered) {
        m_hovered = true;
        m_addButtonDir = addButtonDirection();

        // Raise above sibling nodes so the button is not occluded
        m_savedZValue = zValue();
        setZValue(50);

        if (!m_addButtonOverlay)
            m_addButtonOverlay = new AddButtonOverlay(this);

        startAddButtonAnimation(true);
    }
}

void NodeItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    Q_UNUSED(event);

    // Delay the fade-out so the button doesn't vanish during imprecise mouse movements
    if (!m_hoverLeaveTimer) {
        m_hoverLeaveTimer = new QTimer(this);
        m_hoverLeaveTimer->setSingleShot(true);
        connect(m_hoverLeaveTimer, &QTimer::timeout, this, [this]() {
            m_hovered = false;
            startAddButtonAnimation(false);
            m_hoverLeaveTimer->deleteLater();
            m_hoverLeaveTimer = nullptr;
        });
    }
    m_hoverLeaveTimer->start(150);
}
