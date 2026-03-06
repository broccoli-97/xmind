#pragma once

#include "layout/ILayoutAlgorithm.h"

#include <QStringList>
#include <map>
#include <memory>

class LayoutAlgorithmRegistry {
public:
    static LayoutAlgorithmRegistry& instance();

    void registerBuiltins();
    void registerAlgorithm(std::unique_ptr<ILayoutAlgorithm> algo);

    const ILayoutAlgorithm* algorithm(const QString& name) const;
    QStringList algorithmNames() const;

private:
    LayoutAlgorithmRegistry() = default;
    std::map<QString, std::unique_ptr<ILayoutAlgorithm>> m_algorithms;
};
