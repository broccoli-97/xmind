#include "layout/LayoutAlgorithmBase.h"
#include "scene/NodeItem.h"

#include <QQueue>
#include <algorithm>
#include <cmath>

// ===========================================================================
// LayoutAxis
// ===========================================================================

qreal LayoutAlgorithmBase::LayoutAxis::nodeSpan(NodeItem* node) const {
    if (spreadIsX)
        return node->nodeRect().width();
    else
        return qMax(kNodeHeight, node->nodeRect().height());
}

qreal LayoutAlgorithmBase::LayoutAxis::nodeDepthSpan(NodeItem* node) const {
    if (spreadIsX)
        return qMax(kNodeHeight, node->nodeRect().height());
    else
        return node->nodeRect().width();
}

LayoutAlgorithmBase::LayoutAxis LayoutAlgorithmBase::makeRightAxis(const LayoutParams& p) {
    return {false, p.depthSpacing, p.spreadSpacing, +1};
}

LayoutAlgorithmBase::LayoutAxis LayoutAlgorithmBase::makeLeftAxis(const LayoutParams& p) {
    return {false, p.depthSpacing, p.spreadSpacing, -1};
}

LayoutAlgorithmBase::LayoutAxis LayoutAlgorithmBase::makeTopDownAxis(const LayoutParams& p) {
    // TopDown uses a smaller depth spacing (56% of the configured value to match
    // the original kTopDownGap=56 vs kHGap=100 ratio)
    return {true, p.depthSpacing * 0.56, p.spreadSpacing, +1};
}

// ===========================================================================
// Helpers for initial placement
// ===========================================================================

void LayoutAlgorithmBase::collectAllNodes(NodeItem* node, QList<NodeItem*>& nodes) {
    if (!node)
        return;
    nodes.append(node);
    for (auto* child : node->childNodes())
        collectAllNodes(child, nodes);
}

qreal LayoutAlgorithmBase::findAvailableSpread(qreal candidateSpread, qreal depth,
                                                NodeItem* newNode,
                                                const QList<NodeItem*>& allNodes,
                                                const LayoutAxis& axis) {
    QRectF newRect = newNode->nodeRect();

    for (int attempt = 0; attempt < 50; ++attempt) {
        QPointF candidatePos;
        axis.setSpread(candidatePos, candidateSpread);
        axis.setDepth(candidatePos, depth);

        QRectF candidateWorld(candidatePos.x() + newRect.left(),
                              candidatePos.y() + newRect.top(),
                              newRect.width(), newRect.height());
        candidateWorld.adjust(-axis.spreadSpacing / 2, -axis.spreadSpacing / 2,
                               axis.spreadSpacing / 2,  axis.spreadSpacing / 2);

        bool hasOverlap = false;
        qreal maxShift = 0;

        for (auto* node : allNodes) {
            QRectF nodeRect = node->nodeRect();
            QPointF nodePos = node->pos();
            QRectF nodeWorld(nodePos.x() + nodeRect.left(),
                             nodePos.y() + nodeRect.top(),
                             nodeRect.width(), nodeRect.height());

            if (!candidateWorld.intersects(nodeWorld))
                continue;

            hasOverlap = true;
            qreal shift;
            if (axis.spreadIsX) {
                shift = nodeWorld.right() - candidateWorld.left() + axis.spreadSpacing;
            } else {
                shift = nodeWorld.bottom() - candidateWorld.top() + axis.spreadSpacing;
            }
            if (shift > maxShift)
                maxShift = shift;
        }

        if (!hasOverlap)
            break;

        candidateSpread += maxShift;
    }

    return candidateSpread;
}

// ===========================================================================
// Phase 1: Measure (bottom-up)
// ===========================================================================

