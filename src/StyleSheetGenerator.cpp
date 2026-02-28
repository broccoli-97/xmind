#include "StyleSheetGenerator.h"

#include <QFile>
#include <QHash>
#include <QString>

// ---------------------------------------------------------------------------
// Color definitions for each theme.
// Keys must match {{placeholders}} in resources/theme.qss.
// ---------------------------------------------------------------------------

using ColorMap = QHash<QString, QString>;

// clang-format off
static ColorMap darkColors() {
    return {
        // Window / Dialog
        {"windowBg",             "#2D2D30"},
        {"windowFg",             "#D4D4D4"},
        // Menu bar
        {"menuBarBg",            "#2D2D30"},
        {"menuBarBorder",        "#3F3F46"},
        {"menuBarSelBg",         "#3F3F46"},
        // Menu
        {"menuBg",               "#2D2D30"},
        {"menuBorder",           "#3F3F46"},
        {"menuSelBg",            "#094771"},
        {"menuDisabledFg",       "#5A5A5A"},
        {"menuSepColor",         "#3F3F46"},
        // Toolbar
        {"toolBarBg",            "#2D2D30"},
        {"toolBarBorder",        "#3F3F46"},
        {"toolBtnHoverBg",       "#3F3F46"},
        {"toolBtnPressedBg",     "#094771"},
        {"toolBtnDisabledFg",    "#5A5A5A"},
        // Tab bar
        {"tabBarBg",             "#252526"},
        {"tabBg",                "#2D2D30"},
        {"tabFg",                "#969696"},
        {"tabBorder",            "#3F3F46"},
        {"tabSelBg",             "#1E1E1E"},
        {"tabSelFg",             "#D4D4D4"},
        {"tabHoverBg",           "#2D2D30"},
        {"tabHoverFg",           "#D4D4D4"},
        {"tabCloseBtnHoverBg",   "#3F3F46"},
        // New tab button
        {"newTabFg",             "#969696"},
        {"newTabHoverBg",        "#3F3F46"},
        {"newTabHoverFg",        "#D4D4D4"},
        // Inline toolbar
        {"inlineToolbarBg",      "#2D2D30"},
        {"inlineToolbarBorder",  "#3F3F46"},
        {"inlineToolBtnFg",      "#D4D4D4"},
        {"inlineToolBtnHoverBg", "#3F3F46"},
        {"inlineToolBtnPressedBg","#094771"},
        {"inlineToolBtnDisabledFg","#5A5A5A"},
        {"inlineCloseBtnFg",     "#969696"},
        {"inlineCloseBtnHoverBg","#3F3F46"},
        // Section header
        {"headerBg",             "#2D2D30"},
        {"headerFg",             "#D4D4D4"},
        {"headerBorder",         "#3F3F46"},
        // Toggle panel button
        {"toggleBtnFg",          "#969696"},
        {"toggleBtnHoverBg",     "#3F3F46"},
        {"toggleBtnHoverFg",     "#D4D4D4"},
        {"toggleBtnCheckedBg",   "#094771"},
        {"toggleBtnCheckedFg",   "#D4D4D4"},
        // Close panel button
        {"closeBtnFg",           "#969696"},
        {"closeBtnHoverBg",      "#3F3F46"},
        {"closeBtnHoverFg",      "#D4D4D4"},
        // Tree widget
        {"treeBg",               "#252526"},
        {"treeFg",               "#D4D4D4"},
        {"treeHoverBg",          "#2A2D2E"},
        {"treeSelBg",            "#094771"},
        {"treeSelFg",            "#FFFFFF"},
        {"treeBranchBg",         "#252526"},
        // Status bar
        {"statusBarBg",          "#007ACC"},
        {"statusBarFg",          "#FFFFFF"},
        {"statusBarBorder",      "#0066AA"},
        // Scrollbar
        {"scrollBg",             "#2D2D30"},
        {"scrollHandleBg",       "#424242"},
        {"scrollHandleHover",    "#4F4F4F"},
        // Combo box
        {"comboBg",              "#3C3C3C"},
        {"comboFg",              "#D4D4D4"},
        {"comboBorder",          "#3F3F46"},
        {"comboHoverBorder",     "#007ACC"},
        {"comboDropBg",          "#2D2D30"},
        {"comboDropSelBg",       "#094771"},
        // Spin box
        {"spinBg",               "#3C3C3C"},
        {"spinFg",               "#D4D4D4"},
        {"spinBorder",           "#3F3F46"},
        {"spinHoverBorder",      "#007ACC"},
        // Line edit
        {"lineEditBg",           "#2A2A4A"},
        {"lineEditFg",           "#D4D4D4"},
        {"lineEditBorder",       "#42A5F5"},
        // Checkbox
        {"checkboxFg",           "#D4D4D4"},
        {"checkboxBorder",       "#3F3F46"},
        {"checkboxBg",           "#3C3C3C"},
        {"checkboxCheckedBg",    "#007ACC"},
        {"checkboxCheckedBorder","#007ACC"},
        // Push button
        {"pushBtnBg",            "#0E639C"},
        {"pushBtnFg",            "#FFFFFF"},
        {"pushBtnHoverBg",       "#1177BB"},
        {"pushBtnPressedBg",     "#094771"},
        // Group box
        {"groupBoxFg",           "#D4D4D4"},
        {"groupBoxBorder",       "#3F3F46"},
        // Label
        {"labelFg",              "#D4D4D4"},
        // Tooltip
        {"tooltipBg",            "#2D2D30"},
        {"tooltipFg",            "#D4D4D4"},
        {"tooltipBorder",        "#3F3F46"},
        // Template card
        {"cardBg",               "#2D2D30"},
        {"cardBorder",           "#3F3F46"},
        {"cardFg",               "#D4D4D4"},
        {"cardHoverBorder",      "#007ACC"},
        {"cardHoverBg",          "#333337"},
        {"cardPressedBg",        "#094771"},
        // Blank canvas button
        {"blankBtnBorder",       "#3F3F46"},
        {"blankBtnFg",           "#D4D4D4"},
        {"blankBtnHoverBorder",  "#007ACC"},
        {"blankBtnHoverBg",      "#2D2D30"},
        {"blankBtnPressedBg",    "#094771"},
        // Start page
        {"subtitleFg",           "#999999"},
        // Settings hint
        {"settingsHintFg",       "#888888"},
    };
}

