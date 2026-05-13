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

    auto* mindMapScene = m_node->mindMapScene();
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

// ----- Style resolution helpers -----------------------------------------------
// Centralize the "template field if set, else NodeItem constant" fallback so
// updateGeometry/paint/shape can all read the same effective values.

namespace {

const TemplateDescriptor* nodeTemplate(const MindMapScene* scene) {
    return scene ? scene->templateDescriptor() : nullptr;
}

// Effective node style for a given (template, level) — accounts for rootStyle override.
const TemplateNodeStyle& effStyle(const TemplateDescriptor* td, int level) {
    static const TemplateNodeStyle kDefault{};
    if (!td)
        return kDefault;
    return td->nodeStyleForLevel(level);
}

qreal effPadding(const TemplateDescriptor* td, int level) {
    return effStyle(td, level).padding;
}
qreal effRadius(const TemplateDescriptor* td, int level) {
    return effStyle(td, level).borderRadius;
}
qreal effMinWidth(const TemplateDescriptor* td, int level) {
    return effStyle(td, level).minWidth;
}
qreal effMaxWidth(const TemplateDescriptor* td, int level) {
    return effStyle(td, level).maxWidth;
}

// Resolve which shape to draw — the effective style already encodes rootShape
// (via the rootStyle override synthesized in fromJson).
QString effShape(const TemplateDescriptor* td, int level) {
    return effStyle(td, level).shape;
}

bool effDrawShadow(const TemplateDescriptor* td, int level, const QString& shape) {
    // Shadow only makes sense behind a filled body. Outlined/tinted/non-rounded
    // shapes paint without one.
    if (shape != QLatin1String("roundedRect"))
        return false;
    const auto& s = effStyle(td, level);
    if (s.fillMode != QLatin1String("solid"))
        return false;
    return s.drawShadow;
}

QColor borderColorFor(const TemplateNodeStyle& s, const QColor& nodeColor,
                      const QColor& fixedColor) {
    if (s.borderColorSource == QLatin1String("darker"))
        return nodeColor.darker(125);
    if (s.borderColorSource == QLatin1String("fixed") && fixedColor.isValid())
        return fixedColor;
    return nodeColor;
}

} // namespace

QRectF NodeItem::boundingRect() const {
    const auto* td = nodeTemplate(m_mindMapScene);
    const int lvl = level();
    QString shape = effShape(td, lvl);
    bool withShadow = effDrawShadow(td, lvl, shape);
    if (!withShadow) {
        // No shadow → the rect itself plus a tiny anti-alias margin is enough.
        return m_rect.adjusted(-2, -2, 2, 2);
    }
    const auto& s = effStyle(td, lvl);
    constexpr qreal kMargin = 2.0;
    return m_rect.adjusted(-s.shadowSpread - kMargin, -s.shadowSpread - kMargin,
                           s.shadowSpread + kMargin,
                           s.shadowSpread + s.shadowOffsetY + kMargin);
}

