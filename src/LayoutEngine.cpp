#include "LayoutEngine.h"
#include "EdgeItem.h"
#include "NodeItem.h"

#include <algorithm>

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
    case LayoutStyle::TopDown:
        layoutTopDown(root, edgeFinder, positions);
        resolveOverlapsTopDown(root, edgeFinder, positions);
        break;
    case LayoutStyle::RightTree:
        layoutRightTree(root, edgeFinder, positions);
        resolveOverlapsHorizontal(root, edgeFinder, positions);
        break;
    case LayoutStyle::Bilateral:
    default:
        layoutBilateral(root, edgeFinder, positions);
        resolveOverlapsHorizontal(root, edgeFinder, positions);
        break;
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
// Horizontal helpers (Bilateral + RightTree)
// ===========================================================================

qreal LayoutEngine::subtreeHeight(NodeItem* node) {
    auto children = node->childNodes();
    if (children.isEmpty()) {
        return kNodeHeight;
    }
    qreal total = 0;
    for (int i = 0; i < children.size(); ++i) {
        total += subtreeHeight(children[i]);
        if (i < children.size() - 1) {
            total += kVSpacing;
        }
    }
    return qMax(kNodeHeight, total);
}

void LayoutEngine::calculatePositions(NodeItem* node, qreal x, qreal y, int direction,
                                      const EdgeFinder& edgeFinder,
                                      QMap<NodeItem*, QPointF>& positions) {
    positions[node] = QPointF(x, y);

    auto children = node->childNodes();
    if (children.isEmpty())
        return;

    // Only sum unlocked children for space allocation
    qreal totalH = 0;
    int unlockedCount = 0;
    for (auto* child : children) {
        EdgeItem* edge = edgeFinder(node, child);
        if (edge && edge->isLocked()) continue;
        if (unlockedCount > 0) totalH += kVSpacing;
        totalH += subtreeHeight(child);
        unlockedCount++;
    }

    qreal childX = x + direction * kHSpacing;
    qreal childY = y - totalH / 2;

    for (auto* child : children) {
        EdgeItem* edge = edgeFinder(node, child);
        if (edge && edge->isLocked()) {
            positions[child] = child->pos();
            layoutLockedSubtreeHorizontal(child, direction, edgeFinder, positions);
        } else {
            qreal h = subtreeHeight(child);
            qreal centerY = childY + h / 2;
            calculatePositions(child, childX, centerY, direction, edgeFinder, positions);
            childY += h + kVSpacing;
        }
    }
}

void LayoutEngine::layoutLockedSubtreeHorizontal(NodeItem* child, int direction,
                                                 const EdgeFinder& edgeFinder,
                                                 QMap<NodeItem*, QPointF>& positions) {
    QPointF lockedPos = child->pos();
    auto grandchildren = child->childNodes();
    if (grandchildren.isEmpty())
        return;

    // Only sum unlocked grandchildren
    qreal gcTotalH = 0;
    int unlockedCount = 0;
    for (auto* gc : grandchildren) {
        EdgeItem* edge = edgeFinder(child, gc);
        if (edge && edge->isLocked()) continue;
        if (unlockedCount > 0) gcTotalH += kVSpacing;
        gcTotalH += subtreeHeight(gc);
        unlockedCount++;
    }
    qreal gcY = lockedPos.y() - gcTotalH / 2;
    for (auto* gc : grandchildren) {
        EdgeItem* edge = edgeFinder(child, gc);
        if (edge && edge->isLocked()) {
            positions[gc] = gc->pos();
            layoutLockedSubtreeHorizontal(gc, direction, edgeFinder, positions);
        } else {
            qreal gcH = subtreeHeight(gc);
            calculatePositions(gc, lockedPos.x() + direction * kHSpacing, gcY + gcH / 2, direction,
                               edgeFinder, positions);
            gcY += gcH + kVSpacing;
        }
    }
}

// ===========================================================================
// Bilateral
// ===========================================================================

