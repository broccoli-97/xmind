#include "layout/LayoutEngine.h"
#include "scene/NodeItem.h"

#include <QQueue>
#include <algorithm>
#include <cmath>

// ===========================================================================
// LayoutAxis
// ===========================================================================

qreal LayoutEngine::LayoutAxis::nodeSpan(NodeItem* node) const {
    if (spreadIsX)
        return node->nodeRect().width();
    else
        return qMax(kNodeHeight, node->nodeRect().height());
}

LayoutEngine::LayoutAxis LayoutEngine::makeRightAxis() {
    return {false, kHSpacing, kVSpacing, +1};
}

LayoutEngine::LayoutAxis LayoutEngine::makeLeftAxis() {
    return {false, kHSpacing, kVSpacing, -1};
}

LayoutEngine::LayoutAxis LayoutEngine::makeTopDownAxis() {
    return {true, kTopDownLevelSpacing, kVSpacing, +1};
}

// ===========================================================================
// Public API
// ===========================================================================

QMap<NodeItem*, QPointF> LayoutEngine::computeLayout(NodeItem* root, LayoutStyle style) {
    QMap<NodeItem*, QPointF> positions;
    if (!root)
        return positions;

    positions[root] = QPointF(0, 0);

    switch (style) {
    case LayoutStyle::Bilateral: {
        auto children = root->childNodes();
        QList<NodeItem*> rightChildren, leftChildren;
        for (int i = 0; i < children.size(); ++i) {
            if (i % 2 == 0)
                rightChildren.append(children[i]);
            else
                leftChildren.append(children[i]);
        }

        LayoutAxis rightAxis = makeRightAxis();
        LayoutAxis leftAxis = makeLeftAxis();

        placeChildGroup(root, rightChildren, rightAxis, positions);
        placeChildGroup(root, leftChildren, leftAxis, positions);

        forceDirectedRefinement(root, rightChildren, rightAxis, positions);
        forceDirectedRefinement(root, leftChildren, leftAxis, positions);
        break;
    }
    case LayoutStyle::RightTree: {
        LayoutAxis axis = makeRightAxis();
        placeSubtree(root, QPointF(0, 0), axis, positions);
        forceDirectedRefinement(root, root->childNodes(), axis, positions);
        break;
    }
    case LayoutStyle::TopDown: {
        LayoutAxis axis = makeTopDownAxis();
        placeSubtree(root, QPointF(0, 0), axis, positions);
        forceDirectedRefinement(root, root->childNodes(), axis, positions);
        break;
    }
    }

    return positions;
}

QPointF LayoutEngine::initialChildPosition(NodeItem* parent, NodeItem* root, LayoutStyle style) {
    QPointF parentPos = parent->pos();
    int childCount = parent->childNodes().size();
    qreal yOffset = (childCount - 1) * 60.0;

    if (style == LayoutStyle::TopDown) {
        return QPointF(parentPos.x() + (childCount - 1) * 60.0,
                       parentPos.y() + kTopDownLevelSpacing);
    } else if (style == LayoutStyle::RightTree) {
        return QPointF(parentPos.x() + kHSpacing, parentPos.y() + yOffset);
    } else {
        // Bilateral
        qreal xDir = (parent == root) ? ((childCount % 2 == 1) ? 1.0 : -1.0)
                                      : (parentPos.x() >= 0 ? 1.0 : -1.0);
        return QPointF(parentPos.x() + xDir * kHSpacing, parentPos.y() + yOffset);
    }
}

// ===========================================================================
// Phase 1: Measure (bottom-up)
// ===========================================================================

qreal LayoutEngine::measureSubtree(NodeItem* node, const LayoutAxis& axis) {
    auto children = node->childNodes();
    qreal selfSpan = axis.nodeSpan(node);
    if (children.isEmpty())
        return selfSpan;

    qreal total = 0;
    for (int i = 0; i < children.size(); ++i) {
        if (i > 0) total += axis.spreadSpacing;
        total += measureSubtree(children[i], axis);
    }

    return qMax(selfSpan, total);
}

// ===========================================================================
// Phase 2: Place (top-down)
// ===========================================================================

void LayoutEngine::placeSubtree(NodeItem* node, QPointF position, const LayoutAxis& axis,
                                 QMap<NodeItem*, QPointF>& positions) {
    positions[node] = position;
    placeChildGroup(node, node->childNodes(), axis, positions);
}

