#include "core/MainWindow.h"
#include "core/AboutDialog.h"
#include "core/AppSettings.h"
#include "core/AutoSaveManager.h"
#include "core/FileManager.h"
#include "core/RecoveryManager.h"
#include "core/Services.h"
#include "core/SettingsDialog.h"
#include "core/TemplateRegistry.h"
#include "core/ThemeRegistry.h"
#include "core/UpdateNotifier.h"
#include "layout/LayoutAlgorithmRegistry.h"
#include "scene/MindMapScene.h"
#include "scene/MindMapView.h"
#include "scene/NodeItem.h"
#include "ui/FindBar.h"
#include "ui/IconFactory.h"
#include "ui/MindMapToolBar.h"
#include "ui/OutlineWidget.h"
#include "ui/TabManager.h"
#include "ui/TemplateMenuController.h"
#include "ui/ThemeManager.h"
#include "ui/ThemeMenuController.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSplitter>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QStatusBar>
#include <QTabBar>
#include <QToolButton>
#include <QUndoStack>
#include <QVBoxLayout>

template <typename F> void MainWindow::withCurrentScene(F&& f) {
    if (auto* s = m_tabManager->currentScene())
        f(s);
}

template <typename F> void MainWindow::withCurrentView(F&& f) {
    if (auto* v = m_tabManager->currentView())
        f(v);
}

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
MainWindow::MainWindow(const Services& services, QWidget* parent)
    : QMainWindow(parent), m_services(&services) {
    resize(1280, 800);

    // Initialize registries before anything else
    m_services->layouts->registerBuiltins();
    m_services->templates->loadBuiltins();
    m_services->themes->loadBuiltins();
    MindMapScene::setDefaultStyleProvider(m_services->styleProvider);
    const QString userData =
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/YMind";
    m_services->templates->loadFromDirectory(userData + "/templates");
    // User themes live in their own folder; also accept legacy themes dropped
    // alongside templates so existing downloads keep working.
    m_services->themes->loadFromDirectory(userData + "/themes");
    m_services->themes->loadFromDirectory(userData + "/templates");

    m_tabManager = new TabManager(this);
    m_fileManager = new FileManager(this, m_tabManager, this);

    // Apply theme before creating any tabs so QTabBar computes tab geometry
    // with the stylesheet already in effect — otherwise the first tab is sized
    // using the default style and a visible gap appears next to the "+" button.
    applyTheme();

    setupActions();

    m_tabManager->init(m_undoAct, m_redoAct);

    m_updateNotifier = new UpdateNotifier(this);
    connect(m_updateNotifier, &UpdateNotifier::upToDateMessage, this,
            [this](const QString& title, const QString& msg) {
                QMessageBox::information(this, title, msg);
            });
    connect(m_updateNotifier, &UpdateNotifier::checkFailedMessage, this,
            [this](const QString& title, const QString& msg) {
                QMessageBox::warning(this, title, msg);
            });

    setupCentralLayout();
    setupMenuBar();

    // Wire cross-module signals
    connect(m_tabManager, &TabManager::currentTabChanged, this, [this](int) {
        updateWindowTitle();
        refreshOutline();
        updateContentVisibility();
        connectCurrentSceneToStatusHint();
        updateStatusHint();
        // Search state is per-scene; switching tabs invalidates it.
        if (m_findBar && m_findBar->isVisible())
            closeFindBar();

        auto* scene = m_tabManager->currentScene();
        if (scene) {
            disconnect(scene->undoStack(), &QUndoStack::indexChanged, this, nullptr);
            connect(scene->undoStack(), &QUndoStack::indexChanged, this,
                    &MainWindow::refreshOutline);
        }
    });

    connect(m_tabManager, &TabManager::saveRequested, m_fileManager, &FileManager::saveFile);

    // Recovery before any default tab is created so orphan-session restore can
    // skip the "create empty Untitled tab" step when it has real tabs to load.
    const QString recoveryRoot =
        QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/recovery";
    m_recovery = new RecoveryManager(recoveryRoot, this);
    m_recovery->initialize();

    if (!maybeRestoreOrphanSessions())
        m_tabManager->addNewTab();

    m_autoSave = new AutoSaveManager(m_tabManager, m_recovery, this);
    connect(m_autoSave, &AutoSaveManager::autoSaved, this, [this]() {
        updateWindowTitle();
        statusBar()->showMessage(tr("Auto-saved"), 3000);
    });

    connect(m_services->settings, &AppSettings::themeChanged, this, &MainWindow::applyTheme);

    restoreWindowState();

    setupStatusBar();
}

