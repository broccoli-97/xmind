#pragma once

#include "layout/LayoutStyle.h"

#include <QMap>
#include <QPointF>

class NodeItem;

class LayoutEngine {
public:
    // Compute the target position for every node in the tree rooted at |root|.
    static QMap<NodeItem*, QPointF> computeLayout(NodeItem* root, LayoutStyle style);

    // Compute a reasonable initial position for a newly added child node,
    // avoiding overlap with existing nodes.
    static QPointF initialChildPosition(NodeItem* newNode, NodeItem* parent,
                                        NodeItem* root, LayoutStyle style);

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
        qreal depthSpacing;   // edge-to-edge gap along depth axis
        qreal spreadSpacing;  // kVSpacing
        int depthDirection;   // +1 or -1

        qreal spread(QPointF p) const { return spreadIsX ? p.x() : p.y(); }
        qreal depth(QPointF p) const { return spreadIsX ? p.y() : p.x(); }
        void setSpread(QPointF& p, qreal v) const { if (spreadIsX) p.rx() = v; else p.ry() = v; }
        void setDepth(QPointF& p, qreal v) const { if (spreadIsX) p.ry() = v; else p.rx() = v; }
        qreal nodeSpan(NodeItem* node) const;
        qreal nodeDepthSpan(NodeItem* node) const;
    };

    static LayoutAxis makeRightAxis();
    static LayoutAxis makeLeftAxis();
    static LayoutAxis makeTopDownAxis();

    // Phase 1: Measure (bottom-up) — spread-axis space needed by subtree
    static qreal measureSubtree(NodeItem* node, const LayoutAxis& axis);

    // Phase 2: Place (top-down)
    static void placeSubtree(NodeItem* node, QPointF position, const LayoutAxis& axis,
                             QMap<NodeItem*, QPointF>& positions);
    static void placeChildGroup(NodeItem* parent, const QList<NodeItem*>& children,
                                const LayoutAxis& axis,
                                QMap<NodeItem*, QPointF>& positions);

    // Phase 3: Force-directed refinement (overlap resolution)
    static void forceDirectedRefinement(
        NodeItem* root,
        const QList<NodeItem*>& subtreeRoots,
        const LayoutAxis& axis,
        QMap<NodeItem*, QPointF>& positions);

    // Helpers
    static void collectSubtreeNodes(NodeItem* node, QList<NodeItem*>& nodes,
                                    const QMap<NodeItem*, QPointF>& positions);
    static void collectAllNodes(NodeItem* node, QList<NodeItem*>& nodes);
    static qreal findAvailableSpread(qreal candidateSpread, qreal depth,
                                     NodeItem* newNode,
                                     const QList<NodeItem*>& allNodes,
                                     const LayoutAxis& axis);

    // Force-directed constants
    static constexpr int kMaxIterations = 100;
    static constexpr qreal kInitialTemperature = 150.0;
    static constexpr qreal kCoolingFactor = 0.90;
    static constexpr qreal kConvergenceThreshold = 0.5;
    static constexpr qreal kRepulsionStrength = 1.2;
    static constexpr qreal kSpringStrength = 0.03;

    // Edge-to-edge gap constants for depth axis
    static constexpr qreal kHGap = 100.0;
    static constexpr qreal kTopDownGap = 56.0;
};
