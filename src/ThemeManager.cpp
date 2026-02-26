#include "ThemeManager.h"
#include "AppSettings.h"
#include "MindMapScene.h"
#include "MindMapView.h"
#include "NodeItem.h"
#include "TabManager.h"

#include <QApplication>
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QPainter>
#include <QTimer>

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
    QPen pen(QColor("#D4D4D4"), 2.0);
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
        QPen thickPen(QColor("#D4D4D4"), 3.0);
        p.setPen(thickPen);
        p.drawLine(21, 20, 27, 27);
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
// Apply theme to application and invalidate caches on all scenes
// ---------------------------------------------------------------------------
void ThemeManager::applyTheme(const QList<TabState>& tabs) {
    bool dark = (AppSettings::instance().theme() == AppTheme::Dark);
    if (dark) {
        qApp->setStyleSheet(kDarkStyleSheet);
    } else {
        qApp->setStyleSheet(kLightStyleSheet);
    }

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
