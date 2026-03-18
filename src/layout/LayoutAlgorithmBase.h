#pragma once

#include "layout/ILayoutAlgorithm.h"

#include <QList>
#include <QRectF>

class NodeItem;

class LayoutAlgorithmBase : public ILayoutAlgorithm {
public:
    // Layout constants (public so Commands/MindMapScene can reference them)
    static constexpr qreal kNodeHeight = 44.0;
    static constexpr qreal kTopDownDepthRatio = 0.56;

protected:
    // Axis abstraction -- parameterizes layout direction
    struct LayoutAxis {
        bool spreadIsX;
        qreal depthSpacing;
        qreal spreadSpacing;
        int depthDirection;

        qreal spread(QPointF p) const { return spreadIsX ? p.x() : p.y(); }
        qreal depth(QPointF p) const { return spreadIsX ? p.y() : p.x(); }
        void setSpread(QPointF& p, qreal v) const {
            if (spreadIsX) p.rx() = v; else p.ry() = v;
        }
        void setDepth(QPointF& p, qreal v) const {
            if (spreadIsX) p.ry() = v; else p.rx() = v;
        }
        qreal nodeSpan(NodeItem* node) const;
        qreal nodeDepthSpan(NodeItem* node) const;
    };

    static LayoutAxis makeRightAxis(const LayoutParams& p);
    static LayoutAxis makeLeftAxis(const LayoutParams& p);
    static LayoutAxis makeTopDownAxis(const LayoutParams& p);

    // Phase 1: Measure (bottom-up)
    static qreal measureSubtree(NodeItem* node, const LayoutAxis& axis);

    // Phase 2: Place (top-down)
    static void placeSubtree(NodeItem* node, QPointF position, const LayoutAxis& axis,
                              QMap<NodeItem*, QPointF>& positions);
    static void placeChildGroup(NodeItem* parent, const QList<NodeItem*>& children,
                                 const LayoutAxis& axis,
                                 QMap<NodeItem*, QPointF>& positions);

    // Phase 3: Force-directed refinement
    static void forceDirectedRefinement(NodeItem* root,
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

    // Common initial-child-position logic for single-axis layouts
    static QPointF initialChildPositionForAxis(NodeItem* newNode, NodeItem* parent,
                                                NodeItem* root, const LayoutParams& p,
                                                const LayoutAxis& axis);

    // Force-directed constants
    static constexpr int kMaxIterations = 100;
    static constexpr qreal kInitialTemperature = 150.0;
    static constexpr qreal kCoolingFactor = 0.90;
    static constexpr qreal kConvergenceThreshold = 0.5;
    static constexpr qreal kRepulsionStrength = 1.2;
    static constexpr qreal kSpringStrength = 0.03;
};
