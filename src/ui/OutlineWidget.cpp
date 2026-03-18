#include "ui/OutlineWidget.h"
#include "ui/IconFactory.h"
#include "scene/MindMapScene.h"
#include "scene/MindMapView.h"
#include "scene/NodeItem.h"

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
    if (!node || !m_scene)
        return;

    // Validate the pointer against existing scene nodes to avoid dangling references
    bool found = false;
    const auto sceneItems = m_scene->items();
    for (auto* sceneItem : sceneItems) {
        if (sceneItem == node) {
            found = true;
            break;
        }
    }
    if (!found)
        return;

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
