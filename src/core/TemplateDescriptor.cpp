#include "core/TemplateDescriptor.h"

#include <QJsonArray>

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
    if (json.contains("algorithm"))
        c.algorithm = json["algorithm"].toString(base.algorithm);
    if (json.contains("depthSpacing"))
        c.depthSpacing = json["depthSpacing"].toDouble(base.depthSpacing);
    if (json.contains("spreadSpacing"))
        c.spreadSpacing = json["spreadSpacing"].toDouble(base.spreadSpacing);
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

TemplateDescriptor TemplateDescriptor::fromJson(const QJsonObject& json) {
    TemplateDescriptor td;
    td.id = json["id"].toString();
    td.name = json["name"].toString("Unnamed Template");
    td.description = json["description"].toString();
    td.layout = TemplateLayoutConfig::fromJson(json["layout"].toObject());
    td.content = TemplateContentNode::fromJson(json["content"].toObject());

    // Optional structural overrides, parsed from a single "overrides" object.
    if (json.contains("overrides")) {
        QJsonObject ov = json["overrides"].toObject();
        td.nodeShapeOverride = ov.value("nodeShape").toString();
        td.rootShapeOverride = ov.value("rootShape").toString();
        td.edgeAnchorOverride = ov.value("edgeAnchor").toString();
        td.paletteSourceOverride = ov.value("paletteSource").toString();
        td.edgeColorSourceOverride = ov.value("edgeColorSource").toString();
    }

    return td;
}

QJsonObject TemplateDescriptor::toJson() const {
    QJsonObject obj;
    obj["$schema"] = QStringLiteral("ymind-template-v1");
    obj["id"] = id;
    obj["name"] = name;
    obj["description"] = description;
    obj["layout"] = layout.toJson();
    obj["content"] = content.toJson();

    QJsonObject ov;
    if (!nodeShapeOverride.isEmpty())     ov["nodeShape"] = nodeShapeOverride;
    if (!rootShapeOverride.isEmpty())     ov["rootShape"] = rootShapeOverride;
    if (!edgeAnchorOverride.isEmpty())    ov["edgeAnchor"] = edgeAnchorOverride;
    if (!paletteSourceOverride.isEmpty()) ov["paletteSource"] = paletteSourceOverride;
    if (!edgeColorSourceOverride.isEmpty()) ov["edgeColorSource"] = edgeColorSourceOverride;
    if (!ov.isEmpty())
        obj["overrides"] = ov;

    return obj;
}
