#pragma once

#include "layout/ILayoutAlgorithm.h"
#include "layout/LayoutStyle.h"

#include <QMap>
#include <QPointF>

class NodeItem;

class LayoutEngine {
public:
    // Legacy API: delegates to LayoutAlgorithmRegistry via enum
    static QMap<NodeItem*, QPointF> computeLayout(NodeItem* root, LayoutStyle style);
    static QPointF initialChildPosition(NodeItem* newNode, NodeItem* parent,
                                        NodeItem* root, LayoutStyle style);

    // New API: algorithm name + params
    static QMap<NodeItem*, QPointF> computeLayout(NodeItem* root,
                                                   const QString& algorithmName,
                                                   const LayoutParams& params);
    static QPointF initialChildPosition(NodeItem* newNode, NodeItem* parent,
                                        NodeItem* root,
                                        const QString& algorithmName,
                                        const LayoutParams& params);

private:
    static const ILayoutAlgorithm* resolveAlgorithm(const QString& name);
    static LayoutParams defaultParams();
};
