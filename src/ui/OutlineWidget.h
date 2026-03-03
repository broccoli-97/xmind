#pragma once

#include <QWidget>

class MindMapScene;
class MindMapView;
class NodeItem;
class QTreeWidget;
class QTreeWidgetItem;

class OutlineWidget : public QWidget {
    Q_OBJECT
public:
    explicit OutlineWidget(QWidget* parent = nullptr);

    void refresh(MindMapScene* scene);
    void setView(MindMapView* view);

signals:
    void closeRequested();

private:
    void buildSubtree(NodeItem* node, QTreeWidgetItem* parentItem);
    void onItemClicked(QTreeWidgetItem* item, int column);

    QTreeWidget* m_tree = nullptr;
    MindMapScene* m_scene = nullptr;
    MindMapView* m_view = nullptr;
};