void LayoutEngine::layoutBilateral(NodeItem* root, const EdgeFinder& edgeFinder,
                                   QMap<NodeItem*, QPointF>& positions) {
    auto children = root->childNodes();
    QList<NodeItem*> rightChildren, leftChildren;

    for (int i = 0; i < children.size(); ++i) {
        if (i % 2 == 0) {
            rightChildren.append(children[i]);
        } else {
            leftChildren.append(children[i]);
        }
    }

    // Layout right children
    if (!rightChildren.isEmpty()) {
        qreal totalH = 0;
        int unlockedCount = 0;
        for (auto* child : rightChildren) {
            EdgeItem* edge = edgeFinder(root, child);
            if (edge && edge->isLocked()) continue;
            if (unlockedCount > 0) totalH += kVSpacing;
            totalH += subtreeHeight(child);
            unlockedCount++;
        }
        qreal y = -totalH / 2;
        for (auto* child : rightChildren) {
            EdgeItem* edge = edgeFinder(root, child);
            if (edge && edge->isLocked()) {
                positions[child] = child->pos();
                layoutLockedSubtreeHorizontal(child, 1, edgeFinder, positions);
            } else {
                qreal h = subtreeHeight(child);
                calculatePositions(child, kHSpacing, y + h / 2, 1, edgeFinder, positions);
                y += h + kVSpacing;
            }
        }
    }

    // Layout left children
    if (!leftChildren.isEmpty()) {
        qreal totalH = 0;
        int unlockedCount = 0;
        for (auto* child : leftChildren) {
            EdgeItem* edge = edgeFinder(root, child);
            if (edge && edge->isLocked()) continue;
            if (unlockedCount > 0) totalH += kVSpacing;
            totalH += subtreeHeight(child);
            unlockedCount++;
        }
        qreal y = -totalH / 2;
        for (auto* child : leftChildren) {
            EdgeItem* edge = edgeFinder(root, child);
            if (edge && edge->isLocked()) {
                positions[child] = child->pos();
                layoutLockedSubtreeHorizontal(child, -1, edgeFinder, positions);
            } else {
                qreal h = subtreeHeight(child);
                calculatePositions(child, -kHSpacing, y + h / 2, -1, edgeFinder, positions);
                y += h + kVSpacing;
            }
        }
    }
}

// ===========================================================================
// RightTree
// ===========================================================================

void LayoutEngine::layoutRightTree(NodeItem* root, const EdgeFinder& edgeFinder,
                                   QMap<NodeItem*, QPointF>& positions) {
    auto children = root->childNodes();
    if (children.isEmpty())
        return;

    qreal totalH = 0;
    int unlockedCount = 0;
    for (auto* child : children) {
        EdgeItem* edge = edgeFinder(root, child);
        if (edge && edge->isLocked()) continue;
        if (unlockedCount > 0) totalH += kVSpacing;
        totalH += subtreeHeight(child);
        unlockedCount++;
    }

    qreal y = -totalH / 2;
    for (auto* child : children) {
        EdgeItem* edge = edgeFinder(root, child);
        if (edge && edge->isLocked()) {
            positions[child] = child->pos();
            layoutLockedSubtreeHorizontal(child, 1, edgeFinder, positions);
        } else {
            qreal h = subtreeHeight(child);
            calculatePositions(child, kHSpacing, y + h / 2, 1, edgeFinder, positions);
            y += h + kVSpacing;
        }
    }
}

// ===========================================================================
// TopDown helpers
// ===========================================================================

qreal LayoutEngine::subtreeWidth(NodeItem* node) {
    auto children = node->childNodes();
    if (children.isEmpty()) {
        return node->nodeRect().width();
    }
    qreal total = 0;
    for (int i = 0; i < children.size(); ++i) {
        total += subtreeWidth(children[i]);
        if (i < children.size() - 1) {
            total += kVSpacing;
        }
    }
    return qMax(node->nodeRect().width(), total);
}

void LayoutEngine::calculatePositionsTopDown(NodeItem* node, qreal x, qreal y,
                                             const EdgeFinder& edgeFinder,
                                             QMap<NodeItem*, QPointF>& positions) {
    positions[node] = QPointF(x, y);

    auto children = node->childNodes();
    if (children.isEmpty())
        return;

    // Only sum unlocked children for space allocation
    qreal totalW = 0;
    int unlockedCount = 0;
    for (auto* child : children) {
        EdgeItem* edge = edgeFinder(node, child);
        if (edge && edge->isLocked()) continue;
        if (unlockedCount > 0) totalW += kVSpacing;
        totalW += subtreeWidth(child);
        unlockedCount++;
    }

    qreal childY = y + kTopDownLevelSpacing;
    qreal childX = x - totalW / 2;

    for (auto* child : children) {
        EdgeItem* edge = edgeFinder(node, child);
        if (edge && edge->isLocked()) {
            positions[child] = child->pos();
            layoutLockedSubtreeTopDown(child, edgeFinder, positions);
        } else {
            qreal w = subtreeWidth(child);
            qreal centerX = childX + w / 2;
            calculatePositionsTopDown(child, centerX, childY, edgeFinder, positions);
            childX += w + kVSpacing;
        }
    }
}

