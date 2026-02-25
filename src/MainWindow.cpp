#include "MainWindow.h"
#include "AppSettings.h"
#include "MindMapScene.h"
#include "MindMapView.h"
#include "NodeItem.h"
#include "SettingsDialog.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSplitter>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTabBar>
#include <QTimer>
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
    QToolButton#newTabBtn {
        background-color: transparent;
        color: #969696;
        border: none;
        border-radius: 4px;
        font-size: 16px;
        font-weight: bold;
    }
    QToolButton#newTabBtn:hover {
        background-color: #3F3F46;
        color: #D4D4D4;
    }
    QWidget#inlineToolbar {
        background-color: #2D2D30;
        border-bottom: 1px solid #3F3F46;
    }
    QLabel#sectionHeader, QWidget#sectionHeader {
        background-color: #2D2D30;
        color: #D4D4D4;
        font-weight: bold;
        font-size: 12px;
        padding: 6px 10px;
        border-bottom: 1px solid #3F3F46;
    }
    QToolButton#togglePanelBtn {
        background-color: transparent;
        color: #969696;
        border: none;
        border-radius: 4px;
        padding: 4px;
    }
    QToolButton#togglePanelBtn:hover {
        background-color: #3F3F46;
        color: #D4D4D4;
    }
    QToolButton#togglePanelBtn:checked {
        background-color: #094771;
        color: #D4D4D4;
    }
    QToolButton#closePanelBtn {
        background-color: transparent;
        color: #969696;
        border: none;
        border-radius: 3px;
        padding: 2px;
    }
    QToolButton#closePanelBtn:hover {
        background-color: #3F3F46;
        color: #D4D4D4;
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
    QPushButton#templateCard {
        background-color: #2D2D30;
        border: 2px solid #3F3F46;
        color: #D4D4D4;
    }
    QPushButton#templateCard:hover {
        border-color: #007ACC;
        background-color: #333337;
    }
    QPushButton#templateCard:pressed {
        background-color: #094771;
    }
    QPushButton#blankCanvasBtn {
        border-color: #3F3F46;
        color: #D4D4D4;
    }
    QPushButton#blankCanvasBtn:hover {
        border-color: #007ACC;
        background-color: #2D2D30;
    }
    QPushButton#blankCanvasBtn:pressed {
        background-color: #094771;
    }
)";

