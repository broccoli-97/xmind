#include "MainWindow.h"
#include "AppSettings.h"
#include "MindMapScene.h"
#include "MindMapView.h"
#include "NodeItem.h"
#include "SettingsDialog.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QKeySequence>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QSignalBlocker>
#include <QStatusBar>
#include <QTabBar>
#include <QTabWidget>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QTreeWidget>
#include <QUndoStack>
#include <QVBoxLayout>

// ---------------------------------------------------------------------------
// Dark stylesheet
// ---------------------------------------------------------------------------
static const char* kDarkStyleSheet = R"(
    QMainWindow, QDialog {
        background-color: #2D2D30;
        color: #D4D4D4;
    }
    QMenuBar {
        background-color: #2D2D30;
        color: #D4D4D4;
        border-bottom: 1px solid #3F3F46;
    }
    QMenuBar::item:selected {
        background-color: #3F3F46;
    }
    QMenu {
        background-color: #2D2D30;
        color: #D4D4D4;
        border: 1px solid #3F3F46;
    }
    QMenu::item:selected {
        background-color: #094771;
    }
    QMenu::separator {
        height: 1px;
        background: #3F3F46;
        margin: 4px 8px;
    }
    QToolBar {
        background-color: #2D2D30;
        border-bottom: 1px solid #3F3F46;
        spacing: 2px;
        padding: 4px;
    }
    QToolButton {
        background-color: transparent;
        color: #D4D4D4;
        border: 1px solid transparent;
        border-radius: 4px;
        padding: 4px 8px;
        font-size: 11px;
    }
    QToolButton:hover {
        background-color: #3F3F46;
        border-color: #3F3F46;
    }
    QToolButton:pressed {
        background-color: #094771;
    }
    QTabWidget::pane {
        border: none;
        background-color: #1E1E1E;
    }
    QTabBar {
        background-color: #252526;
    }
    QTabBar::tab {
        background-color: #2D2D30;
        color: #969696;
        border: none;
        border-right: 1px solid #252526;
        padding: 6px 12px;
        min-width: 100px;
        max-width: 200px;
    }
    QTabBar::tab:selected {
        background-color: #1E1E1E;
        color: #D4D4D4;
        border-bottom: 2px solid #007ACC;
    }
    QTabBar::tab:hover:!selected {
        background-color: #2D2D30;
        color: #D4D4D4;
    }
    QTabBar::close-button {
        image: none;
        subcontrol-position: right;
        border: none;
        padding: 2px;
        margin: 2px;
        background: transparent;
        width: 14px;
        height: 14px;
    }
    QTabBar::close-button:hover {
        background-color: #3F3F46;
        border-radius: 3px;
    }
    QDockWidget {
        background-color: #252526;
        color: #D4D4D4;
        titlebar-close-icon: none;
        titlebar-normal-icon: none;
    }
    QDockWidget::title {
        background-color: #252526;
        padding: 0px;
        margin: 0px;
        border: none;
    }
    QLabel#sectionHeader {
        background-color: #2D2D30;
        color: #D4D4D4;
        font-weight: bold;
        font-size: 12px;
        padding: 6px 10px;
        border-bottom: 1px solid #3F3F46;
    }
    QTreeWidget {
        background-color: #252526;
        color: #D4D4D4;
        border: none;
        outline: none;
        font-size: 12px;
    }
    QTreeWidget::item {
        padding: 3px 0px;
    }
    QTreeWidget::item:hover {
        background-color: #2A2D2E;
    }
    QTreeWidget::item:selected {
        background-color: #094771;
        color: #FFFFFF;
    }
    QTreeWidget::branch {
        background-color: #252526;
    }
    QListWidget {
        background-color: #252526;
        color: #D4D4D4;
        border: none;
        outline: none;
    }
    QListWidget::item {
        padding: 4px;
        border-radius: 4px;
    }
    QListWidget::item:hover {
        background-color: #2A2D2E;
    }
    QListWidget::item:selected {
        background-color: #094771;
    }
    QStatusBar {
        background-color: #007ACC;
        color: #FFFFFF;
        font-size: 12px;
    }
    QStatusBar::item {
        border: none;
    }
    QScrollBar:vertical {
        background-color: #2D2D30;
        width: 12px;
        margin: 0;
    }
    QScrollBar::handle:vertical {
        background-color: #424242;
        min-height: 20px;
        border-radius: 4px;
        margin: 2px;
    }
    QScrollBar::handle:vertical:hover {
        background-color: #4F4F4F;
    }
    QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
        height: 0;
    }
    QScrollBar:horizontal {
        background-color: #2D2D30;
        height: 12px;
        margin: 0;
    }
    QScrollBar::handle:horizontal {
        background-color: #424242;
        min-width: 20px;
        border-radius: 4px;
        margin: 2px;
    }
    QScrollBar::handle:horizontal:hover {
        background-color: #4F4F4F;
    }
    QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
        width: 0;
    }
    QComboBox {
        background-color: #3C3C3C;
        color: #D4D4D4;
        border: 1px solid #3F3F46;
        border-radius: 4px;
        padding: 4px 8px;
    }
    QComboBox:hover {
        border-color: #007ACC;
    }
    QComboBox::drop-down {
        border: none;
        width: 20px;
    }
    QComboBox QAbstractItemView {
        background-color: #2D2D30;
        color: #D4D4D4;
        selection-background-color: #094771;
        border: 1px solid #3F3F46;
    }
    QSpinBox {
        background-color: #3C3C3C;
        color: #D4D4D4;
        border: 1px solid #3F3F46;
        border-radius: 4px;
        padding: 4px;
    }
    QSpinBox:hover {
        border-color: #007ACC;
    }
    QCheckBox {
        color: #D4D4D4;
        spacing: 8px;
    }
    QCheckBox::indicator {
        width: 16px;
        height: 16px;
        border: 1px solid #3F3F46;
        border-radius: 3px;
        background-color: #3C3C3C;
    }
    QCheckBox::indicator:checked {
        background-color: #007ACC;
        border-color: #007ACC;
    }
    QPushButton {
        background-color: #0E639C;
        color: #FFFFFF;
        border: none;
        border-radius: 4px;
        padding: 6px 16px;
        font-size: 12px;
    }
    QPushButton:hover {
        background-color: #1177BB;
    }
    QPushButton:pressed {
        background-color: #094771;
    }
    QGroupBox {
        color: #D4D4D4;
        border: 1px solid #3F3F46;
        border-radius: 4px;
        margin-top: 8px;
        padding-top: 16px;
        font-weight: bold;
    }
    QGroupBox::title {
        subcontrol-origin: margin;
        left: 10px;
        padding: 0 4px;
    }
    QToolTip {
        background-color: #2D2D30;
        color: #D4D4D4;
        border: 1px solid #3F3F46;
        padding: 4px;
    }
)";

