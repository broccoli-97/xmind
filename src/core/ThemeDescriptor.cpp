#include "core/ThemeDescriptor.h"
#include "ui/ThemeManager.h"

#include <QJsonArray>

// ---------------------------------------------------------------------------
// Helpers for inheritance-aware JSON parsing
// ---------------------------------------------------------------------------
namespace {

QString getStr(const QJsonObject& j, const QString& key, const QString& fallback) {
    return j.contains(key) ? j.value(key).toString(fallback) : fallback;
}

qreal getDouble(const QJsonObject& j, const QString& key, qreal fallback) {
    return j.contains(key) ? j.value(key).toDouble(fallback) : fallback;
}

int getInt(const QJsonObject& j, const QString& key, int fallback) {
    return j.contains(key) ? j.value(key).toInt(fallback) : fallback;
}

bool getBool(const QJsonObject& j, const QString& key, bool fallback) {
    return j.contains(key) ? j.value(key).toBool(fallback) : fallback;
}

QColor getColor(const QJsonObject& j, const QString& key, const QColor& fallback) {
    if (!j.contains(key))
        return fallback;
    QColor c(j.value(key).toString());
    return c.isValid() ? c : fallback;
}

} // namespace

// ===========================================================================
// ThemeColorScheme
// ===========================================================================

ThemeColorScheme ThemeColorScheme::fromJson(const QJsonObject& json) {
    return fromJson(json, ThemeColorScheme{});
}

ThemeColorScheme ThemeColorScheme::fromJson(const QJsonObject& json,
                                            const ThemeColorScheme& base) {
    ThemeColorScheme cs = base;
    cs.canvasBackground = getColor(json, "canvasBackground", base.canvasBackground);
    cs.canvasGridDot = getColor(json, "canvasGridDot", base.canvasGridDot);

    if (json.contains("nodePalette")) {
        QJsonArray palette = json["nodePalette"].toArray();
        for (int i = 0; i < 6; ++i) {
            if (i < palette.size()) {
                QColor c(palette[i].toString());
                if (c.isValid())
                    cs.nodePalette[i] = c;
            }
        }
    }

    cs.nodeShadow = getColor(json, "nodeShadow", base.nodeShadow);
    cs.nodeSelectionBorder = getColor(json, "nodeSelectionBorder", base.nodeSelectionBorder);
    cs.nodeText = getColor(json, "nodeText", base.nodeText);
    cs.edgeLightenFactor = getInt(json, "edgeLightenFactor", base.edgeLightenFactor);
    cs.exportBackground = getColor(json, "exportBackground", base.exportBackground);
    cs.nodeBorderColor = getColor(json, "nodeBorderColor", base.nodeBorderColor);

    return cs;
}

QJsonObject ThemeColorScheme::toJson() const {
    QJsonObject obj;
    obj["canvasBackground"] = canvasBackground.name(QColor::HexArgb);
    obj["canvasGridDot"] = canvasGridDot.name(QColor::HexArgb);

    QJsonArray palette;
    for (int i = 0; i < 6; ++i)
        palette.append(nodePalette[i].name(QColor::HexArgb));
    obj["nodePalette"] = palette;

    obj["nodeShadow"] = nodeShadow.name(QColor::HexArgb);
    obj["nodeSelectionBorder"] = nodeSelectionBorder.name(QColor::HexArgb);
    obj["nodeText"] = nodeText.name(QColor::HexArgb);
    obj["edgeLightenFactor"] = edgeLightenFactor;
    obj["exportBackground"] = exportBackground.name(QColor::HexArgb);
    if (nodeBorderColor.isValid())
        obj["nodeBorderColor"] = nodeBorderColor.name(QColor::HexArgb);

    return obj;
}

// ===========================================================================
// ThemeNodeStyle
// ===========================================================================

ThemeNodeStyle ThemeNodeStyle::fromJson(const QJsonObject& json) {
    return fromJson(json, ThemeNodeStyle{});
}

