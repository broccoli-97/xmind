#pragma once

#include "layout/LayoutStyle.h"

#include <QMap>
#include <QPointF>
#include <functional>

class NodeItem;
class EdgeItem;

// Callback used by LayoutEngine to query whether an edge is locked.
// Signature: EdgeItem* finder(NodeItem* parent, NodeItem* child)
using EdgeFinder = std::function<EdgeItem*(NodeItem*, NodeItem*)>;

class LayoutEngine {
public:
    // Compute the target position for every node in the tree rooted at |root|.
    // |edgeFinder| is used to check edge lock state.
    static QMap<NodeItem*, QPointF> computeLayout(NodeItem* root, LayoutStyle style,
                                                  const EdgeFinder& edgeFinder);

    // Compute a reasonable initial position for a newly added child node.
    static QPointF initialChildPosition(NodeItem* parent, NodeItem* root, LayoutStyle style);

    // Layout constants (public so Commands/MindMapScene can reference them)
    static constexpr qreal kHSpacing = 220.0;
    static constexpr qreal kVSpacing = 16.0;
    static constexpr qreal kNodeHeight = 44.0;
    static constexpr qreal kTopDownLevelSpacing = 100.0;

private:
    // Axis abstraction — parameterizes layout direction, eliminating
    // horizontal/topdown code duplication.
    struct LayoutAxis {
        bool spreadIsX;       // TopDown: true (children along X), Horizontal: false (along Y)
        qreal depthSpacing;   // kHSpacing or kTopDownLevelSpacing
        qreal spreadSpacing;  // kVSpacing
        int depthDirection;   // +1 or -1

        qreal spread(QPointF p) const { return spreadIsX ? p.x() : p.y(); }
        qreal depth(QPointF p) const { return spreadIsX ? p.y() : p.x(); }
        void setSpread(QPointF& p, qreal v) const { if (spreadIsX) p.rx() = v; else p.ry() = v; }
        void setDepth(QPointF& p, qreal v) const { if (spreadIsX) p.ry() = v; else p.rx() = v; }
        qreal nodeSpan(NodeItem* node) const;
    };

    static LayoutAxis makeRightAxis();
    static LayoutAxis makeLeftAxis();
    static LayoutAxis makeTopDownAxis();

    // Phase 1: Measure (bottom-up) — spread-axis space needed by subtree
    static qreal measureSubtree(NodeItem* node, const LayoutAxis& axis,
                                const EdgeFinder& edgeFinder);

    // Phase 2: Place (top-down)
    static void placeSubtree(NodeItem* node, QPointF position, const LayoutAxis& axis,
                             const EdgeFinder& edgeFinder,
                             QMap<NodeItem*, QPointF>& positions);
    static void placeChildGroup(NodeItem* parent, const QList<NodeItem*>& children,
                                const LayoutAxis& axis, const EdgeFinder& edgeFinder,
                                QMap<NodeItem*, QPointF>& positions);

    // Phase 3: Force-directed refinement (overlap resolution)
    static void forceDirectedRefinement(
        NodeItem* root,
        const QList<NodeItem*>& subtreeRoots,
        const LayoutAxis& axis,
        const EdgeFinder& edgeFinder,
        QMap<NodeItem*, QPointF>& positions);

    // Helpers
    static void collectSubtreeNodes(NodeItem* node, QList<NodeItem*>& nodes,
                                    const QMap<NodeItem*, QPointF>& positions);

    // Force-directed constants
    static constexpr int kMaxIterations = 100;
    static constexpr qreal kInitialTemperature = 150.0;
    static constexpr qreal kCoolingFactor = 0.90;
    static constexpr qreal kConvergenceThreshold = 0.5;
    static constexpr qreal kRepulsionStrength = 1.2;
    static constexpr qreal kSpringStrength = 0.03;
};
