#include "LayoutEngine.h"
#include "EdgeItem.h"
#include "NodeItem.h"

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
        break;
    case LayoutStyle::RightTree:
        layoutRightTree(root, edgeFinder, positions);
        break;
    case LayoutStyle::Bilateral:
    default:
        layoutBilateral(root, edgeFinder, positions);
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

    qreal totalH = 0;
    for (int i = 0; i < children.size(); ++i) {
        totalH += subtreeHeight(children[i]);
        if (i < children.size() - 1) {
            totalH += kVSpacing;
        }
    }

    qreal childX = x + direction * kHSpacing;
    qreal childY = y - totalH / 2;

    for (auto* child : children) {
        qreal h = subtreeHeight(child);
        qreal centerY = childY + h / 2;

        EdgeItem* edge = edgeFinder(node, child);
        if (edge && edge->isLocked()) {
            positions[child] = child->pos();
            layoutLockedSubtreeHorizontal(child, direction, edgeFinder, positions);
        } else {
            calculatePositions(child, childX, centerY, direction, edgeFinder, positions);
        }

        childY += h + kVSpacing;
    }
}

void LayoutEngine::layoutLockedSubtreeHorizontal(NodeItem* child, int direction,
                                                 const EdgeFinder& edgeFinder,
                                                 QMap<NodeItem*, QPointF>& positions) {
    QPointF lockedPos = child->pos();
    auto grandchildren = child->childNodes();
    if (grandchildren.isEmpty())
        return;

    qreal gcTotalH = 0;
    for (int i = 0; i < grandchildren.size(); ++i) {
        gcTotalH += subtreeHeight(grandchildren[i]);
        if (i < grandchildren.size() - 1)
            gcTotalH += kVSpacing;
    }
    qreal gcY = lockedPos.y() - gcTotalH / 2;
    for (auto* gc : grandchildren) {
        qreal gcH = subtreeHeight(gc);
        calculatePositions(gc, lockedPos.x() + direction * kHSpacing, gcY + gcH / 2, direction,
                           edgeFinder, positions);
        gcY += gcH + kVSpacing;
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
        for (int i = 0; i < rightChildren.size(); ++i) {
            totalH += subtreeHeight(rightChildren[i]);
            if (i < rightChildren.size() - 1)
                totalH += kVSpacing;
        }
        qreal y = -totalH / 2;
        for (auto* child : rightChildren) {
            EdgeItem* edge = edgeFinder(root, child);
            qreal h = subtreeHeight(child);
            if (edge && edge->isLocked()) {
                positions[child] = child->pos();
                layoutLockedSubtreeHorizontal(child, 1, edgeFinder, positions);
            } else {
                calculatePositions(child, kHSpacing, y + h / 2, 1, edgeFinder, positions);
            }
            y += h + kVSpacing;
        }
    }

    // Layout left children
    if (!leftChildren.isEmpty()) {
        qreal totalH = 0;
        for (int i = 0; i < leftChildren.size(); ++i) {
            totalH += subtreeHeight(leftChildren[i]);
            if (i < leftChildren.size() - 1)
                totalH += kVSpacing;
        }
        qreal y = -totalH / 2;
        for (auto* child : leftChildren) {
            EdgeItem* edge = edgeFinder(root, child);
            qreal h = subtreeHeight(child);
            if (edge && edge->isLocked()) {
                positions[child] = child->pos();
                layoutLockedSubtreeHorizontal(child, -1, edgeFinder, positions);
            } else {
                calculatePositions(child, -kHSpacing, y + h / 2, -1, edgeFinder, positions);
            }
            y += h + kVSpacing;
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
    for (int i = 0; i < children.size(); ++i) {
        totalH += subtreeHeight(children[i]);
        if (i < children.size() - 1)
            totalH += kVSpacing;
    }

    qreal y = -totalH / 2;
    for (auto* child : children) {
        EdgeItem* edge = edgeFinder(root, child);
        qreal h = subtreeHeight(child);
        if (edge && edge->isLocked()) {
            positions[child] = child->pos();
            layoutLockedSubtreeHorizontal(child, 1, edgeFinder, positions);
        } else {
            calculatePositions(child, kHSpacing, y + h / 2, 1, edgeFinder, positions);
        }
        y += h + kVSpacing;
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

    qreal totalW = 0;
    for (int i = 0; i < children.size(); ++i) {
        totalW += subtreeWidth(children[i]);
        if (i < children.size() - 1) {
            totalW += kVSpacing;
        }
    }

    qreal childY = y + kTopDownLevelSpacing;
    qreal childX = x - totalW / 2;

    for (auto* child : children) {
        qreal w = subtreeWidth(child);
        qreal centerX = childX + w / 2;

        EdgeItem* edge = edgeFinder(node, child);
        if (edge && edge->isLocked()) {
            positions[child] = child->pos();
            layoutLockedSubtreeTopDown(child, edgeFinder, positions);
        } else {
            calculatePositionsTopDown(child, centerX, childY, edgeFinder, positions);
        }

        childX += w + kVSpacing;
    }
}

void LayoutEngine::layoutLockedSubtreeTopDown(NodeItem* child, const EdgeFinder& edgeFinder,
                                              QMap<NodeItem*, QPointF>& positions) {
    auto grandchildren = child->childNodes();
    if (grandchildren.isEmpty())
        return;

    qreal gcTotalW = 0;
    for (int i = 0; i < grandchildren.size(); ++i) {
        gcTotalW += subtreeWidth(grandchildren[i]);
        if (i < grandchildren.size() - 1)
            gcTotalW += kVSpacing;
    }
    qreal gcX = child->pos().x() - gcTotalW / 2;
    for (auto* gc : grandchildren) {
        qreal gcW = subtreeWidth(gc);
        calculatePositionsTopDown(gc, gcX + gcW / 2, child->pos().y() + kTopDownLevelSpacing,
                                  edgeFinder, positions);
        gcX += gcW + kVSpacing;
    }
}

// ===========================================================================
// TopDown
// ===========================================================================

void LayoutEngine::layoutTopDown(NodeItem* root, const EdgeFinder& edgeFinder,
                                 QMap<NodeItem*, QPointF>& positions) {
    calculatePositionsTopDown(root, 0, 0, edgeFinder, positions);
}