ThemeNodeStyle ThemeNodeStyle::fromJson(const QJsonObject& json,
                                        const ThemeNodeStyle& base) {
    ThemeNodeStyle s = base;
    s.borderRadius = getDouble(json, "borderRadius", base.borderRadius);
    s.padding = getDouble(json, "padding", base.padding);
    s.minWidth = getDouble(json, "minWidth", base.minWidth);
    s.maxWidth = getDouble(json, "maxWidth", base.maxWidth);
    s.shape = getStr(json, "shape", base.shape);
    s.rootShape = getStr(json, "rootShape", base.rootShape);
    s.fillMode = getStr(json, "fillMode", base.fillMode);
    s.fillAlpha = getDouble(json, "fillAlpha", base.fillAlpha);
    s.borderWidth = getDouble(json, "borderWidth", base.borderWidth);
    s.borderColorSource = getStr(json, "borderColorSource", base.borderColorSource);
    s.drawShadow = getBool(json, "drawShadow", base.drawShadow);
    s.shadowLayers = getInt(json, "shadowLayers", base.shadowLayers);
    s.shadowSpread = getDouble(json, "shadowSpread", base.shadowSpread);
    s.shadowOffsetY = getDouble(json, "shadowOffsetY", base.shadowOffsetY);
    s.shadowOpacity = getDouble(json, "shadowOpacity", base.shadowOpacity);
    s.selectionWidth = getDouble(json, "selectionWidth", base.selectionWidth);
    s.paletteSource = getStr(json, "paletteSource", base.paletteSource);
    s.roughness = getDouble(json, "roughness", base.roughness);
    s.strokePasses = getInt(json, "strokePasses", base.strokePasses);
    s.fontFamily = getStr(json, "fontFamily", base.fontFamily);
    s.fontPointSize = getDouble(json, "fontPointSize", base.fontPointSize);
    return s;
}

QJsonObject ThemeNodeStyle::toJson() const {
    QJsonObject obj;
    obj["borderRadius"] = borderRadius;
    obj["padding"] = padding;
    obj["minWidth"] = minWidth;
    obj["maxWidth"] = maxWidth;
    obj["shape"] = shape;
    if (!rootShape.isEmpty())
        obj["rootShape"] = rootShape;
    obj["fillMode"] = fillMode;
    obj["fillAlpha"] = fillAlpha;
    obj["borderWidth"] = borderWidth;
    obj["borderColorSource"] = borderColorSource;
    obj["drawShadow"] = drawShadow;
    obj["shadowLayers"] = shadowLayers;
    obj["shadowSpread"] = shadowSpread;
    obj["shadowOffsetY"] = shadowOffsetY;
    obj["shadowOpacity"] = shadowOpacity;
    obj["selectionWidth"] = selectionWidth;
    obj["paletteSource"] = paletteSource;
    if (roughness != 0.0)
        obj["roughness"] = roughness;
    if (strokePasses != 1)
        obj["strokePasses"] = strokePasses;
    if (!fontFamily.isEmpty())
        obj["fontFamily"] = fontFamily;
    if (fontPointSize > 0.0)
        obj["fontPointSize"] = fontPointSize;
    return obj;
}

// ===========================================================================
// ThemeEdgeStyle
// ===========================================================================

ThemeEdgeStyle ThemeEdgeStyle::fromJson(const QJsonObject& json) {
    return fromJson(json, ThemeEdgeStyle{});
}

ThemeEdgeStyle ThemeEdgeStyle::fromJson(const QJsonObject& json,
                                        const ThemeEdgeStyle& base) {
    ThemeEdgeStyle s = base;
    s.width = getDouble(json, "width", base.width);
    s.colorSource = getStr(json, "colorSource", base.colorSource);
    s.anchor = getStr(json, "anchor", base.anchor);
    s.curvature = getDouble(json, "curvature", base.curvature);
    s.dashStyle = getStr(json, "dashStyle", base.dashStyle);
    s.colorModifier = getStr(json, "colorModifier", base.colorModifier);
    s.lineCap = getStr(json, "lineCap", base.lineCap);
    s.roughness = getDouble(json, "roughness", base.roughness);
    s.strokePasses = getInt(json, "strokePasses", base.strokePasses);
    return s;
}

