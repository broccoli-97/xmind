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

    static TemplateNodeStyle fromJson(const QJsonObject& json);
    QJsonObject toJson() const;
};

// ---------------------------------------------------------------------------
// Edge style
// ---------------------------------------------------------------------------
struct TemplateEdgeStyle {
    qreal width = 2.5;

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
