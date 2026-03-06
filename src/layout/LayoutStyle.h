#pragma once

#include <QString>

enum class LayoutStyle { Bilateral = 0, TopDown = 1, RightTree = 2 };

inline QString layoutStyleToAlgorithmName(LayoutStyle style) {
    switch (style) {
    case LayoutStyle::Bilateral: return QStringLiteral("bilateral");
    case LayoutStyle::TopDown:   return QStringLiteral("topdown");
    case LayoutStyle::RightTree: return QStringLiteral("righttree");
    }
    return QStringLiteral("bilateral");
}

inline LayoutStyle algorithmNameToLayoutStyle(const QString& name) {
    if (name == QLatin1String("topdown"))   return LayoutStyle::TopDown;
    if (name == QLatin1String("righttree")) return LayoutStyle::RightTree;
    return LayoutStyle::Bilateral;
}