// ---------------------------------------------------------------------------
// Icon factory
// ---------------------------------------------------------------------------
QIcon MainWindow::makeToolIcon(const QString& name) {
    QPixmap pix(32, 32);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    QPen pen(QColor("#D4D4D4"), 2.0);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);

    if (name == "add-child") {
        // Plus sign
        p.drawLine(16, 8, 16, 24);
        p.drawLine(8, 16, 24, 16);
    } else if (name == "add-sibling") {
        // Two horizontal lines with a plus
        p.drawLine(4, 10, 14, 10);
        p.drawLine(4, 22, 14, 22);
        p.drawLine(24, 14, 24, 22);
        p.drawLine(20, 18, 28, 18);
    } else if (name == "delete") {
        // Trash can outline
        p.drawLine(10, 10, 10, 26);
        p.drawLine(22, 10, 22, 26);
        p.drawLine(10, 26, 22, 26);
        p.drawLine(8, 10, 24, 10);
        p.drawLine(13, 6, 19, 6);
        p.drawLine(13, 6, 13, 10);
        p.drawLine(19, 6, 19, 10);
        // inner lines
        p.drawLine(14, 13, 14, 23);
        p.drawLine(18, 13, 18, 23);
    } else if (name == "auto-layout") {
        // Tree/hierarchy
        p.drawRect(12, 2, 8, 6);
        p.drawRect(2, 22, 8, 6);
        p.drawRect(22, 22, 8, 6);
        p.drawLine(16, 8, 16, 14);
        p.drawLine(6, 14, 26, 14);
        p.drawLine(6, 14, 6, 22);
        p.drawLine(26, 14, 26, 22);
    } else if (name == "zoom") {
        // Magnifying glass
        p.drawEllipse(8, 6, 16, 16);
        QPen thickPen(QColor("#D4D4D4"), 3.0);
        p.setPen(thickPen);
        p.drawLine(21, 20, 27, 27);
    } else if (name == "fit-view") {
        // Expand arrows (four corners)
        // top-left
        p.drawLine(4, 10, 4, 4);
        p.drawLine(4, 4, 10, 4);
        // top-right
        p.drawLine(22, 4, 28, 4);
        p.drawLine(28, 4, 28, 10);
        // bottom-left
        p.drawLine(4, 22, 4, 28);
        p.drawLine(4, 28, 10, 28);
        // bottom-right
        p.drawLine(28, 22, 28, 28);
        p.drawLine(22, 28, 28, 28);
    } else if (name == "export") {
        // Box with upward arrow
        p.drawRect(8, 14, 16, 14);
        p.drawLine(16, 16, 16, 4);
        p.drawLine(12, 8, 16, 4);
        p.drawLine(20, 8, 16, 4);
    }

    p.end();
    return QIcon(pix);
}

