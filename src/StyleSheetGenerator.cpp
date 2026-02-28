#include "StyleSheetGenerator.h"

#include <QString>

// ---------------------------------------------------------------------------
// Color maps for dark and light themes.
// Each map defines the values to substitute into the shared CSS template.
// ---------------------------------------------------------------------------

struct ThemeColorMap {
    // Window / dialog
    const char* windowBg;
    const char* windowFg;
    // Menu bar
    const char* menuBarBg;
    const char* menuBarBorder;
    const char* menuBarSelBg;
    // Menu
    const char* menuBg;
    const char* menuBorder;
    const char* menuSelBg;
    const char* menuDisabledFg;
    const char* menuSepColor;
    // Toolbar
    const char* toolBarBg;
    const char* toolBarBorder;
    const char* toolBtnHoverBg;
    const char* toolBtnPressedBg;
    const char* toolBtnDisabledFg;
    // Tab bar
    const char* tabBarBg;
    const char* tabBg;
    const char* tabFg;
    const char* tabBorder;
    const char* tabSelBg;
    const char* tabSelFg;
    const char* tabHoverBg;
    const char* tabHoverFg;
    const char* tabCloseBtnHoverBg;
    // New tab button
    const char* newTabFg;
    const char* newTabHoverBg;
    const char* newTabHoverFg;
    // Inline toolbar
    const char* inlineToolbarBg;
    const char* inlineToolbarBorder;
    // Inline toolbar buttons (light theme overrides)
    const char* inlineToolBtnFg;
    const char* inlineToolBtnHoverBg;
    const char* inlineToolBtnPressedBg;
    const char* inlineToolBtnDisabledFg;
    const char* inlineCloseBtnFg;
    const char* inlineCloseBtnHoverBg;
    // Section header
    const char* headerBg;
    const char* headerFg;
    const char* headerBorder;
    // Toggle panel button
    const char* toggleBtnFg;
    const char* toggleBtnHoverBg;
    const char* toggleBtnHoverFg;
    const char* toggleBtnCheckedBg;
    const char* toggleBtnCheckedFg;
    // Close panel button
    const char* closeBtnFg;
    const char* closeBtnHoverBg;
    const char* closeBtnHoverFg;
    // Tree widget
    const char* treeBg;
    const char* treeFg;
    const char* treeHoverBg;
    const char* treeSelBg;
    const char* treeSelFg;
    const char* treeBranchBg;
    // Status bar
    const char* statusBarBg;
    const char* statusBarFg;
    // Scrollbar
    const char* scrollBg;
    const char* scrollHandleBg;
    const char* scrollHandleHover;
    // Combo box
    const char* comboBg;
    const char* comboFg;
    const char* comboBorder;
    const char* comboHoverBorder;
    const char* comboDropBg;
    const char* comboDropSelBg;
    // Spin box
    const char* spinBg;
    const char* spinFg;
    const char* spinBorder;
    const char* spinHoverBorder;
    // Line edit
    const char* lineEditBg;
    const char* lineEditFg;
    const char* lineEditBorder;
    // Checkbox
    const char* checkboxFg;
    const char* checkboxBorder;
    const char* checkboxBg;
    const char* checkboxCheckedBg;
    const char* checkboxCheckedBorder;
    // Push button
    const char* pushBtnBg;
    const char* pushBtnFg;
    const char* pushBtnHoverBg;
    const char* pushBtnPressedBg;
    // Group box
    const char* groupBoxFg;
    const char* groupBoxBorder;
    // Label
    const char* labelFg;
    // Tooltip
    const char* tooltipBg;
    const char* tooltipFg;
    const char* tooltipBorder;
    // Template card
    const char* cardBg;
    const char* cardBorder;
    const char* cardFg;
    const char* cardHoverBorder;
    const char* cardHoverBg;
    const char* cardPressedBg;
    // Blank canvas button
    const char* blankBtnBorder;
    const char* blankBtnFg;
    const char* blankBtnHoverBorder;
    const char* blankBtnHoverBg;
    const char* blankBtnPressedBg;
    // Start page
    const char* subtitleFg;
    // Settings hint
    const char* settingsHintFg;
};

