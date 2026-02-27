#include "ThemeManager.h"
#include "AppSettings.h"
#include "MindMapScene.h"
#include "MindMapView.h"
#include "NodeItem.h"
#include "TabManager.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QPainter>
#include <QPalette>
#include <QSettings>
#include <QStandardPaths>
#include <QTimer>
#include <cmath>
#ifdef _WIN32
#include <windows.h>
#endif

// ---------------------------------------------------------------------------
// Centralized color definitions
// ---------------------------------------------------------------------------
static const ThemeColors kLightColors = {
    /* canvasBackground */ QColor("#F8F9FA"),
    /* canvasGridDot    */ QColor("#D8D8D8"),
    /* nodePalette      */
    {QColor("#1565C0"), QColor("#2E7D32"), QColor("#E65100"), QColor("#6A1B9A"), QColor("#C62828"),
     QColor("#00838F")},
    /* edgeLightenFactor */ 140,
    /* editorBackground */ QColor("white"),
    /* editorBorder     */ QColor("#1565C0"),
    /* editorText       */ QColor("#333333"),
    /* iconBaseColor    */ QColor("#3b3838"),
};

static const ThemeColors kDarkColors = {
    /* canvasBackground */ QColor("#1A1A2E"),
    /* canvasGridDot    */ QColor("#2A2A4A"),
    /* nodePalette      */
    {QColor("#42A5F5"), QColor("#66BB6A"), QColor("#FFA726"), QColor("#AB47BC"), QColor("#EF5350"),
     QColor("#26C6DA")},
    /* edgeLightenFactor */ 120,
    /* editorBackground */ QColor("#2A2A4A"),
    /* editorBorder     */ QColor("#42A5F5"),
    /* editorText       */ QColor("#E0E0E0"),
    /* iconBaseColor    */ QColor("#FFFFFF"),
};

bool ThemeManager::isDark() {
    return AppSettings::instance().theme() == AppTheme::Dark;
}

const ThemeColors& ThemeManager::colors() {
    return isDark() ? kDarkColors : kLightColors;
}

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
        border-right: 1px solid #3F3F46;
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
    QLineEdit {
        background-color: #2A2A4A;
        color: #D4D4D4;
        border: 2px solid #42A5F5;
        border-radius: 6px;
        padding: 4px 8px;
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
    QLabel {
        color: #D4D4D4;
    }
    QGroupBox QCheckBox {
        color: #D4D4D4;
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
        border-radius: 8px;
        padding: 8px;
        font-size: 13px;
        color: #D4D4D4;
        text-align: bottom;
    }
    QPushButton#templateCard:hover {
        border-color: #007ACC;
        background-color: #333337;
    }
    QPushButton#templateCard:pressed {
        background-color: #094771;
    }
    QPushButton#blankCanvasBtn {
        background-color: transparent;
        border: 1px solid #3F3F46;
        border-radius: 6px;
        font-size: 13px;
        color: #D4D4D4;
    }
    QPushButton#blankCanvasBtn:hover {
        border-color: #007ACC;
        background-color: #2D2D30;
    }
    QPushButton#blankCanvasBtn:pressed {
        background-color: #094771;
    }
    QLabel#startPageTitle {
        font-size: 24px;
        font-weight: bold;
        margin-bottom: 4px;
        background: transparent;
        border: none;
    }
    QLabel#startPageSubtitle {
        font-size: 14px;
        color: #999999;
        margin-bottom: 24px;
        background: transparent;
        border: none;
    }
    QLabel#settingsHint {
        color: #888888;
        font-size: 9pt;
    }
)";