void LayoutEngine::layoutLockedSubtreeTopDown(NodeItem* child, const EdgeFinder& edgeFinder,
                                              QMap<NodeItem*, QPointF>& positions) {
    auto grandchildren = child->childNodes();
    if (grandchildren.isEmpty())
        return;

    // Only sum unlocked grandchildren
    qreal gcTotalW = 0;
    int unlockedCount = 0;
    for (auto* gc : grandchildren) {
        EdgeItem* edge = edgeFinder(child, gc);
        if (edge && edge->isLocked()) continue;
        if (unlockedCount > 0) gcTotalW += kVSpacing;
        gcTotalW += subtreeWidth(gc);
        unlockedCount++;
    }
    qreal gcX = child->pos().x() - gcTotalW / 2;
    for (auto* gc : grandchildren) {
        EdgeItem* edge = edgeFinder(child, gc);
        if (edge && edge->isLocked()) {
            positions[gc] = gc->pos();
            layoutLockedSubtreeTopDown(gc, edgeFinder, positions);
        } else {
            qreal gcW = subtreeWidth(gc);
            calculatePositionsTopDown(gc, gcX + gcW / 2, child->pos().y() + kTopDownLevelSpacing,
                                      edgeFinder, positions);
            gcX += gcW + kVSpacing;
        }
    }
}

// ===========================================================================
// TopDown
// ===========================================================================

void LayoutEngine::layoutTopDown(NodeItem* root, const EdgeFinder& edgeFinder,
                                 QMap<NodeItem*, QPointF>& positions) {
    calculatePositionsTopDown(root, 0, 0, edgeFinder, positions);
}

// ===========================================================================
// Post-layout overlap resolution
// ===========================================================================

QPair<qreal, qreal> LayoutEngine::subtreeYExtent(NodeItem* node,
                                                   const QMap<NodeItem*, QPointF>& positions) {
    if (!positions.contains(node))
        return {0, 0};
    qreal minY = positions[node].y() - kNodeHeight / 2;
    qreal maxY = positions[node].y() + kNodeHeight / 2;
    for (auto* child : node->childNodes()) {
        if (!positions.contains(child)) continue;
        auto ext = subtreeYExtent(child, positions);
        minY = qMin(minY, ext.first);
        maxY = qMax(maxY, ext.second);
    }
    return {minY, maxY};
}

QPair<qreal, qreal> LayoutEngine::subtreeXExtent(NodeItem* node,
                                                   const QMap<NodeItem*, QPointF>& positions) {
    if (!positions.contains(node))
        return {0, 0};
    qreal halfW = node->nodeRect().width() / 2;
    qreal minX = positions[node].x() - halfW;
    qreal maxX = positions[node].x() + halfW;
    for (auto* child : node->childNodes()) {
        if (!positions.contains(child)) continue;
        auto ext = subtreeXExtent(child, positions);
        minX = qMin(minX, ext.first);
        maxX = qMax(maxX, ext.second);
    }
    return {minX, maxX};
}

void LayoutEngine::shiftSubtreePositions(NodeItem* node, qreal delta, bool isTopDown,
                                          const EdgeFinder& edgeFinder,
                                          QMap<NodeItem*, QPointF>& positions) {
    if (!positions.contains(node)) return;
    if (isTopDown) {
        positions[node].rx() += delta;
    } else {
        positions[node].ry() += delta;
    }
    for (auto* child : node->childNodes()) {
        EdgeItem* edge = edgeFinder(node, child);
        if (edge && edge->isLocked()) continue;
        shiftSubtreePositions(child, delta, isTopDown, edgeFinder, positions);
    }
}