// ---------------------------------------------------------------------------
// Template preview factory
// ---------------------------------------------------------------------------
QPixmap MainWindow::makeTemplatePreview(int index) {
    QPixmap pix(120, 80);
    pix.fill(QColor("#1E1E1E"));
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);

    QPen linePen(QColor("#555555"), 1.5);
    QPen nodePen(QColor("#007ACC"), 1.5);
    QBrush nodeBrush(QColor("#094771"));

    if (index == 0) {
        // Mind Map — central node with 4 radial branches
        p.setPen(nodePen);
        p.setBrush(nodeBrush);
        p.drawRoundedRect(42, 30, 36, 18, 4, 4);
        p.setPen(linePen);
        // branches
        p.drawLine(60, 30, 90, 12);
        p.drawLine(60, 48, 90, 66);
        p.drawLine(42, 30, 18, 14);
        p.drawLine(42, 48, 18, 66);
        // child nodes
        p.setPen(nodePen);
        p.drawRoundedRect(84, 6, 28, 12, 3, 3);
        p.drawRoundedRect(84, 60, 28, 12, 3, 3);
        p.drawRoundedRect(4, 8, 28, 12, 3, 3);
        p.drawRoundedRect(4, 60, 28, 12, 3, 3);
    } else if (index == 1) {
        // Org Chart — top-down
        p.setPen(nodePen);
        p.setBrush(nodeBrush);
        p.drawRoundedRect(42, 6, 36, 14, 3, 3);
        // children
        p.drawRoundedRect(8, 50, 28, 14, 3, 3);
        p.drawRoundedRect(46, 50, 28, 14, 3, 3);
        p.drawRoundedRect(84, 50, 28, 14, 3, 3);
        // lines
        p.setPen(linePen);
        p.drawLine(60, 20, 60, 32);
        p.drawLine(22, 32, 98, 32);
        p.drawLine(22, 32, 22, 50);
        p.drawLine(60, 32, 60, 50);
        p.drawLine(98, 32, 98, 50);
    } else if (index == 2) {
        // Project Plan — multi-level tree
        p.setPen(nodePen);
        p.setBrush(nodeBrush);
        // root
        p.drawRoundedRect(4, 30, 28, 14, 3, 3);
        // phase 1 & 2
        p.drawRoundedRect(44, 10, 28, 12, 3, 3);
        p.drawRoundedRect(44, 52, 28, 12, 3, 3);
        // tasks
        p.drawRoundedRect(84, 4, 28, 10, 2, 2);
        p.drawRoundedRect(84, 20, 28, 10, 2, 2);
        p.drawRoundedRect(84, 46, 28, 10, 2, 2);
        p.drawRoundedRect(84, 62, 28, 10, 2, 2);
        // lines
        p.setPen(linePen);
        p.drawLine(32, 37, 44, 16);
        p.drawLine(32, 37, 44, 58);
        p.drawLine(72, 16, 84, 9);
        p.drawLine(72, 16, 84, 25);
        p.drawLine(72, 58, 84, 51);
        p.drawLine(72, 58, 84, 67);
    }

    // Label at bottom
    p.setPen(QColor("#888888"));
    p.setFont(QFont("sans-serif", 7));
    QStringList names = {"Mind Map", "Org Chart", "Project Plan"};
    if (index >= 0 && index < names.size()) {
        p.drawText(QRect(0, 68, 120, 12), Qt::AlignCenter, names[index]);
    }

    p.end();
    return pix;
}

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    resize(1280, 800);

    setupTabWidget();
    setupActions();
    setupToolBar();
    setupMenuBar();
    setupSidebar();
    addNewTab();

    // Auto-save timer
    m_autoSaveTimer = new QTimer(this);
    connect(m_autoSaveTimer, &QTimer::timeout, this, &MainWindow::onAutoSaveTimeout);
    setupAutoSaveTimer();

    // Settings signals
    connect(&AppSettings::instance(), &AppSettings::autoSaveSettingsChanged, this,
            &MainWindow::onAutoSaveSettingsChanged);
    connect(&AppSettings::instance(), &AppSettings::themeChanged, this, &MainWindow::applyTheme);

    restoreWindowState();
    applyTheme();

    statusBar()->showMessage("Enter: Add Child  |  Ctrl+Enter: Add Sibling  |  Del: Delete  |  "
                             "F2/Double-click: Edit  |  Ctrl+L: Auto Layout  |  Scroll: Zoom  |  "
                             "Middle/Right-drag: Pan");
}

// ---------------------------------------------------------------------------
// Tab widget setup
// ---------------------------------------------------------------------------
void MainWindow::setupTabWidget() {
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);
    m_tabWidget->setDocumentMode(true);

    // "+" button as corner widget for new tab
    auto* newTabBtn = new QToolButton(this);
    newTabBtn->setText("+");
    newTabBtn->setToolTip("New Tab (Ctrl+T)");
    newTabBtn->setAutoRaise(true);
    m_tabWidget->setCornerWidget(newTabBtn, Qt::TopRightCorner);
    connect(newTabBtn, &QToolButton::clicked, this, &MainWindow::addNewTab);

    // Tab switching and closing
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &MainWindow::switchToTab);
    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);

    // Tab bar context menu
    m_tabWidget->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tabWidget->tabBar(), &QWidget::customContextMenuRequested, this,
            &MainWindow::onTabBarContextMenu);

    setCentralWidget(m_tabWidget);
}

// ---------------------------------------------------------------------------
// Tab management
// ---------------------------------------------------------------------------
void MainWindow::addNewTab() {
    auto* scene = new MindMapScene(this);
    auto* view = new MindMapView(this);
    view->setScene(scene);
    addTab(scene, view, QString());
}

void MainWindow::addTab(MindMapScene* scene, MindMapView* view, const QString& filePath) {
    TabState tab;
    tab.scene = scene;
    tab.view = view;
    tab.filePath = filePath;

    connectSceneSignals(scene);

    {
        QSignalBlocker blocker(m_tabWidget);
        m_tabs.append(tab);
        QString label = filePath.isEmpty() ? "Untitled" : QFileInfo(filePath).fileName();
        m_tabWidget->addTab(view, label);
    }

    // Now switch to the new tab (triggers switchToTab)
    m_tabWidget->setCurrentIndex(m_tabs.size() - 1);
    switchToTab(m_tabs.size() - 1);
}

void MainWindow::connectSceneSignals(MindMapScene* scene) {
    connect(scene, &MindMapScene::modifiedChanged, this, [this, scene](bool) {
        // Find which tab this scene belongs to
        for (int i = 0; i < m_tabs.size(); ++i) {
            if (m_tabs[i].scene == scene) {
                updateTabText(i);
                if (i == m_tabWidget->currentIndex())
                    updateWindowTitle();
                break;
            }
        }
    });

    connect(scene, &MindMapScene::fileLoaded, this, [this, scene](const QString& path) {
        for (int i = 0; i < m_tabs.size(); ++i) {
            if (m_tabs[i].scene == scene) {
                m_tabs[i].filePath = path;
                updateTabText(i);
                if (i == m_tabWidget->currentIndex()) {
                    m_currentFile = path;
                    updateWindowTitle();
                }
                break;
            }
        }
    });
}

void MainWindow::switchToTab(int index) {
    if (index < 0 || index >= m_tabs.size())
        return;

    disconnectUndoStack();

    m_scene = m_tabs[index].scene;
    m_view = m_tabs[index].view;
    m_currentFile = m_tabs[index].filePath;

    connectUndoStack();
    updateWindowTitle();
    refreshOutline();
}

