#include "layout/LayoutEngine.h"
#include "layout/LayoutAlgorithmRegistry.h"

// ===========================================================================
// Private helpers
// ===========================================================================

const ILayoutAlgorithm* LayoutEngine::resolveAlgorithm(const QString& name) {
    return LayoutAlgorithmRegistry::instance().algorithm(name);
}

LayoutParams LayoutEngine::defaultParams() {
    return {100.0, 16.0};
}

// ===========================================================================
// Legacy API (enum-based)
// ===========================================================================

QMap<NodeItem*, QPointF> LayoutEngine::computeLayout(NodeItem* root, LayoutStyle style) {
    return computeLayout(root, layoutStyleToAlgorithmName(style), defaultParams());
}

QPointF LayoutEngine::initialChildPosition(NodeItem* newNode, NodeItem* parent,
                                            NodeItem* root, LayoutStyle style) {
    return initialChildPosition(newNode, parent, root,
                                 layoutStyleToAlgorithmName(style), defaultParams());
}

// ===========================================================================
// New API (name + params)
// ===========================================================================

QMap<NodeItem*, QPointF> LayoutEngine::computeLayout(NodeItem* root,
                                                      const QString& algorithmName,
                                                      const LayoutParams& params) {
    const auto* algo = resolveAlgorithm(algorithmName);
    if (!algo) {
        // Fallback to bilateral
        algo = resolveAlgorithm(QStringLiteral("bilateral"));
    }
    if (!algo)
        return {};
    return algo->computeLayout(root, params);
}

QPointF LayoutEngine::initialChildPosition(NodeItem* newNode, NodeItem* parent,
                                            NodeItem* root,
                                            const QString& algorithmName,
                                            const LayoutParams& params) {
    const auto* algo = resolveAlgorithm(algorithmName);
    if (!algo) {
        algo = resolveAlgorithm(QStringLiteral("bilateral"));
    }
    if (!algo)
        return {};
    return algo->initialChildPosition(newNode, parent, root, params);
}
