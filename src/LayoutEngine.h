#pragma once

#include "LayoutStyle.h"

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
    // --- Bilateral / RightTree helpers (horizontal layouts) ---
    static qreal subtreeHeight(NodeItem* node);
    static void calculatePositions(NodeItem* node, qreal x, qreal y, int direction,
                                   const EdgeFinder& edgeFinder,
                                   QMap<NodeItem*, QPointF>& positions);
    static void layoutLockedSubtreeHorizontal(NodeItem* child, int direction,
                                              const EdgeFinder& edgeFinder,
                                              QMap<NodeItem*, QPointF>& positions);

    static void layoutBilateral(NodeItem* root, const EdgeFinder& edgeFinder,
                                QMap<NodeItem*, QPointF>& positions);
    static void layoutRightTree(NodeItem* root, const EdgeFinder& edgeFinder,
                                QMap<NodeItem*, QPointF>& positions);

    // --- TopDown helpers ---
    static qreal subtreeWidth(NodeItem* node);
    static void calculatePositionsTopDown(NodeItem* node, qreal x, qreal y,
                                          const EdgeFinder& edgeFinder,
                                          QMap<NodeItem*, QPointF>& positions);
    static void layoutLockedSubtreeTopDown(NodeItem* child, const EdgeFinder& edgeFinder,
                                           QMap<NodeItem*, QPointF>& positions);

    static void layoutTopDown(NodeItem* root, const EdgeFinder& edgeFinder,
                              QMap<NodeItem*, QPointF>& positions);
};