void MainWindow::disconnectUndoStack() {
    if (!m_scene)
        return;
    auto* stack = m_scene->undoStack();
    disconnect(stack, nullptr, m_undoAct, nullptr);
    disconnect(stack, nullptr, m_redoAct, nullptr);
    disconnect(stack, &QUndoStack::undoTextChanged, this, nullptr);
    disconnect(stack, &QUndoStack::redoTextChanged, this, nullptr);
    disconnect(stack, &QUndoStack::indexChanged, this, nullptr);
}

void MainWindow::updateTabText(int index) {
    if (index < 0 || index >= m_tabs.size())
        return;
    const auto& tab = m_tabs[index];
    QString label = tab.filePath.isEmpty() ? "Untitled" : QFileInfo(tab.filePath).fileName();
    if (tab.scene->isModified())
        label.prepend("* ");
    m_tabWidget->setTabText(index, label);
}

bool MainWindow::isTabEmpty(int index) const {
    if (index < 0 || index >= m_tabs.size())
        return false;
    const auto& tab = m_tabs[index];
    return tab.filePath.isEmpty() && !tab.scene->isModified();
}

int MainWindow::findTabByFilePath(const QString& filePath) const {
    for (int i = 0; i < m_tabs.size(); ++i) {
        if (m_tabs[i].filePath == filePath)
            return i;
    }
    return -1;
}

bool MainWindow::maybeSaveTab(int index) {
    if (index < 0 || index >= m_tabs.size())
        return true;
    auto* scene = m_tabs[index].scene;
    if (!scene->isModified())
        return true;

    QString name = m_tabs[index].filePath.isEmpty() ? "Untitled"
                                                    : QFileInfo(m_tabs[index].filePath).fileName();

    auto ret = QMessageBox::warning(this, "XMind",
                                    QString("The mind map \"%1\" has been modified.\n"
                                            "Do you want to save your changes?")
                                        .arg(name),
                                    QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (ret == QMessageBox::Save) {
        // Temporarily switch cached pointers to this tab for save
        auto* prevScene = m_scene;
        auto* prevView = m_view;
        auto prevFile = m_currentFile;

        m_scene = m_tabs[index].scene;
        m_view = m_tabs[index].view;
        m_currentFile = m_tabs[index].filePath;

        saveFile();

        // Restore cached pointers
        m_scene = prevScene;
        m_view = prevView;
        m_currentFile = prevFile;

        return !m_tabs[index].scene->isModified();
    }
    if (ret == QMessageBox::Cancel)
        return false;

    return true; // Discard
}

void MainWindow::closeTab(int index) {
    if (index < 0 || index >= m_tabs.size())
        return;

    if (!maybeSaveTab(index))
        return;

    auto tab = m_tabs[index];

    m_tabs.removeAt(index);
    m_tabWidget->removeTab(index);

    delete tab.scene;
    // view is deleted by removeTab since it's a child widget of the tab widget

    // Always keep at least one tab
    if (m_tabs.isEmpty())
        addNewTab();
}

void MainWindow::onTabBarContextMenu(const QPoint& pos) {
    int index = m_tabWidget->tabBar()->tabAt(pos);

    QMenu menu(this);

    auto* newTabAct = menu.addAction("New Tab");
    connect(newTabAct, &QAction::triggered, this, &MainWindow::addNewTab);

    if (index >= 0) {
        menu.addSeparator();
        auto* closeAct = menu.addAction("Close");
        connect(closeAct, &QAction::triggered, this, [this, index]() { closeTab(index); });

        auto* closeOthersAct = menu.addAction("Close Others");
        connect(closeOthersAct, &QAction::triggered, this, [this, index]() {
            // Close tabs after the target first (in reverse), then tabs before
            for (int i = m_tabs.size() - 1; i > index; --i)
                closeTab(i);
            for (int i = index - 1; i >= 0; --i)
                closeTab(i);
        });
    }

    menu.exec(m_tabWidget->tabBar()->mapToGlobal(pos));
}

// ---------------------------------------------------------------------------
// Close event
// ---------------------------------------------------------------------------
void MainWindow::closeEvent(QCloseEvent* event) {
    if (maybeSave()) {
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
    if (!m_currentFile.isEmpty()) {
        title = QFileInfo(m_currentFile).fileName() + " - XMind";
    }
    if (m_scene && m_scene->isModified()) {
        title.prepend("* ");
    }
    setWindowTitle(title);
}

// ---------------------------------------------------------------------------
// Maybe save (all tabs)
// ---------------------------------------------------------------------------
bool MainWindow::maybeSave() {
    for (int i = 0; i < m_tabs.size(); ++i) {
        if (!maybeSaveTab(i))
            return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// File operations
// ---------------------------------------------------------------------------
void MainWindow::newFile() {
    int cur = m_tabWidget->currentIndex();
    if (cur >= 0 && isTabEmpty(cur))
        return; // Current tab is already empty+untitled
    addNewTab();
}

void MainWindow::openFile() {
    QString filePath =
        QFileDialog::getOpenFileName(this, "Open Mind Map", QString(),
                                     "XMind Files (*.xmind);;JSON Files (*.json);;All Files (*)");
    if (filePath.isEmpty())
        return;

    // Check if already open in a tab
    int existing = findTabByFilePath(filePath);
    if (existing >= 0) {
        m_tabWidget->setCurrentIndex(existing);
        return;
    }

    // Reuse current tab if it's empty, otherwise create new
    int cur = m_tabWidget->currentIndex();
    if (cur >= 0 && isTabEmpty(cur)) {
        if (!m_scene->loadFromFile(filePath)) {
            QMessageBox::warning(this, "XMind", "Could not open file:\n" + filePath);
            return;
        }
        m_currentFile = filePath;
        m_tabs[cur].filePath = filePath;
        updateTabText(cur);
        updateWindowTitle();
        m_view->zoomToFit();
        refreshOutline();
    } else {
        // Create a new tab for the file
        auto* scene = new MindMapScene(this);
        auto* view = new MindMapView(this);
        view->setScene(scene);

        if (!scene->loadFromFile(filePath)) {
            QMessageBox::warning(this, "XMind", "Could not open file:\n" + filePath);
            delete scene;
            delete view;
            return;
        }

        addTab(scene, view, filePath);
        m_view->zoomToFit();
        refreshOutline();
    }
}

void MainWindow::saveFile() {
    if (m_currentFile.isEmpty()) {
        saveFileAs();
        return;
    }

    if (!m_scene->saveToFile(m_currentFile)) {
        QMessageBox::warning(this, "XMind", "Could not save file:\n" + m_currentFile);
    }

    int cur = m_tabWidget->currentIndex();
    if (cur >= 0) {
        m_tabs[cur].filePath = m_currentFile;
        updateTabText(cur);
    }
    updateWindowTitle();
}

void MainWindow::saveFileAs() {
    QString filePath =
        QFileDialog::getSaveFileName(this, "Save Mind Map", QString(),
                                     "XMind Files (*.xmind);;JSON Files (*.json);;All Files (*)");
    if (filePath.isEmpty())
        return;

    if (!filePath.contains('.'))
        filePath += ".xmind";

    if (!m_scene->saveToFile(filePath)) {
        QMessageBox::warning(this, "XMind", "Could not save file:\n" + filePath);
        return;
    }

    m_currentFile = filePath;
    int cur = m_tabWidget->currentIndex();
    if (cur >= 0) {
        m_tabs[cur].filePath = filePath;
        updateTabText(cur);
    }
    updateWindowTitle();
}

void MainWindow::exportAsText() {
    QString filePath = QFileDialog::getSaveFileName(this, "Export as Text", QString(),
                                                    "Text Files (*.txt);;All Files (*)");
    if (filePath.isEmpty())
        return;

    if (!filePath.endsWith(".txt", Qt::CaseInsensitive))
        filePath += ".txt";

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "XMind", "Could not write file:\n" + filePath);
        return;
    }
    file.write(m_scene->exportToText().toUtf8());
    file.close();

    statusBar()->showMessage("Exported to " + filePath, 3000);
}

void MainWindow::exportAsMarkdown() {
    QString filePath = QFileDialog::getSaveFileName(this, "Export as Markdown", QString(),
                                                    "Markdown Files (*.md);;All Files (*)");
    if (filePath.isEmpty())
        return;

    if (!filePath.endsWith(".md", Qt::CaseInsensitive))
        filePath += ".md";

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "XMind", "Could not write file:\n" + filePath);
        return;
    }
    file.write(m_scene->exportToMarkdown().toUtf8());
    file.close();

    statusBar()->showMessage("Exported to " + filePath, 3000);
}