qreal LayoutAlgorithmBase::measureSubtree(NodeItem* node, const LayoutAxis& axis) {
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

void LayoutAlgorithmBase::placeSubtree(NodeItem* node, QPointF position,
                                        const LayoutAxis& axis,
                                        QMap<NodeItem*, QPointF>& positions) {
    positions[node] = position;
    placeChildGroup(node, node->childNodes(), axis, positions);
}

void LayoutAlgorithmBase::placeChildGroup(NodeItem* parent,
                                           const QList<NodeItem*>& children,
                                           const LayoutAxis& axis,
                                           QMap<NodeItem*, QPointF>& positions) {
    if (children.isEmpty())
        return;

    qreal totalSpan = 0;
    QList<qreal> measures;
    for (int i = 0; i < children.size(); ++i) {
        qreal m = measureSubtree(children[i], axis);
        measures.append(m);
        if (i > 0) totalSpan += axis.spreadSpacing;
        totalSpan += m;
    }

    qreal parentSpread = axis.spread(positions[parent]);
    qreal parentDepth = axis.depth(positions[parent]);
    qreal parentHalfDepth = axis.nodeDepthSpan(parent) / 2;
    qreal cursor = parentSpread - totalSpan / 2;

    for (int i = 0; i < children.size(); ++i) {
        QPointF pos;
        qreal spread = cursor + measures[i] / 2;
        qreal childHalfDepth = axis.nodeDepthSpan(children[i]) / 2;
        qreal childDepth = parentDepth
            + axis.depthDirection * (parentHalfDepth + axis.depthSpacing + childHalfDepth);
        axis.setSpread(pos, spread);
        axis.setDepth(pos, childDepth);
        placeSubtree(children[i], pos, axis, positions);
        cursor += measures[i] + axis.spreadSpacing;
    }
}

// ===========================================================================
// Helper: Collect nodes in BFS order from subtree roots
// ===========================================================================

void LayoutAlgorithmBase::collectSubtreeNodes(NodeItem* node, QList<NodeItem*>& nodes,
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

void LayoutAlgorithmBase::forceDirectedRefinement(
    NodeItem* root,
    const QList<NodeItem*>& subtreeRoots,
    const LayoutAxis& axis,
    QMap<NodeItem*, QPointF>& positions)
{
    QList<NodeItem*> allNodes;
    allNodes.append(root);
    for (auto* sr : subtreeRoots)
        collectSubtreeNodes(sr, allNodes, positions);

    if (allNodes.size() < 2)
        return;

    QSet<NodeItem*> pinnedNodes;
    pinnedNodes.insert(root);

    QMap<NodeItem*, QPointF> restPositions;
    for (auto* node : allNodes)
        restPositions[node] = positions[node];

    QMap<NodeItem*, qreal> initialSpread;
    for (auto* node : allNodes)
        initialSpread[node] = axis.spread(positions[node]);

    QSet<NodeItem*> nodeSet(allNodes.begin(), allNodes.end());

    qreal temperature = kInitialTemperature;

    for (int iter = 0; iter < kMaxIterations; ++iter) {
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

                qreal halfSpanA = axis.nodeSpan(a) / 2;
                qreal halfSpanB = axis.nodeSpan(b) / 2;
                qreal spreadGap = axis.spread(posB) - axis.spread(posA);
                if (spreadGap > halfSpanA + halfSpanB + axis.spreadSpacing)
                    break;

                QRectF worldA(posA.x() + rectA.left(), posA.y() + rectA.top(),
                              rectA.width(), rectA.height());
                QRectF worldB(posB.x() + rectB.left(), posB.y() + rectB.top(),
                              rectB.width(), rectB.height());

                worldA.adjust(-axis.spreadSpacing / 2, -axis.spreadSpacing / 2,
                               axis.spreadSpacing / 2,  axis.spreadSpacing / 2);

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

        for (auto* node : allNodes) {
            if (pinnedNodes.contains(node)) continue;
            qreal d = displacement[node];
            if (std::abs(d) > temperature)
                displacement[node] = (d > 0) ? temperature : -temperature;
        }

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
                qreal minGap = axis.nodeSpan(prev) / 2 + axis.nodeSpan(curr) / 2
                               + axis.spreadSpacing;

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