// ---------------------------------------------------------------------------
// Destructor – disconnect signals before child widgets are destroyed,
// otherwise Qt may invoke slots on a partially-destroyed MainWindow.
// ---------------------------------------------------------------------------
MainWindow::~MainWindow() {
    // Disconnect undo stack signals that target this MainWindow before the base
    // class destructor deletes child scenes (whose undo stacks would emit
    // indexChanged during cleanup, calling refreshOutline on a half-destroyed
    // object).
    for (const auto& tab : m_tabManager->tabs()) {
        if (tab.scene && tab.scene->undoStack())
            disconnect(tab.scene->undoStack(), nullptr, this, nullptr);
    }

    disconnect(m_tabManager, nullptr, this, nullptr);
    if (m_services && m_services->settings)
        disconnect(m_services->settings, nullptr, this, nullptr);
}

// ---------------------------------------------------------------------------
// Central layout: tab bar + toolbar + splitter(outline, content)
// ---------------------------------------------------------------------------
void MainWindow::setupCentralLayout() {
    auto* centralW = new QWidget(this);
    auto* mainLayout = new QVBoxLayout(centralW);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    mainLayout->addWidget(m_updateNotifier->banner());

    // ---- Tab bar row ----
    auto* tabBarRowWidget = new QWidget(this);
    tabBarRowWidget->setObjectName("tabBarRow");
    auto* tabBarRow = new QHBoxLayout(tabBarRowWidget);
    tabBarRow->setContentsMargins(0, 0, 0, 0);
    tabBarRow->setSpacing(0);

    tabBarRow->addWidget(m_tabManager->tabBar());
    tabBarRow->addWidget(m_tabManager->newTabButton());
    tabBarRow->addStretch();

    // Toggle buttons at right end of tab bar row
    m_toggleOutlineBtn = new QToolButton(this);
    m_toggleOutlineBtn->setIcon(IconFactory::makeToolIcon("sidebar"));
    m_toggleOutlineBtn->setProperty("iconName", "sidebar");
    m_toggleOutlineBtn->setToolTip(tr("Toggle Outline Panel"));
    m_toggleOutlineBtn->setCheckable(true);
    m_toggleOutlineBtn->setChecked(true);
    m_toggleOutlineBtn->setAutoRaise(true);
    m_toggleOutlineBtn->setFixedSize(28, 28);
    m_toggleOutlineBtn->setIconSize(QSize(18, 18));
    m_toggleOutlineBtn->setObjectName("togglePanelBtn");
    tabBarRow->addWidget(m_toggleOutlineBtn);

    m_toggleToolbarBtn = new QToolButton(this);
    m_toggleToolbarBtn->setIcon(IconFactory::makeToolIcon("toolbar"));
    m_toggleToolbarBtn->setProperty("iconName", "toolbar");
    m_toggleToolbarBtn->setToolTip(tr("Toggle Toolbar"));
    m_toggleToolbarBtn->setCheckable(true);
    m_toggleToolbarBtn->setChecked(true);
    m_toggleToolbarBtn->setAutoRaise(true);
    m_toggleToolbarBtn->setFixedSize(28, 28);
    m_toggleToolbarBtn->setIconSize(QSize(18, 18));
    m_toggleToolbarBtn->setObjectName("togglePanelBtn");
    tabBarRow->addWidget(m_toggleToolbarBtn);

    mainLayout->addWidget(tabBarRowWidget);

    // ---- Inline toolbar ----
    m_toolbar = new MindMapToolBar(m_tabManager, m_fileManager, m_undoAct, m_redoAct, this);
    connect(m_toolbar, &MindMapToolBar::closeRequested, this, [this]() {
        if (m_toggleToolbarAct)
            m_toggleToolbarAct->setChecked(false);
    });

    // ---- Content area: splitter with outline + right panel (toolbar + tab pages) ----
    m_contentSplitter = new QSplitter(Qt::Horizontal, this);

    m_outlineWidget = new OutlineWidget(this);
    connect(m_outlineWidget, &OutlineWidget::closeRequested, this, [this]() {
        if (m_toggleOutlineAct)
            m_toggleOutlineAct->setChecked(false);
    });
    m_contentSplitter->addWidget(m_outlineWidget);

    m_rightPanel = new QWidget(this);
    auto* rightLayout = new QVBoxLayout(m_rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);
    rightLayout->addWidget(m_toolbar);

    // Find bar sits just above the canvas, hidden by default; toggled by
    // Ctrl+F. Wired to the active scene's findMatches / highlight calls.
    m_findBar = new FindBar(m_rightPanel);
    m_findBar->hide();
    rightLayout->addWidget(m_findBar);
    connect(m_findBar, &FindBar::queryChanged, this, &MainWindow::onFindQueryChanged);
    connect(m_findBar, &FindBar::stepNext, this, [this]() { stepFindMatch(+1); });
    connect(m_findBar, &FindBar::stepPrev, this, [this]() { stepFindMatch(-1); });
    connect(m_findBar, &FindBar::closed, this, &MainWindow::closeFindBar);

    rightLayout->addWidget(m_tabManager->contentStack(), 1);

    m_contentSplitter->addWidget(m_rightPanel);

    m_contentSplitter->setStretchFactor(0, 0);
    m_contentSplitter->setStretchFactor(1, 1);
    m_contentSplitter->setSizes({200, 1080});
    m_contentSplitter->setCollapsible(0, true);
    m_contentSplitter->setCollapsible(1, false);

    mainLayout->addWidget(m_contentSplitter, 1);

    setCentralWidget(centralW);
}

