#pragma once

#include <QObject>
#include <QPointF>

class MindMapScene;
class NodeItem;
class QLineEdit;
class QGraphicsProxyWidget;

class InlineEditController : public QObject {
    Q_OBJECT

public:
    explicit InlineEditController(MindMapScene* scene, QObject* parent = nullptr);

    bool isEditing() const;
    NodeItem* editingNode() const;

    void startEditing(NodeItem* node);
    void finishEditing();
    void cancelEditing();

    // Returns true if the click was outside the editor and editing was committed
    bool handleMousePress(const QPointF& scenePos);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    MindMapScene* m_scene;
    NodeItem* m_editingNode = nullptr;
    QGraphicsProxyWidget* m_editProxy = nullptr;
    QLineEdit* m_editLineEdit = nullptr;
};
