#include "scene/NodeItem.h"
#include "core/AppSettings.h"
#include "core/Commands.h"
#include "core/TemplateDescriptor.h"
#include "scene/EdgeItem.h"
#include "scene/MindMapScene.h"
#include "ui/ThemeManager.h"

#include <QFontMetricsF>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

NodeItem::NodeItem(const QString& text, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_text(text) {
    setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
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

    // Text
    painter->setPen(textColor);
    painter->setFont(m_font);
    QFontMetricsF fm(m_font);
    QString displayText = fm.elidedText(m_text, Qt::ElideRight, m_rect.width() - kPadding * 2);
    painter->drawText(m_rect, Qt::AlignCenter, displayText);
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
    qreal textH = fm.height();
    qreal w = qMax(kMinWidth, qMin(kMaxWidth, textW + kPadding * 2));
    qreal h = textH + kPadding * 2;
    m_rect = QRectF(-w / 2, -h / 2, w, h);
}