// clang-format off
static const ThemeColorMap kDarkMap = {
    "#2D2D30", "#D4D4D4",               // window
    "#2D2D30", "#3F3F46", "#3F3F46",    // menu bar
    "#2D2D30", "#3F3F46", "#094771", "#5A5A5A", "#3F3F46", // menu
    "#2D2D30", "#3F3F46", "#3F3F46", "#094771", "#5A5A5A", // toolbar
    "#252526",                           // tab bar bg
    "#2D2D30", "#969696", "#3F3F46",     // tab bg/fg/border
    "#1E1E1E", "#D4D4D4",               // tab selected
    "#2D2D30", "#D4D4D4",               // tab hover
    "#3F3F46",                           // tab close hover bg
    "#969696", "#3F3F46", "#D4D4D4",     // new tab
    "#2D2D30", "#3F3F46",               // inline toolbar
    "#D4D4D4", "#3F3F46", "#094771", "#5A5A5A", // inline toolbar buttons
    "#969696", "#3F3F46",               // inline close btn
    "#2D2D30", "#D4D4D4", "#3F3F46",    // section header
    "#969696", "#3F3F46", "#D4D4D4", "#094771", "#D4D4D4", // toggle panel
    "#969696", "#3F3F46", "#D4D4D4",     // close panel
    "#252526", "#D4D4D4", "#2A2D2E", "#094771", "#FFFFFF", "#252526", // tree
    "#007ACC", "#FFFFFF",               // status bar
    "#2D2D30", "#424242", "#4F4F4F",     // scroll
    "#3C3C3C", "#D4D4D4", "#3F3F46", "#007ACC", "#2D2D30", "#094771", // combo
    "#3C3C3C", "#D4D4D4", "#3F3F46", "#007ACC", // spin
    "#2A2A4A", "#D4D4D4", "#42A5F5",     // line edit
    "#D4D4D4", "#3F3F46", "#3C3C3C", "#007ACC", "#007ACC", // checkbox
    "#0E639C", "#FFFFFF", "#1177BB", "#094771", // push button
    "#D4D4D4", "#3F3F46",               // group box
    "#D4D4D4",                           // label
    "#2D2D30", "#D4D4D4", "#3F3F46",     // tooltip
    "#2D2D30", "#3F3F46", "#D4D4D4", "#007ACC", "#333337", "#094771", // template card
    "#3F3F46", "#D4D4D4", "#007ACC", "#2D2D30", "#094771", // blank canvas
    "#999999",                           // subtitle
    "#888888",                           // settings hint
};

