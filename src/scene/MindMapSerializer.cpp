#include "scene/MindMapSerializer.h"
#include "core/TemplateRegistry.h"
#include "scene/MindMapScene.h"
#include "scene/NodeItem.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUndoStack>

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
    root["version"] = 2;
    root["layoutStyle"] = static_cast<int>(m_scene->m_layoutStyle);
    if (!m_scene->m_templateId.isEmpty())
        root["templateId"] = m_scene->m_templateId;
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

bool MindMapSerializer::fromJson(const QJsonObject& json) {
    if (json["format"].toString() != "ymind")
        return false;

    m_scene->clearScene();
    m_scene->m_batchLoading = true;

    // Restore layout style (default to Bilateral for old files)
    m_scene->m_layoutStyle = static_cast<LayoutStyle>(json["layoutStyle"].toInt(0));

    // Restore template ID; for old files (v1), map layoutStyle to builtin ID
    if (json.contains("templateId")) {
        m_scene->m_templateId = json["templateId"].toString();
    } else {
        m_scene->m_templateId =
            TemplateRegistry::builtinIdForLayoutStyle(json["layoutStyle"].toInt(0));
    }

    QJsonObject rootObj = json["root"].toObject();
    m_scene->m_rootNode = nodeFromJson(rootObj, nullptr);
    if (!m_scene->m_rootNode) {
        // Fallback: create default root
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