QPainterPath NodeItem::shape() const {
    const auto* td = nodeTemplate(m_mindMapScene);
    const int lvl = level();
    QString s = effShape(td, lvl);
    QPainterPath path;
    if (s == QLatin1String("none") || s == QLatin1String("underline")) {
        // For shapeless / underline styles, hit-test the text rect (a bit
        // padded) so clicks on the text still select the node.
        path.addRect(m_rect.adjusted(-2, -2, 2, 2));
    } else {
        const qreal r = effRadius(td, lvl);
        path.addRoundedRect(m_rect, r, r);
    }
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
    QColor fixedBorderColor;

    auto* mindMapScene = m_mindMapScene;
    const TemplateDescriptor* td = nodeTemplate(mindMapScene);
    if (td) {
        const auto& tc = td->activeColors();
        shadowColor = tc.nodeShadow;
        selectionBorder = tc.nodeSelectionBorder;
        textColor = tc.nodeText;
        fixedBorderColor = tc.nodeBorderColor;
    }

    const int lvl = level();
    const TemplateNodeStyle& style = effStyle(td, lvl);
    const QString shape = style.shape;
    const qreal radius = style.borderRadius;
    const qreal padding = style.padding;
    const QColor nodeCol = nodeColor();

    // ----- Drop shadow -------------------------------------------------------
    if (effDrawShadow(td, lvl, shape)) {
        painter->setPen(Qt::NoPen);
        const int layers = qMax(1, style.shadowLayers);
        const qreal spread = style.shadowSpread;
        const qreal offsetY = style.shadowOffsetY;
        int baseAlpha = qBound(6, shadowColor.alpha() / 3, 20);
        int layerAlpha = qBound(0, int(baseAlpha * style.shadowOpacity), 255);
        for (int i = layers; i >= 1; --i) {
            qreal expand = spread * i / layers;
            QColor sc = shadowColor;
            sc.setAlpha(layerAlpha);
            painter->setBrush(sc);
            QRectF sr = m_rect.adjusted(-expand, -expand, expand, expand)
                            .translated(0, offsetY);
            painter->drawRoundedRect(sr, radius + expand, radius + expand);
        }
    }

    // ----- Body --------------------------------------------------------------
    const bool selected = (option->state & QStyle::State_Selected);

    if (shape == QLatin1String("none")) {
        // No fill, no border — just text. Selection shown as a thin tinted
        // rounded rect around the text bounds.
        if (selected) {
            QColor sel = selectionBorder;
            sel.setAlpha(120);
            painter->setPen(QPen(sel, 1.5, Qt::DashLine));
            painter->setBrush(Qt::NoBrush);
            painter->drawRoundedRect(m_rect, 4, 4);
        }
    } else if (shape == QLatin1String("underline")) {
        // Draw a horizontal accent line under the text instead of a body fill.
        // When the template anchors edges to the baseline, the underline must
        // be a *seamless continuation* of the parent's bezier endpoint:
        //   - same stroke width as the edge (no 1.5x boost)
        //   - centered exactly on m_rect.bottom() (same Y as edge endpoint),
        //     not offset upward by half the line width
        //   - extended to the rect edges (no inset margin)
        // Otherwise (standalone accent under e.g. the root), keep the bolder,
        // slightly inset look that reads better on its own.
        const bool baselineAnchor =
            td && td->edgeStyle.anchor == QLatin1String("baseline");
        const qreal edgeW = td ? td->edgeStyle.width : 2.5;
        const qreal lineW = baselineAnchor ? edgeW : edgeW * 1.5;
        QPen underline(nodeCol, lineW, Qt::SolidLine, Qt::RoundCap);
        painter->setPen(underline);
        painter->setBrush(Qt::NoBrush);
        const qreal y = baselineAnchor ? m_rect.bottom()
                                       : m_rect.bottom() - lineW * 0.5;
        const qreal margin = baselineAnchor ? 0.0 : qMin<qreal>(padding, 4.0);
        painter->drawLine(QPointF(m_rect.left() + margin, y),
                          QPointF(m_rect.right() - margin, y));

        if (selected) {
            QColor sel = selectionBorder;
            sel.setAlpha(120);
            painter->setPen(QPen(sel, 1.5, Qt::DashLine));
            painter->setBrush(Qt::NoBrush);
            painter->drawRoundedRect(m_rect, 4, 4);
        }
    } else {
        // roundedRect (or unknown shape — falls back to rounded rect).
        //
        // Pen logic: selection always wins (override with selection ring).
        // Otherwise, if borderWidth > 0, draw the configured border.
        // Otherwise, no pen.
        QPen pen = Qt::NoPen;
        if (selected) {
            pen = QPen(selectionBorder, style.selectionWidth);
        } else if (style.borderWidth > 0.0) {
            QColor borderC = borderColorFor(style, nodeCol, fixedBorderColor);
            pen = QPen(borderC, style.borderWidth);
        }
        painter->setPen(pen);

        // Brush logic by fillMode.
        QBrush brush(Qt::NoBrush);
        if (style.fillMode == QLatin1String("outlined")) {
            brush = Qt::NoBrush;
        } else if (style.fillMode == QLatin1String("tinted")) {
            QColor tint = nodeCol;
            tint.setAlphaF(qBound(0.0, style.fillAlpha, 1.0));
            brush = QBrush(tint);
        } else {
            // "solid" (default) — 100% colored fill, matching pre-refactor behavior.
            brush = QBrush(nodeCol);
        }
        painter->setBrush(brush);

        painter->drawRoundedRect(m_rect, radius, radius);
    }

    // ----- Text --------------------------------------------------------------
    // For outlined / tinted modes, the body isn't a colored slab so default
    // white text wouldn't read. Use the node color for the text instead so it
    // visually belongs to the node without being invisible on a light fill.
    QColor effTextColor = textColor;
    if (shape == QLatin1String("roundedRect") &&
        (style.fillMode == QLatin1String("outlined") ||
         style.fillMode == QLatin1String("tinted"))) {
        effTextColor = nodeCol.darker(120);
    }
    painter->setPen(effTextColor);
    painter->setFont(m_font);
    QRectF textArea = m_rect.adjusted(padding, padding, -padding, -padding);
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
    const auto* td = nodeTemplate(m_mindMapScene);
    if (td) {
        const auto& palette = td->activeColors().nodePalette;
        if (td->nodeStyle.paletteSource == QLatin1String("branch"))
            return branchColor();
        return palette[level() % 6];
    }
    return ThemeManager::colors().nodePalette[level() % 6];
}