// ---------------------------------------------------------------------------
// Light stylesheet
// ---------------------------------------------------------------------------
static const char* kLightStyleSheet = R"(
    QMainWindow, QDialog {
        background-color: #FFFFFF;
        color: #1E1E1E;
    }
    QMenuBar {
        background-color: #F8F8F8;
        color: #1E1E1E;
        border-bottom: 1px solid #D0D0D0;
    }
    QMenuBar::item:selected {
        background-color: #E1E4E8;
    }
    QMenu {
        background-color: #FFFFFF;
        color: #1E1E1E;
        border: 1px solid #D0D0D0;
    }
    QMenu::item:selected {
        background-color: #E1E4E8;
    }
    QMenu::separator {
        height: 1px;
        background: #D0D0D0;
        margin: 4px 8px;
    }
    QToolBar {
        background-color: #F8F8F8;
        border-bottom: 1px solid #D0D0D0;
        spacing: 2px;
        padding: 4px;
    }
    QToolButton {
        background-color: transparent;
        color: #1E1E1E;
        border: 1px solid transparent;
        border-radius: 4px;
        padding: 4px 8px;
        font-size: 11px;
    }
    QToolButton:hover {
        background-color: #E1E4E8;
        border-color: #E1E4E8;
    }
    QToolButton:pressed {
        background-color: #D0D0D0;
    }
    QTabBar {
        background-color: transparent;
    }
    QTabBar::tab {
        background-color: #E0E0E0;
        color: #1E1E1E;
        border: none;
        border-right: 1px solid #D0D0D0;
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
        background-color: #F0F0F0;
        color: #1E1E1E;
    }
    QTabBar::close-button {
        subcontrol-position: right;
        border: none;
        padding: 2px;
        margin: 2px;
        background: transparent;
        width: 14px;
        height: 14px;
    }
    QTabBar::close-button:hover {
        background-color: #D0D0D0;
        border-radius: 3px;
    }
    QToolButton#newTabBtn {
        background-color: transparent;
        color: #1E1E1E;
        border: none;
        border-radius: 4px;
        font-size: 16px;
        font-weight: bold;
    }
    QToolButton#newTabBtn:hover {
        background-color: #E1E4E8;
        color: #1E1E1E;
    }
    QWidget#inlineToolbar {
        background-color: #F8F8F8;
        border-bottom: 1px solid #D0D0D0;
    }
    QWidget#inlineToolbar QToolButton {
        background-color: transparent;
        color: #1E1E1E;
        border: 1px solid transparent;
        border-radius: 4px;
        padding: 4px 8px;
        font-size: 11px;
    }
    QWidget#inlineToolbar QToolButton:hover {
        background-color: #E1E4E8;
        border-color: #E1E4E8;
    }
    QWidget#inlineToolbar QToolButton:pressed {
        background-color: #B8D4F0;
    }
    QLabel#sectionHeader, QWidget#sectionHeader {
        background-color: #F0F0F0;
        color: #1E1E1E;
        font-weight: bold;
        font-size: 12px;
        padding: 6px 10px;
        border-bottom: 1px solid #D0D0D0;
    }
    QToolButton#togglePanelBtn {
        background-color: transparent;
        color: #1E1E1E;
        border: none;
        border-radius: 4px;
        padding: 4px;
    }
    QToolButton#togglePanelBtn:hover {
        background-color: #E1E4E8;
        color: #1E1E1E;
    }
    QToolButton#togglePanelBtn:checked {
        background-color: #CCE4F7;
        color: #1E1E1E;
    }
    QToolButton#closePanelBtn {
        background-color: transparent;
        color: #1E1E1E;
        border: none;
        border-radius: 3px;
        padding: 2px;
    }
    QToolButton#closePanelBtn:hover {
        background-color: #E1E4E8;
        color: #1E1E1E;
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
        background-color: #F0F0F0;
    }
    QTreeWidget::item:selected {
        background-color: #CCE4F7;
        color: #1E1E1E;
    }
    QTreeWidget::branch {
        background-color: #FFFFFF;
    }
    QScrollBar:vertical {
        background-color: #F8F8F8;
        width: 12px;
        margin: 0;
    }
    QScrollBar::handle:vertical {
        background-color: #CCCCCC;
        min-height: 20px;
        border-radius: 4px;
        margin: 2px;
    }
    QScrollBar::handle:vertical:hover {
        background-color: #BBBBBB;
    }
    QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
        height: 0;
    }
    QScrollBar:horizontal {
        background-color: #F8F8F8;
        height: 12px;
        margin: 0;
    }
    QScrollBar::handle:horizontal {
        background-color: #CCCCCC;
        min-width: 20px;
        border-radius: 4px;
        margin: 2px;
    }
    QScrollBar::handle:horizontal:hover {
        background-color: #BBBBBB;
    }
    QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
        width: 0;
    }
    QLabel#startPageTitle {
        font-size: 24px;
        font-weight: bold;
        margin-bottom: 4px;
        background: transparent;
        border: none;
    }
    QLabel#startPageSubtitle {
        font-size: 14px;
        color: #888888;
        margin-bottom: 24px;
        background: transparent;
        border: none;
    }
    QPushButton#templateCard {
        background-color: #F0F0F0;
        border: 2px solid #D0D0D0;
        border-radius: 8px;
        padding: 8px;
        font-size: 13px;
        color: #333333;
        text-align: bottom;
    }
    QPushButton#templateCard:hover {
        border-color: #007ACC;
        background-color: #E8E8E8;
    }
    QPushButton#templateCard:pressed {
        background-color: #D0E8FF;
    }
    QPushButton#blankCanvasBtn {
        background-color: transparent;
        border: 1px solid #D0D0D0;
        border-radius: 6px;
        font-size: 13px;
        color: #333333;
    }
    QPushButton#blankCanvasBtn:hover {
        border-color: #007ACC;
        background-color: #F0F0F0;
    }
    QPushButton#blankCanvasBtn:pressed {
        background-color: #D0E8FF;
    }
    QGroupBox {
        color: #1E1E1E;
        border: 1px solid #D0D0D0;
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
    QLabel {
        color: #1E1E1E;
    }
    QGroupBox QCheckBox {
        color: #1E1E1E;
    }
    QLineEdit {
        background-color: #FFFFFF;
        color: #1E1E1E;
        border: 2px solid #1565C0;
        border-radius: 6px;
        padding: 4px 8px;
    }
    QLabel#settingsHint {
        color: gray;
        font-size: 9pt;
    }
)";

