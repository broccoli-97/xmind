#include "TabManager.h"
#include "MindMapScene.h"
#include "MindMapView.h"
#include "StartPage.h"
#include "ThemeManager.h"

#include <QAction>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QStackedWidget>
#include <QTabBar>
#include <QTimer>
#include <QToolButton>
#include <QUndoStack>

TabManager::TabManager(QWidget* parent) : QObject(parent), m_parentWidget(parent) {}

void TabManager::init(QAction* undoAct, QAction* redoAct) {
    m_undoAct = undoAct;
    m_redoAct = redoAct;

    // Create tab bar
    m_tabBar = new QTabBar(m_parentWidget);
    m_tabBar->setTabsClosable(true);
    m_tabBar->setMovable(true);
    m_tabBar->setDocumentMode(true);
    m_tabBar->setDrawBase(false);
    m_tabBar->setExpanding(false);
    m_tabBar->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    m_tabBar->setContextMenuPolicy(Qt::CustomContextMenu);

    // Create "+" button
    m_newTabBtn = new QToolButton(m_parentWidget);
    m_newTabBtn->setText("+");
    m_newTabBtn->setToolTip("New Tab (Ctrl+T)");
    m_newTabBtn->setAutoRaise(true);
    m_newTabBtn->setFixedSize(28, 28);
    m_newTabBtn->setObjectName("newTabBtn");

    // Create content stack
    m_contentStack = new QStackedWidget(m_parentWidget);

    // Connect tab bar signals
    connect(m_tabBar, &QTabBar::currentChanged, this, &TabManager::switchToTab);
    connect(m_tabBar, &QTabBar::tabCloseRequested, this, &TabManager::closeTab);
    connect(m_tabBar, &QTabBar::tabMoved, this, &TabManager::onTabMoved);
    connect(m_tabBar, &QWidget::customContextMenuRequested, this, &TabManager::onTabBarContextMenu);

    // Connect "+" button
    connect(m_newTabBtn, &QToolButton::clicked, this, &TabManager::addNewTab);
}

void TabManager::onTabMoved(int from, int to) {
    m_tabs.move(from, to);
    QWidget* w = m_contentStack->widget(from);
    QSignalBlocker blocker(m_contentStack);
    m_contentStack->removeWidget(w);
    m_contentStack->insertWidget(to, w);
    m_contentStack->setCurrentIndex(m_tabBar->currentIndex());
}

void TabManager::addNewTab() {
    auto* scene = new MindMapScene(m_parentWidget);
    auto* view = new MindMapView(m_parentWidget);
    view->setScene(scene);

    auto* stack = new QStackedWidget(m_parentWidget);
    auto* startPage = StartPage::create(
        this,
        [this](int index) {
            int tabIdx = m_tabBar->currentIndex();
            if (tabIdx < 0)
                return;

            if (!isTabEmpty(tabIdx)) {
                addNewTab();
                tabIdx = m_tabBar->currentIndex();
            }

            m_currentFile.clear();
            m_tabs[tabIdx].filePath.clear();

            StartPage::loadTemplate(index, m_tabs[tabIdx].scene);

            // switch to view first
            if (m_tabs[tabIdx].stack)
                m_tabs[tabIdx].stack->setCurrentIndex(1);

            // defer zoomToFit until after layout/resize completes
            if (m_tabs[tabIdx].view) {
                QTimer::singleShot(0, m_tabs[tabIdx].view,
                                   [view = m_tabs[tabIdx].view]() { view->zoomToFit(); });
            }

            updateTabText(tabIdx);
            emit currentTabChanged(tabIdx);
        },
        [this]() {
            int tabIdx = m_tabBar->currentIndex();
            if (tabIdx < 0)
                return;

            if (m_tabs[tabIdx].stack)
                m_tabs[tabIdx].stack->setCurrentIndex(1);

            updateTabText(tabIdx);
            emit currentTabChanged(tabIdx);
        });
    stack->addWidget(startPage); // index 0 — start page
    stack->addWidget(view);      // index 1 — mind map view
    stack->setCurrentIndex(0);   // show start page

    addTab(scene, view, stack, QString());
}