static ColorMap lightColors() {
    return {
        // Window / Dialog
        {"windowBg",             "#FFFFFF"},
        {"windowFg",             "#1E1E1E"},
        // Menu bar
        {"menuBarBg",            "#F8F8F8"},
        {"menuBarBorder",        "#D0D0D0"},
        {"menuBarSelBg",         "#E1E4E8"},
        // Menu
        {"menuBg",               "#FFFFFF"},
        {"menuBorder",           "#D0D0D0"},
        {"menuSelBg",            "#E1E4E8"},
        {"menuDisabledFg",       "#B0B0B0"},
        {"menuSepColor",         "#D0D0D0"},
        // Toolbar
        {"toolBarBg",            "#F8F8F8"},
        {"toolBarBorder",        "#D0D0D0"},
        {"toolBtnHoverBg",       "#E1E4E8"},
        {"toolBtnPressedBg",     "#D0D0D0"},
        {"toolBtnDisabledFg",    "#B0B0B0"},
        // Tab bar
        {"tabBarBg",             "transparent"},
        {"tabBg",                "#E0E0E0"},
        {"tabFg",                "#1E1E1E"},
        {"tabBorder",            "#D0D0D0"},
        {"tabSelBg",             "#FFFFFF"},
        {"tabSelFg",             "#1E1E1E"},
        {"tabHoverBg",           "#F0F0F0"},
        {"tabHoverFg",           "#1E1E1E"},
        {"tabCloseBtnHoverBg",   "#D0D0D0"},
        // New tab button
        {"newTabFg",             "#1E1E1E"},
        {"newTabHoverBg",        "#E1E4E8"},
        {"newTabHoverFg",        "#1E1E1E"},
        // Inline toolbar
        {"inlineToolbarBg",      "#F8F8F8"},
        {"inlineToolbarBorder",  "#D0D0D0"},
        {"inlineToolBtnFg",      "#1E1E1E"},
        {"inlineToolBtnHoverBg", "#E1E4E8"},
        {"inlineToolBtnPressedBg","#B8D4F0"},
        {"inlineToolBtnDisabledFg","#B0B0B0"},
        {"inlineCloseBtnFg",     "#1E1E1E"},
        {"inlineCloseBtnHoverBg","#E1E4E8"},
        // Section header
        {"headerBg",             "#F0F0F0"},
        {"headerFg",             "#1E1E1E"},
        {"headerBorder",         "#D0D0D0"},
        // Toggle panel button
        {"toggleBtnFg",          "#1E1E1E"},
        {"toggleBtnHoverBg",     "#E1E4E8"},
        {"toggleBtnHoverFg",     "#1E1E1E"},
        {"toggleBtnCheckedBg",   "#CCE4F7"},
        {"toggleBtnCheckedFg",   "#1E1E1E"},
        // Close panel button
        {"closeBtnFg",           "#1E1E1E"},
        {"closeBtnHoverBg",      "#E1E4E8"},
        {"closeBtnHoverFg",      "#1E1E1E"},
        // Tree widget
        {"treeBg",               "#FFFFFF"},
        {"treeFg",               "#1E1E1E"},
        {"treeHoverBg",          "#F0F0F0"},
        {"treeSelBg",            "#CCE4F7"},
        {"treeSelFg",            "#1E1E1E"},
        {"treeBranchBg",         "#FFFFFF"},
        // Status bar
        {"statusBarBg",          "#F0F0F0"},
        {"statusBarFg",          "#505050"},
        {"statusBarBorder",      "#D0D0D0"},
        // Scrollbar
        {"scrollBg",             "#F8F8F8"},
        {"scrollHandleBg",       "#CCCCCC"},
        {"scrollHandleHover",    "#BBBBBB"},
        // Combo box
        {"comboBg",              "#FFFFFF"},
        {"comboFg",              "#1E1E1E"},
        {"comboBorder",          "#D0D0D0"},
        {"comboHoverBorder",     "#007ACC"},
        {"comboDropBg",          "#FFFFFF"},
        {"comboDropSelBg",       "#CCE4F7"},
        // Spin box
        {"spinBg",               "#FFFFFF"},
        {"spinFg",               "#1E1E1E"},
        {"spinBorder",           "#D0D0D0"},
        {"spinHoverBorder",      "#007ACC"},
        // Line edit
        {"lineEditBg",           "#FFFFFF"},
        {"lineEditFg",           "#1E1E1E"},
        {"lineEditBorder",       "#1565C0"},
        // Checkbox
        {"checkboxFg",           "#1E1E1E"},
        {"checkboxBorder",       "#D0D0D0"},
        {"checkboxBg",           "#FFFFFF"},
        {"checkboxCheckedBg",    "#007ACC"},
        {"checkboxCheckedBorder","#007ACC"},
        // Push button
        {"pushBtnBg",            "#E1E4E8"},
        {"pushBtnFg",            "#1E1E1E"},
        {"pushBtnHoverBg",       "#D0D0D0"},
        {"pushBtnPressedBg",     "#C0C0C0"},
        // Group box
        {"groupBoxFg",           "#1E1E1E"},
        {"groupBoxBorder",       "#D0D0D0"},
        // Label
        {"labelFg",              "#1E1E1E"},
        // Tooltip
        {"tooltipBg",            "#F5F5F5"},
        {"tooltipFg",            "#1E1E1E"},
        {"tooltipBorder",        "#D0D0D0"},
        // Template card
        {"cardBg",               "#F0F0F0"},
        {"cardBorder",           "#D0D0D0"},
        {"cardFg",               "#333333"},
        {"cardHoverBorder",      "#007ACC"},
        {"cardHoverBg",          "#E8E8E8"},
        {"cardPressedBg",        "#D0E8FF"},
        // Blank canvas button
        {"blankBtnBorder",       "#D0D0D0"},
        {"blankBtnFg",           "#333333"},
        {"blankBtnHoverBorder",  "#007ACC"},
        {"blankBtnHoverBg",      "#F0F0F0"},
        {"blankBtnPressedBg",    "#D0E8FF"},
        // Start page
        {"subtitleFg",           "#888888"},
        // Settings hint
        {"settingsHintFg",       "gray"},
    };
}
// clang-format on

// ---------------------------------------------------------------------------
// Load the .qss template from Qt resources and substitute {{placeholders}}.
// ---------------------------------------------------------------------------
static QString buildStyleSheet(const ColorMap& colors) {
    QFile file(":/styles/theme.qss");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};

    QString css = QString::fromUtf8(file.readAll());

    for (auto it = colors.cbegin(); it != colors.cend(); ++it) {
        css.replace(QLatin1String("{{") + it.key() + QLatin1String("}}"), it.value());
    }
    return css;
}

// ---------------------------------------------------------------------------
// Public API — cached results for stable const char* pointers.
// ---------------------------------------------------------------------------
const char* StyleSheetGenerator::darkStyleSheet() {
    static QByteArray cached = buildStyleSheet(darkColors()).toUtf8();
    return cached.constData();
}

const char* StyleSheetGenerator::lightStyleSheet() {
    static QByteArray cached = buildStyleSheet(lightColors()).toUtf8();
    return cached.constData();
}
