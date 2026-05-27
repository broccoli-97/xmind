#include "ui/MindMapToolBar.h"
#include "core/FileManager.h"
#include "scene/MindMapScene.h"
#include "scene/MindMapView.h"
#include "ui/IconFactory.h"
#include "ui/TabManager.h"

#include <QAction>
#include <QFrame>
#include <QHBoxLayout>
#include <QMenu>
#include <QToolButton>
#include <QUndoStack>

MindMapToolBar::MindMapToolBar(TabManager* tabManager,
                               FileManager* fileManager,
                               QAction* undoAct,
                               QAction* redoAct,
                               QWidget* parent)
    : QWidget(parent),
      m_tabManager(tabManager),
      m_fileManager(fileManager),
      m_undoAct(undoAct),
      m_redoAct(redoAct) {
    setObjectName("inlineToolbar");
    buildContent();
}

QToolButton* MindMapToolBar::addButton(const QString& iconName, const QString& text,
                                       const QString& tooltip) {
    auto* btn = new QToolButton(this);
    btn->setProperty("iconName", iconName);
    btn->setIcon(IconFactory::makeToolIcon(iconName));
    btn->setText(text);
    btn->setToolTip(tooltip);
    btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btn->setAutoRaise(true);
    btn->setIconSize(QSize(24, 24));
    m_layout->addWidget(btn);
    return btn;
}

void MindMapToolBar::addSeparator() {
    auto* sep = new QFrame(this);
    sep->setFrameShape(QFrame::VLine);
    sep->setFrameShadow(QFrame::Sunken);
    sep->setFixedHeight(28);
    m_layout->addWidget(sep);
}

void MindMapToolBar::buildContent() {
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(4, 2, 4, 2);
    m_layout->setSpacing(2);

    m_layout->addStretch();

    auto withScene = [this](auto&& f) {
        if (auto* s = m_tabManager->currentScene())
            f(s);
    };
    auto withView = [this](auto&& f) {
        if (auto* v = m_tabManager->currentView())
            f(v);
    };

    auto* undoBtn = addButton("undo", tr("Undo"), tr("Undo last action (Ctrl+Z)"));
    undoBtn->setEnabled(false);
    connect(undoBtn, &QToolButton::clicked, this,
            [withScene]() { withScene([](MindMapScene* s) { s->undoStack()->undo(); }); });
    connect(m_undoAct, &QAction::changed, undoBtn,
            [this, undoBtn]() { undoBtn->setEnabled(m_undoAct->isEnabled()); });

    auto* redoBtn = addButton("redo", tr("Redo"), tr("Redo last action (Ctrl+Y)"));
    redoBtn->setEnabled(false);
    connect(redoBtn, &QToolButton::clicked, this,
            [withScene]() { withScene([](MindMapScene* s) { s->undoStack()->redo(); }); });
    connect(m_redoAct, &QAction::changed, redoBtn,
            [this, redoBtn]() { redoBtn->setEnabled(m_redoAct->isEnabled()); });

    addSeparator();

    auto* addChildBtn = addButton("add-child", tr("Add Child"), tr("Add a child node (Enter)"));
    connect(addChildBtn, &QToolButton::clicked, this,
            [withScene]() { withScene([](MindMapScene* s) { s->addChildToSelected(); }); });

    auto* addSiblingBtn =
        addButton("add-sibling", tr("Add Sibling"), tr("Add a sibling node (Ctrl+Enter)"));
    connect(addSiblingBtn, &QToolButton::clicked, this,
            [withScene]() { withScene([](MindMapScene* s) { s->addSiblingToSelected(); }); });

    auto* deleteBtn = addButton("delete", tr("Delete"), tr("Delete selected node (Del)"));
    connect(deleteBtn, &QToolButton::clicked, this,
            [withScene]() { withScene([](MindMapScene* s) { s->deleteSelected(); }); });

    addSeparator();

    auto* layoutBtn =
        addButton("auto-layout", tr("Auto Layout"), tr("Automatically arrange all nodes (Ctrl+L)"));
    connect(layoutBtn, &QToolButton::clicked, this,
            [withScene]() { withScene([](MindMapScene* s) { s->autoLayout(); }); });

    addSeparator();

    auto* zoomInBtn = addButton("zoom-in", tr("Zoom In"), tr("Zoom in (Ctrl++)"));
    connect(zoomInBtn, &QToolButton::clicked, this,
            [withView]() { withView([](MindMapView* v) { v->zoomIn(); }); });

    auto* zoomOutBtn = addButton("zoom-out", tr("Zoom Out"), tr("Zoom out (Ctrl+-)"));
    connect(zoomOutBtn, &QToolButton::clicked, this,
            [withView]() { withView([](MindMapView* v) { v->zoomOut(); }); });

    auto* fitBtn = addButton("fit-view", tr("Fit View"), tr("Fit all nodes in view (Ctrl+0)"));
    connect(fitBtn, &QToolButton::clicked, this,
            [withView]() { withView([](MindMapView* v) { v->zoomToFit(); }); });

    addSeparator();

    auto* exportBtn = new QToolButton(this);
    exportBtn->setProperty("iconName", "export");
    exportBtn->setIcon(IconFactory::makeToolIcon("export"));
    exportBtn->setText(tr("Export"));
    exportBtn->setToolTip(tr("Export mind map"));
    exportBtn->setPopupMode(QToolButton::InstantPopup);
    exportBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    exportBtn->setAutoRaise(true);
    exportBtn->setIconSize(QSize(24, 24));
    auto* exportMenu = new QMenu(exportBtn);
    exportMenu->addAction(tr("As Text..."), m_fileManager, &FileManager::exportAsText);
    exportMenu->addAction(tr("As Markdown..."), m_fileManager, &FileManager::exportAsMarkdown);
    exportMenu->addSeparator();
    exportMenu->addAction(tr("As PNG..."), m_fileManager, &FileManager::exportAsPng);
    exportMenu->addAction(tr("As SVG..."), m_fileManager, &FileManager::exportAsSvg);
    exportMenu->addAction(tr("As PDF..."), m_fileManager, &FileManager::exportAsPdf);
    exportBtn->setMenu(exportMenu);
    m_layout->addWidget(exportBtn);

    m_layout->addStretch();

    auto* closeBtn = new QToolButton(this);
    closeBtn->setIcon(IconFactory::makeToolIcon("close-panel"));
    closeBtn->setProperty("iconName", "close-panel");
    closeBtn->setToolTip(tr("Hide Toolbar"));
    closeBtn->setAutoRaise(true);
    closeBtn->setFixedSize(20, 20);
    closeBtn->setIconSize(QSize(14, 14));
    closeBtn->setObjectName("closePanelBtn");
    connect(closeBtn, &QToolButton::clicked, this, &MindMapToolBar::closeRequested);
    m_layout->addWidget(closeBtn);
}
