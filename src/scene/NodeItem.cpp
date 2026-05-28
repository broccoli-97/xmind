#include "scene/NodeItem.h"
#include "core/AppSettings.h"
#include "core/Commands.h"
#include "core/TemplateDescriptor.h"
#include "core/ThemeDescriptor.h"
#include "layout/LayoutStyle.h"
#include "scene/AddButtonOverlay.h"
#include "scene/EdgeItem.h"
#include "scene/MindMapScene.h"
#include "scene/NodeStyleResolver.h"
#include "scene/SketchyPainter.h"
#include "ui/ThemeManager.h"

#include <QFontMetricsF>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QTimer>
#include <QVariantAnimation>

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

// Style resolution lives in NodeStyleResolver. These two helpers just resolve
// the descriptors a node currently sees through its (possibly null) scene.

namespace {

const ThemeDescriptor* nodeTheme(const MindMapScene* scene) {
    return scene ? scene->themeDescriptor() : nullptr;
}

const TemplateDescriptor* nodeTemplate(const MindMapScene* scene) {
    return scene ? scene->templateDescriptor() : nullptr;
}

} // namespace

QRectF NodeItem::boundingRect() const {
    NodeStyleResolver resolver(nodeTheme(m_mindMapScene), nodeTemplate(m_mindMapScene));
    const auto s = resolver.styleForLevel(level());
    const bool withShadow = NodeStyleResolver::drawsShadow(s, s.shape);
    // Sketch outlines wobble outside m_rect by up to ~5px — match the cap in
    // SketchyPainter::drawRoughRoundedRect so the AA strokes aren't clipped.
    const qreal sketchPad = s.roughness > 0.0 ? 6.0 : 0.0;
    if (!withShadow) {
        // No shadow → the rect itself plus a tiny anti-alias margin is enough.
        const qreal m = 2.0 + sketchPad;
        return m_rect.adjusted(-m, -m, m, m);
    }
    constexpr qreal kMargin = 2.0;
    return m_rect.adjusted(-s.shadowSpread - kMargin - sketchPad,
                           -s.shadowSpread - kMargin - sketchPad,
                           s.shadowSpread + kMargin + sketchPad,
                           s.shadowSpread + s.shadowOffsetY + kMargin + sketchPad);
}

QPainterPath NodeItem::shape() const {
    NodeStyleResolver resolver(nodeTheme(m_mindMapScene), nodeTemplate(m_mindMapScene));
    const auto style = resolver.styleForLevel(level());
    QPainterPath path;
    if (style.shape == QLatin1String("none") || style.shape == QLatin1String("underline")) {
        // For shapeless / underline styles, hit-test the text rect (a bit
        // padded) so clicks on the text still select the node.
        path.addRect(m_rect.adjusted(-2, -2, 2, 2));
    } else {
        path.addRoundedRect(m_rect, style.borderRadius, style.borderRadius);
    }
    return path;
}

void NodeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
                     QWidget* /*widget*/) {
    painter->setRenderHint(QPainter::Antialiasing);

    // Resolve colors from the active theme (falls back to global ThemeManager).
    const ThemeColors& globalTC = ThemeManager::colors();
    QColor shadowColor = globalTC.nodeShadow;
    QColor selectionBorder = globalTC.nodeSelectionBorder;
    QColor textColor = globalTC.nodeText;
    QColor fixedBorderColor;

    auto* mindMapScene = m_mindMapScene;
    const ThemeDescriptor* th = nodeTheme(mindMapScene);
    const TemplateDescriptor* td = nodeTemplate(mindMapScene);
    if (th) {
        const auto& tc = th->activeColors();
        shadowColor = tc.nodeShadow;
        selectionBorder = tc.nodeSelectionBorder;
        textColor = tc.nodeText;
        fixedBorderColor = tc.nodeBorderColor;
    }

    NodeStyleResolver resolver(th, td);
    const int lvl = level();
    const ThemeNodeStyle style = resolver.styleForLevel(lvl);
    const QString shape = style.shape;
    const qreal radius = style.borderRadius;
    const qreal padding = style.padding;
    const QColor nodeCol = nodeColor();

    // ----- Drop shadow -------------------------------------------------------
    if (NodeStyleResolver::drawsShadow(style, shape)) {
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
            QRectF sr = m_rect.adjusted(-expand, -expand, expand, expand).translated(0, offsetY);
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
        // When the theme anchors edges to the baseline, the underline must be
        // a *seamless continuation* of the parent's bezier endpoint:
        //   - same stroke width as the edge (no 1.5x boost)
        //   - centered exactly on m_rect.bottom() (same Y as edge endpoint),
        //     not offset upward by half the line width
        //   - extended to the rect edges (no inset margin)
        // Otherwise (standalone accent under e.g. the root), keep the bolder,
        // slightly inset look that reads better on its own.
        // Template overrides edge anchor (Lined forces baseline).
        QString anchor = th ? th->edgeStyle.anchor : QStringLiteral("center");
        if (td && !td->edgeAnchorOverride.isEmpty())
            anchor = td->edgeAnchorOverride;
        const bool baselineAnchor = (anchor == QLatin1String("baseline"));
        const qreal edgeW = th ? th->edgeStyle.width : 2.5;
        const qreal lineW = baselineAnchor ? edgeW : edgeW * 1.5;
        QPen underline(nodeCol, lineW, Qt::SolidLine, Qt::RoundCap);
        painter->setPen(underline);
        painter->setBrush(Qt::NoBrush);
        const qreal y = baselineAnchor ? m_rect.bottom() : m_rect.bottom() - lineW * 0.5;
        const qreal margin = baselineAnchor ? 0.0 : qMin<qreal>(padding, 4.0);
        painter->drawLine(QPointF(m_rect.left() + margin, y), QPointF(m_rect.right() - margin, y));

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
            QColor borderC =
                NodeStyleResolver::resolveBorderColor(style, nodeCol, fixedBorderColor);
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

        if (style.roughness > 0.0 && !selected) {
            // Sketch mode: fill stays a clean rounded rect (jittering the
            // fill bleeds outside the body and looks dirty); only the
            // outline gets the rough multi-pass treatment. Selection state
            // skips sketch — the focus ring should read as a deliberate
            // clean overlay, not part of the drawn shape.
            painter->setPen(Qt::NoPen);
            painter->drawRoundedRect(m_rect, radius, radius);
            if (pen.style() != Qt::NoPen) {
                painter->setPen(pen);
                painter->setBrush(Qt::NoBrush);
                SketchyPainter::drawRoughRoundedRect(painter, m_rect, radius, style.roughness,
                                                     qMax(1, style.strokePasses),
                                                     quint32(reinterpret_cast<quintptr>(this)));
            }
        } else {
            painter->drawRoundedRect(m_rect, radius, radius);
        }
    }

    // ----- Text --------------------------------------------------------------
    // When the node has no colored slab behind the text (underline / none
    // shapes, or outlined / tinted roundedRect fills), default white text
    // would vanish on a light canvas. Tint the text from the node color
    // instead — darker than the accent in light themes, lighter in dark
    // themes — so it reads against the canvas while still belonging to the
    // node's hue.
    const bool textOnCanvas =
        shape == QLatin1String("underline") || shape == QLatin1String("none") ||
        (shape == QLatin1String("roundedRect") && (style.fillMode == QLatin1String("outlined") ||
                                                   style.fillMode == QLatin1String("tinted")));
    QColor effTextColor = textColor;
    if (textOnCanvas) {
        effTextColor = ThemeManager::isDark() ? nodeCol.lighter(140) : nodeCol.darker(120);
    }
    painter->setPen(effTextColor);
    painter->setFont(resolver.fontForLevel(lvl, m_font));
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
    if (m_parentNode == parent)
        return;
    m_parentNode = parent;
    // Level changed → re-measure. Themes whose rootStyle overrides padding or
    // font (e.g. Whimsy: Georgia 22pt) would otherwise leave a child sized
    // for level=0 when it was added to the scene before being parented.
    updateGeometry();
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
    const auto* th = nodeTheme(m_mindMapScene);
    const auto* td = nodeTemplate(m_mindMapScene);

    QString paletteSource = th ? th->nodeStyle.paletteSource : QStringLiteral("level");
    if (td && !td->paletteSourceOverride.isEmpty())
        paletteSource = td->paletteSourceOverride;

    if (paletteSource == QLatin1String("branch"))
        return branchColor();

    if (th)
        return th->activeColors().nodePalette[level() % 6];
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

    const auto* th = nodeTheme(m_mindMapScene);
    if (th)
        return th->activeColors().nodePalette[branchIndex % 6];
    return ThemeManager::colors().nodePalette[branchIndex % 6];
}

