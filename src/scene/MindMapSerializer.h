#pragma once

#include <QString>

class MindMapScene;
class NodeItem;
class QJsonObject;

class MindMapSerializer {
public:
    explicit MindMapSerializer(MindMapScene* scene);

    // Current on-disk format version. Bump when toJson() changes shape, and
    // add a migrator in MindMapSerializer.cpp's `migrators` table.
    // v4: added per-node `collapsed` boolean (omitted when false).
    static constexpr int kCurrentVersion = 4;

    QJsonObject toJson() const;
    bool fromJson(const QJsonObject& json);
    bool saveToFile(const QString& filePath);
    bool loadFromFile(const QString& filePath);

    // Run the chain of registered migrators on `json` until it reaches the
    // current version. Exposed for tests; fromJson() runs it internally.
    static QJsonObject migrate(QJsonObject json);

private:
    QJsonObject nodeToJson(NodeItem* node) const;
    NodeItem* nodeFromJson(const QJsonObject& json, NodeItem* parent);

    MindMapScene* m_scene;
};