void LayoutEngine::placeChildGroup(NodeItem* parent, const QList<NodeItem*>& children,
                                    const LayoutAxis& axis,
                                    QMap<NodeItem*, QPointF>& positions) {
    if (children.isEmpty())
        return;

    // Compute total span needed
    qreal totalSpan = 0;
    QList<qreal> measures;
    for (int i = 0; i < children.size(); ++i) {
        qreal m = measureSubtree(children[i], axis);
        measures.append(m);
        if (i > 0) totalSpan += axis.spreadSpacing;
        totalSpan += m;
    }

    qreal parentSpread = axis.spread(positions[parent]);
    qreal childDepth = axis.depth(positions[parent]) + axis.depthSpacing * axis.depthDirection;
    qreal cursor = parentSpread - totalSpan / 2;

    for (int i = 0; i < children.size(); ++i) {
        QPointF pos;
        qreal spread = cursor + measures[i] / 2;
        axis.setSpread(pos, spread);
        axis.setDepth(pos, childDepth);
        placeSubtree(children[i], pos, axis, positions);
        cursor += measures[i] + axis.spreadSpacing;
    }
}

// ===========================================================================
// Helper: Collect nodes in BFS order from subtree roots
// ===========================================================================

void LayoutEngine::collectSubtreeNodes(NodeItem* node, QList<NodeItem*>& nodes,
                                        const QMap<NodeItem*, QPointF>& positions) {
    if (!positions.contains(node))
        return;
    nodes.append(node);
    for (auto* child : node->childNodes())
        collectSubtreeNodes(child, nodes, positions);
}

// ===========================================================================
// Phase 3: Force-directed refinement
// ===========================================================================