static const ThemeColorMap kLightMap = {
    "#FFFFFF", "#1E1E1E",               // window
    "#F8F8F8", "#D0D0D0", "#E1E4E8",    // menu bar
    "#FFFFFF", "#D0D0D0", "#E1E4E8", "#B0B0B0", "#D0D0D0", // menu
    "#F8F8F8", "#D0D0D0", "#E1E4E8", "#D0D0D0", "#B0B0B0", // toolbar
    "transparent",                       // tab bar bg
    "#E0E0E0", "#1E1E1E", "#D0D0D0",     // tab bg/fg/border
    "#FFFFFF", "#1E1E1E",               // tab selected
    "#F0F0F0", "#1E1E1E",               // tab hover
    "#D0D0D0",                           // tab close hover bg
    "#1E1E1E", "#E1E4E8", "#1E1E1E",     // new tab
    "#F8F8F8", "#D0D0D0",               // inline toolbar
    "#1E1E1E", "#E1E4E8", "#B8D4F0", "#B0B0B0", // inline toolbar buttons
    "#1E1E1E", "#E1E4E8",               // inline close btn
    "#F0F0F0", "#1E1E1E", "#D0D0D0",    // section header
    "#1E1E1E", "#E1E4E8", "#1E1E1E", "#CCE4F7", "#1E1E1E", // toggle panel
    "#1E1E1E", "#E1E4E8", "#1E1E1E",     // close panel
    "#FFFFFF", "#1E1E1E", "#F0F0F0", "#CCE4F7", "#1E1E1E", "#FFFFFF", // tree
    "#007ACC", "#FFFFFF",               // status bar (same as dark)
    "#F8F8F8", "#CCCCCC", "#BBBBBB",     // scroll
    "", "", "", "", "", "",              // combo (not in light)
    "", "", "", "",                      // spin (not in light)
    "#FFFFFF", "#1E1E1E", "#1565C0",     // line edit
    "#1E1E1E", "", "", "", "",           // checkbox (not in light)
    "", "", "", "",                      // push button (not in light)
    "#1E1E1E", "#D0D0D0",               // group box
    "#1E1E1E",                           // label
    "", "", "",                          // tooltip (not in light)
    "#F0F0F0", "#D0D0D0", "#333333", "#007ACC", "#E8E8E8", "#D0E8FF", // template card
    "#D0D0D0", "#333333", "#007ACC", "#F0F0F0", "#D0E8FF", // blank canvas
    "#888888",                           // subtitle
    "gray",                              // settings hint
};
// clang-format on

