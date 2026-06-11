#pragma once

#include <QColor>
#include <QFont>
#include <QGraphicsObject>
#include <QList>

class AddButtonOverlay;
class EdgeItem;
class MindMapScene;
class QTimer;
class QVariantAnimation;

class NodeItem : public QGraphicsObject {
    Q_OBJECT

public:
    explicit NodeItem(const QString& text, QGraphicsItem* parent = nullptr);
    ~NodeItem() override;

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    QString text() const;
    void setText(const QString& text);

    NodeItem* parentNode() const;
    void setParentNode(NodeItem* parent);

    QList<NodeItem*> childNodes() const;
    void addChild(NodeItem* child);
    void insertChild(int index, NodeItem* child);
    void removeChild(NodeItem* child);

    int level() const;
    QColor nodeColor() const;
    // Color of the level-1 ancestor's palette slot. Every node in the same
    // top-level branch returns the same color. Used by templates whose
    // paletteSource/colorSource is "branch".
    QColor branchColor() const;
    QFont font() const;

    void addEdge(EdgeItem* edge);
    void removeEdge(EdgeItem* edge);
    QList<EdgeItem*> edges() const { return m_edges; }

    QRectF nodeRect() const;
    void moveSubtree(const QPointF& delta);

    // Re-measure this node against the current theme/template. Call after a
    // theme swap so existing nodes pick up new padding/font/min-max widths.
    void refreshGeometry();

    void showAddButton();
    void hideAddButton();
    // Tear down the add-button overlay immediately (without the fade-out
    // animation). Called when inline editing begins so the overlay doesn't
    // visibly hang while the editor takes focus.
    void cancelAddButton();

    // Collapse / expand state for this node's subtree. Collapsed nodes hide
    // all descendant nodes and incident edges (recursively respecting nested
    // collapse states). Persisted in JSON (format v4+). This is the raw state
    // setter (used by deserialization and tests); interactive toggles go
    // through MindMapScene::toggleNodeCollapsed, which also re-layouts.
    bool isCollapsed() const { return m_collapsed; }
    void setCollapsed(bool collapsed);
    // Toggle wrapper for convenience (keyboard shortcut etc).
    void toggleCollapsed() { setCollapsed(!m_collapsed); }

    // Total number of nodes in this node's subtree (excluding itself) — the
    // figure shown on the collapse badge while folded.
    int descendantCount() const;

    // The fold control at this node's child-side junction (local coords):
    // while collapsed it is the always-visible count badge (a pill that
    // widens for multi-digit counts, painted by this item); while expanded
    // it is the circle the hover overlay paints its collapse chevron in.
    QRectF collapseControlRect() const;
    // True when the count badge is showing (collapsed with hidden children).
    bool collapseBadgeVisible() const { return m_collapsed && !m_children.isEmpty(); }

    // Search highlight state (driven by the in-map find bar). Only paints a
    // visible overlay; doesn't touch selection. `setSearchCurrent` raises one
    // match as the currently-focused one (brighter ring).
    void setSearchMatch(bool match);
    void setSearchCurrent(bool current);
    bool isSearchMatch() const { return m_searchMatch; }

signals:
    void doubleClicked(NodeItem* node);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

private:
    friend class AddButtonOverlay;

    enum class ButtonDirection { Right, Left, Bottom };

    void updateGeometry();
    ButtonDirection addButtonDirection() const;
    QRectF addButtonRect() const;
    void startAddButtonAnimation(bool fadeIn);
    MindMapScene* mindMapScene() const;
    QString collapseBadgeLabel() const;
    QFont collapseBadgeFont() const;

    // Apply this node's visibility to all descendants. If `force` is true, the
    // subtree is hidden regardless of m_collapsed (used when an ancestor is
    // collapsed). Edges connecting hidden nodes are also hidden.
    void applyDescendantVisibility(bool force);

    QString m_text;
    QFont m_font;
    QRectF m_rect;
    bool m_collapsed = false;
    bool m_searchMatch = false;
    bool m_searchCurrent = false;
    // 1 → 0 while the "settle" pulse of the current-match glow plays.
    qreal m_searchPulse = 0.0;
    QVariantAnimation* m_searchPulseAnim = nullptr;
    NodeItem* m_parentNode = nullptr;
    QList<NodeItem*> m_children;
    QList<EdgeItem*> m_edges;
    QPointF m_dragStartPos;
    QPointF m_dragOrigPos;
    bool m_dragging = false;
    bool m_hovered = false;
    bool m_badgeHovered = false;
    bool m_badgePressed = false;
    MindMapScene* m_mindMapScene = nullptr;
    qreal m_savedZValue = 0.0;
    ButtonDirection m_addButtonDir = ButtonDirection::Right;
    QVariantAnimation* m_addButtonAnimation = nullptr;
    QTimer* m_hoverLeaveTimer = nullptr;
    AddButtonOverlay* m_addButtonOverlay = nullptr;

    static constexpr qreal kAddButtonRadius = 12.0;
    static constexpr qreal kAddButtonOffset = 6.0;
    static constexpr qreal kHoverZoneMargin = 10.0;
    // Fold control (count badge / collapse chevron) at the child-side
    // junction: a touch smaller than the add button so the pair reads as
    // [node][fold][add] with the add action as the primary affordance.
    static constexpr qreal kCollapseControlRadius = 9.0;
    static constexpr qreal kCollapseControlGap = 2.0;
    // Raised z while hovered. Just enough to top sibling nodes at z=0 without
    // a big visual jump; well under the inline editor at z=100.
    static constexpr qreal kHoverZ = 2.0;

public:
    // Defaults applied when the active template doesn't override them.
    static constexpr qreal kMinWidth = 120.0;
    static constexpr qreal kMaxWidth = 300.0;
    static constexpr qreal kPadding = 16.0;
    static constexpr qreal kRadius = 10.0;
};
