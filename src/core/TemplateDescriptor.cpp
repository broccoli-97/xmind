#include "core/TemplateDescriptor.h"
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
// TemplateColorScheme
// ===========================================================================

TemplateColorScheme TemplateColorScheme::fromJson(const QJsonObject& json) {
    return fromJson(json, TemplateColorScheme{});
}

TemplateColorScheme TemplateColorScheme::fromJson(const QJsonObject& json,
                                                  const TemplateColorScheme& base) {
    TemplateColorScheme cs = base;
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
            // else: keep base.nodePalette[i] (already copied above)
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

QJsonObject TemplateColorScheme::toJson() const {
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
// TemplateNodeStyle
// ===========================================================================

TemplateNodeStyle TemplateNodeStyle::fromJson(const QJsonObject& json) {
    return fromJson(json, TemplateNodeStyle{});
}

TemplateNodeStyle TemplateNodeStyle::fromJson(const QJsonObject& json,
                                              const TemplateNodeStyle& base) {
    TemplateNodeStyle s = base;
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
    return s;
}

QJsonObject TemplateNodeStyle::toJson() const {
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
    return obj;
}

// ===========================================================================
// TemplateEdgeStyle
// ===========================================================================

TemplateEdgeStyle TemplateEdgeStyle::fromJson(const QJsonObject& json) {
    return fromJson(json, TemplateEdgeStyle{});
}

TemplateEdgeStyle TemplateEdgeStyle::fromJson(const QJsonObject& json,
                                              const TemplateEdgeStyle& base) {
    TemplateEdgeStyle s = base;
    s.width = getDouble(json, "width", base.width);
    s.colorSource = getStr(json, "colorSource", base.colorSource);
    s.anchor = getStr(json, "anchor", base.anchor);
    s.curvature = getDouble(json, "curvature", base.curvature);
    s.dashStyle = getStr(json, "dashStyle", base.dashStyle);
    s.colorModifier = getStr(json, "colorModifier", base.colorModifier);
    s.lineCap = getStr(json, "lineCap", base.lineCap);
    return s;
}

QJsonObject TemplateEdgeStyle::toJson() const {
    QJsonObject obj;
    obj["width"] = width;
    obj["colorSource"] = colorSource;
    obj["anchor"] = anchor;
    obj["curvature"] = curvature;
    obj["dashStyle"] = dashStyle;
    obj["colorModifier"] = colorModifier;
    obj["lineCap"] = lineCap;
    return obj;
}

// ===========================================================================
// TemplateContentNode
// ===========================================================================

TemplateContentNode TemplateContentNode::fromJson(const QJsonObject& json) {
    TemplateContentNode n;
    n.text = json["text"].toString("Topic");
    QJsonArray arr = json["children"].toArray();
    for (const auto& val : arr)
        n.children.append(TemplateContentNode::fromJson(val.toObject()));
    return n;
}

QJsonObject TemplateContentNode::toJson() const {
    QJsonObject obj;
    obj["text"] = text;
    if (!children.isEmpty()) {
        QJsonArray arr;
        for (const auto& child : children)
            arr.append(child.toJson());
        obj["children"] = arr;
    }
    return obj;
}

// ===========================================================================
// TemplateLayoutConfig
// ===========================================================================

TemplateLayoutConfig TemplateLayoutConfig::fromJson(const QJsonObject& json) {
    return fromJson(json, TemplateLayoutConfig{});
}

TemplateLayoutConfig TemplateLayoutConfig::fromJson(const QJsonObject& json,
                                                    const TemplateLayoutConfig& base) {
    TemplateLayoutConfig c = base;
    c.algorithm = getStr(json, "algorithm", base.algorithm);
    c.depthSpacing = getDouble(json, "depthSpacing", base.depthSpacing);
    c.spreadSpacing = getDouble(json, "spreadSpacing", base.spreadSpacing);
    return c;
}

QJsonObject TemplateLayoutConfig::toJson() const {
    QJsonObject obj;
    obj["algorithm"] = algorithm;
    obj["depthSpacing"] = depthSpacing;
    obj["spreadSpacing"] = spreadSpacing;
    return obj;
}

// ===========================================================================
// TemplateDescriptor
// ===========================================================================

const TemplateColorScheme& TemplateDescriptor::activeColors() const {
    return ThemeManager::isDark() ? darkColors : lightColors;
}

const TemplateNodeStyle& TemplateDescriptor::nodeStyleForLevel(int level) const {
    if (level == 0 && hasRootStyle)
        return rootStyle;
    return nodeStyle;
}

TemplateDescriptor TemplateDescriptor::fromJson(const QJsonObject& json) {
    return fromJson(json, TemplateColorScheme{}, TemplateColorScheme{});
}

TemplateDescriptor TemplateDescriptor::fromJson(const QJsonObject& json,
                                                const TemplateColorScheme& lightDefaults,
                                                const TemplateColorScheme& darkDefaults) {
    TemplateDescriptor td;
    td.id = json["id"].toString();
    td.name = json["name"].toString("Unnamed Template");
    td.description = json["description"].toString();

    td.layout = TemplateLayoutConfig::fromJson(json["layout"].toObject());

    QJsonObject colors = json["colors"].toObject();
    td.lightColors = TemplateColorScheme::fromJson(colors["light"].toObject(), lightDefaults);
    td.darkColors = TemplateColorScheme::fromJson(colors["dark"].toObject(), darkDefaults);

    td.nodeStyle = TemplateNodeStyle::fromJson(json["nodeStyle"].toObject());
    td.edgeStyle = TemplateEdgeStyle::fromJson(json["edgeStyle"].toObject());

    // Root style override: full TemplateNodeStyle block inherits from nodeStyle.
    // Legacy: a bare "rootShape" string on nodeStyle implies a rootStyle with
    // only its shape overridden.
    if (json.contains("rootStyle")) {
        td.rootStyle = TemplateNodeStyle::fromJson(json["rootStyle"].toObject(), td.nodeStyle);
        td.hasRootStyle = true;
    } else if (!td.nodeStyle.rootShape.isEmpty()) {
        td.rootStyle = td.nodeStyle;
        td.rootStyle.shape = td.nodeStyle.rootShape;
        td.hasRootStyle = true;
    }

    td.backgroundPattern = json.contains("backgroundPattern")
                               ? json["backgroundPattern"].toString("dots")
                               : QStringLiteral("dots");

    td.content = TemplateContentNode::fromJson(json["content"].toObject());

    return td;
}

QJsonObject TemplateDescriptor::toJson() const {
    QJsonObject obj;
    obj["$schema"] = QStringLiteral("ymind-template-v1");
    obj["id"] = id;
    obj["name"] = name;
    obj["description"] = description;
    obj["layout"] = layout.toJson();

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
    obj["content"] = content.toJson();

    return obj;
}