void LayoutEngine::forceDirectedRefinement(
    NodeItem* root,
    const QList<NodeItem*>& subtreeRoots,
    const LayoutAxis& axis,
    QMap<NodeItem*, QPointF>& positions)
{
    // Collect all nodes from the given subtree roots (BFS order)
    QList<NodeItem*> allNodes;
    allNodes.append(root);
    for (auto* sr : subtreeRoots)
        collectSubtreeNodes(sr, allNodes, positions);

    if (allNodes.size() < 2)
        return;

    // Root is always pinned
    QSet<NodeItem*> pinnedNodes;
    pinnedNodes.insert(root);

    // Save rest positions (the initial tree-computed positions)
    QMap<NodeItem*, QPointF> restPositions;
    for (auto* node : allNodes)
        restPositions[node] = positions[node];

    // Record initial sibling orderings (spread-axis order within each parent)
    QMap<NodeItem*, qreal> initialSpread;
    for (auto* node : allNodes)
        initialSpread[node] = axis.spread(positions[node]);

    // Build parent-to-children map for BFS propagation (only nodes in our set)
    QSet<NodeItem*> nodeSet(allNodes.begin(), allNodes.end());

    qreal temperature = kInitialTemperature;

    for (int iter = 0; iter < kMaxIterations; ++iter) {
        // Accumulate displacements (spread-axis only)
        QMap<NodeItem*, qreal> displacement;
        for (auto* node : allNodes)
            displacement[node] = 0.0;

        // --- 1. Repulsive forces between overlapping pairs ---
        QList<NodeItem*> sorted = allNodes;
        std::sort(sorted.begin(), sorted.end(), [&](NodeItem* a, NodeItem* b) {
            return axis.spread(positions[a]) < axis.spread(positions[b]);
        });

        for (int i = 0; i < sorted.size(); ++i) {
            NodeItem* a = sorted[i];
            QPointF posA = positions[a];
            QRectF rectA = a->nodeRect();

            for (int j = i + 1; j < sorted.size(); ++j) {
                NodeItem* b = sorted[j];
                QPointF posB = positions[b];
                QRectF rectB = b->nodeRect();

                // Early termination
                qreal halfSpanA = axis.nodeSpan(a) / 2;
                qreal halfSpanB = axis.nodeSpan(b) / 2;
                qreal spreadGap = axis.spread(posB) - axis.spread(posA);
                if (spreadGap > halfSpanA + halfSpanB + kVSpacing)
                    break;

                // Full 2D AABB overlap check
                QRectF worldA(posA.x() + rectA.left(), posA.y() + rectA.top(),
                              rectA.width(), rectA.height());
                QRectF worldB(posB.x() + rectB.left(), posB.y() + rectB.top(),
                              rectB.width(), rectB.height());

                worldA.adjust(-kVSpacing / 2, -kVSpacing / 2,
                               kVSpacing / 2,  kVSpacing / 2);

                if (!worldA.intersects(worldB))
                    continue;

                qreal overlap;
                if (axis.spreadIsX) {
                    overlap = worldA.right() - worldB.left();
                } else {
                    overlap = worldA.bottom() - worldB.top();
                }
                if (overlap <= 0)
                    continue;

                qreal force = overlap * kRepulsionStrength;

                bool aPinned = pinnedNodes.contains(a);
                bool bPinned = pinnedNodes.contains(b);

                if (aPinned && bPinned) {
                    continue;
                } else if (aPinned) {
                    displacement[b] += force;
                } else if (bPinned) {
                    displacement[a] -= force;
                } else {
                    displacement[a] -= force / 2;
                    displacement[b] += force / 2;
                }
            }
        }

        // --- 2. Rest-position springs ---
        for (auto* node : allNodes) {
            if (pinnedNodes.contains(node))
                continue;
            qreal rest = axis.spread(restPositions[node]);
            qreal curr = axis.spread(positions[node]);
            displacement[node] += kSpringStrength * (rest - curr);
        }

        // --- 3. Apply displacements top-down (BFS) with subtree propagation ---
        for (auto* node : pinnedNodes)
            displacement[node] = 0.0;

        // Clamp by temperature
        for (auto* node : allNodes) {
            if (pinnedNodes.contains(node)) continue;
            qreal d = displacement[node];
            if (std::abs(d) > temperature)
                displacement[node] = (d > 0) ? temperature : -temperature;
        }

        // Propagate parent displacement to children
        QMap<NodeItem*, qreal> totalDisplacement;
        for (auto* node : allNodes)
            totalDisplacement[node] = 0.0;

        for (auto* node : allNodes) {
            qreal inherited = 0.0;
            NodeItem* par = node->parentNode();
            if (par && totalDisplacement.contains(par) && !pinnedNodes.contains(node)) {
                inherited = totalDisplacement[par];
            }
            if (pinnedNodes.contains(node)) {
                totalDisplacement[node] = 0.0;
            } else {
                totalDisplacement[node] = displacement[node] + inherited;
            }
        }

        // Apply total displacements
        qreal maxDisp = 0.0;
        for (auto* node : allNodes) {
            qreal d = totalDisplacement[node];
            if (std::abs(d) > 1e-6) {
                qreal s = axis.spread(positions[node]);
                axis.setSpread(positions[node], s + d);
            }
            maxDisp = qMax(maxDisp, std::abs(d));
        }

        // --- 4. Enforce sibling ordering constraint ---
        QSet<NodeItem*> processed;
        for (auto* node : allNodes) {
            NodeItem* par = node->parentNode();
            if (!par || processed.contains(par))
                continue;
            processed.insert(par);

            QList<NodeItem*> siblings;
            for (auto* child : par->childNodes()) {
                if (nodeSet.contains(child))
                    siblings.append(child);
            }
            if (siblings.size() < 2)
                continue;

            std::sort(siblings.begin(), siblings.end(), [&](NodeItem* a, NodeItem* b) {
                return initialSpread[a] < initialSpread[b];
            });

            for (int i = 1; i < siblings.size(); ++i) {
                NodeItem* prev = siblings[i - 1];
                NodeItem* curr = siblings[i];

                qreal prevSpread = axis.spread(positions[prev]);
                qreal currSpread = axis.spread(positions[curr]);
                qreal minGap = axis.nodeSpan(prev) / 2 + axis.nodeSpan(curr) / 2 + kVSpacing;

                if (currSpread - prevSpread < minGap) {
                    qreal mid = (prevSpread + currSpread) / 2;
                    axis.setSpread(positions[prev], mid - minGap / 2);
                    qreal shift = (mid + minGap / 2) - currSpread;
                    for (int j = i; j < siblings.size(); ++j) {
                        qreal s = axis.spread(positions[siblings[j]]);
                        axis.setSpread(positions[siblings[j]], s + shift);
                    }
                }
            }
        }

        // --- 5. Cool temperature and check convergence ---
        temperature *= kCoolingFactor;
        if (maxDisp < kConvergenceThreshold)
            break;
    }
}