// ---------------------------------------------------------------------------
// Actions (undo/redo)
// ---------------------------------------------------------------------------
void MainWindow::setupActions() {
    m_undoAct = new QAction(tr("&Undo"), this);
    m_undoAct->setShortcut(QKeySequence::Undo);
    m_undoAct->setEnabled(false);
    connect(m_undoAct, &QAction::triggered, this, [this]() {
        withCurrentScene([](MindMapScene* s) {
            if (!s->isEditing())
                s->undoStack()->undo();
        });
    });

    m_redoAct = new QAction(tr("&Redo"), this);
    m_redoAct->setShortcuts({QKeySequence::Redo, QKeySequence("Ctrl+Y")});
    m_redoAct->setEnabled(false);
    connect(m_redoAct, &QAction::triggered, this, [this]() {
        withCurrentScene([](MindMapScene* s) {
            if (!s->isEditing())
                s->undoStack()->redo();
        });
    });
}

// ---------------------------------------------------------------------------
// Menu bar
// ---------------------------------------------------------------------------
void MainWindow::setupMenuBar() {
    // ---- File menu ----
    auto* fileMenu = menuBar()->addMenu(tr("&File"));

    auto* newAct = fileMenu->addAction(tr("&New"));
    newAct->setShortcut(QKeySequence::New);
    connect(newAct, &QAction::triggered, m_fileManager, &FileManager::newFile);

    auto* newTabAct = fileMenu->addAction(tr("New &Tab"));
    newTabAct->setShortcut(QKeySequence("Ctrl+T"));
    connect(newTabAct, &QAction::triggered, m_tabManager, &TabManager::addNewTab);

    auto* openAct = fileMenu->addAction(tr("&Open..."));
    openAct->setShortcut(QKeySequence::Open);
    connect(openAct, &QAction::triggered, m_fileManager, &FileManager::openFile);

    fileMenu->addSeparator();

    auto* saveAct = fileMenu->addAction(tr("&Save"));
    saveAct->setShortcut(QKeySequence::Save);
    connect(saveAct, &QAction::triggered, m_fileManager, &FileManager::saveFile);

    auto* saveAsAct = fileMenu->addAction(tr("Save &As..."));
    saveAsAct->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAct, &QAction::triggered, m_fileManager, &FileManager::saveFileAs);

    fileMenu->addSeparator();

    auto* closeTabAct = fileMenu->addAction(tr("&Close Tab"));
    closeTabAct->setShortcut(QKeySequence("Ctrl+W"));
    connect(closeTabAct, &QAction::triggered, this,
            [this]() { m_tabManager->closeTab(m_tabManager->currentIndex()); });

    fileMenu->addSeparator();

    auto* importAct = fileMenu->addAction(tr("&Import from Markdown..."));
    connect(importAct, &QAction::triggered, m_fileManager, &FileManager::importFromMarkdown);

    auto* exportMenu = fileMenu->addMenu(tr("&Export"));
    exportMenu->addAction(tr("As &Text..."), m_fileManager, &FileManager::exportAsText);
    exportMenu->addAction(tr("As &Markdown..."), m_fileManager, &FileManager::exportAsMarkdown);
    exportMenu->addSeparator();
    exportMenu->addAction(tr("As &PNG..."), m_fileManager, &FileManager::exportAsPng);
    exportMenu->addAction(tr("As &SVG..."), m_fileManager, &FileManager::exportAsSvg);
    exportMenu->addAction(tr("As P&DF..."), m_fileManager, &FileManager::exportAsPdf);

    fileMenu->addSeparator();

    auto* exitAct = fileMenu->addAction(tr("E&xit"));
    exitAct->setShortcut(QKeySequence::Quit);
    connect(exitAct, &QAction::triggered, this, &QWidget::close);

    // ---- Edit menu ----
    auto* editMenu = menuBar()->addMenu(tr("&Edit"));

    editMenu->addAction(m_undoAct);
    editMenu->addAction(m_redoAct);
    editMenu->addSeparator();

    m_addChildAct = editMenu->addAction(tr("Add &Child"));
    m_addChildAct->setToolTip(tr("Add a child node (Enter)"));
    connect(m_addChildAct, &QAction::triggered, this,
            [this]() { withCurrentScene([](MindMapScene* s) { s->addChildToSelected(); }); });

    m_addSiblingAct = editMenu->addAction(tr("Add &Sibling"));
    m_addSiblingAct->setToolTip(tr("Add a sibling node (Ctrl+Enter)"));
    connect(m_addSiblingAct, &QAction::triggered, this,
            [this]() { withCurrentScene([](MindMapScene* s) { s->addSiblingToSelected(); }); });

    auto* deleteAct = editMenu->addAction(tr("&Delete"));
    deleteAct->setToolTip(tr("Delete selected node (Del)"));
    connect(deleteAct, &QAction::triggered, this,
            [this]() { withCurrentScene([](MindMapScene* s) { s->deleteSelected(); }); });

    editMenu->addSeparator();

    auto* autoLayoutAct = editMenu->addAction(tr("&Auto Layout"));
    autoLayoutAct->setShortcut(QKeySequence("Ctrl+L"));
    connect(autoLayoutAct, &QAction::triggered, this,
            [this]() { withCurrentScene([](MindMapScene* s) { s->autoLayout(); }); });

    editMenu->addSeparator();

    auto* findAct = editMenu->addAction(tr("&Find..."));
    findAct->setShortcut(QKeySequence::Find);
    connect(findAct, &QAction::triggered, this, &MainWindow::openFindBar);

    editMenu->addSeparator();

    auto* settingsAct = editMenu->addAction(tr("&Preferences..."));
    settingsAct->setShortcut(QKeySequence("Ctrl+,"));
    settingsAct->setMenuRole(QAction::PreferencesRole);
    connect(settingsAct, &QAction::triggered, this, &MainWindow::openSettings);

    // ---- View menu ----
    auto* viewMenu = menuBar()->addMenu(tr("&View"));

    auto* zoomInAct = viewMenu->addAction(tr("Zoom &In"));
    zoomInAct->setShortcut(QKeySequence::ZoomIn);
    connect(zoomInAct, &QAction::triggered, this,
            [this]() { withCurrentView([](MindMapView* v) { v->zoomIn(); }); });

    auto* zoomOutAct = viewMenu->addAction(tr("Zoom &Out"));
    zoomOutAct->setShortcut(QKeySequence::ZoomOut);
    connect(zoomOutAct, &QAction::triggered, this,
            [this]() { withCurrentView([](MindMapView* v) { v->zoomOut(); }); });

    auto* fitAct = viewMenu->addAction(tr("&Fit to View"));
    fitAct->setShortcut(QKeySequence("Ctrl+0"));
    connect(fitAct, &QAction::triggered, this,
            [this]() { withCurrentView([](MindMapView* v) { v->zoomToFit(); }); });

    viewMenu->addSeparator();

    m_toggleToolbarAct = viewMenu->addAction(tr("&Toolbar"));
    m_toggleToolbarAct->setCheckable(true);
    m_toggleToolbarAct->setChecked(true);
    connect(m_toggleToolbarAct, &QAction::toggled, this, [this](bool checked) {
        {
            QSignalBlocker blocker(m_toggleToolbarBtn);
            m_toggleToolbarBtn->setChecked(checked);
        }
        updateContentVisibility();
    });

    m_toggleOutlineAct = viewMenu->addAction(tr("&Outline"));
    m_toggleOutlineAct->setCheckable(true);
    m_toggleOutlineAct->setChecked(true);
    connect(m_toggleOutlineAct, &QAction::toggled, this, [this](bool checked) {
        {
            QSignalBlocker blocker(m_toggleOutlineBtn);
            m_toggleOutlineBtn->setChecked(checked);
        }
        updateContentVisibility();
    });

    // Bidirectional sync: tab bar toggle buttons -> View menu actions
    connect(m_toggleOutlineBtn, &QToolButton::toggled, this, [this](bool checked) {
        QSignalBlocker blocker(m_toggleOutlineAct);
        m_toggleOutlineAct->setChecked(checked);
        updateContentVisibility();
    });
    connect(m_toggleToolbarBtn, &QToolButton::toggled, this, [this](bool checked) {
        QSignalBlocker blocker(m_toggleToolbarAct);
        m_toggleToolbarAct->setChecked(checked);
        updateContentVisibility();
    });

    // ---- Template menu ----
    auto* templateMenu = menuBar()->addMenu(tr("Te&mplate"));
    m_templateMenuController = new TemplateMenuController(templateMenu, m_tabManager, this);

    // ---- Theme menu ----
    auto* themeMenu = menuBar()->addMenu(tr("&Theme"));
    m_themeMenuController = new ThemeMenuController(themeMenu, m_tabManager, this);

    // ---- Help menu ----
    auto* helpMenu = menuBar()->addMenu(tr("&Help"));

    auto* checkUpdatesAct = helpMenu->addAction(tr("Check for &Updates..."));
    connect(checkUpdatesAct, &QAction::triggered, m_updateNotifier, &UpdateNotifier::checkNow);

    helpMenu->addSeparator();

    auto* aboutAct = helpMenu->addAction(tr("About &YMind..."));
    connect(aboutAct, &QAction::triggered, this, &MainWindow::openAbout);

    auto* aboutQtAct = helpMenu->addAction(tr("About &Qt..."));
    connect(aboutQtAct, &QAction::triggered, qApp, &QApplication::aboutQt);
}