void MainWindow::exportAsPng() {
    QString filePath = QFileDialog::getSaveFileName(this, "Export as PNG", QString(),
                                                    "PNG Images (*.png);;All Files (*)");
    if (filePath.isEmpty())
        return;

    if (!filePath.endsWith(".png", Qt::CaseInsensitive))
        filePath += ".png";

    if (!m_scene->exportToPng(filePath)) {
        QMessageBox::warning(this, "XMind", "Could not export PNG:\n" + filePath);
        return;
    }
    statusBar()->showMessage("Exported to " + filePath, 3000);
}

void MainWindow::exportAsSvg() {
    QString filePath = QFileDialog::getSaveFileName(this, "Export as SVG", QString(),
                                                    "SVG Files (*.svg);;All Files (*)");
    if (filePath.isEmpty())
        return;

    if (!filePath.endsWith(".svg", Qt::CaseInsensitive))
        filePath += ".svg";

    if (!m_scene->exportToSvg(filePath)) {
        QMessageBox::warning(this, "XMind", "Could not export SVG:\n" + filePath);
        return;
    }
    statusBar()->showMessage("Exported to " + filePath, 3000);
}

void MainWindow::exportAsPdf() {
    QString filePath = QFileDialog::getSaveFileName(this, "Export as PDF", QString(),
                                                    "PDF Files (*.pdf);;All Files (*)");
    if (filePath.isEmpty())
        return;

    if (!filePath.endsWith(".pdf", Qt::CaseInsensitive))
        filePath += ".pdf";

    if (!m_scene->exportToPdf(filePath)) {
        QMessageBox::warning(this, "XMind", "Could not export PDF:\n" + filePath);
        return;
    }
    statusBar()->showMessage("Exported to " + filePath, 3000);
}

