#pragma once

#include "layout/LayoutAlgorithmBase.h"

class RightTreeLayout : public LayoutAlgorithmBase {
public:
    QString name() const override;
    QString displayName() const override;
    QMap<NodeItem*, QPointF> computeLayout(NodeItem* root,
                                            const LayoutParams& p) const override;
    QPointF initialChildPosition(NodeItem* newNode, NodeItem* parent,
                                  NodeItem* root,
                                  const LayoutParams& p) const override;
};
