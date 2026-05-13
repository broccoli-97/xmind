#pragma once

#include <QColor>
#include <QJsonObject>
#include <QString>

// ---------------------------------------------------------------------------
// Color scheme (light + dark variants)
//
// Originally lived on TemplateDescriptor; moved here when the visual style
// concept was split out from layout/content. fromJson(base) lets a JSON
// override only specific keys while inheriting the rest from a fallback
// (the global ThemeManager palette for built-ins).
// ---------------------------------------------------------------------------
struct ThemeColorScheme {
    QColor canvasBackground;
    QColor canvasGridDot;
    QColor nodePalette[6];
    QColor nodeShadow;
    QColor nodeSelectionBorder;
    QColor nodeText;
    int edgeLightenFactor = 140;
    QColor exportBackground;
    // Consulted only when ThemeNodeStyle::borderColorSource == "fixed".
    QColor nodeBorderColor;

    static ThemeColorScheme fromJson(const QJsonObject& json);
    static ThemeColorScheme fromJson(const QJsonObject& json,
                                     const ThemeColorScheme& base);
    QJsonObject toJson() const;
};

// ---------------------------------------------------------------------------
// Node style
// ---------------------------------------------------------------------------
struct ThemeNodeStyle {
    qreal borderRadius = 10.0;
    qreal padding = 16.0;
    qreal minWidth = 120.0;
    qreal maxWidth = 300.0;

    // "roundedRect" (default) | "none" | "underline"
    QString shape = "roundedRect";
    // Legacy shortcut for the level-0 root's shape. Equivalent to setting
    // rootStyle.shape; kept for backward compatibility with existing JSON.
    QString rootShape;

    // Body fill mode:
    //   "solid"    (default — 100% colored fill, no separate border)
    //   "tinted"   (alpha-blended fill; pair with borderWidth>0 for a card look)
    //   "outlined" (transparent fill, colored border only)
    QString fillMode = "solid";
    // Alpha multiplier for "tinted" mode, 0.0 (transparent) – 1.0 (opaque).
    qreal fillAlpha = 0.15;

    // Border drawn in every visual state. 0 disables the border entirely
    // (current behavior). The selection ring is drawn independently.
    qreal borderWidth = 0.0;
    // Where the border color comes from:
    //   "node"   (default — same as node's palette color)
    //   "darker" (palette color darkened by ~25%)
    //   "fixed"  (use ThemeColorScheme::nodeBorderColor)
    QString borderColorSource = "node";

    // Multi-layer soft shadow tuning (only used when drawShadow=true).
    bool drawShadow = true;
    int shadowLayers = 5;
    qreal shadowSpread = 10.0;
    qreal shadowOffsetY = 4.0;
    // Multiplier on the palette nodeShadow alpha (1.0 = unchanged, 0.0 = invisible).
    qreal shadowOpacity = 1.0;

    // Width of the selection-state ring (px).
    qreal selectionWidth = 3.0;

    // "level" (default — node color from depth-indexed palette)
    // "branch" (node color from level-1 ancestor's index — every node in the
    //   same top-level branch shares one color)
    QString paletteSource = "level";

    static ThemeNodeStyle fromJson(const QJsonObject& json);
    static ThemeNodeStyle fromJson(const QJsonObject& json,
                                   const ThemeNodeStyle& base);
    QJsonObject toJson() const;
};

// ---------------------------------------------------------------------------
// Edge style
// ---------------------------------------------------------------------------
struct ThemeEdgeStyle {
    qreal width = 2.5;
    // "target" (default — edge color follows target node)
    // "branch" (edge color follows target's branch root)
    QString colorSource = "target";
    // "center" (default — edge meets node at vertical/horizontal midpoint)
    // "baseline" (edge meets node at its bottom — used by the lined style so
    //   text appears to float above the line)
    QString anchor = "center";

    // Bezier control-point strength relative to span. 0.0 = straight line,
    // 0.5 = current default (loose S-curve), 0.9 = very loose.
    qreal curvature = 0.5;

    // Pen dash pattern: "solid" (default) | "dashed" | "dotted".
    QString dashStyle = "solid";

    // How the edge color is derived from the node color:
    //   "lighten" (default — uses ThemeColorScheme::edgeLightenFactor)
    //   "same"    (raw node color, no modification)
    //   "darken"  (darken by the inverse of edgeLightenFactor)
    QString colorModifier = "lighten";

    // Line cap: "round" (default) | "flat" | "square".
    QString lineCap = "round";

    static ThemeEdgeStyle fromJson(const QJsonObject& json);
    static ThemeEdgeStyle fromJson(const QJsonObject& json,
                                   const ThemeEdgeStyle& base);
    QJsonObject toJson() const;
};

// ---------------------------------------------------------------------------
// Theme Descriptor — pure visual style. Independent of layout/content.
//
// A theme is applied on top of any template. Switching themes never changes
// the content tree or layout algorithm — it only changes how nodes and edges
// look.
// ---------------------------------------------------------------------------
class ThemeDescriptor {
public:
    QString id;
    QString name;
    QString description;

    ThemeColorScheme lightColors;
    ThemeColorScheme darkColors;
    ThemeNodeStyle nodeStyle;
    ThemeEdgeStyle edgeStyle;

    // Optional level-0 override. Inherits any unspecified field from nodeStyle.
    bool hasRootStyle = false;
    ThemeNodeStyle rootStyle;

    // "dots" (default) | "lines" | "none".
    QString backgroundPattern = "dots";

    // Returns the active color scheme based on ThemeManager::isDark().
    const ThemeColorScheme& activeColors() const;
    // Returns rootStyle when applicable (level==0 and hasRootStyle), else nodeStyle.
    const ThemeNodeStyle& nodeStyleForLevel(int level) const;

    // The `themeDefaults` schemes are used as the per-side base when a key is
    // missing from the theme JSON. Built-ins thus inherit global theme colors
    // automatically without restating them.
    static ThemeDescriptor fromJson(const QJsonObject& json);
    static ThemeDescriptor fromJson(const QJsonObject& json,
                                    const ThemeColorScheme& lightDefaults,
                                    const ThemeColorScheme& darkDefaults);
    QJsonObject toJson() const;
};