void MainWindow::importFromText() {
    QString filePath = QFileDialog::getOpenFileName(this, "Import from Text", QString(),
                                                    "Text Files (*.txt);;All Files (*)");
    if (filePath.isEmpty())
        return;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "XMind", "Could not read file:\n" + filePath);
        return;
    }
    QString text = QString::fromUtf8(file.readAll());
    file.close();

    // Reuse current tab if empty, otherwise create new
    int cur = m_tabWidget->currentIndex();
    if (cur >= 0 && isTabEmpty(cur)) {
        if (!m_scene->importFromText(text)) {
            QMessageBox::warning(this, "XMind", "Could not parse text file:\n" + filePath);
            return;
        }
        m_currentFile.clear();
        updateWindowTitle();
        m_view->zoomToFit();
        refreshOutline();
    } else {
        auto* scene = new MindMapScene(this);
        auto* view = new MindMapView(this);
        view->setScene(scene);

        if (!scene->importFromText(text)) {
            QMessageBox::warning(this, "XMind", "Could not parse text file:\n" + filePath);
            delete scene;
            delete view;
            return;
        }

        addTab(scene, view, QString());
        m_view->zoomToFit();
        refreshOutline();
    }

    statusBar()->showMessage("Imported from " + filePath, 3000);
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
    // Auto-save ALL tabs that have a file path and are modified
    for (int i = 0; i < m_tabs.size(); ++i) {
        const auto& tab = m_tabs[i];
        if (!tab.filePath.isEmpty() && tab.scene->isModified()) {
            if (tab.scene->saveToFile(tab.filePath)) {
                updateTabText(i);
            }
        }
    }
    // Update window title for current tab
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
    bool dark = (AppSettings::instance().theme() == AppTheme::Dark);
    if (dark) {
        qApp->setStyleSheet(kDarkStyleSheet);
    } else {
        qApp->setStyleSheet(QString());
    }

    // Invalidate caches on ALL open scenes
    for (const auto& tab : m_tabs) {
        const auto items = tab.scene->items();
        for (auto* item : items) {
            item->setCacheMode(QGraphicsItem::NoCache);
        }
        tab.view->viewport()->update();
    }

    // Re-enable cache after repaint
    QTimer::singleShot(0, this, [this]() {
        for (const auto& tab : m_tabs) {
            const auto items = tab.scene->items();
            for (auto* item : items) {
                if (dynamic_cast<NodeItem*>(item))
                    item->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
            }
        }
    });
}

// ---------------------------------------------------------------------------
// Actions (undo/redo)
// ---------------------------------------------------------------------------
void MainWindow::setupActions() {
    m_undoAct = new QAction("&Undo", this);
    m_undoAct->setShortcut(QKeySequence::Undo);
    m_undoAct->setEnabled(false);
    connect(m_undoAct, &QAction::triggered, this, [this]() {
        if (!m_scene->isEditing())
            m_scene->undoStack()->undo();
    });

    m_redoAct = new QAction("&Redo", this);
    m_redoAct->setShortcuts({QKeySequence::Redo, QKeySequence("Ctrl+Y")});
    m_redoAct->setEnabled(false);
    connect(m_redoAct, &QAction::triggered, this, [this]() {
        if (!m_scene->isEditing())
            m_scene->undoStack()->redo();
    });
}

void MainWindow::connectUndoStack() {
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

    // Refresh outline on any undo/redo/push
    connect(stack, &QUndoStack::indexChanged, this, &MainWindow::refreshOutline);
}

// ---------------------------------------------------------------------------
// Toolbar
// ---------------------------------------------------------------------------
void MainWindow::setupToolBar() {
    auto* toolbar = addToolBar("Main");
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(24, 24));
    toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    auto* addChildAct = toolbar->addAction(makeToolIcon("add-child"), "Add Child");
    addChildAct->setToolTip("Add a child node (Enter)");
    connect(addChildAct, &QAction::triggered, this, [this]() { m_scene->addChildToSelected(); });

    auto* addSiblingAct = toolbar->addAction(makeToolIcon("add-sibling"), "Add Sibling");
    addSiblingAct->setToolTip("Add a sibling node (Ctrl+Enter)");
    connect(addSiblingAct, &QAction::triggered, this,
            [this]() { m_scene->addSiblingToSelected(); });

    auto* deleteAct = toolbar->addAction(makeToolIcon("delete"), "Delete");
    deleteAct->setToolTip("Delete selected node (Del)");
    connect(deleteAct, &QAction::triggered, this, [this]() { m_scene->deleteSelected(); });

    toolbar->addSeparator();

    auto* layoutAct = toolbar->addAction(makeToolIcon("auto-layout"), "Auto Layout");
    layoutAct->setToolTip("Automatically arrange all nodes (Ctrl+L)");
    connect(layoutAct, &QAction::triggered, this, [this]() { m_scene->autoLayout(); });

    toolbar->addSeparator();

    auto* zoomAct = toolbar->addAction(makeToolIcon("zoom"), "Zoom");
    zoomAct->setToolTip("Zoom in (Ctrl++)");
    connect(zoomAct, &QAction::triggered, this, [this]() { m_view->zoomIn(); });

    auto* fitAct = toolbar->addAction(makeToolIcon("fit-view"), "Fit View");
    fitAct->setToolTip("Fit all nodes in view (Ctrl+0)");
    connect(fitAct, &QAction::triggered, this, [this]() { m_view->zoomToFit(); });

    toolbar->addSeparator();

    auto* exportBtn = new QToolButton(this);
    exportBtn->setIcon(makeToolIcon("export"));
    exportBtn->setText("Export");
    exportBtn->setToolTip("Export mind map");
    exportBtn->setPopupMode(QToolButton::InstantPopup);
    exportBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    auto* exportBtnMenu = new QMenu(exportBtn);
    exportBtnMenu->addAction("As Text...", this, &MainWindow::exportAsText);
    exportBtnMenu->addAction("As Markdown...", this, &MainWindow::exportAsMarkdown);
    exportBtnMenu->addSeparator();
    exportBtnMenu->addAction("As PNG...", this, &MainWindow::exportAsPng);
    exportBtnMenu->addAction("As SVG...", this, &MainWindow::exportAsSvg);
    exportBtnMenu->addAction("As PDF...", this, &MainWindow::exportAsPdf);
    exportBtn->setMenu(exportBtnMenu);
    toolbar->addWidget(exportBtn);
}

