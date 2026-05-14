#include "core/MainWindow.h"
#include "core/AppSettings.h"
#include "core/FileManager.h"
#include "core/TemplateRegistry.h"
#include "core/ThemeRegistry.h"
#include "layout/LayoutAlgorithmRegistry.h"
#include "ui/IconFactory.h"
#include "scene/MindMapScene.h"
#include "scene/MindMapView.h"
#include "scene/NodeItem.h"
#include "ui/OutlineWidget.h"
#include "core/AboutDialog.h"
#include "core/SettingsDialog.h"
#include "core/UpdateChecker.h"
#include "ui/TabManager.h"
#include "ui/ThemeManager.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QFileInfo>
#include <QFrame>
#include <QGraphicsItem>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSplitter>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QStatusBar>
#include <QTabBar>
#include <QTimer>
#include <QToolButton>
#include <QUndoStack>
#include <QUrl>
#include <QVBoxLayout>

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    resize(1280, 800);

    // Initialize registries before anything else
    LayoutAlgorithmRegistry::instance().registerBuiltins();
    TemplateRegistry::instance().loadBuiltins();
    ThemeRegistry::instance().loadBuiltins();
    const QString userData =
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/YMind";
    TemplateRegistry::instance().loadFromDirectory(userData + "/templates");
    // User themes live in their own folder; also accept legacy themes dropped
    // alongside templates so existing downloads keep working.
    ThemeRegistry::instance().loadFromDirectory(userData + "/themes");
    ThemeRegistry::instance().loadFromDirectory(userData + "/templates");

    m_tabManager = new TabManager(this);
    m_fileManager = new FileManager(this, m_tabManager, this);

    // Apply theme before creating any tabs so that QTabBar computes
    // tab geometry with the stylesheet already in effect.  Otherwise the
    // first tab is sized using the default style and a visible gap appears
    // between the tab and the "+" button.
    applyTheme();

    setupActions();

    // Initialize TabManager (creates tab bar, "+" button, content stack)
    m_tabManager->init(m_undoAct, m_redoAct);

    setupCentralLayout();
    setupMenuBar();

    // Wire cross-module signals
    connect(m_tabManager, &TabManager::currentTabChanged, this, [this](int) {
        updateWindowTitle();
        refreshOutline();
        updateContentVisibility();

        // Reconnect undo stack -> refreshOutline for the new tab's scene
        auto* scene = m_tabManager->currentScene();
        if (scene) {
            disconnect(scene->undoStack(), &QUndoStack::indexChanged, this, nullptr);
            connect(scene->undoStack(), &QUndoStack::indexChanged, this,
                    &MainWindow::refreshOutline);
        }
    });

    connect(m_tabManager, &TabManager::saveRequested, m_fileManager, &FileManager::saveFile);

    m_tabManager->addNewTab();

    // Auto-save timer
    m_autoSaveTimer = new QTimer(this);
    connect(m_autoSaveTimer, &QTimer::timeout, this, &MainWindow::onAutoSaveTimeout);
    setupAutoSaveTimer();

    // Settings signals
    connect(&AppSettings::instance(), &AppSettings::autoSaveSettingsChanged, this,
            &MainWindow::onAutoSaveSettingsChanged);
    connect(&AppSettings::instance(), &AppSettings::themeChanged, this, &MainWindow::applyTheme);

    restoreWindowState();

    // Update checker
    m_updateChecker = new UpdateChecker(this);
    connect(m_updateChecker, &UpdateChecker::updateAvailable, this,
            &MainWindow::onUpdateAvailable);
    connect(m_updateChecker, &UpdateChecker::upToDate, this, [this]() {
        QMessageBox::information(this, tr("Check for Updates"),
                                 tr("You are running the latest version of YMind."));
    });
    connect(m_updateChecker, &UpdateChecker::checkFailed, this, [this](const QString& msg) {
        QMessageBox::warning(this, tr("Check for Updates"),
                             tr("Could not check for updates:\n%1").arg(msg));
    });

    setupStatusBar();

    if (AppSettings::instance().checkForUpdatesEnabled())
        QTimer::singleShot(3000, m_updateChecker, [this]() { m_updateChecker->checkForUpdates(false); });
}