// ---------------------------------------------------------------------------
// Find bar
// ---------------------------------------------------------------------------
void MainWindow::openFindBar() {
    if (!m_findBar)
        return;
    m_findBar->activate();
    // Re-apply highlights from the existing query (or empty state) so the bar
    // opens consistently after a tab switch.
    onFindQueryChanged(m_findBar->query());
}

void MainWindow::closeFindBar() {
    if (!m_findBar)
        return;
    m_findBar->hide();
    withCurrentScene([](MindMapScene* s) { s->clearSearchHighlights(); });
    m_findMatches.clear();
    m_findCurrentIdx = -1;
}

void MainWindow::onFindQueryChanged(const QString& query) {
    // Drop prior highlights regardless of query (so deletion of the last char
    // clears the previous match set).
    withCurrentScene([](MindMapScene* s) { s->clearSearchHighlights(); });
    m_findMatches.clear();
    m_findCurrentIdx = -1;

    auto* scene = m_tabManager ? m_tabManager->currentScene() : nullptr;
    if (!scene || query.isEmpty()) {
        if (m_findBar)
            m_findBar->setMatchStatus(0, 0);
        return;
    }

    m_findMatches = scene->findMatches(query);
    for (auto* n : m_findMatches)
        n->setSearchMatch(true);

    if (!m_findMatches.isEmpty()) {
        m_findCurrentIdx = 0;
        m_findMatches[0]->setSearchCurrent(true);
        withCurrentView([n = m_findMatches[0]](MindMapView* v) { v->ensureNodeVisible(n); });
    }
    if (m_findBar)
        m_findBar->setMatchStatus(m_findMatches.isEmpty() ? 0 : 1, m_findMatches.size());
}

