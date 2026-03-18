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
    return initialChildPositionForAxis(newNode, parent, root, p, makeTopDownAxis(p));
}