// ---------------------------------------------------------------------------
// Destructor – disconnect signals before child widgets are destroyed,
// otherwise Qt may invoke slots on a partially-destroyed MainWindow.
// ---------------------------------------------------------------------------
MainWindow::~MainWindow() {
    m_autoSaveTimer->stop();

    // Disconnect undo stack signals that target this MainWindow before the base
    // class destructor deletes child scenes (whose undo stacks would emit
    // indexChanged during cleanup, calling refreshOutline on a half-destroyed object).
    for (const auto& tab : m_tabManager->tabs()) {
        if (tab.scene && tab.scene->undoStack())
            disconnect(tab.scene->undoStack(), nullptr, this, nullptr);
    }

    disconnect(m_tabManager, nullptr, this, nullptr);
    disconnect(&AppSettings::instance(), nullptr, this, nullptr);
}

// ---------------------------------------------------------------------------
// Central layout: tab bar + toolbar + splitter(outline, content)
// ---------------------------------------------------------------------------
void MainWindow::setupCentralLayout() {
    auto* centralW = new QWidget(this);
    auto* mainLayout = new QVBoxLayout(centralW);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ---- Update notification banner (hidden until an update is found) ----
    m_updateBanner = new QFrame(this);
    m_updateBanner->setObjectName("updateBanner");
    m_updateBanner->setVisible(false);
    auto* bannerLayout = new QHBoxLayout(m_updateBanner);
    bannerLayout->setContentsMargins(12, 6, 6, 6);
    bannerLayout->setSpacing(8);

    auto* bannerIcon = new QLabel(m_updateBanner);
    bannerIcon->setPixmap(IconFactory::makeToolIcon("update-available").pixmap(18, 18));
    bannerLayout->addWidget(bannerIcon);

    m_updateBannerLabel = new QLabel(m_updateBanner);
    m_updateBannerLabel->setObjectName("updateBannerLabel");
    m_updateBannerLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    m_updateBannerLabel->setOpenExternalLinks(false);
    connect(m_updateBannerLabel, &QLabel::linkActivated, this, [this](const QString& link) {
        QDesktopServices::openUrl(QUrl(link));
    });
    bannerLayout->addWidget(m_updateBannerLabel, 1);

    auto* bannerClose = new QToolButton(m_updateBanner);
    bannerClose->setObjectName("updateBannerClose");
    bannerClose->setIcon(IconFactory::makeToolIcon("close-panel"));
    bannerClose->setIconSize(QSize(14, 14));
    bannerClose->setAutoRaise(true);
    bannerClose->setFixedSize(22, 22);
    bannerClose->setToolTip(tr("Dismiss"));
    connect(bannerClose, &QToolButton::clicked, m_updateBanner, &QFrame::hide);
    bannerLayout->addWidget(bannerClose);

    mainLayout->addWidget(m_updateBanner);

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
    setupToolBar();

    // ---- Content area: splitter with outline + right panel (toolbar + tab pages) ----
    m_contentSplitter = new QSplitter(Qt::Horizontal, this);

    m_outlineWidget = new OutlineWidget(this);
    connect(m_outlineWidget, &OutlineWidget::closeRequested, this, [this]() {
        if (m_toggleOutlineAct)
            m_toggleOutlineAct->setChecked(false);
    });
    m_contentSplitter->addWidget(m_outlineWidget);

    // Right panel: toolbar above content stack
    m_rightPanel = new QWidget(this);
    auto* rightLayout = new QVBoxLayout(m_rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);
    rightLayout->addWidget(m_toolbarWidget);

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
        auto* scene = m_tabManager->currentScene();
        if (scene && !scene->isEditing())
            scene->undoStack()->undo();
    });

    m_redoAct = new QAction(tr("&Redo"), this);
    m_redoAct->setShortcuts({QKeySequence::Redo, QKeySequence("Ctrl+Y")});
    m_redoAct->setEnabled(false);
    connect(m_redoAct, &QAction::triggered, this, [this]() {
        auto* scene = m_tabManager->currentScene();
        if (scene && !scene->isEditing())
            scene->undoStack()->redo();
    });
}

