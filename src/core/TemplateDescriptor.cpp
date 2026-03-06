#include "core/TemplateDescriptor.h"
#include "ui/ThemeManager.h"

#include <QJsonArray>

// ===========================================================================
// TemplateColorScheme
// ===========================================================================

TemplateColorScheme TemplateColorScheme::fromJson(const QJsonObject& json) {
    TemplateColorScheme cs;
    cs.canvasBackground = QColor(json["canvasBackground"].toString("#F8F9FA"));
    cs.canvasGridDot = QColor(json["canvasGridDot"].toString("#D8D8D8"));

    QJsonArray palette = json["nodePalette"].toArray();
    const QStringList defaultLight = {"#1565C0","#2E7D32","#E65100","#6A1B9A","#C62828","#00838F"};
    for (int i = 0; i < 6; ++i) {
        if (i < palette.size())
            cs.nodePalette[i] = QColor(palette[i].toString());
        else
            cs.nodePalette[i] = QColor(defaultLight[i]);
    }

    cs.nodeShadow = QColor(json["nodeShadow"].toString("#0000001E"));
    cs.nodeSelectionBorder = QColor(json["nodeSelectionBorder"].toString("#FF6F00"));
    cs.nodeText = QColor(json["nodeText"].toString("#FFFFFF"));
    cs.edgeLightenFactor = json["edgeLightenFactor"].toInt(140);
    cs.exportBackground = QColor(json["exportBackground"].toString("#FFFFFF"));

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

    return obj;
}

// ===========================================================================
// TemplateNodeStyle
// ===========================================================================

TemplateNodeStyle TemplateNodeStyle::fromJson(const QJsonObject& json) {
    TemplateNodeStyle s;
    s.borderRadius = json["borderRadius"].toDouble(10.0);
    s.padding = json["padding"].toDouble(16.0);
    s.minWidth = json["minWidth"].toDouble(120.0);
    s.maxWidth = json["maxWidth"].toDouble(300.0);
    return s;
}

QJsonObject TemplateNodeStyle::toJson() const {
    QJsonObject obj;
    obj["borderRadius"] = borderRadius;
    obj["padding"] = padding;
    obj["minWidth"] = minWidth;
    obj["maxWidth"] = maxWidth;
    return obj;
}

// ===========================================================================
// TemplateEdgeStyle
// ===========================================================================

TemplateEdgeStyle TemplateEdgeStyle::fromJson(const QJsonObject& json) {
    TemplateEdgeStyle s;
    s.width = json["width"].toDouble(2.5);
    return s;
}

QJsonObject TemplateEdgeStyle::toJson() const {
    QJsonObject obj;
    obj["width"] = width;
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
    TemplateLayoutConfig c;
    c.algorithm = json["algorithm"].toString("bilateral");
    c.depthSpacing = json["depthSpacing"].toDouble(100.0);
    c.spreadSpacing = json["spreadSpacing"].toDouble(16.0);
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

TemplateDescriptor TemplateDescriptor::fromJson(const QJsonObject& json) {
    TemplateDescriptor td;
    td.id = json["id"].toString();
    td.name = json["name"].toString("Unnamed Template");
    td.description = json["description"].toString();

    td.layout = TemplateLayoutConfig::fromJson(json["layout"].toObject());

    QJsonObject colors = json["colors"].toObject();
    td.lightColors = TemplateColorScheme::fromJson(colors["light"].toObject());
    td.darkColors = TemplateColorScheme::fromJson(colors["dark"].toObject());

    td.nodeStyle = TemplateNodeStyle::fromJson(json["nodeStyle"].toObject());
    td.edgeStyle = TemplateEdgeStyle::fromJson(json["edgeStyle"].toObject());
    td.content = TemplateContentNode::fromJson(json["content"].toObject());

    return td;
}

QJsonObject TemplateDescriptor::toJson() const {
    QJsonObject obj;
    obj["$schema"] = QStringLiteral("xmind-template-v1");
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
    obj["content"] = content.toJson();

    return obj;
}
