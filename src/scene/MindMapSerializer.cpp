#include "scene/MindMapSerializer.h"
#include "scene/MindMapScene.h"
#include "scene/NodeItem.h"
#include "scene/StyleProvider.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUndoStack>
#include <functional>

MindMapSerializer::MindMapSerializer(MindMapScene* scene) : m_scene(scene) {}

QJsonObject MindMapSerializer::nodeToJson(NodeItem* node) const {
    QJsonObject obj;
    obj["text"] = node->text();
    obj["x"] = node->pos().x();
    obj["y"] = node->pos().y();

    QJsonArray children;
    for (auto* child : node->childNodes()) {
        children.append(nodeToJson(child));
    }
    obj["children"] = children;
    return obj;
}

QJsonObject MindMapSerializer::toJson() const {
    QJsonObject root;
    root["format"] = QStringLiteral("ymind");
    root["version"] = kCurrentVersion;
    root["layoutStyle"] = static_cast<int>(m_scene->m_layoutStyle);
    if (m_scene->m_templateId.isValid())
        root["templateId"] = m_scene->m_templateId.toString();
    if (m_scene->m_themeId.isValid())
        root["themeId"] = m_scene->m_themeId.toString();
    if (m_scene->m_rootNode) {
        root["root"] = nodeToJson(m_scene->m_rootNode);
    }
    return root;
}

NodeItem* MindMapSerializer::nodeFromJson(const QJsonObject& json, NodeItem* parent) {
    QString text = json["text"].toString("Topic");
    qreal x = json["x"].toDouble(0);
    qreal y = json["y"].toDouble(0);

    NodeItem* node;
    if (!parent) {
        // This is the root node
        node = m_scene->createRootNode(text);
        node->setPos(x, y);
    } else {
        node = m_scene->addNode(text, parent);
        if (node)
            node->setPos(x, y);
    }

    if (!node)
        return nullptr;

    QJsonArray children = json["children"].toArray();
    for (const auto& childVal : children) {
        nodeFromJson(childVal.toObject(), node);
    }
    return node;
}

// ----------------------------------------------------------------------------
// Versioned migrators
//
// Each entry takes a JSON object at version N and returns one at version N+1.
// The chain runs in `migrate()` until the document reports `kCurrentVersion`.
// Files without an explicit "version" field are treated as v1 (the original
// format had no version field at all).
// ----------------------------------------------------------------------------

namespace {

using Migrator = std::function<QJsonObject(QJsonObject, const StyleProvider*)>;

// v1 → v2: v1 carried only `layoutStyle`. v2 promoted layout selection from
// "implicit in scene.m_layoutStyle" to an explicit `templateId` that points
// at a registered template. Map the old style enum onto the matching
// builtin id via the injected StyleProvider.
QJsonObject migrate_v1_to_v2(QJsonObject json, const StyleProvider* sp) {
    if (!json.contains("templateId") && sp) {
        json["templateId"] = sp->builtinTemplateIdForLayoutStyle(json["layoutStyle"].toInt(0));
    }
    json["version"] = 2;
    return json;
}

// v2 → v3: v2's templateId conflated visual styling and structural layout.
// Outlined/Tinted/Morandi were really themes wearing template costumes;
// promote them to `themeId` and fall back to the matching plain-layout
// template. Lined is genuinely structural (underline shape, baseline edges)
// so it stays as a template — only its theme defaults to builtin.default.
QJsonObject migrate_v2_to_v3(QJsonObject json, const StyleProvider* sp) {
    if (!json.contains("themeId")) {
        const QString legacy = json["templateId"].toString();
        if (legacy == QLatin1String("builtin.outlined") ||
            legacy == QLatin1String("builtin.tinted") ||
            legacy == QLatin1String("builtin.morandi")) {
            json["themeId"] = legacy;
            if (sp)
                json["templateId"] =
                    sp->builtinTemplateIdForLayoutStyle(json["layoutStyle"].toInt(0));
        } else {
            json["themeId"] = QStringLiteral("builtin.default");
        }
    }
    json["version"] = 3;
    return json;
}

// Indexed by source version: migrators[N] turns version-N JSON into
// version-(N+1). Adding a v3→v4 step means appending to this vector and
// bumping kCurrentVersion in the header.
const std::vector<Migrator>& migrators() {
    static const std::vector<Migrator> kMigrators = {
        migrate_v1_to_v2,
        migrate_v2_to_v3,
    };
    return kMigrators;
}

} // namespace

QJsonObject MindMapSerializer::migrate(QJsonObject json) {
    const StyleProvider* sp = MindMapScene::defaultStyleProvider();
    int version = json["version"].toInt(1); // No "version" field == v1.
    while (version < kCurrentVersion) {
        const size_t idx = static_cast<size_t>(version - 1);
        if (idx >= migrators().size())
            break; // Future-version file from a newer build; load as-is.
        json = migrators()[idx](json, sp);
        version = json["version"].toInt(version + 1);
    }
    return json;
}

bool MindMapSerializer::fromJson(const QJsonObject& json) {
    if (json["format"].toString() != "ymind")
        return false;

    const QJsonObject migrated = migrate(json);

    m_scene->clearScene();
    m_scene->m_batchLoading = true;

    m_scene->m_layoutStyle = static_cast<LayoutStyle>(migrated["layoutStyle"].toInt(0));
    m_scene->m_templateId = TemplateId(migrated["templateId"].toString());
    m_scene->m_themeId = ThemeId(migrated["themeId"].toString());

    QJsonObject rootObj = migrated["root"].toObject();
    m_scene->m_rootNode = nodeFromJson(rootObj, nullptr);
    if (!m_scene->m_rootNode) {
        m_scene->m_rootNode = m_scene->createRootNode(MindMapScene::tr("Central Topic"));
    }

    m_scene->m_undoStack->clear();
    m_scene->m_batchLoading = false;
    m_scene->setModified(false);
    return true;
}

bool MindMapSerializer::saveToFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QJsonDocument doc(toJson());
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    m_scene->m_undoStack->setClean();
    m_scene->setModified(false);
    return true;
}

bool MindMapSerializer::loadFromFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError)
        return false;

    if (!fromJson(doc.object()))
        return false;

    emit m_scene->fileLoaded(filePath);
    return true;
}
