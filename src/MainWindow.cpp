#include "MainWindow.h"
#include "AppSettings.h"
#include "FileManager.h"
#include "MindMapScene.h"
#include "MindMapView.h"
#include "OutlineWidget.h"
#include "SettingsDialog.h"
#include "TabManager.h"
#include "ThemeManager.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QSignalBlocker>
#include <QSplitter>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTabBar>
#include <QTimer>
#include <QToolButton>
#include <QUndoStack>
#include <QVBoxLayout>

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    resize(1280, 800);

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
    });

    connect(m_tabManager, &TabManager::saveRequested, m_fileManager, &FileManager::saveFile);

    // Connect undo stack indexChanged -> refreshOutline when tab changes
    connect(m_tabManager, &TabManager::currentTabChanged, this, [this](int) {
        auto* scene = m_tabManager->currentScene();
        if (scene) {
            // Disconnect previous connections to avoid duplicates
            disconnect(scene->undoStack(), &QUndoStack::indexChanged, this, nullptr);
            connect(scene->undoStack(), &QUndoStack::indexChanged, this,
                    &MainWindow::refreshOutline);
        }
    });

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

    statusBar()->showMessage("Enter: Add Child  |  Ctrl+Enter: Add Sibling  |  Del: Delete  |  "
                             "F2/Double-click: Edit  |  Ctrl+L: Auto Layout  |  Scroll: Zoom  |  "
                             "Middle/Right-drag: Pan");
}

