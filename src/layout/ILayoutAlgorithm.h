#pragma once

#include <QMap>
#include <QPointF>
#include <QString>

class NodeItem;

struct LayoutParams {
    qreal depthSpacing = 100.0;
    qreal spreadSpacing = 16.0;
};

class ILayoutAlgorithm {
public:
    virtual ~ILayoutAlgorithm() = default;
    virtual QString name() const = 0;
    virtual QString displayName() const = 0;
    virtual QMap<NodeItem*, QPointF> computeLayout(NodeItem* root,
                                                    const LayoutParams& p) const = 0;
    virtual QPointF initialChildPosition(NodeItem* newNode, NodeItem* parent,
                                          NodeItem* root,
                                          const LayoutParams& p) const = 0;
};