// ---------------------------------------------------------------------------
// Menu bar
// ---------------------------------------------------------------------------
void MainWindow::setupMenuBar() {
    // ---- File menu ----
    auto* fileMenu = menuBar()->addMenu("&File");

    auto* newAct = fileMenu->addAction("&New");
    newAct->setShortcut(QKeySequence::New);
    connect(newAct, &QAction::triggered, this, &MainWindow::newFile);

    auto* newTabAct = fileMenu->addAction("New &Tab");
    newTabAct->setShortcut(QKeySequence("Ctrl+T"));
    connect(newTabAct, &QAction::triggered, this, &MainWindow::addNewTab);

    auto* openAct = fileMenu->addAction("&Open...");
    openAct->setShortcut(QKeySequence::Open);
    connect(openAct, &QAction::triggered, this, &MainWindow::openFile);

    fileMenu->addSeparator();

    auto* saveAct = fileMenu->addAction("&Save");
    saveAct->setShortcut(QKeySequence::Save);
    connect(saveAct, &QAction::triggered, this, &MainWindow::saveFile);

    auto* saveAsAct = fileMenu->addAction("Save &As...");
    saveAsAct->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAct, &QAction::triggered, this, &MainWindow::saveFileAs);

    fileMenu->addSeparator();

    auto* closeTabAct = fileMenu->addAction("&Close Tab");
    closeTabAct->setShortcut(QKeySequence("Ctrl+W"));
    connect(closeTabAct, &QAction::triggered, this,
            [this]() { closeTab(m_tabWidget->currentIndex()); });

    fileMenu->addSeparator();

    auto* exportMenu = fileMenu->addMenu("&Export");

    auto* exportTextAct = exportMenu->addAction("As &Text...");
    connect(exportTextAct, &QAction::triggered, this, &MainWindow::exportAsText);

    auto* exportMdAct = exportMenu->addAction("As &Markdown...");
    connect(exportMdAct, &QAction::triggered, this, &MainWindow::exportAsMarkdown);

    exportMenu->addSeparator();

    auto* exportPngAct = exportMenu->addAction("As &PNG...");
    connect(exportPngAct, &QAction::triggered, this, &MainWindow::exportAsPng);

    auto* exportSvgAct = exportMenu->addAction("As &SVG...");
    connect(exportSvgAct, &QAction::triggered, this, &MainWindow::exportAsSvg);

    auto* exportPdfAct = exportMenu->addAction("As P&DF...");
    connect(exportPdfAct, &QAction::triggered, this, &MainWindow::exportAsPdf);

    auto* importAct = fileMenu->addAction("&Import from Text...");
    connect(importAct, &QAction::triggered, this, &MainWindow::importFromText);

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
    connect(deleteAct, &QAction::triggered, this, [this]() { m_scene->deleteSelected(); });

    // ---- View menu ----
    auto* viewMenu = menuBar()->addMenu("&View");

    auto* zoomInAct = viewMenu->addAction("Zoom &In");
    zoomInAct->setShortcut(QKeySequence::ZoomIn);
    connect(zoomInAct, &QAction::triggered, this, [this]() { m_view->zoomIn(); });

    auto* zoomOutAct = viewMenu->addAction("Zoom &Out");
    zoomOutAct->setShortcut(QKeySequence::ZoomOut);
    connect(zoomOutAct, &QAction::triggered, this, [this]() { m_view->zoomOut(); });

    auto* fitAct = viewMenu->addAction("&Fit to View");
    fitAct->setShortcut(QKeySequence("Ctrl+0"));
    connect(fitAct, &QAction::triggered, this, [this]() { m_view->zoomToFit(); });

    viewMenu->addSeparator();
    // Sidebar toggle will be added after setupSidebar()

    // ---- Layout menu ----
    auto* layoutMenu = menuBar()->addMenu("&Layout");

    auto* autoLayoutAct = layoutMenu->addAction("&Auto Layout");
    autoLayoutAct->setShortcut(QKeySequence("Ctrl+L"));
    connect(autoLayoutAct, &QAction::triggered, this, [this]() { m_scene->autoLayout(); });

    // ---- Insert menu ----
    auto* insertMenu = menuBar()->addMenu("&Insert");

    auto* addChildAct = insertMenu->addAction("Add &Child");
    addChildAct->setToolTip("Add a child node (Enter)");
    connect(addChildAct, &QAction::triggered, this, [this]() { m_scene->addChildToSelected(); });

    auto* addSiblingAct = insertMenu->addAction("Add &Sibling");
    addSiblingAct->setToolTip("Add a sibling node (Ctrl+Enter)");
    connect(addSiblingAct, &QAction::triggered, this,
            [this]() { m_scene->addSiblingToSelected(); });

    // ---- Style menu ----
    auto* styleMenu = menuBar()->addMenu("&Style");

    auto* settingsAct = styleMenu->addAction("&Settings...");
    settingsAct->setShortcut(QKeySequence("Ctrl+,"));
    connect(settingsAct, &QAction::triggered, this, &MainWindow::openSettings);
}