// ---------------------------------------------------------------------------
// Central layout: tab bar + toolbar + splitter(outline, content)
// ---------------------------------------------------------------------------
void MainWindow::setupCentralLayout() {
    auto* centralW = new QWidget(this);
    auto* mainLayout = new QVBoxLayout(centralW);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ---- Tab bar row ----
    auto* tabBarRow = new QHBoxLayout();
    tabBarRow->setContentsMargins(0, 0, 0, 0);
    tabBarRow->setSpacing(0);

    tabBarRow->addWidget(m_tabManager->tabBar());
    tabBarRow->addWidget(m_tabManager->newTabButton());
    tabBarRow->addStretch();

    // Toggle buttons at right end of tab bar row
    m_toggleOutlineBtn = new QToolButton(this);
    m_toggleOutlineBtn->setIcon(ThemeManager::makeToolIcon("sidebar"));
    m_toggleOutlineBtn->setProperty("iconName", "sidebar");
    m_toggleOutlineBtn->setToolTip("Toggle Outline Panel");
    m_toggleOutlineBtn->setCheckable(true);
    m_toggleOutlineBtn->setChecked(true);
    m_toggleOutlineBtn->setAutoRaise(true);
    m_toggleOutlineBtn->setFixedSize(28, 28);
    m_toggleOutlineBtn->setIconSize(QSize(18, 18));
    m_toggleOutlineBtn->setObjectName("togglePanelBtn");
    tabBarRow->addWidget(m_toggleOutlineBtn);

    m_toggleToolbarBtn = new QToolButton(this);
    m_toggleToolbarBtn->setIcon(ThemeManager::makeToolIcon("toolbar"));
    m_toggleToolbarBtn->setProperty("iconName", "toolbar");
    m_toggleToolbarBtn->setToolTip("Toggle Toolbar");
    m_toggleToolbarBtn->setCheckable(true);
    m_toggleToolbarBtn->setChecked(true);
    m_toggleToolbarBtn->setAutoRaise(true);
    m_toggleToolbarBtn->setFixedSize(28, 28);
    m_toggleToolbarBtn->setIconSize(QSize(18, 18));
    m_toggleToolbarBtn->setObjectName("togglePanelBtn");
    tabBarRow->addWidget(m_toggleToolbarBtn);

    mainLayout->addLayout(tabBarRow);

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
    m_undoAct = new QAction("&Undo", this);
    m_undoAct->setShortcut(QKeySequence::Undo);
    m_undoAct->setEnabled(false);
    connect(m_undoAct, &QAction::triggered, this, [this]() {
        auto* scene = m_tabManager->currentScene();
        if (scene && !scene->isEditing())
            scene->undoStack()->undo();
    });

    m_redoAct = new QAction("&Redo", this);
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
        btn->setIcon(ThemeManager::makeToolIcon(iconName));
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
    auto* undoBtn = addButton("undo", "Undo", "Undo last action (Ctrl+Z)");
    connect(undoBtn, &QToolButton::clicked, this, [this]() {
        auto* scene = m_tabManager->currentScene();
        if (scene)
            scene->undoStack()->undo();
    });

    auto* redoBtn = addButton("redo", "Redo", "Redo last action (Ctrl+Y)");
    connect(redoBtn, &QToolButton::clicked, this, [this]() {
        auto* scene = m_tabManager->currentScene();
        if (scene)
            scene->undoStack()->redo();
    });

    addSeparator();

    auto* addChildBtn = addButton("add-child", "Add Child", "Add a child node (Enter)");
    connect(addChildBtn, &QToolButton::clicked, this,
            [this]() { m_tabManager->currentScene()->addChildToSelected(); });

    auto* addSiblingBtn =
        addButton("add-sibling", "Add Sibling", "Add a sibling node (Ctrl+Enter)");
    connect(addSiblingBtn, &QToolButton::clicked, this,
            [this]() { m_tabManager->currentScene()->addSiblingToSelected(); });

    auto* deleteBtn = addButton("delete", "Delete", "Delete selected node (Del)");
    connect(deleteBtn, &QToolButton::clicked, this,
            [this]() { m_tabManager->currentScene()->deleteSelected(); });

    addSeparator();

    auto* layoutBtn =
        addButton("auto-layout", "Auto Layout", "Automatically arrange all nodes (Ctrl+L)");
    connect(layoutBtn, &QToolButton::clicked, this,
            [this]() { m_tabManager->currentScene()->autoLayout(); });

    addSeparator();

    auto* zoomInBtn = addButton("zoom-in", "Zoom In", "Zoom in (Ctrl++)");
    connect(zoomInBtn, &QToolButton::clicked, this,
            [this]() { m_tabManager->currentView()->zoomIn(); });

    auto* zoomOutBtn = addButton("zoom-out", "Zoom Out", "Zoom out (Ctrl+-)");
    connect(zoomOutBtn, &QToolButton::clicked, this,
            [this]() { m_tabManager->currentView()->zoomOut(); });

    auto* fitBtn = addButton("fit-view", "Fit View", "Fit all nodes in view (Ctrl+0)");
    connect(fitBtn, &QToolButton::clicked, this,
            [this]() { m_tabManager->currentView()->zoomToFit(); });

    addSeparator();

    auto* exportBtn = new QToolButton(m_toolbarWidget);
    exportBtn->setIcon(ThemeManager::makeToolIcon("export"));
    exportBtn->setText("Export");
    exportBtn->setToolTip("Export mind map");
    exportBtn->setPopupMode(QToolButton::InstantPopup);
    exportBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    exportBtn->setAutoRaise(true);
    exportBtn->setIconSize(QSize(24, 24));
    auto* exportBtnMenu = new QMenu(exportBtn);
    exportBtnMenu->addAction("As Text...", m_fileManager, &FileManager::exportAsText);
    exportBtnMenu->addAction("As Markdown...", m_fileManager, &FileManager::exportAsMarkdown);
    exportBtnMenu->addSeparator();
    exportBtnMenu->addAction("As PNG...", m_fileManager, &FileManager::exportAsPng);
    exportBtnMenu->addAction("As SVG...", m_fileManager, &FileManager::exportAsSvg);
    exportBtnMenu->addAction("As PDF...", m_fileManager, &FileManager::exportAsPdf);
    exportBtn->setMenu(exportBtnMenu);
    layout->addWidget(exportBtn);

    // Add stretch at the end to center buttons and push close button to the right
    layout->addStretch();

    // Close button at right end of toolbar
    auto* closeBtn = new QToolButton(m_toolbarWidget);
    closeBtn->setIcon(ThemeManager::makeToolIcon("close-panel"));
    closeBtn->setToolTip("Hide Toolbar");
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
    auto* fileMenu = menuBar()->addMenu("&File");

    auto* newAct = fileMenu->addAction("&New");
    newAct->setShortcut(QKeySequence::New);
    connect(newAct, &QAction::triggered, m_fileManager, &FileManager::newFile);

    auto* newTabAct = fileMenu->addAction("New &Tab");
    newTabAct->setShortcut(QKeySequence("Ctrl+T"));
    connect(newTabAct, &QAction::triggered, m_tabManager, &TabManager::addNewTab);

    auto* openAct = fileMenu->addAction("&Open...");
    openAct->setShortcut(QKeySequence::Open);
    connect(openAct, &QAction::triggered, m_fileManager, &FileManager::openFile);

    fileMenu->addSeparator();

    auto* saveAct = fileMenu->addAction("&Save");
    saveAct->setShortcut(QKeySequence::Save);
    connect(saveAct, &QAction::triggered, m_fileManager, &FileManager::saveFile);

    auto* saveAsAct = fileMenu->addAction("Save &As...");
    saveAsAct->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAct, &QAction::triggered, m_fileManager, &FileManager::saveFileAs);

    fileMenu->addSeparator();

    auto* closeTabAct = fileMenu->addAction("&Close Tab");
    closeTabAct->setShortcut(QKeySequence("Ctrl+W"));
    connect(closeTabAct, &QAction::triggered, this,
            [this]() { m_tabManager->closeTab(m_tabManager->currentIndex()); });

    fileMenu->addSeparator();

    auto* exportMenu = fileMenu->addMenu("&Export");

    auto* exportTextAct = exportMenu->addAction("As &Text...");
    connect(exportTextAct, &QAction::triggered, m_fileManager, &FileManager::exportAsText);

    auto* exportMdAct = exportMenu->addAction("As &Markdown...");
    connect(exportMdAct, &QAction::triggered, m_fileManager, &FileManager::exportAsMarkdown);

    exportMenu->addSeparator();

    auto* exportPngAct = exportMenu->addAction("As &PNG...");
    connect(exportPngAct, &QAction::triggered, m_fileManager, &FileManager::exportAsPng);

    auto* exportSvgAct = exportMenu->addAction("As &SVG...");
    connect(exportSvgAct, &QAction::triggered, m_fileManager, &FileManager::exportAsSvg);

    auto* exportPdfAct = exportMenu->addAction("As P&DF...");
    connect(exportPdfAct, &QAction::triggered, m_fileManager, &FileManager::exportAsPdf);

    auto* importAct = fileMenu->addAction("&Import from Text...");
    connect(importAct, &QAction::triggered, m_fileManager, &FileManager::importFromText);

    fileMenu->addSeparator();

    auto* exitAct = fileMenu->addAction("E&xit");
    exitAct->setShortcut(QKeySequence::Quit);
    connect(exitAct, &QAction::triggered, this, &QWidget::close);

    // ---- Edit menu ----
    auto* editMenu = menuBar()->addMenu("&Edit");

    editMenu->addAction(m_undoAct);
    editMenu->addAction(m_redoAct);
    editMenu->addSeparator();

    auto* deleteAct = editMenu->addAction("&Delete");
    deleteAct->setToolTip("Delete selected node (Del)");
    connect(deleteAct, &QAction::triggered, this,
            [this]() { m_tabManager->currentScene()->deleteSelected(); });

    // ---- View menu ----
    auto* viewMenu = menuBar()->addMenu("&View");

    auto* zoomInAct = viewMenu->addAction("Zoom &In");
    zoomInAct->setShortcut(QKeySequence::ZoomIn);
    connect(zoomInAct, &QAction::triggered, this,
            [this]() { m_tabManager->currentView()->zoomIn(); });

    auto* zoomOutAct = viewMenu->addAction("Zoom &Out");
    zoomOutAct->setShortcut(QKeySequence::ZoomOut);
    connect(zoomOutAct, &QAction::triggered, this,
            [this]() { m_tabManager->currentView()->zoomOut(); });

    auto* fitAct = viewMenu->addAction("&Fit to View");
    fitAct->setShortcut(QKeySequence("Ctrl+0"));
    connect(fitAct, &QAction::triggered, this,
            [this]() { m_tabManager->currentView()->zoomToFit(); });

    viewMenu->addSeparator();

    m_toggleToolbarAct = viewMenu->addAction("&Toolbar");
    m_toggleToolbarAct->setCheckable(true);
    m_toggleToolbarAct->setChecked(true);
    connect(m_toggleToolbarAct, &QAction::toggled, this, [this](bool checked) {
        {
            QSignalBlocker blocker(m_toggleToolbarBtn);
            m_toggleToolbarBtn->setChecked(checked);
        }
        updateContentVisibility();
    });

    m_toggleOutlineAct = viewMenu->addAction("&Outline");
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

    // ---- Layout menu ----
    auto* layoutMenu = menuBar()->addMenu("&Layout");

    auto* autoLayoutAct = layoutMenu->addAction("&Auto Layout");
    autoLayoutAct->setShortcut(QKeySequence("Ctrl+L"));
    connect(autoLayoutAct, &QAction::triggered, this,
            [this]() { m_tabManager->currentScene()->autoLayout(); });

    // ---- Insert menu ----
    auto* insertMenu = menuBar()->addMenu("&Insert");

    m_addChildAct = insertMenu->addAction("Add &Child");
    m_addChildAct->setToolTip("Add a child node (Enter)");
    connect(m_addChildAct, &QAction::triggered, this,
            [this]() { m_tabManager->currentScene()->addChildToSelected(); });

    m_addSiblingAct = insertMenu->addAction("Add &Sibling");
    m_addSiblingAct->setToolTip("Add a sibling node (Ctrl+Enter)");
    connect(m_addSiblingAct, &QAction::triggered, this,
            [this]() { m_tabManager->currentScene()->addSiblingToSelected(); });

    // ---- Style menu ----
    auto* styleMenu = menuBar()->addMenu("&Style");

    auto* settingsAct = styleMenu->addAction("&Settings...");
    settingsAct->setShortcut(QKeySequence("Ctrl+,"));
    connect(settingsAct, &QAction::triggered, this, &MainWindow::openSettings);
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
    QString title = "XMind - Mind Map Editor";
    QString filePath = m_tabManager->currentFilePath();
    if (!filePath.isEmpty()) {
        title = QFileInfo(filePath).fileName() + " - XMind";
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
    statusBar()->showMessage("Auto-saved", 3000);
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
        m_toggleOutlineBtn->setIcon(ThemeManager::makeToolIcon("sidebar"));
    if (m_toggleToolbarBtn)
        m_toggleToolbarBtn->setIcon(ThemeManager::makeToolIcon("toolbar"));

    // Update all tool buttons that expose an "iconName" property
    const auto btns = this->findChildren<QToolButton*>();
    for (auto* btn : btns) {
        QVariant prop = btn->property("iconName");
        if (prop.isValid() && prop.canConvert<QString>()) {
            QString iconName = prop.toString();
            if (!iconName.isEmpty())
                btn->setIcon(ThemeManager::makeToolIcon(iconName));
        }
    }
}
