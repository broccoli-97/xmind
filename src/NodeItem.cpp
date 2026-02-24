#include "NodeItem.h"
#include "AppSettings.h"
#include "Commands.h"
#include "EdgeItem.h"
#include "MindMapScene.h"

#include <QFontMetricsF>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

static const QColor kPalette[] = {
    QColor("#1565C0"), // Root - Blue
    QColor("#2E7D32"), // Level 1 - Green
    QColor("#E65100"), // Level 2 - Orange
    QColor("#6A1B9A"), // Level 3 - Purple
    QColor("#C62828"), // Level 4 - Red
    QColor("#00838F"), // Level 5 - Teal
};

static const QColor kDarkPalette[] = {
    QColor("#42A5F5"), // Root - Blue
    QColor("#66BB6A"), // Level 1 - Green
    QColor("#FFA726"), // Level 2 - Orange
    QColor("#AB47BC"), // Level 3 - Purple
    QColor("#EF5350"), // Level 4 - Red
    QColor("#26C6DA"), // Level 5 - Teal
};
static constexpr int kPaletteSize = sizeof(kPalette) / sizeof(kPalette[0]);

NodeItem::NodeItem(const QString& text, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_text(text) {
    setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    m_font.setPointSize(AppSettings::instance().defaultFontSize());
    m_font.setFamilies(
        {"Segoe UI", "Microsoft YaHei", "Noto Sans CJK SC", "PingFang SC", "sans-serif"});
    updateGeometry();
}

NodeItem::~NodeItem() = default;

QRectF NodeItem::boundingRect() const {
    return m_rect.adjusted(-3, -3, 5, 5);
}

QPainterPath NodeItem::shape() const {
    QPainterPath path;
    path.addRoundedRect(m_rect, kRadius, kRadius);
    return path;
}

void NodeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
                     QWidget* /*widget*/) {
    painter->setRenderHint(QPainter::Antialiasing);

    QColor bg = nodeColor();

    // Shadow
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(0, 0, 0, 30));
    painter->drawRoundedRect(m_rect.translated(2, 3), kRadius, kRadius);

    // Body
    QColor border = bg.darker(120);
    if (option->state & QStyle::State_Selected) {
        border = QColor("#FF6F00");
        painter->setPen(QPen(border, 3));
    } else {
        painter->setPen(QPen(border, 1.5));
    }
    painter->setBrush(bg);
    painter->drawRoundedRect(m_rect, kRadius, kRadius);

    // Text
    painter->setPen(Qt::white);
    painter->setFont(m_font);
    painter->drawText(m_rect, Qt::AlignCenter, m_text);
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
    if (AppSettings::instance().theme() == AppTheme::Dark)
        return kDarkPalette[level() % kPaletteSize];
    return kPalette[level() % kPaletteSize];
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
    qreal w = qMax(kMinWidth, textW + kPadding * 2);
    qreal h = textH + kPadding * 2;
    m_rect = QRectF(-w / 2, -h / 2, w, h);
}
