#include "ui/OutlineWidget.h"
#include "scene/MindMapScene.h"
#include "scene/MindMapView.h"
#include "scene/NodeItem.h"
#include "ui/IconFactory.h"
#include "ui/OutlineItemDelegate.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItemIterator>
#include <QVBoxLayout>

OutlineWidget::OutlineWidget(QWidget* parent) : QWidget(parent) {
    setMinimumWidth(120);
    setObjectName("outlinePanel");
    setAttribute(Qt::WA_StyledBackground, true);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Title row with close button
    auto* titleRow = new QHBoxLayout();
    titleRow->setContentsMargins(0, 0, 0, 0);
    titleRow->setSpacing(0);
    auto* titleLabel = new QLabel(tr("Outline"));
    titleLabel->setObjectName("outlineTitle");
    titleRow->addWidget(titleLabel);
    titleRow->addStretch();
    auto* closeBtn = new QToolButton(this);
    closeBtn->setIcon(IconFactory::makeToolIcon("close-panel"));
    closeBtn->setProperty("iconName", "close-panel");
    closeBtn->setToolTip(tr("Hide Outline"));
    closeBtn->setAutoRaise(true);
    closeBtn->setFixedSize(20, 20);
    closeBtn->setIconSize(QSize(14, 14));
    closeBtn->setObjectName("closePanelBtn");
    connect(closeBtn, &QToolButton::clicked, this, &OutlineWidget::closeRequested);
    titleRow->addWidget(closeBtn);
    layout->addLayout(titleRow);

    m_tree = new QTreeWidget();
    m_tree->setObjectName("outlineTree");
    m_tree->setHeaderHidden(true);
    m_tree->setAnimated(true);
    m_tree->setIndentation(20);
    m_tree->setExpandsOnDoubleClick(false);
    m_tree->setRootIsDecorated(true);
    m_tree->setFocusPolicy(Qt::NoFocus);
    m_tree->setMouseTracking(true); // enable hover state for the rounded hover pill
    m_tree->setItemDelegate(new OutlineItemDelegate(m_tree));
    connect(m_tree, &QTreeWidget::itemClicked, this, &OutlineWidget::onItemClicked);
    connect(m_tree, &QTreeWidget::itemCollapsed, this, &OutlineWidget::onItemCollapsed);
    connect(m_tree, &QTreeWidget::itemExpanded, this, &OutlineWidget::onItemExpanded);
    layout->addWidget(m_tree, 1);
}

void OutlineWidget::refresh(MindMapScene* scene) {
    // Disconnect old scene
    if (m_scene) {
        disconnect(m_scene, &QGraphicsScene::selectionChanged, this, &OutlineWidget::syncSelection);
        disconnect(m_scene, &MindMapScene::nodeCollapseChanged, this,
                   &OutlineWidget::syncCollapseState);
    }

    m_scene = scene;

    if (!m_tree)
        return;

    // The whole rebuild sets item fold state programmatically; suppress the
    // resulting itemCollapsed/itemExpanded signals so they don't echo back into
    // the scene.
    m_syncing = true;
    m_tree->clear();

    if (!m_scene) {
        m_syncing = false;
        return;
    }

    // Connect selection changes so outline tracks clicks/edits on the canvas,
    // and collapse changes so a Space-key fold on the canvas mirrors here.
    connect(m_scene, &QGraphicsScene::selectionChanged, this, &OutlineWidget::syncSelection);
    connect(m_scene, &MindMapScene::nodeCollapseChanged, this, &OutlineWidget::syncCollapseState);

    auto* root = m_scene->rootNode();
    if (!root) {
        m_syncing = false;
        return;
    }

    auto* rootItem = new QTreeWidgetItem(m_tree);
    rootItem->setText(0, root->text());
    rootItem->setData(0, Qt::UserRole, QVariant::fromValue(reinterpret_cast<quintptr>(root)));
    rootItem->setExpanded(!root->isCollapsed());

    buildSubtree(root, rootItem);

    m_syncing = false;

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
        childItem->setExpanded(!child->isCollapsed());
        buildSubtree(child, childItem);
    }
}

NodeItem* OutlineWidget::nodeForItem(QTreeWidgetItem* item) const {
    if (!item || !m_scene)
        return nullptr;

    quintptr ptr = item->data(0, Qt::UserRole).value<quintptr>();
    auto* node = reinterpret_cast<NodeItem*>(ptr);
    if (!node)
        return nullptr;

    // Validate the pointer against existing scene nodes to avoid dangling references
    const auto sceneItems = m_scene->items();
    for (auto* sceneItem : sceneItems) {
        if (sceneItem == node)
            return node;
    }
    return nullptr;
}

void OutlineWidget::onItemClicked(QTreeWidgetItem* item, int /*column*/) {
    auto* node = nodeForItem(item);
    if (!node)
        return;

    m_scene->clearSelection();
    node->setSelected(true);
    if (m_view)
        m_view->centerOn(node);
}

void OutlineWidget::onItemCollapsed(QTreeWidgetItem* item) {
    if (m_syncing)
        return;
    // toggleNodeCollapsed owns the modified flag, the change signal, and the
    // re-layout that tightens the canvas around the folded branch.
    if (auto* node = nodeForItem(item); node && !node->isCollapsed())
        m_scene->toggleNodeCollapsed(node);
}

void OutlineWidget::onItemExpanded(QTreeWidgetItem* item) {
    if (m_syncing)
        return;
    if (auto* node = nodeForItem(item); node && node->isCollapsed())
        m_scene->toggleNodeCollapsed(node);
}

void OutlineWidget::syncCollapseState(NodeItem* node) {
    if (!node || !m_tree)
        return;

    quintptr target = reinterpret_cast<quintptr>(node);
    QTreeWidgetItemIterator it(m_tree);
    while (*it) {
        if ((*it)->data(0, Qt::UserRole).value<quintptr>() == target) {
            // Mirror the canvas fold onto this item without letting the
            // resulting signal loop back into the scene.
            m_syncing = true;
            (*it)->setExpanded(!node->isCollapsed());
            m_syncing = false;
            break;
        }
        ++it;
    }
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