// ---------------------------------------------------------------------------
// Light stylesheet
// ---------------------------------------------------------------------------
static const char* kLightStyleSheet = R"(
    QTabBar {
        background-color: #E8E8E8;
    }
    QTabBar::tab {
        background-color: #D6D6D6;
        color: #666666;
        border: none;
        border-right: 1px solid #C8C8C8;
        padding: 6px 12px;
        min-width: 100px;
        max-width: 200px;
    }
    QTabBar::tab:selected {
        background-color: #FFFFFF;
        color: #1E1E1E;
        border-bottom: 2px solid #007ACC;
    }
    QTabBar::tab:hover:!selected {
        background-color: #E0E0E0;
        color: #333333;
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
        background-color: #C8C8C8;
        border-radius: 3px;
    }
    QToolButton#newTabBtn {
        background-color: transparent;
        color: #666666;
        border: none;
        border-radius: 4px;
        font-size: 16px;
        font-weight: bold;
    }
    QToolButton#newTabBtn:hover {
        background-color: #D0D0D0;
        color: #333333;
    }
    QWidget#inlineToolbar {
        background-color: #F3F3F3;
        border-bottom: 1px solid #D0D0D0;
    }
    QWidget#inlineToolbar QToolButton {
        background-color: transparent;
        color: #333333;
        border: 1px solid transparent;
        border-radius: 4px;
        padding: 4px 8px;
        font-size: 11px;
    }
    QWidget#inlineToolbar QToolButton:hover {
        background-color: #D0D0D0;
        border-color: #D0D0D0;
    }
    QWidget#inlineToolbar QToolButton:pressed {
        background-color: #B8D4F0;
    }
    QLabel#sectionHeader, QWidget#sectionHeader {
        background-color: #F0F0F0;
        color: #333333;
        font-weight: bold;
        font-size: 12px;
        padding: 6px 10px;
        border-bottom: 1px solid #D0D0D0;
    }
    QToolButton#togglePanelBtn {
        background-color: transparent;
        color: #666666;
        border: none;
        border-radius: 4px;
        padding: 4px;
    }
    QToolButton#togglePanelBtn:hover {
        background-color: #D0D0D0;
        color: #333333;
    }
    QToolButton#togglePanelBtn:checked {
        background-color: #CCE4F7;
        color: #1E1E1E;
    }
    QToolButton#closePanelBtn {
        background-color: transparent;
        color: #666666;
        border: none;
        border-radius: 3px;
        padding: 2px;
    }
    QToolButton#closePanelBtn:hover {
        background-color: #D0D0D0;
        color: #333333;
    }
    QTreeWidget {
        background-color: #FFFFFF;
        color: #1E1E1E;
        border: none;
        outline: none;
        font-size: 12px;
    }
    QTreeWidget::item {
        padding: 3px 0px;
    }
    QTreeWidget::item:hover {
        background-color: #E8E8E8;
    }
    QTreeWidget::item:selected {
        background-color: #CCE4F7;
        color: #1E1E1E;
    }
    QTreeWidget::branch {
        background-color: #FFFFFF;
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
    } else if (name == "sidebar") {
        // Outline/sidebar icon — panel with lines
        p.drawRect(4, 4, 24, 24);
        p.drawLine(14, 4, 14, 28);
        p.drawLine(17, 10, 25, 10);
        p.drawLine(17, 16, 25, 16);
        p.drawLine(17, 22, 25, 22);
    } else if (name == "toolbar") {
        // Toolbar icon — horizontal bar with buttons
        p.drawRect(4, 10, 24, 12);
        p.drawLine(10, 10, 10, 22);
        p.drawLine(16, 10, 16, 22);
        p.drawLine(22, 10, 22, 22);
    } else if (name == "close-panel") {
        // Small X
        p.drawLine(10, 10, 22, 22);
        p.drawLine(22, 10, 10, 22);
    }

    p.end();
    return QIcon(pix);
}