void MainWindow::stepFindMatch(int delta) {
    if (m_findMatches.isEmpty())
        return;
    // Clear "current" ring on the old match (the match-tint stays).
    if (m_findCurrentIdx >= 0 && m_findCurrentIdx < m_findMatches.size())
        m_findMatches[m_findCurrentIdx]->setSearchCurrent(false);

    const int n = m_findMatches.size();
    m_findCurrentIdx = ((m_findCurrentIdx + delta) % n + n) % n; // wrap both ways
    auto* target = m_findMatches[m_findCurrentIdx];
    target->setSearchCurrent(true);
    withCurrentView([target](MindMapView* v) { v->ensureNodeVisible(target); });
    if (m_findBar)
        m_findBar->setMatchStatus(m_findCurrentIdx + 1, n);
}

// ---------------------------------------------------------------------------
// Orphan-session restore (untitled tabs recovered from a prior crash)
// ---------------------------------------------------------------------------
bool MainWindow::maybeRestoreOrphanSessions() {
    if (!m_recovery)
        return false;

    const auto orphans = m_recovery->findOrphanSessions();
    if (orphans.isEmpty())
        return false;

    // Collect snapshots up front so we know whether the prompt is worth
    // showing — an empty orphan dir is just leftover cruft.
    QList<QJsonObject> allSnapshots;
    for (const auto& o : orphans) {
        const auto snaps = m_recovery->loadSnapshots(o.dirPath);
        allSnapshots.append(snaps);
    }

    if (allSnapshots.isEmpty()) {
        for (const auto& o : orphans)
            m_recovery->discardSession(o.dirPath);
        return false;
    }

    const auto reply =
        QMessageBox::question(this, tr("Restore unsaved tabs"),
                              tr("YMind found %1 unsaved tab(s) from a previous session.\n"
                                 "Restore them now?")
                                  .arg(allSnapshots.size()),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

    if (reply != QMessageBox::Yes) {
        for (const auto& o : orphans)
            m_recovery->discardSession(o.dirPath);
        return false;
    }

    // Restore each snapshot as a fresh untitled tab. The scene starts
    // `modified=true` so the user is reminded they still need to Save As.
    bool restoredAny = false;
    for (const auto& json : allSnapshots) {
        auto* scene = new MindMapScene(this);
        if (!scene->fromJson(json)) {
            delete scene;
            continue;
        }
        scene->setModified(true);

        auto* view = new MindMapView(this);
        view->setScene(scene);

        auto* stack = new QStackedWidget(this);
        stack->addWidget(view);
        stack->setCurrentIndex(0);

        m_tabManager->addTab(scene, view, stack, QString());
        restoredAny = true;
    }

    // Discard the orphan dirs now that we've absorbed their content; the
    // current session will start writing fresh snapshots on its own ticks.
    for (const auto& o : orphans)
        m_recovery->discardSession(o.dirPath);

    return restoredAny;
}

// ---------------------------------------------------------------------------
// Close event
// ---------------------------------------------------------------------------
void MainWindow::closeEvent(QCloseEvent* event) {
    if (m_tabManager->maybeSave()) {
        saveWindowState();
        // Graceful exit: blow away our own recovery dir so the next launch
        // doesn't see it as an orphan and offer to restore stale tabs.
        if (m_recovery)
            m_recovery->clearCurrentSession();
        event->accept();
    } else {
        event->ignore();
    }
}

// ---------------------------------------------------------------------------
// Window title
// ---------------------------------------------------------------------------
void MainWindow::updateWindowTitle() {
    QString title = tr("YMind - Mind Map Editor");
    QString filePath = m_tabManager->currentFilePath();
    if (!filePath.isEmpty()) {
        title = QFileInfo(filePath).fileName() + " - YMind";
    }
    auto* scene = m_tabManager->currentScene();
    if (scene && scene->isModified()) {
        title.prepend("* ");
    }
    setWindowTitle(title);
}

// ---------------------------------------------------------------------------
// Content visibility (toolbar + outline hidden on start page)
// ---------------------------------------------------------------------------
void MainWindow::updateContentVisibility() {
    int idx = m_tabManager->currentIndex();
    bool onStartPage = false;
    if (idx >= 0 && idx < m_tabManager->tabCount()) {
        auto& tab = m_tabManager->tab(idx);
        if (tab.stack) {
            QWidget* current = tab.stack->currentWidget();
            onStartPage = current && current->objectName() == "startPage";
        }
    }

    bool showToolbar = !onStartPage && m_toggleToolbarAct && m_toggleToolbarAct->isChecked();
    bool showOutline = !onStartPage && m_toggleOutlineAct && m_toggleOutlineAct->isChecked();

    m_toolbar->setVisible(showToolbar);
    m_outlineWidget->setVisible(showOutline);

    if (m_toggleOutlineBtn)
        m_toggleOutlineBtn->setVisible(!onStartPage);
    if (m_toggleToolbarBtn)
        m_toggleToolbarBtn->setVisible(!onStartPage);

    if (m_addChildAct)
        m_addChildAct->setEnabled(!onStartPage);
    if (m_addSiblingAct)
        m_addSiblingAct->setEnabled(!onStartPage);

    updateStatusHint();
}

// ---------------------------------------------------------------------------
// Outline refresh
// ---------------------------------------------------------------------------
void MainWindow::refreshOutline() {
    m_outlineWidget->refresh(m_tabManager->currentScene());
    m_outlineWidget->setView(m_tabManager->currentView());
}

// ---------------------------------------------------------------------------
// Settings / state
// ---------------------------------------------------------------------------
void MainWindow::openSettings() {
    SettingsDialog dlg(this);
    dlg.exec();
}

void MainWindow::openAbout() {
    AboutDialog dlg(this);
    dlg.exec();
}

void MainWindow::saveWindowState() {
    auto* s = m_services->settings;
    s->setWindowGeometry(saveGeometry());
    s->setWindowState(QMainWindow::saveState());
}

void MainWindow::restoreWindowState() {
    auto* s = m_services->settings;
    QByteArray geo = s->windowGeometry();
    if (!geo.isEmpty())
        restoreGeometry(geo);
    QByteArray state = s->windowState();
    if (!state.isEmpty())
        QMainWindow::restoreState(state);
}

// ---------------------------------------------------------------------------
// Theme
// ---------------------------------------------------------------------------
void MainWindow::applyTheme() {
    ThemeManager::applyTheme(m_tabManager->tabs());

    if (m_toggleOutlineBtn)
        m_toggleOutlineBtn->setIcon(IconFactory::makeToolIcon("sidebar"));
    if (m_toggleToolbarBtn)
        m_toggleToolbarBtn->setIcon(IconFactory::makeToolIcon("toolbar"));

    // Refresh all tool buttons that expose an "iconName" property
    const auto btns = this->findChildren<QToolButton*>();
    for (auto* btn : btns) {
        QVariant prop = btn->property("iconName");
        if (prop.isValid() && prop.canConvert<QString>()) {
            QString iconName = prop.toString();
            if (!iconName.isEmpty())
                btn->setIcon(IconFactory::makeToolIcon(iconName));
        }
    }

    m_tabManager->updateAllTabIcons();

    if (auto* stack = m_tabManager->contentStack()) {
        const auto cards = stack->findChildren<QPushButton*>("templateCard");
        for (auto* card : cards) {
            QString tid = card->property("templateId").toString();
            if (!tid.isEmpty())
                card->setIcon(QIcon(IconFactory::makeTemplatePreview(tid, 160, 100)));
        }
    }
}

// ---------------------------------------------------------------------------
// Status bar: contextual hint on the left, update icon + version on the right
// ---------------------------------------------------------------------------
void MainWindow::setupStatusBar() {
    m_statusHelpLabel = new QLabel(this);
    m_statusHelpLabel->setAlignment(Qt::AlignCenter);
    statusBar()->addWidget(m_statusHelpLabel, 1);

    statusBar()->addPermanentWidget(m_updateNotifier->statusButton());
    statusBar()->addPermanentWidget(m_updateNotifier->versionLabel());

    connectCurrentSceneToStatusHint();
    updateStatusHint();
}

void MainWindow::connectCurrentSceneToStatusHint() {
    auto* scene = m_tabManager->currentScene();
    if (!scene)
        return;
    // Drop any prior connection so we don't accumulate one per tab switch.
    disconnect(scene, &MindMapScene::editingStarted, this, nullptr);
    disconnect(scene, &MindMapScene::editingFinished, this, nullptr);
    connect(scene, &MindMapScene::editingStarted, this, [this](NodeItem*) { updateStatusHint(); });
    connect(scene, &MindMapScene::editingFinished, this, &MainWindow::updateStatusHint);
}

void MainWindow::updateStatusHint() {
    if (!m_statusHelpLabel)
        return;

    // Determine current high-level state: start page > editing > idle.
    bool onStartPage = false;
    int idx = m_tabManager ? m_tabManager->currentIndex() : -1;
    if (idx >= 0 && idx < m_tabManager->tabCount()) {
        auto& tab = m_tabManager->tab(idx);
        if (tab.stack) {
            QWidget* current = tab.stack->currentWidget();
            onStartPage = current && current->objectName() == "startPage";
        }
    }

    if (onStartPage) {
        m_statusHelpLabel->setText(tr("Pick a template or open an existing map."));
        return;
    }

    auto* scene = m_tabManager ? m_tabManager->currentScene() : nullptr;
    if (scene && scene->isEditing()) {
        m_statusHelpLabel->setText(tr("Enter: Commit  |  Esc: Cancel"));
        return;
    }

    m_statusHelpLabel->setText(
        tr("Enter: Add Child  |  Ctrl+Enter: Add Sibling  |  Del: Delete  |  "
           "F2/Double-click: Edit  |  Ctrl+L: Auto Layout  |  Scroll: Zoom  |  "
           "Middle/Right-drag: Pan"));
}