void TabManager::addTab(MindMapScene* scene, MindMapView* view, QStackedWidget* stack,
                        const QString& filePath) {
    TabState tab;
    tab.scene = scene;
    tab.view = view;
    tab.stack = stack;
    tab.filePath = filePath;

    connectSceneSignals(scene);

    {
        QSignalBlocker blocker(m_tabBar);
        m_tabs.append(tab);
        QString label = filePath.isEmpty() ? "Untitled" : QFileInfo(filePath).fileName();
        m_tabBar->addTab(label);
        m_contentStack->addWidget(stack);
    }

    m_tabBar->setCurrentIndex(m_tabs.size() - 1);
    switchToTab(m_tabs.size() - 1);
}

void TabManager::connectSceneSignals(MindMapScene* scene) {
    connect(scene, &MindMapScene::modifiedChanged, this, [this, scene](bool) {
        for (int i = 0; i < m_tabs.size(); ++i) {
            if (m_tabs[i].scene == scene) {
                updateTabText(i);
                if (i == m_tabBar->currentIndex())
                    emit currentTabChanged(i);
                break;
            }
        }
    });

    connect(scene, &MindMapScene::fileLoaded, this, [this, scene](const QString& path) {
        for (int i = 0; i < m_tabs.size(); ++i) {
            if (m_tabs[i].scene == scene) {
                m_tabs[i].filePath = path;
                updateTabText(i);
                if (i == m_tabBar->currentIndex()) {
                    m_currentFile = path;
                    emit currentTabChanged(i);
                }
                break;
            }
        }
    });
}

void TabManager::switchToTab(int index) {
    if (index < 0 || index >= m_tabs.size())
        return;

    disconnectUndoStack();

    m_scene = m_tabs[index].scene;
    m_view = m_tabs[index].view;
    m_currentFile = m_tabs[index].filePath;

    m_contentStack->setCurrentIndex(index);

    connectUndoStack();
    emit currentTabChanged(index);
}

void TabManager::disconnectUndoStack() {
    if (!m_scene)
        return;
    auto* stack = m_scene->undoStack();
    disconnect(stack, nullptr, m_undoAct, nullptr);
    disconnect(stack, nullptr, m_redoAct, nullptr);
    disconnect(stack, &QUndoStack::undoTextChanged, this, nullptr);
    disconnect(stack, &QUndoStack::redoTextChanged, this, nullptr);
    disconnect(stack, &QUndoStack::indexChanged, this, nullptr);
}

void TabManager::connectUndoStack() {
    if (!m_scene)
        return;
    auto* stack = m_scene->undoStack();
    connect(stack, &QUndoStack::canUndoChanged, m_undoAct, &QAction::setEnabled);
    connect(stack, &QUndoStack::canRedoChanged, m_redoAct, &QAction::setEnabled);
    connect(stack, &QUndoStack::undoTextChanged, this, [this](const QString& text) {
        m_undoAct->setText(text.isEmpty() ? "&Undo" : "&Undo " + text);
    });
    connect(stack, &QUndoStack::redoTextChanged, this, [this](const QString& text) {
        m_redoAct->setText(text.isEmpty() ? "&Redo" : "&Redo " + text);
    });
    m_undoAct->setEnabled(stack->canUndo());
    m_redoAct->setEnabled(stack->canRedo());
}

void TabManager::updateTabText(int index) {
    if (index < 0 || index >= m_tabs.size())
        return;
    const auto& tab = m_tabs[index];
    QString label = tab.filePath.isEmpty() ? "Untitled" : QFileInfo(tab.filePath).fileName();
    if (tab.scene->isModified())
        label.prepend("* ");
    m_tabBar->setTabText(index, label);
}

bool TabManager::isTabEmpty(int index) const {
    if (index < 0 || index >= m_tabs.size())
        return false;
    const auto& tab = m_tabs[index];
    if (tab.stack) {
        QWidget* current = tab.stack->currentWidget();
        if (current && current->objectName() == "startPage")
            return true;
    }
    return tab.filePath.isEmpty() && !tab.scene->isModified();
}

