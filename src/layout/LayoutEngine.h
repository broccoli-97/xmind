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

    // Layout constants kept for backward compatibility
    static constexpr qreal kHSpacing = 220.0;
    static constexpr qreal kVSpacing = 16.0;
    static constexpr qreal kNodeHeight = 44.0;
    static constexpr qreal kTopDownLevelSpacing = 100.0;

private:
    static const ILayoutAlgorithm* resolveAlgorithm(const QString& name);
    static LayoutParams defaultParams();
};