QJsonObject ThemeEdgeStyle::toJson() const {
    QJsonObject obj;
    obj["width"] = width;
    obj["colorSource"] = colorSource;
    obj["anchor"] = anchor;
    obj["curvature"] = curvature;
    obj["dashStyle"] = dashStyle;
    obj["colorModifier"] = colorModifier;
    obj["lineCap"] = lineCap;
    if (roughness != 0.0)
        obj["roughness"] = roughness;
    if (strokePasses != 1)
        obj["strokePasses"] = strokePasses;
    return obj;
}

// ===========================================================================
// ThemeDescriptor
// ===========================================================================

const ThemeColorScheme& ThemeDescriptor::activeColors() const {
    return ThemeManager::isDark() ? darkColors : lightColors;
}

const ThemeNodeStyle& ThemeDescriptor::nodeStyleForLevel(int level) const {
    if (level == 0 && hasRootStyle)
        return rootStyle;
    return nodeStyle;
}

ThemeDescriptor ThemeDescriptor::fromJson(const QJsonObject& json) {
    return fromJson(json, ThemeColorScheme{}, ThemeColorScheme{});
}

ThemeDescriptor ThemeDescriptor::fromJson(const QJsonObject& json,
                                          const ThemeColorScheme& lightDefaults,
                                          const ThemeColorScheme& darkDefaults) {
    ThemeDescriptor td;
    td.id = json["id"].toString();
    td.name = json["name"].toString("Unnamed Theme");
    td.description = json["description"].toString();

    QJsonObject colors = json["colors"].toObject();
    td.lightColors = ThemeColorScheme::fromJson(colors["light"].toObject(), lightDefaults);
    td.darkColors = ThemeColorScheme::fromJson(colors["dark"].toObject(), darkDefaults);

    td.nodeStyle = ThemeNodeStyle::fromJson(json["nodeStyle"].toObject());
    td.edgeStyle = ThemeEdgeStyle::fromJson(json["edgeStyle"].toObject());

    // Root style override: full ThemeNodeStyle block inherits from nodeStyle.
    // Legacy: a bare "rootShape" string on nodeStyle implies a rootStyle with
    // only its shape overridden.
    if (json.contains("rootStyle")) {
        td.rootStyle = ThemeNodeStyle::fromJson(json["rootStyle"].toObject(), td.nodeStyle);
        td.hasRootStyle = true;
    } else if (!td.nodeStyle.rootShape.isEmpty()) {
        td.rootStyle = td.nodeStyle;
        td.rootStyle.shape = td.nodeStyle.rootShape;
        td.hasRootStyle = true;
    }

    td.backgroundPattern = json.contains("backgroundPattern")
                               ? json["backgroundPattern"].toString("dots")
                               : QStringLiteral("dots");

    return td;
}

QJsonObject ThemeDescriptor::toJson() const {
    QJsonObject obj;
    obj["$schema"] = QStringLiteral("ymind-theme-v1");
    obj["id"] = id;
    obj["name"] = name;
    obj["description"] = description;

    QJsonObject colors;
    colors["light"] = lightColors.toJson();
    colors["dark"] = darkColors.toJson();
    obj["colors"] = colors;

    obj["nodeStyle"] = nodeStyle.toJson();
    obj["edgeStyle"] = edgeStyle.toJson();
    if (hasRootStyle)
        obj["rootStyle"] = rootStyle.toJson();
    if (backgroundPattern != QLatin1String("dots"))
        obj["backgroundPattern"] = backgroundPattern;

    return obj;
}