// ---------------------------------------------------------------------------
// Build stylesheet string from a color map + template
// ---------------------------------------------------------------------------
static QString buildStyleSheet(const ThemeColorMap& c) {
    QString css;
    css.reserve(4096);

    // Window / Dialog
    css += QString("QMainWindow, QDialog { background-color: %1; color: %2; }\n")
               .arg(c.windowBg, c.windowFg);

    // Menu bar
    css += QString("QMenuBar { background-color: %1; color: %2; border-bottom: 1px solid %3; }\n")
               .arg(c.menuBarBg, c.windowFg, c.menuBarBorder);
    css += QString("QMenuBar::item:selected { background-color: %1; }\n").arg(c.menuBarSelBg);

    // Menu
    css += QString("QMenu { background-color: %1; color: %2; border: 1px solid %3; }\n")
               .arg(c.menuBg, c.windowFg, c.menuBorder);
    css += QString("QMenu::item:selected { background-color: %1; }\n").arg(c.menuSelBg);
    css += QString("QMenu::item:disabled { color: %1; }\n").arg(c.menuDisabledFg);
    css += QString("QMenu::separator { height: 1px; background: %1; margin: 4px 8px; }\n")
               .arg(c.menuSepColor);

    // Toolbar
    css += QString(
               "QToolBar { background-color: %1; border-bottom: 1px solid %2; spacing: 2px; "
               "padding: 4px; }\n")
               .arg(c.toolBarBg, c.toolBarBorder);
    css += QString(
               "QToolButton { background-color: transparent; color: %1; border: 1px solid "
               "transparent; border-radius: 4px; padding: 4px 8px; font-size: 11px; }\n")
               .arg(c.windowFg);
    css += QString("QToolButton:hover { background-color: %1; border-color: %1; }\n")
               .arg(c.toolBtnHoverBg);
    css += QString("QToolButton:pressed { background-color: %1; }\n").arg(c.toolBtnPressedBg);
    css += QString("QToolButton:disabled { color: %1; }\n").arg(c.toolBtnDisabledFg);

    // Tab bar
    css += QString("QTabBar { background-color: %1; }\n").arg(c.tabBarBg);
    css += QString(
               "QTabBar::tab { background-color: %1; color: %2; border: none; border-right: 1px "
               "solid %3; padding: 6px 12px; min-width: 100px; max-width: 200px; }\n")
               .arg(c.tabBg, c.tabFg, c.tabBorder);
    css += QString(
               "QTabBar::tab:selected { background-color: %1; color: %2; border-bottom: 2px solid "
               "#007ACC; }\n")
               .arg(c.tabSelBg, c.tabSelFg);
    css += QString("QTabBar::tab:hover:!selected { background-color: %1; color: %2; }\n")
               .arg(c.tabHoverBg, c.tabHoverFg);

    // Tab close button
    css += "QTabBar::close-button { subcontrol-position: right; border: none; padding: 2px; "
           "margin: 2px; background: transparent; width: 14px; height: 14px; }\n";
    css += QString("QTabBar::close-button:hover { background-color: %1; border-radius: 3px; }\n")
               .arg(c.tabCloseBtnHoverBg);

    // New tab button
    css += QString(
               "QToolButton#newTabBtn { background-color: transparent; color: %1; border: none; "
               "border-radius: 4px; font-size: 16px; font-weight: bold; }\n")
               .arg(c.newTabFg);
    css += QString("QToolButton#newTabBtn:hover { background-color: %1; color: %2; }\n")
               .arg(c.newTabHoverBg, c.newTabHoverFg);

    // Inline toolbar
    css += QString("QWidget#inlineToolbar { background-color: %1; border-bottom: 1px solid %2; }\n")
               .arg(c.inlineToolbarBg, c.inlineToolbarBorder);

    // Light theme adds specific inline toolbar button overrides
    if (c.inlineToolBtnFg[0] != '\0') {
        css += QString(
                   "QWidget#inlineToolbar QToolButton { background-color: transparent; color: %1; "
                   "border: 1px solid transparent; border-radius: 4px; padding: 4px 8px; "
                   "font-size: 11px; }\n")
                   .arg(c.inlineToolBtnFg);
        css += QString(
                   "QWidget#inlineToolbar QToolButton:hover { background-color: %1; border-color: "
                   "%1; }\n")
                   .arg(c.inlineToolBtnHoverBg);
        css +=
            QString("QWidget#inlineToolbar QToolButton:pressed { background-color: %1; }\n")
                .arg(c.inlineToolBtnPressedBg);
        css +=
            QString("QWidget#inlineToolbar QToolButton:disabled { color: %1; }\n")
                .arg(c.inlineToolBtnDisabledFg);
        css += QString(
                   "QWidget#inlineToolbar QToolButton#closePanelBtn { background-color: "
                   "transparent; color: %1; border: none; border-radius: 3px; padding: 2px; }\n")
                   .arg(c.inlineCloseBtnFg);
        css += QString(
                   "QWidget#inlineToolbar QToolButton#closePanelBtn:hover { background-color: %1; "
                   "color: %2; }\n")
                   .arg(c.inlineCloseBtnHoverBg, c.inlineCloseBtnFg);
    }

    // Section header
    css += QString(
               "QLabel#sectionHeader, QWidget#sectionHeader { background-color: %1; color: %2; "
               "font-weight: bold; font-size: 12px; padding: 6px 10px; border-bottom: 1px solid "
               "%3; }\n")
               .arg(c.headerBg, c.headerFg, c.headerBorder);

    // Toggle panel button
    css += QString(
               "QToolButton#togglePanelBtn { background-color: transparent; color: %1; border: "
               "none; border-radius: 4px; padding: 4px; }\n")
               .arg(c.toggleBtnFg);
    css += QString("QToolButton#togglePanelBtn:hover { background-color: %1; color: %2; }\n")
               .arg(c.toggleBtnHoverBg, c.toggleBtnHoverFg);
    css += QString("QToolButton#togglePanelBtn:checked { background-color: %1; color: %2; }\n")
               .arg(c.toggleBtnCheckedBg, c.toggleBtnCheckedFg);

    // Close panel button
    css += QString(
               "QToolButton#closePanelBtn { background-color: transparent; color: %1; border: "
               "none; border-radius: 3px; padding: 2px; }\n")
               .arg(c.closeBtnFg);
    css += QString("QToolButton#closePanelBtn:hover { background-color: %1; color: %2; }\n")
               .arg(c.closeBtnHoverBg, c.closeBtnHoverFg);

    // Tree widget
    css += QString(
               "QTreeWidget { background-color: %1; color: %2; border: none; outline: none; "
               "font-size: 12px; }\n")
               .arg(c.treeBg, c.treeFg);
    css += "QTreeWidget::item { padding: 3px 0px; }\n";
    css += QString("QTreeWidget::item:hover { background-color: %1; }\n").arg(c.treeHoverBg);
    css += QString("QTreeWidget::item:selected { background-color: %1; color: %2; }\n")
               .arg(c.treeSelBg, c.treeSelFg);
    css += QString("QTreeWidget::branch { background-color: %1; }\n").arg(c.treeBranchBg);

    // Status bar
    css += QString("QStatusBar { background-color: %1; color: %2; font-size: 12px; }\n")
               .arg(c.statusBarBg, c.statusBarFg);
    css += "QStatusBar::item { border: none; }\n";

    // Scrollbars (vertical)
    css += QString(
               "QScrollBar:vertical { background-color: %1; width: 12px; margin: 0; }\n")
               .arg(c.scrollBg);
    css += QString(
               "QScrollBar::handle:vertical { background-color: %1; min-height: 20px; "
               "border-radius: 4px; margin: 2px; }\n")
               .arg(c.scrollHandleBg);
    css += QString("QScrollBar::handle:vertical:hover { background-color: %1; }\n")
               .arg(c.scrollHandleHover);
    css += "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }\n";

    // Scrollbars (horizontal)
    css += QString(
               "QScrollBar:horizontal { background-color: %1; height: 12px; margin: 0; }\n")
               .arg(c.scrollBg);
    css += QString(
               "QScrollBar::handle:horizontal { background-color: %1; min-width: 20px; "
               "border-radius: 4px; margin: 2px; }\n")
               .arg(c.scrollHandleBg);
    css += QString("QScrollBar::handle:horizontal:hover { background-color: %1; }\n")
               .arg(c.scrollHandleHover);
    css += "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }\n";

    // Combo box (dark theme only provides these; light theme doesn't)
    if (c.comboBg[0] != '\0') {
        css += QString(
                   "QComboBox { background-color: %1; color: %2; border: 1px solid %3; "
                   "border-radius: 4px; padding: 4px 8px; }\n")
                   .arg(c.comboBg, c.comboFg, c.comboBorder);
        css += QString("QComboBox:hover { border-color: %1; }\n").arg(c.comboHoverBorder);
        css += "QComboBox::drop-down { border: none; width: 20px; }\n";
        css += QString(
                   "QComboBox QAbstractItemView { background-color: %1; color: %2; "
                   "selection-background-color: %3; border: 1px solid %4; }\n")
                   .arg(c.comboDropBg, c.comboFg, c.comboDropSelBg, c.comboBorder);
    }

    // Spin box (dark theme only)
    if (c.spinBg[0] != '\0') {
        css += QString(
                   "QSpinBox { background-color: %1; color: %2; border: 1px solid %3; "
                   "border-radius: 4px; padding: 4px; }\n")
                   .arg(c.spinBg, c.spinFg, c.spinBorder);
        css += QString("QSpinBox:hover { border-color: %1; }\n").arg(c.spinHoverBorder);
    }

    // Line edit
    css += QString(
               "QLineEdit { background-color: %1; color: %2; border: 2px solid %3; border-radius: "
               "6px; padding: 4px 8px; }\n")
               .arg(c.lineEditBg, c.lineEditFg, c.lineEditBorder);

    // Checkbox (dark theme only provides full spec)
    if (c.checkboxBorder[0] != '\0') {
        css += QString("QCheckBox { color: %1; spacing: 8px; }\n").arg(c.checkboxFg);
        css += QString(
                   "QCheckBox::indicator { width: 16px; height: 16px; border: 1px solid %1; "
                   "border-radius: 3px; background-color: %2; }\n")
                   .arg(c.checkboxBorder, c.checkboxBg);
        css += QString(
                   "QCheckBox::indicator:checked { background-color: %1; border-color: %2; }\n")
                   .arg(c.checkboxCheckedBg, c.checkboxCheckedBorder);
    }

    // Push button (dark theme only)
    if (c.pushBtnBg[0] != '\0') {
        css += QString(
                   "QPushButton { background-color: %1; color: %2; border: none; border-radius: "
                   "4px; padding: 6px 16px; font-size: 12px; }\n")
                   .arg(c.pushBtnBg, c.pushBtnFg);
        css += QString("QPushButton:hover { background-color: %1; }\n").arg(c.pushBtnHoverBg);
        css += QString("QPushButton:pressed { background-color: %1; }\n").arg(c.pushBtnPressedBg);
    }

    // Group box
    css += QString(
               "QGroupBox { color: %1; border: 1px solid %2; border-radius: 4px; margin-top: 8px; "
               "padding-top: 16px; font-weight: bold; }\n")
               .arg(c.groupBoxFg, c.groupBoxBorder);
    css += "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 4px; }\n";

    // Label
    css += QString("QLabel { color: %1; }\n").arg(c.labelFg);
    css += QString("QGroupBox QCheckBox { color: %1; }\n").arg(c.labelFg);

    // Tooltip (dark theme only)
    if (c.tooltipBg[0] != '\0') {
        css += QString(
                   "QToolTip { background-color: %1; color: %2; border: 1px solid %3; padding: "
                   "4px; }\n")
                   .arg(c.tooltipBg, c.tooltipFg, c.tooltipBorder);
    }

    // Template card
    css += QString(
               "QPushButton#templateCard { background-color: %1; border: 2px solid %2; "
               "border-radius: 8px; padding: 8px; font-size: 13px; color: %3; text-align: "
               "bottom; }\n")
               .arg(c.cardBg, c.cardBorder, c.cardFg);
    css += QString(
               "QPushButton#templateCard:hover { border-color: %1; background-color: %2; }\n")
               .arg(c.cardHoverBorder, c.cardHoverBg);
    css +=
        QString("QPushButton#templateCard:pressed { background-color: %1; }\n")
            .arg(c.cardPressedBg);

    // Blank canvas button
    css += QString(
               "QPushButton#blankCanvasBtn { background-color: transparent; border: 1px solid %1; "
               "border-radius: 6px; font-size: 13px; color: %2; }\n")
               .arg(c.blankBtnBorder, c.blankBtnFg);
    css += QString(
               "QPushButton#blankCanvasBtn:hover { border-color: %1; background-color: %2; }\n")
               .arg(c.blankBtnHoverBorder, c.blankBtnHoverBg);
    css +=
        QString("QPushButton#blankCanvasBtn:pressed { background-color: %1; }\n")
            .arg(c.blankBtnPressedBg);

    // Start page labels
    css += "QLabel#startPageTitle { font-size: 24px; font-weight: bold; margin-bottom: 4px; "
           "background: transparent; border: none; }\n";
    css += QString(
               "QLabel#startPageSubtitle { font-size: 14px; color: %1; margin-bottom: 24px; "
               "background: transparent; border: none; }\n")
               .arg(c.subtitleFg);

    // Settings hint
    css += QString("QLabel#settingsHint { color: %1; font-size: 9pt; }\n").arg(c.settingsHintFg);

    return css;
}

// ---------------------------------------------------------------------------
// Cache the generated stylesheets so we build them only once per call.
// Using QByteArray for stable const char* pointers.
// ---------------------------------------------------------------------------
const char* StyleSheetGenerator::darkStyleSheet() {
    static QByteArray cached = buildStyleSheet(kDarkMap).toUtf8();
    return cached.constData();
}

const char* StyleSheetGenerator::lightStyleSheet() {
    static QByteArray cached = buildStyleSheet(kLightMap).toUtf8();
    return cached.constData();
}