// ---------------------------------------------------------------------------
// Template preview factory
// ---------------------------------------------------------------------------
QPixmap MainWindow::makeTemplatePreview(int index, int width, int height) {
    QPixmap pix(width, height);
    pix.fill(QColor("#1E1E1E"));
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);

    // Scale drawing to requested size (reference size is 120x80)
    qreal sx = width / 120.0;
    qreal sy = height / 80.0;
    p.scale(sx, sy);

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

    // Label at bottom (drawn in reference 120x80 coordinates, scaling handles the rest)
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
// Start page factory
// ---------------------------------------------------------------------------
QWidget* MainWindow::createStartPage() {
    auto* page = new QWidget();
    page->setObjectName("startPage");

    auto* outer = new QVBoxLayout(page);
    outer->setAlignment(Qt::AlignCenter);

    // Title
    auto* title = new QLabel("Create a New Mind Map");
    title->setObjectName("startPageTitle");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 24px; font-weight: bold; margin-bottom: 4px;"
                         " background: transparent; border: none;");
    outer->addWidget(title);

    // Subtitle
    auto* subtitle = new QLabel("Choose a template to get started");
    subtitle->setObjectName("startPageSubtitle");
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setStyleSheet("font-size: 14px; color: #888888; margin-bottom: 24px;"
                            " background: transparent; border: none;");
    outer->addWidget(subtitle);

    // Template cards row
    auto* cardRow = new QWidget();
    auto* cardLayout = new QHBoxLayout(cardRow);
    cardLayout->setAlignment(Qt::AlignCenter);
    cardLayout->setSpacing(24);

    QStringList templateNames = {"Mind Map", "Org Chart", "Project Plan"};
    for (int i = 0; i < 3; ++i) {
        auto* card = new QPushButton();
        card->setObjectName("templateCard");
        card->setFixedSize(180, 140);
        card->setIconSize(QSize(160, 106));
        card->setIcon(QIcon(makeTemplatePreview(i, 160, 106)));
        card->setText(templateNames[i]);
        card->setToolTip(templateNames[i]);
        card->setStyleSheet(
            "QPushButton#templateCard {"
            "  background-color: #F0F0F0;"
            "  border: 2px solid #D0D0D0;"
            "  border-radius: 8px;"
            "  padding: 8px;"
            "  font-size: 13px;"
            "  color: #333333;"
            "  text-align: bottom;"
            "}"
            "QPushButton#templateCard:hover {"
            "  border-color: #007ACC;"
            "  background-color: #E8E8E8;"
            "}"
            "QPushButton#templateCard:pressed {"
            "  background-color: #D0E8FF;"
            "}");
        connect(card, &QPushButton::clicked, this, [this, i]() { loadTemplate(i); });
        cardLayout->addWidget(card);
    }
    outer->addWidget(cardRow);

    // Spacing
    outer->addSpacing(16);

    // Blank Canvas button
    auto* blankBtn = new QPushButton("Blank Canvas");
    blankBtn->setObjectName("blankCanvasBtn");
    blankBtn->setFixedSize(160, 36);
    blankBtn->setStyleSheet(
        "QPushButton#blankCanvasBtn {"
        "  background-color: transparent;"
        "  border: 1px solid #D0D0D0;"
        "  border-radius: 6px;"
        "  font-size: 13px;"
        "  color: #333333;"
        "}"
        "QPushButton#blankCanvasBtn:hover {"
        "  border-color: #007ACC;"
        "  background-color: #F0F0F0;"
        "}"
        "QPushButton#blankCanvasBtn:pressed {"
        "  background-color: #D0E8FF;"
        "}");
    connect(blankBtn, &QPushButton::clicked, this, &MainWindow::activateBlankCanvas);

    auto* blankRow = new QHBoxLayout();
    blankRow->setAlignment(Qt::AlignCenter);
    blankRow->addWidget(blankBtn);
    outer->addLayout(blankRow);

    return page;
}

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    resize(1280, 800);

    setupActions();
    setupCentralLayout();
    setupMenuBar();
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

    m_tabBar = new QTabBar(this);
    m_tabBar->setTabsClosable(true);
    m_tabBar->setMovable(true);
    m_tabBar->setDocumentMode(true);
    m_tabBar->setExpanding(false);
    tabBarRow->addWidget(m_tabBar);

    m_newTabBtn = new QToolButton(this);
    m_newTabBtn->setText("+");
    m_newTabBtn->setToolTip("New Tab (Ctrl+T)");
    m_newTabBtn->setAutoRaise(true);
    m_newTabBtn->setFixedSize(28, 28);
    m_newTabBtn->setObjectName("newTabBtn");
    connect(m_newTabBtn, &QToolButton::clicked, this, &MainWindow::addNewTab);
    tabBarRow->addWidget(m_newTabBtn);
    tabBarRow->addStretch();

    // Toggle buttons at right end of tab bar row
    m_toggleOutlineBtn = new QToolButton(this);
    m_toggleOutlineBtn->setIcon(makeToolIcon("sidebar"));
    m_toggleOutlineBtn->setToolTip("Toggle Outline Panel");
    m_toggleOutlineBtn->setCheckable(true);
    m_toggleOutlineBtn->setChecked(true);
    m_toggleOutlineBtn->setAutoRaise(true);
    m_toggleOutlineBtn->setFixedSize(28, 28);
    m_toggleOutlineBtn->setIconSize(QSize(18, 18));
    m_toggleOutlineBtn->setObjectName("togglePanelBtn");
    tabBarRow->addWidget(m_toggleOutlineBtn);

    m_toggleToolbarBtn = new QToolButton(this);
    m_toggleToolbarBtn->setIcon(makeToolIcon("toolbar"));
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

    setupOutlinePanel();
    m_contentSplitter->addWidget(m_outlinePanel);

    // Right panel: toolbar above content stack
    m_rightPanel = new QWidget(this);
    auto* rightLayout = new QVBoxLayout(m_rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);
    rightLayout->addWidget(m_toolbarWidget);

    m_contentStack = new QStackedWidget(this);
    rightLayout->addWidget(m_contentStack, 1);

    m_contentSplitter->addWidget(m_rightPanel);

    m_contentSplitter->setStretchFactor(0, 0);
    m_contentSplitter->setStretchFactor(1, 1);
    m_contentSplitter->setSizes({200, 1080});
    m_contentSplitter->setCollapsible(0, true);
    m_contentSplitter->setCollapsible(1, false);

    mainLayout->addWidget(m_contentSplitter, 1);

    setCentralWidget(centralW);

    // ---- Signals ----
    connect(m_tabBar, &QTabBar::currentChanged, this, &MainWindow::switchToTab);
    connect(m_tabBar, &QTabBar::tabCloseRequested, this, &MainWindow::closeTab);
    connect(m_tabBar, &QTabBar::tabMoved, this, [this](int from, int to) {
        m_tabs.move(from, to);
        QWidget* w = m_contentStack->widget(from);
        QSignalBlocker blocker(m_contentStack);
        m_contentStack->removeWidget(w);
        m_contentStack->insertWidget(to, w);
        m_contentStack->setCurrentIndex(m_tabBar->currentIndex());
    });

    m_tabBar->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tabBar, &QWidget::customContextMenuRequested, this,
            &MainWindow::onTabBarContextMenu);
}

