#pragma once

#include <QJsonObject>
#include <QList>
#include <QString>

// ---------------------------------------------------------------------------
// Content tree node — starter content for a new mind map.
// ---------------------------------------------------------------------------
struct TemplateContentNode {
    QString text;
    QList<TemplateContentNode> children;

    static TemplateContentNode fromJson(const QJsonObject& json);
    QJsonObject toJson() const;
};

// ---------------------------------------------------------------------------
// Layout configuration — which algorithm to use and its spacing.
// ---------------------------------------------------------------------------
struct TemplateLayoutConfig {
    QString algorithm = "bilateral";
    qreal depthSpacing = 100.0;
    qreal spreadSpacing = 16.0;

    static TemplateLayoutConfig fromJson(const QJsonObject& json);
    static TemplateLayoutConfig fromJson(const QJsonObject& json,
                                         const TemplateLayoutConfig& base);
    QJsonObject toJson() const;
};

// ---------------------------------------------------------------------------
// Template Descriptor — layout + starter content + a handful of *structural*
// overrides (node shape, edge anchor, palette routing) that some templates
// need to look right regardless of which theme is applied. The Lined
// template, for example, requires underline-shaped nodes and baseline-
// anchored connectors — those aren't optional decoration, they're the whole
// point of the template. Visual styling (fills, borders, shadows, colors)
// stays on ThemeDescriptor and can be swapped independently.
//
// Each override field is empty by default. An empty value means "defer to
// the active theme's value for this property".
// ---------------------------------------------------------------------------
class TemplateDescriptor {
public:
    QString id;
    QString name;
    QString description;

    TemplateLayoutConfig layout;
    TemplateContentNode content;

    // Structural overrides. Empty == use the theme's value.
    QString nodeShapeOverride;
    QString rootShapeOverride;
    QString edgeAnchorOverride;
    QString paletteSourceOverride;
    QString edgeColorSourceOverride;

    static TemplateDescriptor fromJson(const QJsonObject& json);
    QJsonObject toJson() const;
};
