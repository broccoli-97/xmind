#pragma once

#include <QColor>
#include <QJsonObject>
#include <QString>
#include <QList>

// ---------------------------------------------------------------------------
// Color scheme (light + dark variants)
// ---------------------------------------------------------------------------
struct TemplateColorScheme {
    QColor canvasBackground;
    QColor canvasGridDot;
    QColor nodePalette[6];
    QColor nodeShadow;
    QColor nodeSelectionBorder;
    QColor nodeText;
    int edgeLightenFactor = 140;
    QColor exportBackground;

    static TemplateColorScheme fromJson(const QJsonObject& json);
    QJsonObject toJson() const;
};

// ---------------------------------------------------------------------------
// Node style
// ---------------------------------------------------------------------------
struct TemplateNodeStyle {
    qreal borderRadius = 10.0;
    qreal padding = 16.0;
    qreal minWidth = 120.0;
    qreal maxWidth = 300.0;

    // "roundedRect" (default) | "none" | "underline"
    QString shape = "roundedRect";
    // Optional override for the level-0 (root) node. Empty = use shape.
    QString rootShape;
    // Drop the multi-layer drop-shadow (set false for borderless / lined styles).
    bool drawShadow = true;
    // "level" (default — node color comes from depth-indexed palette)
    // "branch" (node color comes from level-1 ancestor's index — every node in
    //   the same top-level branch shares one color)
    QString paletteSource = "level";

    static TemplateNodeStyle fromJson(const QJsonObject& json);
    QJsonObject toJson() const;
};

// ---------------------------------------------------------------------------
// Edge style
// ---------------------------------------------------------------------------
struct TemplateEdgeStyle {
    qreal width = 2.5;
    // "target" (default — edge color follows target node)
    // "branch" (edge color follows target's branch root)
    QString colorSource = "target";
    // "center" (default — edge meets node at vertical/horizontal midpoint)
    // "baseline" (edge meets node at its bottom — used by the lined style so
    //   text appears to float above the line)
    QString anchor = "center";

    static TemplateEdgeStyle fromJson(const QJsonObject& json);
    QJsonObject toJson() const;
};

// ---------------------------------------------------------------------------
// Content tree node
// ---------------------------------------------------------------------------
struct TemplateContentNode {
    QString text;
    QList<TemplateContentNode> children;

    static TemplateContentNode fromJson(const QJsonObject& json);
    QJsonObject toJson() const;
};

// ---------------------------------------------------------------------------
// Layout config
// ---------------------------------------------------------------------------
struct TemplateLayoutConfig {
    QString algorithm = "bilateral";
    qreal depthSpacing = 100.0;
    qreal spreadSpacing = 16.0;

    static TemplateLayoutConfig fromJson(const QJsonObject& json);
    QJsonObject toJson() const;
};

// ---------------------------------------------------------------------------
// Template Descriptor
// ---------------------------------------------------------------------------
class TemplateDescriptor {
public:
    QString id;
    QString name;
    QString description;

    TemplateLayoutConfig layout;
    TemplateColorScheme lightColors;
    TemplateColorScheme darkColors;
    TemplateNodeStyle nodeStyle;
    TemplateEdgeStyle edgeStyle;
    TemplateContentNode content;

    // Returns the active color scheme based on ThemeManager::isDark()
    const TemplateColorScheme& activeColors() const;

    static TemplateDescriptor fromJson(const QJsonObject& json);
    QJsonObject toJson() const;
};
