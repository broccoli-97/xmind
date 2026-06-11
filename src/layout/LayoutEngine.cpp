#include "layout/LayoutEngine.h"
#include "layout/LayoutAlgorithmRegistry.h"
#include "scene/NodeItem.h"

// ===========================================================================
// Private helpers
// ===========================================================================

namespace {

void stackDescendantsAt(NodeItem* node, const QPointF& anchor,
                        QMap<NodeItem*, QPointF>& positions) {
    for (auto* child : node->childNodes()) {
        positions[child] = anchor;
        stackDescendantsAt(child, anchor, positions);
    }
}

// Layout algorithms treat collapsed nodes as leaves and assign no positions
// to their hidden descendants. Stack those descendants onto the collapsed
// ancestor's final position instead of leaving them wherever they last were:
// itemsBoundingRect() counts invisible items, so strays would inflate
// zoom-to-fit and exports, and expanding later animates the children
// unfolding outward from their parent rather than flying in from stale spots.
void snapCollapsedDescendants(NodeItem* node, QMap<NodeItem*, QPointF>& positions) {
    if (!node || !positions.contains(node))
        return;
    if (node->isCollapsed()) {
        stackDescendantsAt(node, positions[node], positions);
        return;
    }
    for (auto* child : node->childNodes())
        snapCollapsedDescendants(child, positions);
}

} // namespace

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

QPointF LayoutEngine::initialChildPosition(NodeItem* newNode, NodeItem* parent, NodeItem* root,
                                           LayoutStyle style) {
    return initialChildPosition(newNode, parent, root, layoutStyleToAlgorithmName(style),
                                defaultParams());
}

// ===========================================================================
// New API (name + params)
// ===========================================================================

QMap<NodeItem*, QPointF> LayoutEngine::computeLayout(NodeItem* root, const QString& algorithmName,
                                                     const LayoutParams& params) {
    const auto* algo = resolveAlgorithm(algorithmName);
    if (!algo) {
        // Fallback to bilateral
        algo = resolveAlgorithm(QStringLiteral("bilateral"));
    }
    if (!algo)
        return {};
    auto positions = algo->computeLayout(root, params);
    snapCollapsedDescendants(root, positions);
    return positions;
}

QPointF LayoutEngine::initialChildPosition(NodeItem* newNode, NodeItem* parent, NodeItem* root,
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