// ---------------------------------------------------------------------------
// Toolbar
// ---------------------------------------------------------------------------
void MainWindow::setupToolBar() {
    m_toolbarWidget = new QWidget(this);
    m_toolbarWidget->setObjectName("inlineToolbar");

    auto* layout = new QHBoxLayout(m_toolbarWidget);
    layout->setContentsMargins(4, 2, 4, 2);
    layout->setSpacing(2);

    // Add stretch at the beginning to center buttons
    layout->addStretch();

    auto addButton = [&](const QString& iconName, const QString& text,
                         const QString& tooltip) -> QToolButton* {
        auto* btn = new QToolButton(m_toolbarWidget);
        btn->setProperty("iconName", iconName);
        btn->setIcon(IconFactory::makeToolIcon(iconName));
        btn->setText(text);
        btn->setToolTip(tooltip);
        btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        btn->setAutoRaise(true);
        btn->setIconSize(QSize(24, 24));
        layout->addWidget(btn);
        return btn;
    };

    auto addSeparator = [&]() {
        auto* sep = new QFrame(m_toolbarWidget);
        sep->setFrameShape(QFrame::VLine);
        sep->setFrameShadow(QFrame::Sunken);
        sep->setFixedHeight(28);
        layout->addWidget(sep);
    };

    // Undo/Redo at the very left
    m_undoBtn = addButton("undo", tr("Undo"), tr("Undo last action (Ctrl+Z)"));
    m_undoBtn->setEnabled(false);
    connect(m_undoBtn, &QToolButton::clicked, this, [this]() {
        auto* scene = m_tabManager->currentScene();
        if (scene)
            scene->undoStack()->undo();
    });
    connect(m_undoAct, &QAction::changed, m_undoBtn, [this]() {
        m_undoBtn->setEnabled(m_undoAct->isEnabled());
    });

    m_redoBtn = addButton("redo", tr("Redo"), tr("Redo last action (Ctrl+Y)"));
    m_redoBtn->setEnabled(false);
    connect(m_redoBtn, &QToolButton::clicked, this, [this]() {
        auto* scene = m_tabManager->currentScene();
        if (scene)
            scene->undoStack()->redo();
    });
    connect(m_redoAct, &QAction::changed, m_redoBtn, [this]() {
        m_redoBtn->setEnabled(m_redoAct->isEnabled());
    });

    addSeparator();

    auto* addChildBtn = addButton("add-child", tr("Add Child"), tr("Add a child node (Enter)"));
    connect(addChildBtn, &QToolButton::clicked, this,
            [this]() { if (auto* s = m_tabManager->currentScene()) s->addChildToSelected(); });

    auto* addSiblingBtn =
        addButton("add-sibling", tr("Add Sibling"), tr("Add a sibling node (Ctrl+Enter)"));
    connect(addSiblingBtn, &QToolButton::clicked, this,
            [this]() { if (auto* s = m_tabManager->currentScene()) s->addSiblingToSelected(); });

    auto* deleteBtn = addButton("delete", tr("Delete"), tr("Delete selected node (Del)"));
    connect(deleteBtn, &QToolButton::clicked, this,
            [this]() { if (auto* s = m_tabManager->currentScene()) s->deleteSelected(); });

    addSeparator();

    auto* layoutBtn =
        addButton("auto-layout", tr("Auto Layout"), tr("Automatically arrange all nodes (Ctrl+L)"));
    connect(layoutBtn, &QToolButton::clicked, this,
            [this]() { if (auto* s = m_tabManager->currentScene()) s->autoLayout(); });

    addSeparator();

    auto* zoomInBtn = addButton("zoom-in", tr("Zoom In"), tr("Zoom in (Ctrl++)"));
    connect(zoomInBtn, &QToolButton::clicked, this,
            [this]() { if (auto* v = m_tabManager->currentView()) v->zoomIn(); });

    auto* zoomOutBtn = addButton("zoom-out", tr("Zoom Out"), tr("Zoom out (Ctrl+-)"));
    connect(zoomOutBtn, &QToolButton::clicked, this,
            [this]() { if (auto* v = m_tabManager->currentView()) v->zoomOut(); });

    auto* fitBtn = addButton("fit-view", tr("Fit View"), tr("Fit all nodes in view (Ctrl+0)"));
    connect(fitBtn, &QToolButton::clicked, this,
            [this]() { if (auto* v = m_tabManager->currentView()) v->zoomToFit(); });

    addSeparator();

    auto* exportBtn = new QToolButton(m_toolbarWidget);
    exportBtn->setProperty("iconName", "export");
    exportBtn->setIcon(IconFactory::makeToolIcon("export"));
    exportBtn->setText(tr("Export"));
    exportBtn->setToolTip(tr("Export mind map"));
    exportBtn->setPopupMode(QToolButton::InstantPopup);
    exportBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    exportBtn->setAutoRaise(true);
    exportBtn->setIconSize(QSize(24, 24));
    auto* exportBtnMenu = new QMenu(exportBtn);
    exportBtnMenu->addAction(tr("As Text..."), m_fileManager, &FileManager::exportAsText);
    exportBtnMenu->addAction(tr("As Markdown..."), m_fileManager, &FileManager::exportAsMarkdown);
    exportBtnMenu->addSeparator();
    exportBtnMenu->addAction(tr("As PNG..."), m_fileManager, &FileManager::exportAsPng);
    exportBtnMenu->addAction(tr("As SVG..."), m_fileManager, &FileManager::exportAsSvg);
    exportBtnMenu->addAction(tr("As PDF..."), m_fileManager, &FileManager::exportAsPdf);
    exportBtn->setMenu(exportBtnMenu);
    layout->addWidget(exportBtn);

    // Add stretch at the end to center buttons and push close button to the right
    layout->addStretch();

    // Close button at right end of toolbar
    auto* closeBtn = new QToolButton(m_toolbarWidget);
    closeBtn->setIcon(IconFactory::makeToolIcon("close-panel"));
    closeBtn->setProperty("iconName", "close-panel");
    closeBtn->setToolTip(tr("Hide Toolbar"));
    closeBtn->setAutoRaise(true);
    closeBtn->setFixedSize(20, 20);
    closeBtn->setIconSize(QSize(14, 14));
    closeBtn->setObjectName("closePanelBtn");
    connect(closeBtn, &QToolButton::clicked, this, [this]() {
        if (m_toggleToolbarAct)
            m_toggleToolbarAct->setChecked(false);
    });
    layout->addWidget(closeBtn);
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

    auto* exportTextAct = exportMenu->addAction(tr("As &Text..."));
    connect(exportTextAct, &QAction::triggered, m_fileManager, &FileManager::exportAsText);

    auto* exportMdAct = exportMenu->addAction(tr("As &Markdown..."));
    connect(exportMdAct, &QAction::triggered, m_fileManager, &FileManager::exportAsMarkdown);

    exportMenu->addSeparator();

    auto* exportPngAct = exportMenu->addAction(tr("As &PNG..."));
    connect(exportPngAct, &QAction::triggered, m_fileManager, &FileManager::exportAsPng);

    auto* exportSvgAct = exportMenu->addAction(tr("As &SVG..."));
    connect(exportSvgAct, &QAction::triggered, m_fileManager, &FileManager::exportAsSvg);

    auto* exportPdfAct = exportMenu->addAction(tr("As P&DF..."));
    connect(exportPdfAct, &QAction::triggered, m_fileManager, &FileManager::exportAsPdf);

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
            [this]() { if (auto* s = m_tabManager->currentScene()) s->addChildToSelected(); });

    m_addSiblingAct = editMenu->addAction(tr("Add &Sibling"));
    m_addSiblingAct->setToolTip(tr("Add a sibling node (Ctrl+Enter)"));
    connect(m_addSiblingAct, &QAction::triggered, this,
            [this]() { if (auto* s = m_tabManager->currentScene()) s->addSiblingToSelected(); });

    auto* deleteAct = editMenu->addAction(tr("&Delete"));
    deleteAct->setToolTip(tr("Delete selected node (Del)"));
    connect(deleteAct, &QAction::triggered, this,
            [this]() { if (auto* s = m_tabManager->currentScene()) s->deleteSelected(); });

    editMenu->addSeparator();

    auto* autoLayoutAct = editMenu->addAction(tr("&Auto Layout"));
    autoLayoutAct->setShortcut(QKeySequence("Ctrl+L"));
    connect(autoLayoutAct, &QAction::triggered, this,
            [this]() { if (auto* s = m_tabManager->currentScene()) s->autoLayout(); });

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
            [this]() { if (auto* v = m_tabManager->currentView()) v->zoomIn(); });

    auto* zoomOutAct = viewMenu->addAction(tr("Zoom &Out"));
    zoomOutAct->setShortcut(QKeySequence::ZoomOut);
    connect(zoomOutAct, &QAction::triggered, this,
            [this]() { if (auto* v = m_tabManager->currentView()) v->zoomOut(); });

    auto* fitAct = viewMenu->addAction(tr("&Fit to View"));
    fitAct->setShortcut(QKeySequence("Ctrl+0"));
    connect(fitAct, &QAction::triggered, this,
            [this]() { if (auto* v = m_tabManager->currentView()) v->zoomToFit(); });

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
    // Templates are listed inline as checkable, mutually-exclusive actions.
    // Clicking one applies it immediately — no intermediate dialog.
    m_templateMenu = menuBar()->addMenu(tr("Te&mplate"));
    connect(m_templateMenu, &QMenu::aboutToShow, this, &MainWindow::rebuildTemplateMenu);
    rebuildTemplateMenu();

    // ---- Theme menu ----
    // Themes are listed inline as checkable, mutually-exclusive actions.
    // Clicking one applies it immediately — no intermediate dialog.
    m_themeMenu = menuBar()->addMenu(tr("&Theme"));
    connect(m_themeMenu, &QMenu::aboutToShow, this, &MainWindow::rebuildThemeMenu);
    rebuildThemeMenu();

    // ---- Help menu ----
    auto* helpMenu = menuBar()->addMenu(tr("&Help"));

    auto* checkUpdatesAct = helpMenu->addAction(tr("Check for &Updates..."));
    connect(checkUpdatesAct, &QAction::triggered, this,
            [this]() { m_updateChecker->checkForUpdates(true); });

    helpMenu->addSeparator();

    auto* aboutAct = helpMenu->addAction(tr("About &YMind..."));
    connect(aboutAct, &QAction::triggered, this, &MainWindow::openAbout);

    auto* aboutQtAct = helpMenu->addAction(tr("About &Qt..."));
    connect(aboutQtAct, &QAction::triggered, qApp, &QApplication::aboutQt);
}

