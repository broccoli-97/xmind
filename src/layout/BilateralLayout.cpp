#include "layout/BilateralLayout.h"
#include "scene/NodeItem.h"

QString BilateralLayout::name() const { return QStringLiteral("bilateral"); }
QString BilateralLayout::displayName() const { return QStringLiteral("Bilateral"); }

QMap<NodeItem*, QPointF> BilateralLayout::computeLayout(NodeItem* root,
                                                         const LayoutParams& p) const {
    QMap<NodeItem*, QPointF> positions;
    if (!root)
        return positions;

    positions[root] = QPointF(0, 0);

    auto children = root->childNodes();
    QList<NodeItem*> rightChildren, leftChildren;
    for (int i = 0; i < children.size(); ++i) {
        if (i % 2 == 0)
            rightChildren.append(children[i]);
        else
            leftChildren.append(children[i]);
    }

    LayoutAxis rightAxis = makeRightAxis(p);
    LayoutAxis leftAxis = makeLeftAxis(p);

    placeChildGroup(root, rightChildren, rightAxis, positions);
    placeChildGroup(root, leftChildren, leftAxis, positions);

    forceDirectedRefinement(root, rightChildren, rightAxis, positions);
    forceDirectedRefinement(root, leftChildren, leftAxis, positions);

    return positions;
}

QPointF BilateralLayout::initialChildPosition(NodeItem* newNode, NodeItem* parent,
                                               NodeItem* root,
                                               const LayoutParams& p) const {
    QPointF parentPos = parent->pos();
    auto allChildren = parent->childNodes();

    QList<NodeItem*> existingSiblings;
    for (auto* child : allChildren) {
        if (child != newNode)
            existingSiblings.append(child);
    }

    QList<NodeItem*> allNodes;
    collectAllNodes(root, allNodes);
    allNodes.removeOne(newNode);

    // Determine side
    qreal xDir;
    if (parent == root) {
        int newIndex = allChildren.indexOf(newNode);
        xDir = (newIndex % 2 == 0) ? 1.0 : -1.0;
    } else {
        xDir = (parentPos.x() >= 0) ? 1.0 : -1.0;
    }
    LayoutAxis axis = (xDir > 0) ? makeRightAxis(p) : makeLeftAxis(p);

    // Only consider siblings on the same side
    QList<NodeItem*> relevantSiblings;
    for (auto* sib : existingSiblings) {
        if ((xDir > 0 && sib->pos().x() > parentPos.x()) ||
            (xDir < 0 && sib->pos().x() < parentPos.x()))
            relevantSiblings.append(sib);
    }

    qreal parentHalfDepth = axis.nodeDepthSpan(parent) / 2;
    qreal newNodeHalfDepth = axis.nodeDepthSpan(newNode) / 2;
    qreal depth = axis.depth(parentPos)
        + axis.depthDirection * (parentHalfDepth + axis.depthSpacing + newNodeHalfDepth);

    qreal spread = axis.spread(parentPos);

    if (!relevantSiblings.isEmpty()) {
        qreal maxSpreadEnd = -1e18;
        for (auto* sib : relevantSiblings) {
            qreal s = axis.spread(sib->pos());
            qreal halfSpan = axis.nodeSpan(sib) / 2;
            qreal end = s + halfSpan;
            if (end > maxSpreadEnd)
                maxSpreadEnd = end;
        }
        spread = maxSpreadEnd + p.spreadSpacing + axis.nodeSpan(newNode) / 2;
    }

    spread = findAvailableSpread(spread, depth, newNode, allNodes, axis);

    QPointF pos;
    axis.setSpread(pos, spread);
    axis.setDepth(pos, depth);
    return pos;
}