// ---------------------------------------------------------------------------
// Sidebar
// ---------------------------------------------------------------------------
void MainWindow::setupSidebar() {
    m_sidebarDock = new QDockWidget(this);
    m_sidebarDock->setWindowTitle("Sidebar");
    m_sidebarDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    m_sidebarDock->setTitleBarWidget(new QWidget()); // hide title bar
    m_sidebarDock->setMinimumWidth(180);
    m_sidebarDock->setMaximumWidth(260);

    auto* container = new QWidget();
    auto* layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Outline section header
    auto* outlineLabel = new QLabel("Outline");
    outlineLabel->setObjectName("sectionHeader");
    layout->addWidget(outlineLabel);

    // Outline tree
    m_outlineTree = new QTreeWidget();
    m_outlineTree->setHeaderHidden(true);
    m_outlineTree->setAnimated(true);
    m_outlineTree->setIndentation(16);
    m_outlineTree->setExpandsOnDoubleClick(false);
    connect(m_outlineTree, &QTreeWidget::itemClicked, this, &MainWindow::onOutlineItemClicked);
    layout->addWidget(m_outlineTree, 1);

    // Templates section header
    auto* templatesLabel = new QLabel("Templates");
    templatesLabel->setObjectName("sectionHeader");
    layout->addWidget(templatesLabel);

    // Templates list
    m_templateList = new QListWidget();
    m_templateList->setViewMode(QListView::IconMode);
    m_templateList->setIconSize(QSize(120, 80));
    m_templateList->setGridSize(QSize(130, 100));
    m_templateList->setResizeMode(QListView::Adjust);
    m_templateList->setMovement(QListView::Static);
    m_templateList->setWrapping(true);

    // Add template items
    QStringList templateNames = {"Mind Map", "Org Chart", "Project Plan"};
    for (int i = 0; i < templateNames.size(); ++i) {
        auto* item = new QListWidgetItem(QIcon(makeTemplatePreview(i)), templateNames[i]);
        m_templateList->addItem(item);
    }

    connect(m_templateList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        int idx = m_templateList->row(item);
        loadTemplate(idx);
    });

    layout->addWidget(m_templateList, 1);

    m_sidebarDock->setWidget(container);
    addDockWidget(Qt::LeftDockWidgetArea, m_sidebarDock);

    // Add sidebar toggle to View menu
    auto* viewMenu = menuBar()->findChild<QMenu*>(QString(), Qt::FindDirectChildrenOnly);
    // Find the View menu by iterating
    for (auto* action : menuBar()->actions()) {
        if (action->text() == "&View") {
            if (auto* menu = action->menu()) {
                auto* toggleAct = m_sidebarDock->toggleViewAction();
                toggleAct->setText("&Sidebar");
                menu->addAction(toggleAct);
            }
            break;
        }
    }
}

// ---------------------------------------------------------------------------
// Outline sync
// ---------------------------------------------------------------------------
void MainWindow::refreshOutline() {
    if (!m_outlineTree)
        return;

    m_outlineTree->clear();

    if (!m_scene)
        return;

    auto* root = m_scene->rootNode();
    if (!root)
        return;

    auto* rootItem = new QTreeWidgetItem(m_outlineTree);
    rootItem->setText(0, root->text());
    rootItem->setData(0, Qt::UserRole, QVariant::fromValue(reinterpret_cast<quintptr>(root)));
    rootItem->setExpanded(true);

    buildOutlineSubtree(root, rootItem);
}

void MainWindow::buildOutlineSubtree(NodeItem* node, QTreeWidgetItem* parentItem) {
    for (auto* child : node->childNodes()) {
        auto* childItem = new QTreeWidgetItem(parentItem);
        childItem->setText(0, child->text());
        childItem->setData(0, Qt::UserRole, QVariant::fromValue(reinterpret_cast<quintptr>(child)));
        childItem->setExpanded(true);
        buildOutlineSubtree(child, childItem);
    }
}

void MainWindow::onOutlineItemClicked(QTreeWidgetItem* item, int /*column*/) {
    quintptr ptr = item->data(0, Qt::UserRole).value<quintptr>();
    auto* node = reinterpret_cast<NodeItem*>(ptr);
    if (!node)
        return;

    // Clear selection, select the node, center on it
    m_scene->clearSelection();
    node->setSelected(true);
    m_view->centerOn(node);
}

// ---------------------------------------------------------------------------
// Template loading
// ---------------------------------------------------------------------------
void MainWindow::loadTemplate(int index) {
    // If current tab is not empty, create a new tab
    int cur = m_tabWidget->currentIndex();
    if (cur >= 0 && !isTabEmpty(cur)) {
        addNewTab();
    }

    m_currentFile.clear();
    int tabIdx = m_tabWidget->currentIndex();
    if (tabIdx >= 0)
        m_tabs[tabIdx].filePath.clear();

    // Replace the scene within the current tab
    auto* oldScene = m_scene;
    m_scene = new MindMapScene(this);
    m_view->setScene(m_scene);

    if (tabIdx >= 0)
        m_tabs[tabIdx].scene = m_scene;

    connectSceneSignals(m_scene);

    // Disconnect old undo stack connections from the old scene (already replaced)
    if (oldScene) {
        auto* oldStack = oldScene->undoStack();
        disconnect(oldStack, nullptr, m_undoAct, nullptr);
        disconnect(oldStack, nullptr, m_redoAct, nullptr);
        disconnect(oldStack, &QUndoStack::undoTextChanged, this, nullptr);
        disconnect(oldStack, &QUndoStack::redoTextChanged, this, nullptr);
        disconnect(oldStack, &QUndoStack::indexChanged, this, nullptr);
        delete oldScene;
    }

    connectUndoStack();

    auto* root = m_scene->rootNode();

    if (index == 0) {
        // Mind Map — central topic with 4 branches
        root->setText("Central Topic");
        m_scene->addNode("Branch 1", root);
        m_scene->addNode("Branch 2", root);
        m_scene->addNode("Branch 3", root);
        m_scene->addNode("Branch 4", root);
    } else if (index == 1) {
        // Org Chart — CEO -> 3 departments
        root->setText("CEO");
        m_scene->addNode("Engineering", root);
        m_scene->addNode("Marketing", root);
        m_scene->addNode("Sales", root);
    } else if (index == 2) {
        // Project Plan — root -> phases -> tasks
        root->setText("Project");
        auto* phase1 = m_scene->addNode("Phase 1", root);
        auto* phase2 = m_scene->addNode("Phase 2", root);
        m_scene->addNode("Task 1.1", phase1);
        m_scene->addNode("Task 1.2", phase1);
        m_scene->addNode("Task 2.1", phase2);
        m_scene->addNode("Task 2.2", phase2);
    }

    m_scene->autoLayout();
    m_scene->undoStack()->clear();
    m_scene->setModified(false);

    updateTabText(tabIdx);
    updateWindowTitle();
    applyTheme();
    refreshOutline();
    m_view->zoomToFit();

    statusBar()->showMessage("Template loaded", 3000);
}
