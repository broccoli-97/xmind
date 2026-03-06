#include "layout/TopDownLayout.h"
#include "scene/NodeItem.h"

QString TopDownLayout::name() const { return QStringLiteral("topdown"); }
QString TopDownLayout::displayName() const { return QStringLiteral("Top Down"); }

QMap<NodeItem*, QPointF> TopDownLayout::computeLayout(NodeItem* root,
                                                       const LayoutParams& p) const {
    QMap<NodeItem*, QPointF> positions;
    if (!root)
        return positions;

    positions[root] = QPointF(0, 0);

    LayoutAxis axis = makeTopDownAxis(p);
    placeSubtree(root, QPointF(0, 0), axis, positions);
    forceDirectedRefinement(root, root->childNodes(), axis, positions);

    return positions;
}

QPointF TopDownLayout::initialChildPosition(NodeItem* newNode, NodeItem* parent,
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

    LayoutAxis axis = makeTopDownAxis(p);

    qreal parentHalfDepth = axis.nodeDepthSpan(parent) / 2;
    qreal newNodeHalfDepth = axis.nodeDepthSpan(newNode) / 2;
    qreal depth = axis.depth(parentPos)
        + axis.depthDirection * (parentHalfDepth + axis.depthSpacing + newNodeHalfDepth);

    qreal spread = axis.spread(parentPos);

    if (!existingSiblings.isEmpty()) {
        qreal maxSpreadEnd = -1e18;
        for (auto* sib : existingSiblings) {
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