void LayoutEngine::resolveOverlapGroup(QList<NodeItem*> siblings, NodeItem* parent,
                                        const EdgeFinder& edgeFinder,
                                        QMap<NodeItem*, QPointF>& positions, bool isTopDown) {
    if (siblings.size() < 2) return;

    // Sort siblings by position (Y for horizontal, X for top-down)
    std::sort(siblings.begin(), siblings.end(), [&](NodeItem* a, NodeItem* b) {
        if (isTopDown)
            return positions[a].x() < positions[b].x();
        else
            return positions[a].y() < positions[b].y();
    });

    // Forward sweep
    for (int i = 1; i < siblings.size(); ++i) {
        auto* prev = siblings[i - 1];
        auto* curr = siblings[i];

        qreal prevMax, currMin;
        if (isTopDown) {
            prevMax = subtreeXExtent(prev, positions).second;
            currMin = subtreeXExtent(curr, positions).first;
        } else {
            prevMax = subtreeYExtent(prev, positions).second;
            currMin = subtreeYExtent(curr, positions).first;
        }

        qreal gap = currMin - prevMax;
        if (gap < kVSpacing) {
            qreal shift = kVSpacing - gap;
            EdgeItem* currEdge = edgeFinder(parent, curr);
            bool currLocked = currEdge && currEdge->isLocked();

            if (!currLocked) {
                shiftSubtreePositions(curr, shift, isTopDown, edgeFinder, positions);
            } else {
                EdgeItem* prevEdge = edgeFinder(parent, prev);
                bool prevLocked = prevEdge && prevEdge->isLocked();
                if (!prevLocked) {
                    shiftSubtreePositions(prev, -shift, isTopDown, edgeFinder, positions);
                }
            }
        }
    }

    // Backward sweep
    for (int i = siblings.size() - 2; i >= 0; --i) {
        auto* curr = siblings[i];
        auto* next = siblings[i + 1];

        qreal currMax, nextMin;
        if (isTopDown) {
            currMax = subtreeXExtent(curr, positions).second;
            nextMin = subtreeXExtent(next, positions).first;
        } else {
            currMax = subtreeYExtent(curr, positions).second;
            nextMin = subtreeYExtent(next, positions).first;
        }

        qreal gap = nextMin - currMax;
        if (gap < kVSpacing) {
            qreal shift = kVSpacing - gap;
            EdgeItem* currEdge = edgeFinder(parent, curr);
            bool currLocked = currEdge && currEdge->isLocked();

            if (!currLocked) {
                shiftSubtreePositions(curr, -shift, isTopDown, edgeFinder, positions);
            } else {
                EdgeItem* nextEdge = edgeFinder(parent, next);
                bool nextLocked = nextEdge && nextEdge->isLocked();
                if (!nextLocked) {
                    shiftSubtreePositions(next, shift, isTopDown, edgeFinder, positions);
                }
            }
        }
    }
}

void LayoutEngine::resolveOverlapsHorizontal(NodeItem* node, const EdgeFinder& edgeFinder,
                                              QMap<NodeItem*, QPointF>& positions) {
    auto children = node->childNodes();
    if (children.size() >= 2) {
        // Split children by side (right vs left of parent)
        QList<NodeItem*> rightGroup, leftGroup;
        qreal parentX = positions.contains(node) ? positions[node].x() : 0;
        for (auto* child : children) {
            if (!positions.contains(child)) continue;
            if (positions[child].x() >= parentX)
                rightGroup.append(child);
            else
                leftGroup.append(child);
        }
        if (rightGroup.size() >= 2)
            resolveOverlapGroup(rightGroup, node, edgeFinder, positions, false);
        if (leftGroup.size() >= 2)
            resolveOverlapGroup(leftGroup, node, edgeFinder, positions, false);
    }
    // Recurse into children
    for (auto* child : children) {
        resolveOverlapsHorizontal(child, edgeFinder, positions);
    }
}

void LayoutEngine::resolveOverlapsTopDown(NodeItem* node, const EdgeFinder& edgeFinder,
                                           QMap<NodeItem*, QPointF>& positions) {
    auto children = node->childNodes();
    if (children.size() >= 2) {
        resolveOverlapGroup(children, node, edgeFinder, positions, true);
    }
    for (auto* child : children) {
        resolveOverlapsTopDown(child, edgeFinder, positions);
    }
}
