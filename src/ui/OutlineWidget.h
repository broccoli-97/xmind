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
    void syncSelection();

signals:
    void closeRequested();

private:
    void buildSubtree(NodeItem* node, QTreeWidgetItem* parentItem);
    void onItemClicked(QTreeWidgetItem* item, int column);
    // Tree fold → scene collapse. Folding/unfolding a branch in the outline
    // drives NodeItem::setCollapsed so the canvas hides/shows the subtree and
    // paints the collapsed indicator.
    void onItemCollapsed(QTreeWidgetItem* item);
    void onItemExpanded(QTreeWidgetItem* item);
    // Scene collapse → tree fold. Mirrors a canvas-side toggle (Space) onto the
    // matching outline item.
    void syncCollapseState(NodeItem* node);

    // Resolve an item's stored NodeItem*, validated against the current scene's
    // items so a stale pointer can't be dereferenced.
    NodeItem* nodeForItem(QTreeWidgetItem* item) const;

    QTreeWidget* m_tree = nullptr;
    MindMapScene* m_scene = nullptr;
    MindMapView* m_view = nullptr;
    // Guards the tree↔scene mirror against feedback: set while we mutate item
    // fold state programmatically so the resulting itemCollapsed/itemExpanded
    // signals don't loop back into the scene.
    bool m_syncing = false;
};
