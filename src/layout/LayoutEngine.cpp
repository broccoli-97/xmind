#include "layout/LayoutEngine.h"
#include "scene/EdgeItem.h"
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

QMap<NodeItem*, QPointF> LayoutEngine::computeLayout(NodeItem* root, LayoutStyle style,
                                                     const EdgeFinder& edgeFinder) {
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

        placeChildGroup(root, rightChildren, rightAxis, edgeFinder, positions);
        placeChildGroup(root, leftChildren, leftAxis, edgeFinder, positions);

        forceDirectedRefinement(root, rightChildren, rightAxis, edgeFinder, positions);
        forceDirectedRefinement(root, leftChildren, leftAxis, edgeFinder, positions);
        break;
    }
    case LayoutStyle::RightTree: {
        LayoutAxis axis = makeRightAxis();
        placeSubtree(root, QPointF(0, 0), axis, edgeFinder, positions);
        forceDirectedRefinement(root, root->childNodes(), axis, edgeFinder, positions);
        break;
    }
    case LayoutStyle::TopDown: {
        LayoutAxis axis = makeTopDownAxis();
        placeSubtree(root, QPointF(0, 0), axis, edgeFinder, positions);
        forceDirectedRefinement(root, root->childNodes(), axis, edgeFinder, positions);
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

qreal LayoutEngine::measureSubtree(NodeItem* node, const LayoutAxis& axis,
                                    const EdgeFinder& edgeFinder) {
    auto children = node->childNodes();
    qreal selfSpan = axis.nodeSpan(node);
    if (children.isEmpty())
        return selfSpan;

    qreal total = 0;
    int unlockedCount = 0;
    for (auto* child : children) {
        EdgeItem* edge = edgeFinder(node, child);
        if (edge && edge->isLocked()) continue;
        if (unlockedCount > 0) total += axis.spreadSpacing;
        total += measureSubtree(child, axis, edgeFinder);
        unlockedCount++;
    }

    return qMax(selfSpan, total);
}

// ===========================================================================
// Phase 2: Place (top-down)
// ===========================================================================

void LayoutEngine::placeSubtree(NodeItem* node, QPointF position, const LayoutAxis& axis,
                                 const EdgeFinder& edgeFinder,
                                 QMap<NodeItem*, QPointF>& positions) {
    positions[node] = position;
    placeChildGroup(node, node->childNodes(), axis, edgeFinder, positions);
}

void LayoutEngine::placeChildGroup(NodeItem* parent, const QList<NodeItem*>& children,
                                    const LayoutAxis& axis, const EdgeFinder& edgeFinder,
                                    QMap<NodeItem*, QPointF>& positions) {
    if (children.isEmpty())
        return;

    // --- Step 1: Separate locked and unlocked children ---
    struct ChildInfo {
        NodeItem* node;
        qreal measure;
        qreal spread;
        bool locked;
        int unlockedIndex; // index into unlockedInfos, -1 for locked
    };

    QList<ChildInfo> lockedInfos;
    QList<ChildInfo> unlockedInfos;

    for (auto* child : children) {
        EdgeItem* edge = edgeFinder(parent, child);
        if (edge && edge->isLocked()) {
            positions[child] = child->pos();
            placeChildGroup(child, child->childNodes(), axis, edgeFinder, positions);
            qreal m = measureSubtree(child, axis, edgeFinder);
            lockedInfos.append({child, m, axis.spread(positions[child]), true, -1});
        }
    }

    int idx = 0;
    for (auto* child : children) {
        EdgeItem* edge = edgeFinder(parent, child);
        if (edge && edge->isLocked()) continue;
        qreal m = measureSubtree(child, axis, edgeFinder);
        unlockedInfos.append({child, m, 0.0, false, idx++});
    }

    if (unlockedInfos.isEmpty())
        return;

    // --- Step 2: Compute initial positions for unlocked children (centered) ---
    qreal totalSpan = 0;
    for (int i = 0; i < unlockedInfos.size(); ++i) {
        if (i > 0) totalSpan += axis.spreadSpacing;
        totalSpan += unlockedInfos[i].measure;
    }

    qreal parentSpread = axis.spread(positions[parent]);
    qreal childDepth = axis.depth(positions[parent]) + axis.depthSpacing * axis.depthDirection;
    qreal cursor = parentSpread - totalSpan / 2;

    for (int i = 0; i < unlockedInfos.size(); ++i) {
        unlockedInfos[i].spread = cursor + unlockedInfos[i].measure / 2;
        cursor += unlockedInfos[i].measure + axis.spreadSpacing;
    }

    // --- Step 3: Resolve overlaps between locked and unlocked siblings ---
    if (!lockedInfos.isEmpty()) {
        // Build merged list of all siblings
        QList<ChildInfo*> allSiblings;
        for (auto& info : lockedInfos)
            allSiblings.append(&info);
        for (auto& info : unlockedInfos)
            allSiblings.append(&info);

        // Sort by current spread position
        std::sort(allSiblings.begin(), allSiblings.end(),
                  [](const ChildInfo* a, const ChildInfo* b) {
                      return a->spread < b->spread;
                  });

        // Forward pass: enforce minimum gaps
        for (int i = 1; i < allSiblings.size(); ++i) {
            ChildInfo* prev = allSiblings[i - 1];
            ChildInfo* curr = allSiblings[i];
            qreal minGap = prev->measure / 2 + curr->measure / 2 + axis.spreadSpacing;

            if (curr->spread - prev->spread < minGap) {
                if (prev->locked && curr->locked) {
                    continue; // both locked, can't adjust
                } else if (prev->locked) {
                    // Shift curr and all subsequent unlocked siblings
                    qreal shift = prev->spread + minGap - curr->spread;
                    for (int j = i; j < allSiblings.size(); ++j) {
                        if (!allSiblings[j]->locked)
                            allSiblings[j]->spread += shift;
                    }
                } else if (curr->locked) {
                    prev->spread = curr->spread - minGap;
                } else {
                    qreal mid = (prev->spread + curr->spread) / 2;
                    prev->spread = mid - minGap / 2;
                    qreal shift = (mid + minGap / 2) - curr->spread;
                    for (int j = i; j < allSiblings.size(); ++j) {
                        if (!allSiblings[j]->locked)
                            allSiblings[j]->spread += shift;
                    }
                }
            }
        }

        // Backward pass: fix remaining issues from forward pass pushing prev backward
        for (int i = allSiblings.size() - 2; i >= 0; --i) {
            ChildInfo* curr = allSiblings[i];
            ChildInfo* next = allSiblings[i + 1];
            qreal minGap = curr->measure / 2 + next->measure / 2 + axis.spreadSpacing;

            if (next->spread - curr->spread < minGap) {
                if (curr->locked && next->locked) {
                    continue;
                } else if (next->locked) {
                    curr->spread = next->spread - minGap;
                } else if (curr->locked) {
                    qreal shift = curr->spread + minGap - next->spread;
                    for (int j = i + 1; j < allSiblings.size(); ++j) {
                        if (!allSiblings[j]->locked)
                            allSiblings[j]->spread += shift;
                    }
                } else {
                    qreal mid = (curr->spread + next->spread) / 2;
                    curr->spread = mid - minGap / 2;
                    qreal shift = (mid + minGap / 2) - next->spread;
                    for (int j = i + 1; j < allSiblings.size(); ++j) {
                        if (!allSiblings[j]->locked)
                            allSiblings[j]->spread += shift;
                    }
                }
            }
        }
    }

    // --- Step 4: Place unlocked children at resolved positions ---
    for (auto& info : unlockedInfos) {
        QPointF pos;
        axis.setSpread(pos, info.spread);
        axis.setDepth(pos, childDepth);
        placeSubtree(info.node, pos, axis, edgeFinder, positions);
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
    const EdgeFinder& edgeFinder,
    QMap<NodeItem*, QPointF>& positions)
{
    // Collect all nodes from the given subtree roots (BFS order)
    QList<NodeItem*> allNodes;
    allNodes.append(root);
    for (auto* sr : subtreeRoots)
        collectSubtreeNodes(sr, allNodes, positions);

    if (allNodes.size() < 2)
        return;

    // Build locked set: root is always pinned; nodes whose incoming edge is locked
    QSet<NodeItem*> lockedNodes;
    lockedNodes.insert(root);
    for (auto* node : allNodes) {
        if (node == root) continue;
        NodeItem* par = node->parentNode();
        if (par) {
            EdgeItem* edge = edgeFinder(par, node);
            if (edge && edge->isLocked())
                lockedNodes.insert(node);
        }
    }

    // Save rest positions (the initial tree-computed positions)
    QMap<NodeItem*, QPointF> restPositions;
    for (auto* node : allNodes)
        restPositions[node] = positions[node];

    // Record initial sibling orderings (spread-axis order within each parent)
    // We store the initial spread value for each node to enforce ordering constraints
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
        // Sort by spread axis for sweep-based early termination
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

                // Early termination: if b is far enough on spread axis, no overlap
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

                // Inflate A by spacing
                worldA.adjust(-kVSpacing / 2, -kVSpacing / 2,
                               kVSpacing / 2,  kVSpacing / 2);

                if (!worldA.intersects(worldB))
                    continue;

                // Compute overlap on spread axis
                qreal overlap;
                if (axis.spreadIsX) {
                    overlap = worldA.right() - worldB.left();
                } else {
                    overlap = worldA.bottom() - worldB.top();
                }
                if (overlap <= 0)
                    continue;

                qreal force = overlap * kRepulsionStrength;

                bool aLocked = lockedNodes.contains(a);
                bool bLocked = lockedNodes.contains(b);

                if (aLocked && bLocked) {
                    // Both locked — nothing we can do
                    continue;
                } else if (aLocked) {
                    displacement[b] += force;
                } else if (bLocked) {
                    displacement[a] -= force;
                } else {
                    displacement[a] -= force / 2;
                    displacement[b] += force / 2;
                }
            }
        }

        // --- 2. Rest-position springs ---
        for (auto* node : allNodes) {
            if (lockedNodes.contains(node))
                continue;
            qreal rest = axis.spread(restPositions[node]);
            qreal curr = axis.spread(positions[node]);
            displacement[node] += kSpringStrength * (rest - curr);
        }

        // --- 3. Apply displacements top-down (BFS) with subtree propagation ---
        // Zero out locked node displacements
        for (auto* node : lockedNodes)
            displacement[node] = 0.0;

        // Clamp by temperature
        for (auto* node : allNodes) {
            if (lockedNodes.contains(node)) continue;
            qreal d = displacement[node];
            if (std::abs(d) > temperature)
                displacement[node] = (d > 0) ? temperature : -temperature;
        }

        // Propagate parent displacement to children (BFS order — allNodes is already
        // collected in parent-before-child order from collectSubtreeNodes)
        QMap<NodeItem*, qreal> totalDisplacement;
        for (auto* node : allNodes)
            totalDisplacement[node] = 0.0;

        for (auto* node : allNodes) {
            qreal inherited = 0.0;
            NodeItem* par = node->parentNode();
            if (par && totalDisplacement.contains(par) && !lockedNodes.contains(node)) {
                // Check if edge from parent to this node is locked
                EdgeItem* edge = edgeFinder(par, node);
                if (!(edge && edge->isLocked())) {
                    inherited = totalDisplacement[par];
                }
            }
            if (lockedNodes.contains(node)) {
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
        // For each parent in the node set, check that children maintain their
        // initial relative order on the spread axis
        QSet<NodeItem*> processed;
        for (auto* node : allNodes) {
            NodeItem* par = node->parentNode();
            if (!par || processed.contains(par))
                continue;
            processed.insert(par);

            // Gather siblings of this node that are in our node set
            QList<NodeItem*> siblings;
            for (auto* child : par->childNodes()) {
                if (nodeSet.contains(child))
                    siblings.append(child);
            }
            if (siblings.size() < 2)
                continue;

            // Sort siblings by their initial spread order
            std::sort(siblings.begin(), siblings.end(), [&](NodeItem* a, NodeItem* b) {
                return initialSpread[a] < initialSpread[b];
            });

            // Enforce: each sibling's spread position must maintain the initial ordering.
            // When shifting a sibling, shift ALL subsequent unlocked siblings by the
            // same amount to prevent chain reactions that push the last sibling far away.
            for (int i = 1; i < siblings.size(); ++i) {
                NodeItem* prev = siblings[i - 1];
                NodeItem* curr = siblings[i];

                if (lockedNodes.contains(prev) && lockedNodes.contains(curr))
                    continue;

                qreal prevSpread = axis.spread(positions[prev]);
                qreal currSpread = axis.spread(positions[curr]);
                qreal minGap = axis.nodeSpan(prev) / 2 + axis.nodeSpan(curr) / 2 + kVSpacing;

                if (currSpread - prevSpread < minGap) {
                    if (lockedNodes.contains(prev)) {
                        qreal shift = prevSpread + minGap - currSpread;
                        for (int j = i; j < siblings.size(); ++j) {
                            if (!lockedNodes.contains(siblings[j])) {
                                qreal s = axis.spread(positions[siblings[j]]);
                                axis.setSpread(positions[siblings[j]], s + shift);
                            }
                        }
                    } else if (lockedNodes.contains(curr)) {
                        axis.setSpread(positions[prev], currSpread - minGap);
                    } else {
                        qreal mid = (prevSpread + currSpread) / 2;
                        axis.setSpread(positions[prev], mid - minGap / 2);
                        qreal shift = (mid + minGap / 2) - currSpread;
                        for (int j = i; j < siblings.size(); ++j) {
                            if (!lockedNodes.contains(siblings[j])) {
                                qreal s = axis.spread(positions[siblings[j]]);
                                axis.setSpread(positions[siblings[j]], s + shift);
                            }
                        }
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