int TabManager::findTabByFilePath(const QString& filePath) const {
    for (int i = 0; i < m_tabs.size(); ++i) {
        if (m_tabs[i].filePath == filePath)
            return i;
    }
    return -1;
}

bool TabManager::maybeSaveTab(int index) {
    if (index < 0 || index >= m_tabs.size())
        return true;
    auto* scene = m_tabs[index].scene;
    if (!scene->isModified())
        return true;

    QString name = m_tabs[index].filePath.isEmpty() ? "Untitled"
                                                    : QFileInfo(m_tabs[index].filePath).fileName();

    auto ret = QMessageBox::warning(m_parentWidget, "XMind",
                                    QString("The mind map \"%1\" has been modified.\n"
                                            "Do you want to save your changes?")
                                        .arg(name),
                                    QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (ret == QMessageBox::Save) {
        auto* prevScene = m_scene;
        auto* prevView = m_view;
        auto prevFile = m_currentFile;

        m_scene = m_tabs[index].scene;
        m_view = m_tabs[index].view;
        m_currentFile = m_tabs[index].filePath;

        emit saveRequested();

        m_scene = prevScene;
        m_view = prevView;
        m_currentFile = prevFile;

        return !m_tabs[index].scene->isModified();
    }
    if (ret == QMessageBox::Cancel)
        return false;

    return true; // Discard
}

bool TabManager::maybeSave() {
    for (int i = 0; i < m_tabs.size(); ++i) {
        if (!maybeSaveTab(i))
            return false;
    }
    return true;
}

void TabManager::closeTab(int index) {
    if (index < 0 || index >= m_tabs.size())
        return;

    if (!maybeSaveTab(index))
        return;

    auto tab = m_tabs[index];

    m_tabs.removeAt(index);
    m_tabBar->removeTab(index);
    m_contentStack->removeWidget(tab.stack);

    delete tab.scene;
    delete tab.stack;

    if (m_tabs.isEmpty())
        addNewTab();
}

void TabManager::onTabBarContextMenu(const QPoint& pos) {
    int index = m_tabBar->tabAt(pos);

    QMenu menu(m_parentWidget);

    auto* newTabAct = menu.addAction("New Tab");
    connect(newTabAct, &QAction::triggered, this, &TabManager::addNewTab);

    if (index >= 0) {
        menu.addSeparator();
        auto* closeAct = menu.addAction("Close");
        connect(closeAct, &QAction::triggered, this, [this, index]() { closeTab(index); });

        auto* closeOthersAct = menu.addAction("Close Others");
        connect(closeOthersAct, &QAction::triggered, this, [this, index]() {
            for (int i = m_tabs.size() - 1; i > index; --i)
                closeTab(i);
            for (int i = index - 1; i >= 0; --i)
                closeTab(i);
        });
    }

    menu.exec(m_tabBar->mapToGlobal(pos));
}

// Accessors
int TabManager::currentIndex() const {
    return m_tabBar ? m_tabBar->currentIndex() : -1;
}

int TabManager::tabCount() const {
    return m_tabs.size();
}

const QList<TabState>& TabManager::tabs() const {
    return m_tabs;
}

TabState& TabManager::tab(int index) {
    return m_tabs[index];
}

MindMapScene* TabManager::currentScene() const {
    return m_scene;
}

MindMapView* TabManager::currentView() const {
    return m_view;
}

QString TabManager::currentFilePath() const {
    return m_currentFile;
}

void TabManager::setCurrentFilePath(const QString& path) {
    m_currentFile = path;
    int cur = m_tabBar->currentIndex();
    if (cur >= 0 && cur < m_tabs.size())
        m_tabs[cur].filePath = path;
}

QTabBar* TabManager::tabBar() const {
    return m_tabBar;
}

QToolButton* TabManager::newTabButton() const {
    return m_newTabBtn;
}

QStackedWidget* TabManager::contentStack() const {
    return m_contentStack;
}