// ---------------------------------------------------------------------------
// Tab management
// ---------------------------------------------------------------------------
void MainWindow::addNewTab() {
    auto* scene = new MindMapScene(this);
    auto* view = new MindMapView(this);
    view->setScene(scene);

    auto* stack = new QStackedWidget(this);
    stack->addWidget(createStartPage()); // index 0 — start page
    stack->addWidget(view);              // index 1 — mind map view
    stack->setCurrentIndex(0);           // show start page

    addTab(scene, view, stack, QString());
}

void MainWindow::addTab(MindMapScene* scene, MindMapView* view, QStackedWidget* stack,
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

    // Now switch to the new tab (triggers switchToTab)
    m_tabBar->setCurrentIndex(m_tabs.size() - 1);
    switchToTab(m_tabs.size() - 1);
}

void MainWindow::connectSceneSignals(MindMapScene* scene) {
    connect(scene, &MindMapScene::modifiedChanged, this, [this, scene](bool) {
        // Find which tab this scene belongs to
        for (int i = 0; i < m_tabs.size(); ++i) {
            if (m_tabs[i].scene == scene) {
                updateTabText(i);
                if (i == m_tabBar->currentIndex())
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
                if (i == m_tabBar->currentIndex()) {
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

    m_contentStack->setCurrentIndex(index);

    connectUndoStack();
    updateWindowTitle();
    refreshOutline();
    updateContentVisibility();
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
    m_tabBar->setTabText(index, label);
}

bool MainWindow::isTabEmpty(int index) const {
    if (index < 0 || index >= m_tabs.size())
        return false;
    const auto& tab = m_tabs[index];
    // Tab is empty if showing start page, or if untitled and unmodified
    if (tab.stack) {
        QWidget* current = tab.stack->currentWidget();
        if (current && current->objectName() == "startPage")
            return true;
    }
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
    m_tabBar->removeTab(index);
    m_contentStack->removeWidget(tab.stack);

    delete tab.scene;
    delete tab.stack;

    // Always keep at least one tab
    if (m_tabs.isEmpty())
        addNewTab();
}

void MainWindow::onTabBarContextMenu(const QPoint& pos) {
    int index = m_tabBar->tabAt(pos);

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

    menu.exec(m_tabBar->mapToGlobal(pos));
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
    int cur = m_tabBar->currentIndex();
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
        m_tabBar->setCurrentIndex(existing);
        return;
    }

    // Reuse current tab if it's empty, otherwise create new
    int cur = m_tabBar->currentIndex();
    if (cur >= 0 && isTabEmpty(cur)) {
        if (!m_scene->loadFromFile(filePath)) {
            QMessageBox::warning(this, "XMind", "Could not open file:\n" + filePath);
            return;
        }
        m_currentFile = filePath;
        m_tabs[cur].filePath = filePath;
        // Switch from start page to mind map view
        if (m_tabs[cur].stack)
            m_tabs[cur].stack->setCurrentIndex(1);
        updateTabText(cur);
        updateWindowTitle();
        m_view->zoomToFit();
        refreshOutline();
        updateContentVisibility();
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

        // Create stack showing view directly (no start page needed)
        auto* stack = new QStackedWidget(this);
        stack->addWidget(view);
        stack->setCurrentIndex(0);

        addTab(scene, view, stack, filePath);
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

    int cur = m_tabBar->currentIndex();
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
    int cur = m_tabBar->currentIndex();
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
    int cur = m_tabBar->currentIndex();
    if (cur >= 0 && isTabEmpty(cur)) {
        if (!m_scene->importFromText(text)) {
            QMessageBox::warning(this, "XMind", "Could not parse text file:\n" + filePath);
            return;
        }
        m_currentFile.clear();
        // Switch from start page to mind map view
        if (m_tabs[cur].stack)
            m_tabs[cur].stack->setCurrentIndex(1);
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

        auto* stack = new QStackedWidget(this);
        stack->addWidget(view);
        stack->setCurrentIndex(0);

        addTab(scene, view, stack, QString());
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
        qApp->setStyleSheet(kLightStyleSheet);
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
    m_toolbarWidget = new QWidget(this);
    m_toolbarWidget->setObjectName("inlineToolbar");

    auto* layout = new QHBoxLayout(m_toolbarWidget);
    layout->setContentsMargins(4, 2, 4, 2);
    layout->setSpacing(2);
    layout->addStretch();

    auto addButton = [&](const QString& iconName, const QString& text,
                         const QString& tooltip) -> QToolButton* {
        auto* btn = new QToolButton(m_toolbarWidget);
        btn->setIcon(makeToolIcon(iconName));
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

    auto* addChildBtn = addButton("add-child", "Add Child", "Add a child node (Enter)");
    connect(addChildBtn, &QToolButton::clicked, this,
            [this]() { m_scene->addChildToSelected(); });

    auto* addSiblingBtn = addButton("add-sibling", "Add Sibling", "Add a sibling node (Ctrl+Enter)");
    connect(addSiblingBtn, &QToolButton::clicked, this,
            [this]() { m_scene->addSiblingToSelected(); });

    auto* deleteBtn = addButton("delete", "Delete", "Delete selected node (Del)");
    connect(deleteBtn, &QToolButton::clicked, this,
            [this]() { m_scene->deleteSelected(); });

    addSeparator();

    auto* layoutBtn = addButton("auto-layout", "Auto Layout", "Automatically arrange all nodes (Ctrl+L)");
    connect(layoutBtn, &QToolButton::clicked, this,
            [this]() { m_scene->autoLayout(); });

    addSeparator();

    auto* zoomBtn = addButton("zoom", "Zoom", "Zoom in (Ctrl++)");
    connect(zoomBtn, &QToolButton::clicked, this,
            [this]() { m_view->zoomIn(); });

    auto* fitBtn = addButton("fit-view", "Fit View", "Fit all nodes in view (Ctrl+0)");
    connect(fitBtn, &QToolButton::clicked, this,
            [this]() { m_view->zoomToFit(); });

    addSeparator();

    auto* exportBtn = new QToolButton(m_toolbarWidget);
    exportBtn->setIcon(makeToolIcon("export"));
    exportBtn->setText("Export");
    exportBtn->setToolTip("Export mind map");
    exportBtn->setPopupMode(QToolButton::InstantPopup);
    exportBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    exportBtn->setAutoRaise(true);
    exportBtn->setIconSize(QSize(24, 24));
    auto* exportBtnMenu = new QMenu(exportBtn);
    exportBtnMenu->addAction("As Text...", this, &MainWindow::exportAsText);
    exportBtnMenu->addAction("As Markdown...", this, &MainWindow::exportAsMarkdown);
    exportBtnMenu->addSeparator();
    exportBtnMenu->addAction("As PNG...", this, &MainWindow::exportAsPng);
    exportBtnMenu->addAction("As SVG...", this, &MainWindow::exportAsSvg);
    exportBtnMenu->addAction("As PDF...", this, &MainWindow::exportAsPdf);
    exportBtn->setMenu(exportBtnMenu);
    layout->addWidget(exportBtn);

    layout->addStretch();

    // Close button at right end of toolbar
    auto* closeBtn = new QToolButton(m_toolbarWidget);
    closeBtn->setIcon(makeToolIcon("close-panel"));
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
            [this]() { closeTab(m_tabBar->currentIndex()); });

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

    // Bidirectional sync: tab bar toggle buttons → View menu actions
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
    connect(autoLayoutAct, &QAction::triggered, this, [this]() { m_scene->autoLayout(); });

    // ---- Insert menu ----
    auto* insertMenu = menuBar()->addMenu("&Insert");

    m_addChildAct = insertMenu->addAction("Add &Child");
    m_addChildAct->setToolTip("Add a child node (Enter)");
    connect(m_addChildAct, &QAction::triggered, this, [this]() { m_scene->addChildToSelected(); });

    m_addSiblingAct = insertMenu->addAction("Add &Sibling");
    m_addSiblingAct->setToolTip("Add a sibling node (Ctrl+Enter)");
    connect(m_addSiblingAct, &QAction::triggered, this,
            [this]() { m_scene->addSiblingToSelected(); });

    // ---- Style menu ----
    auto* styleMenu = menuBar()->addMenu("&Style");

    auto* settingsAct = styleMenu->addAction("&Settings...");
    settingsAct->setShortcut(QKeySequence("Ctrl+,"));
    connect(settingsAct, &QAction::triggered, this, &MainWindow::openSettings);
}

// ---------------------------------------------------------------------------
// Outline panel (embedded in content splitter)
// ---------------------------------------------------------------------------
void MainWindow::setupOutlinePanel() {
    m_outlinePanel = new QWidget(this);
    m_outlinePanel->setMinimumWidth(120);

    auto* layout = new QVBoxLayout(m_outlinePanel);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Header row with label + close button
    auto* header = new QWidget();
    header->setObjectName("sectionHeader");
    auto* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(10, 6, 4, 6);
    headerLayout->setSpacing(0);

    auto* outlineLabel = new QLabel("Outline");
    outlineLabel->setStyleSheet("background: transparent; border: none; font-weight: bold; font-size: 12px;");
    headerLayout->addWidget(outlineLabel);
    headerLayout->addStretch();

    auto* closeBtn = new QToolButton();
    closeBtn->setIcon(makeToolIcon("close-panel"));
    closeBtn->setToolTip("Hide Outline");
    closeBtn->setAutoRaise(true);
    closeBtn->setFixedSize(20, 20);
    closeBtn->setIconSize(QSize(14, 14));
    closeBtn->setObjectName("closePanelBtn");
    connect(closeBtn, &QToolButton::clicked, this, [this]() {
        if (m_toggleOutlineAct)
            m_toggleOutlineAct->setChecked(false);
    });
    headerLayout->addWidget(closeBtn);

    layout->addWidget(header);

    m_outlineTree = new QTreeWidget();
    m_outlineTree->setHeaderHidden(true);
    m_outlineTree->setAnimated(true);
    m_outlineTree->setIndentation(16);
    m_outlineTree->setExpandsOnDoubleClick(false);
    connect(m_outlineTree, &QTreeWidget::itemClicked, this, &MainWindow::onOutlineItemClicked);
    layout->addWidget(m_outlineTree, 1);
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
    int tabIdx = m_tabBar->currentIndex();
    if (tabIdx < 0)
        return;

    // If current tab is not empty (already has content), create a new tab first
    if (!isTabEmpty(tabIdx)) {
        addNewTab();
        tabIdx = m_tabBar->currentIndex();
    }

    m_currentFile.clear();
    m_tabs[tabIdx].filePath.clear();

    auto* root = m_scene->rootNode();

    if (index == 0) {
        // Mind Map — central topic with 4 branches
        root->setText("Central Topic");
        m_scene->addNode("Branch 1", root);
        m_scene->addNode("Branch 2", root);
        m_scene->addNode("Branch 3", root);
        m_scene->addNode("Branch 4", root);
        m_scene->setLayoutStyle(LayoutStyle::Bilateral);
    } else if (index == 1) {
        // Org Chart — CEO -> 3 departments
        root->setText("CEO");
        m_scene->setLayoutStyle(LayoutStyle::TopDown);
        m_scene->addNode("Engineering", root);
        m_scene->addNode("Marketing", root);
        m_scene->addNode("Sales", root);
    } else if (index == 2) {
        // Project Plan — root -> phases -> tasks
        root->setText("Project");
        m_scene->setLayoutStyle(LayoutStyle::RightTree);
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

    // Switch from start page to mind map view
    if (m_tabs[tabIdx].stack)
        m_tabs[tabIdx].stack->setCurrentIndex(1);

    updateTabText(tabIdx);
    updateWindowTitle();
    refreshOutline();
    m_view->zoomToFit();
    updateContentVisibility();

    statusBar()->showMessage("Template loaded", 3000);
}

// ---------------------------------------------------------------------------
// Blank canvas
// ---------------------------------------------------------------------------
void MainWindow::activateBlankCanvas() {
    int tabIdx = m_tabBar->currentIndex();
    if (tabIdx < 0)
        return;

    // Switch from start page to mind map view (scene already has default "Central Topic")
    if (m_tabs[tabIdx].stack)
        m_tabs[tabIdx].stack->setCurrentIndex(1);

    updateTabText(tabIdx);
    updateWindowTitle();
    refreshOutline();
    updateContentVisibility();
}

// ---------------------------------------------------------------------------
// Content visibility (toolbar + outline hidden on start page)
// ---------------------------------------------------------------------------
void MainWindow::updateContentVisibility() {
    int idx = m_tabBar->currentIndex();
    bool onStartPage = false;
    if (idx >= 0 && idx < m_tabs.size() && m_tabs[idx].stack) {
        QWidget* current = m_tabs[idx].stack->currentWidget();
        onStartPage = current && current->objectName() == "startPage";
    }

    bool showToolbar = !onStartPage && m_toggleToolbarAct && m_toggleToolbarAct->isChecked();
    bool showOutline = !onStartPage && m_toggleOutlineAct && m_toggleOutlineAct->isChecked();

    m_toolbarWidget->setVisible(showToolbar);
    m_outlinePanel->setVisible(showOutline);

    // Hide/show tab bar toggle buttons on start page
    if (m_toggleOutlineBtn)
        m_toggleOutlineBtn->setVisible(!onStartPage);
    if (m_toggleToolbarBtn)
        m_toggleToolbarBtn->setVisible(!onStartPage);

    // Disable/enable Insert actions on start page
    if (m_addChildAct)
        m_addChildAct->setEnabled(!onStartPage);
    if (m_addSiblingAct)
        m_addSiblingAct->setEnabled(!onStartPage);
}