QFont NodeItem::font() const {
    return NodeStyleResolver(nodeTheme(m_mindMapScene), nodeTemplate(m_mindMapScene))
        .fontForLevel(level(), m_font);
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

void NodeItem::refreshGeometry() {
    updateGeometry();
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

        // Raise just enough to top sibling nodes (all at z=0 by default). A
        // big jump (we used to set z=50) made for a visible flicker against
        // adjacent items; z=2 is sufficient and leaves plenty of headroom
        // under the inline editor at z=100. The guard avoids overwriting an
        // already-saved baseline during a rapid leave/re-enter cycle.
        if (zValue() < kHoverZ)
            m_savedZValue = zValue();
        setZValue(kHoverZ);

        if (!m_addButtonOverlay)
            m_addButtonOverlay = new AddButtonOverlay(this);

        startAddButtonAnimation(true);
    }
}

void NodeItem::cancelAddButton() {
    // Stop any in-flight fade animation so it can't fire a delayed setZValue
    // / setVisible after we've torn down state below.
    if (m_addButtonAnimation) {
        m_addButtonAnimation->stop();
        m_addButtonAnimation->deleteLater();
        m_addButtonAnimation = nullptr;
    }
    if (m_hoverLeaveTimer) {
        m_hoverLeaveTimer->stop();
        delete m_hoverLeaveTimer;
        m_hoverLeaveTimer = nullptr;
    }
    if (m_addButtonOverlay) {
        m_addButtonOverlay->setButtonOpacity(0.0);
        m_addButtonOverlay->setVisible(false);
    }
    m_hovered = false;
    setZValue(m_savedZValue);
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
        // The constructor measured this node with no scene — themes that
        // bump the font (e.g. sketch swaps in Caveat at 17pt) need a re-fit
        // now that we can read theme overrides.
        updateGeometry();
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
    NodeStyleResolver resolver(nodeTheme(m_mindMapScene), nodeTemplate(m_mindMapScene));
    const int lvl = level();
    const auto style = resolver.styleForLevel(lvl);
    const qreal pad = style.padding;
    const qreal minW = style.minWidth;
    const qreal maxW = style.maxWidth;

    QFontMetricsF fm(resolver.fontForLevel(lvl, m_font));
    // Slanted/cursive fonts (e.g. Caveat in the sketch theme) draw the final
    // glyph past its advance metric — the italic angle pushes the upper-right
    // corner outside horizontalAdvance. Use the largest of advance, the
    // logical bounding rect, and the ink-tight bounding rect so the
    // rendered rectangle is wide enough for any font shape.
    qreal textW = fm.horizontalAdvance(m_text);
    textW = qMax(textW, fm.boundingRect(m_text).width());
    textW = qMax(textW, fm.tightBoundingRect(m_text).width() + fm.ascent() * 0.2);
    qreal w = qMax(minW, qMin(maxW, textW + pad * 2));

    // Wrap layout. boundingRect-with-flags reports slightly wider than
    // horizontalAdvance for slanted fonts; without a small slack it can
    // wrap a string that we already sized to fit. The 2px buffer absorbs
    // that measurement jitter without altering wrap behaviour for genuinely
    // long text (which hits maxW long before slack matters).
    qreal availableTextW = w - pad * 2 + 2.0;
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
