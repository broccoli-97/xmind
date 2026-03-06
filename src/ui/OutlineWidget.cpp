#include "ui/OutlineWidget.h"
#include "scene/MindMapScene.h"
#include "scene/MindMapView.h"
#include "scene/NodeItem.h"

#include <QLabel>
#include <QTreeWidget>
#include <QTreeWidgetItemIterator>
#include <QVBoxLayout>

OutlineWidget::OutlineWidget(QWidget* parent) : QWidget(parent) {
    setMinimumWidth(120);
    setObjectName("outlinePanel");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Bold "Outline" title
    auto* titleLabel = new QLabel("Outline");
    titleLabel->setObjectName("outlineTitle");
    layout->addWidget(titleLabel);

    m_tree = new QTreeWidget();
    m_tree->setObjectName("outlineTree");
    m_tree->setHeaderHidden(true);
    m_tree->setAnimated(true);
    m_tree->setIndentation(20);
    m_tree->setExpandsOnDoubleClick(false);
    m_tree->setRootIsDecorated(true);
    m_tree->setFocusPolicy(Qt::NoFocus);
    connect(m_tree, &QTreeWidget::itemClicked, this, &OutlineWidget::onItemClicked);
    layout->addWidget(m_tree, 1);
}

void OutlineWidget::refresh(MindMapScene* scene) {
    // Disconnect old scene
    if (m_scene)
        disconnect(m_scene, &QGraphicsScene::selectionChanged, this, &OutlineWidget::syncSelection);

    m_scene = scene;

    if (!m_tree)
        return;

    m_tree->clear();

    if (!m_scene)
        return;

    // Connect selection changes so outline tracks clicks/edits on the canvas
    connect(m_scene, &QGraphicsScene::selectionChanged, this, &OutlineWidget::syncSelection);

    auto* root = m_scene->rootNode();
    if (!root)
        return;

    auto* rootItem = new QTreeWidgetItem(m_tree);
    rootItem->setText(0, root->text());
    rootItem->setData(0, Qt::UserRole, QVariant::fromValue(reinterpret_cast<quintptr>(root)));
    rootItem->setExpanded(true);

    buildSubtree(root, rootItem);

    syncSelection();
}

void OutlineWidget::setView(MindMapView* view) {
    m_view = view;
}

void OutlineWidget::buildSubtree(NodeItem* node, QTreeWidgetItem* parentItem) {
    for (auto* child : node->childNodes()) {
        auto* childItem = new QTreeWidgetItem(parentItem);
        childItem->setText(0, child->text());
        childItem->setData(0, Qt::UserRole, QVariant::fromValue(reinterpret_cast<quintptr>(child)));
        childItem->setExpanded(true);
        buildSubtree(child, childItem);
    }
}

void OutlineWidget::onItemClicked(QTreeWidgetItem* item, int /*column*/) {
    quintptr ptr = item->data(0, Qt::UserRole).value<quintptr>();
    auto* node = reinterpret_cast<NodeItem*>(ptr);
    if (!node)
        return;

    if (m_scene)
        m_scene->clearSelection();
    node->setSelected(true);
    if (m_view)
        m_view->centerOn(node);
}

void OutlineWidget::syncSelection() {
    if (!m_scene || !m_tree)
        return;

    NodeItem* selected = m_scene->selectedNode();
    if (!selected) {
        m_tree->clearSelection();
        m_tree->setCurrentItem(nullptr);
        return;
    }

    // Block signals to avoid feedback loop (setCurrentItem would trigger itemClicked)
    bool blocked = m_tree->blockSignals(true);

    quintptr target = reinterpret_cast<quintptr>(selected);
    QTreeWidgetItemIterator it(m_tree);
    while (*it) {
        if ((*it)->data(0, Qt::UserRole).value<quintptr>() == target) {
            m_tree->setCurrentItem(*it);
            m_tree->scrollToItem(*it);
            break;
        }
        ++it;
    }

    m_tree->blockSignals(blocked);
}