// ---------------------------------------------------------------------------
// Stylesheet accessors
// ---------------------------------------------------------------------------
const char* ThemeManager::darkStyleSheet() {
    return kDarkStyleSheet;
}

const char* ThemeManager::lightStyleSheet() {
    return kLightStyleSheet;
}

// ---------------------------------------------------------------------------
// Icon factory
// ---------------------------------------------------------------------------
QIcon ThemeManager::makeToolIcon(const QString& name) {
    QPixmap pix(32, 32);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);

    // choose base color depending on current theme
    QColor baseColor = colors().iconBaseColor;
    QPen pen(baseColor, 2.0);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);

    if (name == "add-child") {
        p.drawLine(16, 8, 16, 24);
        p.drawLine(8, 16, 24, 16);
    } else if (name == "add-sibling") {
        p.drawLine(4, 10, 14, 10);
        p.drawLine(4, 22, 14, 22);
        p.drawLine(24, 14, 24, 22);
        p.drawLine(20, 18, 28, 18);
    } else if (name == "delete") {
        p.drawLine(10, 10, 10, 26);
        p.drawLine(22, 10, 22, 26);
        p.drawLine(10, 26, 22, 26);
        p.drawLine(8, 10, 24, 10);
        p.drawLine(13, 6, 19, 6);
        p.drawLine(13, 6, 13, 10);
        p.drawLine(19, 6, 19, 10);
        p.drawLine(14, 13, 14, 23);
        p.drawLine(18, 13, 18, 23);
    } else if (name == "auto-layout") {
        p.drawRect(12, 2, 8, 6);
        p.drawRect(2, 22, 8, 6);
        p.drawRect(22, 22, 8, 6);
        p.drawLine(16, 8, 16, 14);
        p.drawLine(6, 14, 26, 14);
        p.drawLine(6, 14, 6, 22);
        p.drawLine(26, 14, 26, 22);
    } else if (name == "zoom") {
        p.drawEllipse(8, 6, 16, 16);
        QPen thickPen(colors().iconBaseColor, 3.0);
        p.setPen(thickPen);
        p.drawLine(21, 20, 27, 27);
    } else if (name == "zoom-in") {
        p.drawEllipse(6, 4, 18, 18);
        p.drawLine(15, 9, 15, 17);
        p.drawLine(11, 13, 19, 13);
        QPen thickPen(colors().iconBaseColor, 3.0);
        p.setPen(thickPen);
        p.drawLine(22, 21, 28, 27);
    } else if (name == "zoom-out") {
        p.drawEllipse(6, 4, 18, 18);
        p.drawLine(11, 13, 19, 13);
        QPen thickPen(colors().iconBaseColor, 3.0);
        p.setPen(thickPen);
        p.drawLine(22, 21, 28, 27);
    } else if (name == "undo") {
        p.drawPath(QPainterPath());
        p.setPen(pen);
        // Curved arrow pointing left
        p.drawArc(8, 8, 14, 14, 0, 180 * 16);
        p.drawLine(8, 15, 4, 11);
        p.drawLine(8, 15, 6, 20);
    } else if (name == "redo") {
        p.drawPath(QPainterPath());
        p.setPen(pen);
        // Curved arrow pointing right
        p.drawArc(10, 8, 14, 14, 180 * 16, 180 * 16);
        p.drawLine(24, 15, 28, 11);
        p.drawLine(24, 15, 26, 20);
    } else if (name == "fit-view") {
        p.drawLine(4, 10, 4, 4);
        p.drawLine(4, 4, 10, 4);
        p.drawLine(22, 4, 28, 4);
        p.drawLine(28, 4, 28, 10);
        p.drawLine(4, 22, 4, 28);
        p.drawLine(4, 28, 10, 28);
        p.drawLine(28, 22, 28, 28);
        p.drawLine(22, 28, 28, 28);
    } else if (name == "export") {
        p.drawRect(8, 14, 16, 14);
        p.drawLine(16, 16, 16, 4);
        p.drawLine(12, 8, 16, 4);
        p.drawLine(20, 8, 16, 4);
    } else if (name == "sidebar") {
        p.drawRect(4, 4, 24, 24);
        p.drawLine(14, 4, 14, 28);
        p.drawLine(17, 10, 25, 10);
        p.drawLine(17, 16, 25, 16);
        p.drawLine(17, 22, 25, 22);
    } else if (name == "toolbar") {
        p.drawRect(4, 10, 24, 12);
        p.drawLine(10, 10, 10, 22);
        p.drawLine(16, 10, 16, 22);
        p.drawLine(22, 10, 22, 22);
    } else if (name == "close-panel") {
        p.drawLine(10, 10, 22, 22);
        p.drawLine(22, 10, 10, 22);
    }

    p.end();
    return QIcon(pix);
}

