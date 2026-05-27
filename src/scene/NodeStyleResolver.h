#pragma once

#include "core/ThemeDescriptor.h" // ThemeNodeStyle is a value type used in the API

class TemplateDescriptor;
class QFont;
class QColor;
class QString;

// Resolves the effective per-level node style by composing theme defaults with
// template structural overrides (shape, palette source). Pointers may be null —
// a null theme yields a default ThemeNodeStyle; a null template applies no
// overrides. Construct cheaply at the call site; later we can memoize keyed on
// (theme, template, level) if it shows up hot.
class NodeStyleResolver {
public:
    NodeStyleResolver(const ThemeDescriptor* theme, const TemplateDescriptor* tpl);

    // Theme's per-level style, plus template overrides: rootShapeOverride at
    // level 0, nodeShapeOverride elsewhere, paletteSourceOverride always when
    // set. Used by NodeItem::boundingRect/shape/paint/updateGeometry.
    ThemeNodeStyle styleForLevel(int level) const;

    // `base` with the level's theme font-family / point-size applied on top.
    // Themes can swap to a handwritten font (e.g. Caveat) or bump the size
    // without touching the app-wide preference.
    QFont fontForLevel(int level, const QFont& base) const;

    // True only for roundedRect + solid fill + style.drawShadow opt-in.
    static bool drawsShadow(const ThemeNodeStyle& style, const QString& shape);

    // Per the style's borderColorSource: "darker" darkens nodeColor; "fixed"
    // returns fixedColor when valid; otherwise nodeColor itself.
    static QColor resolveBorderColor(const ThemeNodeStyle& style,
                                     const QColor& nodeColor,
                                     const QColor& fixedColor);

private:
    const ThemeDescriptor* m_theme;
    const TemplateDescriptor* m_template;
};
