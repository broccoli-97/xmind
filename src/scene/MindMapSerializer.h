#pragma once

#include <QString>

class MindMapScene;
class NodeItem;
class QJsonObject;

class MindMapSerializer {
public:
    explicit MindMapSerializer(MindMapScene* scene);

    QJsonObject toJson() const;
    bool fromJson(const QJsonObject& json);
    bool saveToFile(const QString& filePath);
    bool loadFromFile(const QString& filePath);

private:
    QJsonObject nodeToJson(NodeItem* node) const;
    NodeItem* nodeFromJson(const QJsonObject& json, NodeItem* parent);

    MindMapScene* m_scene;
};
