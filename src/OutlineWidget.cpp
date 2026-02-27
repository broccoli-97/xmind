#include "OutlineWidget.h"
#include "MindMapScene.h"
#include "MindMapView.h"
#include "NodeItem.h"
#include "ThemeManager.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

OutlineWidget::OutlineWidget(QWidget* parent) : QWidget(parent) {
    setMinimumWidth(120);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Header row with label + close button
    auto* header = new QWidget();
    header->setObjectName("sectionHeader");
    auto* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(10, 6, 4, 6);
    headerLayout->setSpacing(0);

    auto* outlineLabel = new QLabel("Outline");
    headerLayout->addWidget(outlineLabel);
    headerLayout->addStretch();

    auto* closeBtn = new QToolButton();
    closeBtn->setIcon(ThemeManager::makeToolIcon("close-panel"));
    closeBtn->setProperty("iconName", "close-panel");
    closeBtn->setToolTip("Hide Outline");
    closeBtn->setAutoRaise(true);
    closeBtn->setFixedSize(20, 20);
    closeBtn->setIconSize(QSize(14, 14));
    closeBtn->setObjectName("closePanelBtn");
    connect(closeBtn, &QToolButton::clicked, this, &OutlineWidget::closeRequested);
    headerLayout->addWidget(closeBtn);

    layout->addWidget(header);

    m_tree = new QTreeWidget();
    m_tree->setHeaderHidden(true);
    m_tree->setAnimated(true);
    m_tree->setIndentation(16);
    m_tree->setExpandsOnDoubleClick(false);
    connect(m_tree, &QTreeWidget::itemClicked, this, &OutlineWidget::onItemClicked);
    layout->addWidget(m_tree, 1);
}

void OutlineWidget::refresh(MindMapScene* scene) {
    m_scene = scene;

    if (!m_tree)
        return;

    m_tree->clear();

    if (!m_scene)
        return;

    auto* root = m_scene->rootNode();
    if (!root)
        return;

    auto* rootItem = new QTreeWidgetItem(m_tree);
    rootItem->setText(0, root->text());
    rootItem->setData(0, Qt::UserRole, QVariant::fromValue(reinterpret_cast<quintptr>(root)));
    rootItem->setExpanded(true);

    buildSubtree(root, rootItem);
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
