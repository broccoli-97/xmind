#include "layout/RightTreeLayout.h"
#include "scene/NodeItem.h"

QString RightTreeLayout::name() const { return QStringLiteral("righttree"); }
QString RightTreeLayout::displayName() const { return QStringLiteral("Right Tree"); }

QMap<NodeItem*, QPointF> RightTreeLayout::computeLayout(NodeItem* root,
                                                         const LayoutParams& p) const {
    QMap<NodeItem*, QPointF> positions;
    if (!root)
        return positions;

    positions[root] = QPointF(0, 0);

    LayoutAxis axis = makeRightAxis(p);
    placeSubtree(root, QPointF(0, 0), axis, positions);
    forceDirectedRefinement(root, root->childNodes(), axis, positions);

    return positions;
}

QPointF RightTreeLayout::initialChildPosition(NodeItem* newNode, NodeItem* parent,
                                               NodeItem* root,
                                               const LayoutParams& p) const {
    return initialChildPositionForAxis(newNode, parent, root, p, makeRightAxis(p));
}