// ---------------------------------------------------------------------------
// Template preview factory
// ---------------------------------------------------------------------------
QPixmap ThemeManager::makeTemplatePreview(int index, int width, int height) {
    QPixmap pix(width, height);
    pix.fill(QColor("#1E1E1E"));
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);

    qreal sx = width / 120.0;
    qreal sy = height / 80.0;
    p.scale(sx, sy);

    QPen linePen(QColor("#555555"), 1.5);
    QPen nodePen(QColor("#007ACC"), 1.5);
    QBrush nodeBrush(QColor("#094771"));

    if (index == 0) {
        p.setPen(nodePen);
        p.setBrush(nodeBrush);
        p.drawRoundedRect(42, 30, 36, 18, 4, 4);
        p.setPen(linePen);
        p.drawLine(60, 30, 90, 12);
        p.drawLine(60, 48, 90, 66);
        p.drawLine(42, 30, 18, 14);
        p.drawLine(42, 48, 18, 66);
        p.setPen(nodePen);
        p.drawRoundedRect(84, 6, 28, 12, 3, 3);
        p.drawRoundedRect(84, 60, 28, 12, 3, 3);
        p.drawRoundedRect(4, 8, 28, 12, 3, 3);
        p.drawRoundedRect(4, 60, 28, 12, 3, 3);
    } else if (index == 1) {
        p.setPen(nodePen);
        p.setBrush(nodeBrush);
        p.drawRoundedRect(42, 6, 36, 14, 3, 3);
        p.drawRoundedRect(8, 50, 28, 14, 3, 3);
        p.drawRoundedRect(46, 50, 28, 14, 3, 3);
        p.drawRoundedRect(84, 50, 28, 14, 3, 3);
        p.setPen(linePen);
        p.drawLine(60, 20, 60, 32);
        p.drawLine(22, 32, 98, 32);
        p.drawLine(22, 32, 22, 50);
        p.drawLine(60, 32, 60, 50);
        p.drawLine(98, 32, 98, 50);
    } else if (index == 2) {
        p.setPen(nodePen);
        p.setBrush(nodeBrush);
        p.drawRoundedRect(4, 30, 28, 14, 3, 3);
        p.drawRoundedRect(44, 10, 28, 12, 3, 3);
        p.drawRoundedRect(44, 52, 28, 12, 3, 3);
        p.drawRoundedRect(84, 4, 28, 10, 2, 2);
        p.drawRoundedRect(84, 20, 28, 10, 2, 2);
        p.drawRoundedRect(84, 46, 28, 10, 2, 2);
        p.drawRoundedRect(84, 62, 28, 10, 2, 2);
        p.setPen(linePen);
        p.drawLine(32, 37, 44, 16);
        p.drawLine(32, 37, 44, 58);
        p.drawLine(72, 16, 84, 9);
        p.drawLine(72, 16, 84, 25);
        p.drawLine(72, 58, 84, 51);
        p.drawLine(72, 58, 84, 67);
    }

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
// Generate a close-button icon for the tab bar and return its file path
// ---------------------------------------------------------------------------
static QString generateCloseIcon(bool dark) {
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QDir().mkpath(tempDir);
    QString path = tempDir + (dark ? "/xmind-tab-close-dark.png" : "/xmind-tab-close-light.png");

    const int sz = 16;
    QPixmap pix(sz, sz);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    QColor color = dark ? QColor("#CCCCCC") : QColor("#5A5A5A");
    QPen pen(color, 1.5);
    pen.setCapStyle(Qt::RoundCap);
    p.setPen(pen);
    p.drawLine(4, 4, sz - 5, sz - 5);
    p.drawLine(sz - 5, 4, 4, sz - 5);
    p.end();

    pix.save(path, "PNG");
    return path;
}