QColor NodeItem::branchColor() const {
    // Walk up to find the level-1 ancestor (direct child of the root). Its
    // index among the root's children determines the palette slot. The root
    // itself uses palette[0] for visual consistency with its first branch.
    const NodeItem* cur = this;
    const NodeItem* parent = m_parentNode;
    while (parent && parent->parentNode()) {
        cur = parent;
        parent = parent->parentNode();
    }

    int branchIndex = 0;
    if (parent) {
        // 'parent' is the root, 'cur' is its child (the branch root)
        branchIndex = parent->childNodes().indexOf(const_cast<NodeItem*>(cur));
        if (branchIndex < 0)
            branchIndex = 0;
    }

    const auto* td = nodeTemplate(m_mindMapScene);
    if (td)
        return td->activeColors().nodePalette[branchIndex % 6];
    return ThemeManager::colors().nodePalette[branchIndex % 6];
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

MindMapScene* NodeItem::mindMapScene() const {
    return m_mindMapScene;
}

void NodeItem::showAddButton() {
    if (m_mindMapScene && m_mindMapScene->isEditing())
        return;

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

void NodeItem::hideAddButton() {
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

QVariant NodeItem::itemChange(GraphicsItemChange change, const QVariant& value) {
    if (change == ItemPositionHasChanged) {
        for (auto* edge : m_edges) {
            edge->updatePath();
        }
    } else if (change == ItemSceneHasChanged) {
        m_mindMapScene = dynamic_cast<MindMapScene*>(scene());
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
    if (m_mindMapScene && m_mindMapScene->isEditing()) {
        m_mindMapScene->cancelEditing();
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
        if (m_mindMapScene) {
            m_mindMapScene->undoStack()->push(new MoveNodeCommand(this, m_dragOrigPos, pos()));
        }
    }
    m_dragging = false;
    QGraphicsObject::mouseReleaseEvent(event);
}

void NodeItem::updateGeometry() {
    prepareGeometryChange();
    const auto* td = nodeTemplate(m_mindMapScene);
    const int lvl = level();
    const qreal pad = effPadding(td, lvl);
    const qreal minW = effMinWidth(td, lvl);
    const qreal maxW = effMaxWidth(td, lvl);

    QFontMetricsF fm(m_font);
    qreal textW = fm.horizontalAdvance(m_text);
    qreal w = qMax(minW, qMin(maxW, textW + pad * 2));

    // When text exceeds available width, wrap to multiple lines
    qreal availableTextW = w - pad * 2;
    QRectF textRect =
        fm.boundingRect(QRectF(0, 0, availableTextW, 0), Qt::TextWrapAnywhere, m_text);
    qreal h = textRect.height() + pad * 2;

    m_rect = QRectF(-w / 2, -h / 2, w, h);

    // Update connected edges since node geometry changed
    for (auto* edge : m_edges) {
        edge->updatePath();
    }
}

NodeItem::ButtonDirection NodeItem::addButtonDirection() const {
    if (!m_mindMapScene)
        return ButtonDirection::Right;

    // Determine effective layout style (template overrides scene default)
    LayoutStyle style = m_mindMapScene->layoutStyle();
    const auto* td = m_mindMapScene->templateDescriptor();
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
    showAddButton();
}

void NodeItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    Q_UNUSED(event);
    hideAddButton();
}