// ---------------------------------------------------------------------------
// Close event
// ---------------------------------------------------------------------------
void MainWindow::closeEvent(QCloseEvent* event) {
    if (m_tabManager->maybeSave()) {
        saveWindowState();
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

    m_toolbarWidget->setVisible(showToolbar);
    m_outlineWidget->setVisible(showOutline);

    if (m_toggleOutlineBtn)
        m_toggleOutlineBtn->setVisible(!onStartPage);
    if (m_toggleToolbarBtn)
        m_toggleToolbarBtn->setVisible(!onStartPage);

    if (m_addChildAct)
        m_addChildAct->setEnabled(!onStartPage);
    if (m_addSiblingAct)
        m_addSiblingAct->setEnabled(!onStartPage);
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

// ---------------------------------------------------------------------------
// Rebuild the Template menu in place. Called on aboutToShow so the list picks
// up templates registered after startup (e.g. dropped in via the Start Page)
// and the checkmark tracks the *current* tab's templateId.
// ---------------------------------------------------------------------------
void MainWindow::rebuildTemplateMenu() {
    if (!m_templateMenu)
        return;

    m_templateMenu->clear();

    auto* scene = m_tabManager ? m_tabManager->currentScene() : nullptr;
    const QString activeId = scene ? scene->templateId() : QString();

    auto* group = new QActionGroup(m_templateMenu);
    group->setExclusive(true);

    const auto templates = TemplateRegistry::instance().allTemplates();
    for (const auto* td : templates) {
        const QString id = td->id;
        QAction* act = m_templateMenu->addAction(td->name);
        act->setCheckable(true);
        act->setChecked(id == activeId);
        group->addAction(act);
        connect(act, &QAction::triggered, this, [this, id]() { applyTemplateId(id); });
    }
}

// ---------------------------------------------------------------------------
// Apply the chosen template to the currently visible scene. Reapplies the
// new template's layout algorithm + structural overrides; keeps the user's
// content (every node text + any structural changes they made) intact, and
// keeps whichever theme is currently applied.
// ---------------------------------------------------------------------------
void MainWindow::applyTemplateId(const QString& templateId) {
    auto* scene = m_tabManager->currentScene();
    if (!scene || templateId.isEmpty())
        return;
    if (scene->templateId() == templateId)
        return;

    // Drop device caches so every item repaints with the new template's
    // shape/anchor overrides, then re-run autoLayout against the new
    // algorithm + spacing.
    const auto items = scene->items();
    for (auto* item : items)
        item->setCacheMode(QGraphicsItem::NoCache);
    if (auto* view = m_tabManager->currentView())
        view->viewport()->update();

    scene->setTemplateId(templateId);
    scene->autoLayout();

    QPointer<QGraphicsScene> guard(scene);
    QTimer::singleShot(0, this, [guard]() {
        if (!guard)
            return;
        const auto its = guard->items();
        for (auto* item : its) {
            if (dynamic_cast<NodeItem*>(item))
                item->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
        }
    });
}

// ---------------------------------------------------------------------------
// Rebuild the Theme menu in place. Called on aboutToShow so the list reflects
// any themes registered after startup (e.g. dropped in via the Start Page) and
// the checkmark tracks the *current* tab's theme.
// ---------------------------------------------------------------------------
void MainWindow::rebuildThemeMenu() {
    if (!m_themeMenu)
        return;

    m_themeMenu->clear();

    auto* scene = m_tabManager ? m_tabManager->currentScene() : nullptr;
    const QString activeId = (scene && !scene->themeId().isEmpty())
                                 ? scene->themeId()
                                 : ThemeRegistry::defaultThemeId();

    auto* group = new QActionGroup(m_themeMenu);
    group->setExclusive(true);

    const auto themes = ThemeRegistry::instance().allThemes();
    for (const auto* th : themes) {
        const QString id = th->id;
        QAction* act = m_themeMenu->addAction(th->name);
        act->setCheckable(true);
        act->setChecked(id == activeId);
        group->addAction(act);
        connect(act, &QAction::triggered, this, [this, id]() { applyThemeId(id); });
    }

    m_themeMenu->addSeparator();
    auto* browseThemesAct = m_themeMenu->addAction(tr("&Browse Themes Online..."));
    connect(browseThemesAct, &QAction::triggered, this, []() {
        QDesktopServices::openUrl(QUrl("https://broccoli-97.github.io/xmind/#themes"));
    });
}

// ---------------------------------------------------------------------------
// Apply the chosen theme to the currently visible scene. Re-skins the map
// (colors, fills, borders, connectors) without touching layout or content.
// ---------------------------------------------------------------------------
void MainWindow::applyThemeId(const QString& themeId) {
    auto* scene = m_tabManager->currentScene();
    if (!scene || themeId.isEmpty())
        return;
    if (scene->themeId() == themeId)
        return;

    // Drop device caches so every item repaints with the new theme's
    // fill/border/edge style, then restore the cache after the repaint.
    const auto items = scene->items();
    for (auto* item : items)
        item->setCacheMode(QGraphicsItem::NoCache);
    if (auto* view = m_tabManager->currentView())
        view->viewport()->update();

    scene->setThemeId(themeId);

    QPointer<QGraphicsScene> guard(scene);
    QTimer::singleShot(0, this, [guard]() {
        if (!guard)
            return;
        const auto its = guard->items();
        for (auto* item : its) {
            if (dynamic_cast<NodeItem*>(item))
                item->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
        }
    });
}

void MainWindow::openAbout() {
    AboutDialog dlg(this);
    dlg.exec();
}

void MainWindow::saveWindowState() {
    auto& s = AppSettings::instance();
    s.setWindowGeometry(saveGeometry());
    s.setWindowState(QMainWindow::saveState());
}

void MainWindow::restoreWindowState() {
    auto& s = AppSettings::instance();
    QByteArray geo = s.windowGeometry();
    if (!geo.isEmpty())
        restoreGeometry(geo);
    QByteArray state = s.windowState();
    if (!state.isEmpty())
        QMainWindow::restoreState(state);
}

void MainWindow::setupAutoSaveTimer() {
    auto& s = AppSettings::instance();
    if (s.autoSaveEnabled()) {
        m_autoSaveTimer->start(s.autoSaveIntervalMinutes() * 60 * 1000);
    } else {
        m_autoSaveTimer->stop();
    }
}

void MainWindow::onAutoSaveTimeout() {
    for (int i = 0; i < m_tabManager->tabCount(); ++i) {
        const auto& tab = m_tabManager->tabs()[i];
        if (!tab.filePath.isEmpty() && tab.scene->isModified()) {
            if (tab.scene->saveToFile(tab.filePath)) {
                m_tabManager->updateTabText(i);
            }
        }
    }
    updateWindowTitle();
    statusBar()->showMessage(tr("Auto-saved"), 3000);
}

void MainWindow::onAutoSaveSettingsChanged() {
    setupAutoSaveTimer();
}

// ---------------------------------------------------------------------------
// Theme
// ---------------------------------------------------------------------------
void MainWindow::applyTheme() {
    ThemeManager::applyTheme(m_tabManager->tabs());

    // Refresh icons that are generated dynamically so they match the new theme
    // Update toggle buttons
    if (m_toggleOutlineBtn)
        m_toggleOutlineBtn->setIcon(IconFactory::makeToolIcon("sidebar"));
    if (m_toggleToolbarBtn)
        m_toggleToolbarBtn->setIcon(IconFactory::makeToolIcon("toolbar"));

    // Update all tool buttons that expose an "iconName" property
    const auto btns = this->findChildren<QToolButton*>();
    for (auto* btn : btns) {
        QVariant prop = btn->property("iconName");
        if (prop.isValid() && prop.canConvert<QString>()) {
            QString iconName = prop.toString();
            if (!iconName.isEmpty())
                btn->setIcon(IconFactory::makeToolIcon(iconName));
        }
    }

    // Refresh tab icons for the new theme
    m_tabManager->updateAllTabIcons();

    // Refresh template preview icons on any visible start pages
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
// Status bar: help text on the left, update icon + version on the right
// ---------------------------------------------------------------------------
void MainWindow::setupStatusBar() {
    m_statusHelpLabel = new QLabel(
        tr("Enter: Add Child  |  Ctrl+Enter: Add Sibling  |  Del: Delete  |  "
           "F2/Double-click: Edit  |  Ctrl+L: Auto Layout  |  Scroll: Zoom  |  "
           "Middle/Right-drag: Pan"),
        this);
    m_statusHelpLabel->setAlignment(Qt::AlignCenter);
    statusBar()->addWidget(m_statusHelpLabel, 1);

    m_updateStatusBtn = new QToolButton(this);
    m_updateStatusBtn->setObjectName("statusBarUpdateBtn");
    m_updateStatusBtn->setAutoRaise(true);
    m_updateStatusBtn->setFixedSize(24, 24);
    m_updateStatusBtn->setIconSize(QSize(18, 18));
    connect(m_updateStatusBtn, &QToolButton::clicked, this, &MainWindow::onUpdateIconClicked);
    statusBar()->addPermanentWidget(m_updateStatusBtn);

    m_versionLabel = new QLabel(QString("v%1").arg(QCoreApplication::applicationVersion()), this);
    m_versionLabel->setObjectName("statusBarVersionLabel");
    statusBar()->addPermanentWidget(m_versionLabel);

    refreshUpdateIcon();
}

// ---------------------------------------------------------------------------
// Update notifications
// ---------------------------------------------------------------------------
void MainWindow::onUpdateAvailable(const QString& latestVersion, const QString& releaseUrl) {
    m_pendingUpdateVersion = latestVersion;
    m_pendingUpdateUrl = releaseUrl;

    if (m_updateBannerLabel && m_updateBanner) {
        m_updateBannerLabel->setText(
            tr("A new version <b>v%1</b> of YMind is available. "
               "<a href=\"%2\" style=\"color: inherit; text-decoration: underline;\">Download</a>")
                .arg(latestVersion, releaseUrl));
        m_updateBanner->setVisible(true);
    }

    refreshUpdateIcon();
}

void MainWindow::onUpdateIconClicked() {
    if (!m_pendingUpdateUrl.isEmpty()) {
        QDesktopServices::openUrl(QUrl(m_pendingUpdateUrl));
        return;
    }
    if (m_updateChecker)
        m_updateChecker->checkForUpdates(true);
}

void MainWindow::refreshUpdateIcon() {
    if (!m_updateStatusBtn)
        return;

    bool available = !m_pendingUpdateVersion.isEmpty();
    m_updateStatusBtn->setIcon(
        IconFactory::makeToolIcon(available ? "update-available" : "update"));
    m_updateStatusBtn->setToolTip(
        available
            ? tr("Update available: v%1 — click to open download page").arg(m_pendingUpdateVersion)
            : tr("Check for updates"));
}
