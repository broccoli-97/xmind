#include "layout/LayoutAlgorithmRegistry.h"
#include "layout/BilateralLayout.h"
#include "layout/RightTreeLayout.h"
#include "layout/TopDownLayout.h"

LayoutAlgorithmRegistry& LayoutAlgorithmRegistry::instance() {
    static LayoutAlgorithmRegistry s_instance;
    return s_instance;
}

void LayoutAlgorithmRegistry::registerBuiltins() {
    registerAlgorithm(std::make_unique<BilateralLayout>());
    registerAlgorithm(std::make_unique<TopDownLayout>());
    registerAlgorithm(std::make_unique<RightTreeLayout>());
}

void LayoutAlgorithmRegistry::registerAlgorithm(std::unique_ptr<ILayoutAlgorithm> algo) {
    QString n = algo->name();
    m_algorithms[n] = std::move(algo);
}

const ILayoutAlgorithm* LayoutAlgorithmRegistry::algorithm(const QString& name) const {
    auto it = m_algorithms.find(name);
    if (it != m_algorithms.end())
        return it->second.get();
    return nullptr;
}

QStringList LayoutAlgorithmRegistry::algorithmNames() const {
    QStringList result;
    for (const auto& [name, algo] : m_algorithms)
        result.append(name);
    return result;
}