// ---------------------------------------------------------------------------
// Apply theme to application and invalidate caches on all scenes
// ---------------------------------------------------------------------------
void ThemeManager::applyTheme(const QList<TabState>& tabs) {
    bool dark = (AppSettings::instance().theme() == AppTheme::Dark);

    QString stylesheet = dark ? QString(kDarkStyleSheet) : QString(kLightStyleSheet);

    // Generate a close-button icon matching the theme and inject into stylesheet
    QString iconPath = generateCloseIcon(dark);
    iconPath.replace("\\", "/"); // Qt URL paths require forward slashes
    stylesheet += QString("\nQTabBar::close-button { image: url(%1); }").arg(iconPath);

    qApp->setStyleSheet(stylesheet);

    // Invalidate caches on ALL open scenes
    for (const auto& tab : tabs) {
        const auto items = tab.scene->items();
        for (auto* item : items) {
            item->setCacheMode(QGraphicsItem::NoCache);
        }
        tab.view->viewport()->update();
    }

    // Re-enable cache after repaint
    QTimer::singleShot(0, qApp, [tabs]() {
        for (const auto& tab : tabs) {
            const auto items = tab.scene->items();
            for (auto* item : items) {
                if (dynamic_cast<NodeItem*>(item))
                    item->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
            }
        }
    });
}

// ---------------------------------------------------------------------------
// Calculate the relative luminance of a color (WCAG formula)
// ---------------------------------------------------------------------------
static double getRelativeLuminance(const QColor& color) {
    double r = color.redF();
    double g = color.greenF();
    double b = color.blueF();

    // Apply gamma correction
    auto linearize = [](double c) {
        return c <= 0.03928 ? c / 12.92 : std::pow((c + 0.055) / 1.055, 2.4);
    };

    r = linearize(r);
    g = linearize(g);
    b = linearize(b);

    return 0.2126 * r + 0.7152 * g + 0.0722 * b;
}

// ---------------------------------------------------------------------------
// Calculate contrast ratio between two colors (WCAG formula)
// Range: 1.0 (no contrast) to 21.0 (maximum contrast)
// Minimum 4.5:1 recommended for normal text, 7:1 for AAA compliance
// ---------------------------------------------------------------------------
double ThemeManager::colorContrast(const QColor& color1, const QColor& color2) {
    double l1 = getRelativeLuminance(color1);
    double l2 = getRelativeLuminance(color2);

    double lighter = (l1 > l2) ? l1 : l2;
    double darker = (l1 > l2) ? l2 : l1;

    return (lighter + 0.05) / (darker + 0.05);
}

// ---------------------------------------------------------------------------
// Detect if Windows is in dark mode
// ---------------------------------------------------------------------------
bool ThemeManager::isSystemDarkMode() {
#if _WIN32
    try {
        DWORD value = 0;
        DWORD size = sizeof(value);
        LONG result = RegGetValueA(
            HKEY_CURRENT_USER, R"(Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)",
            "AppsUseLightTheme", RRF_RT_REG_DWORD, nullptr, &value, &size);

        if (result == ERROR_SUCCESS) {
            // AppsUseLightTheme: 1 = light mode, 0 = dark mode
            return value == 0;
        }
    } catch (...) {
    }
#endif
    return false;
}

// ---------------------------------------------------------------------------
// Setup monitoring for system theme changes (future enhancement)
// Currently called from MainWindow constructor
// ---------------------------------------------------------------------------
void ThemeManager::setupSystemThemeMonitoring() {
    // This can be expanded to monitor Windows theme changes
    // and automatically apply the corresponding app theme
    // For now, users can manually switch themes in Settings
}
