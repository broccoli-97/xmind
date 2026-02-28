#include "ThemeManager.h"
#include "AppSettings.h"
#include "MindMapScene.h"
#include "MindMapView.h"
#include "NodeItem.h"
#include "StyleSheetGenerator.h"
#include "TabManager.h"

#include <QApplication>
#include <QDir>
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QPainter>
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
    /* canvasBackground    */ QColor("#F8F9FA"),
    /* canvasGridDot       */ QColor("#D8D8D8"),
    /* nodePalette         */
    {QColor("#1565C0"), QColor("#2E7D32"), QColor("#E65100"), QColor("#6A1B9A"), QColor("#C62828"),
     QColor("#00838F")},
    /* nodeShadow          */ QColor(0, 0, 0, 30),
    /* nodeSelectionBorder */ QColor("#FF6F00"),
    /* nodeText            */ QColor("#FFFFFF"),
    /* edgeLightenFactor   */ 140,
    /* lockIconBackground  */ QColor(255, 255, 255, 220),
    /* lockIconLocked      */ QColor("#FF9800"),
    /* lockIconUnlocked    */ QColor("#9E9E9E"),
    /* lockIconKeyhole     */ QColor("#FFFFFF"),
    /* exportBackground    */ QColor("#FFFFFF"),
    /* editorBackground    */ QColor("white"),
    /* editorBorder        */ QColor("#1565C0"),
    /* editorText          */ QColor("#333333"),
    /* iconBaseColor       */ QColor("#3b3838"),
    /* previewBackground   */ QColor("#F0F2F5"),
    /* previewLine         */ QColor("#B0B0B0"),
    /* previewNodeBorder   */ QColor("#1565C0"),
    /* previewNodeFill     */ QColor("#DBEAF8"),
    /* previewText         */ QColor("#666666"),
    /* closeIconColor      */ QColor("#5A5A5A"),
};

static const ThemeColors kDarkColors = {
    /* canvasBackground    */ QColor("#1A1A2E"),
    /* canvasGridDot       */ QColor("#2A2A4A"),
    /* nodePalette         */
    {QColor("#42A5F5"), QColor("#66BB6A"), QColor("#FFA726"), QColor("#AB47BC"), QColor("#EF5350"),
     QColor("#26C6DA")},
    /* nodeShadow          */ QColor(0, 0, 0, 50),
    /* nodeSelectionBorder */ QColor("#FFB300"),
    /* nodeText            */ QColor("#FFFFFF"),
    /* edgeLightenFactor   */ 120,
    /* lockIconBackground  */ QColor(30, 30, 46, 220),
    /* lockIconLocked      */ QColor("#FFB300"),
    /* lockIconUnlocked    */ QColor("#757575"),
    /* lockIconKeyhole     */ QColor("#1A1A2E"),
    /* exportBackground    */ QColor("#1A1A2E"),
    /* editorBackground    */ QColor("#2A2A4A"),
    /* editorBorder        */ QColor("#42A5F5"),
    /* editorText          */ QColor("#E0E0E0"),
    /* iconBaseColor       */ QColor("#FFFFFF"),
    /* previewBackground   */ QColor("#1E1E1E"),
    /* previewLine         */ QColor("#555555"),
    /* previewNodeBorder   */ QColor("#007ACC"),
    /* previewNodeFill     */ QColor("#094771"),
    /* previewText         */ QColor("#888888"),
    /* closeIconColor      */ QColor("#CCCCCC"),
};

bool ThemeManager::isDark() {
    return AppSettings::instance().theme() == AppTheme::Dark;
}

const ThemeColors& ThemeManager::colors() {
    return isDark() ? kDarkColors : kLightColors;
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
    QColor color = dark ? kDarkColors.closeIconColor : kLightColors.closeIconColor;
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

    QString stylesheet =
        dark ? QString(StyleSheetGenerator::darkStyleSheet())
             : QString(StyleSheetGenerator::lightStyleSheet());

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
// WCAG contrast utilities
// ---------------------------------------------------------------------------
static double getRelativeLuminance(const QColor& color) {
    double r = color.redF();
    double g = color.greenF();
    double b = color.blueF();

    auto linearize = [](double c) {
        return c <= 0.03928 ? c / 12.92 : std::pow((c + 0.055) / 1.055, 2.4);
    };

    r = linearize(r);
    g = linearize(g);
    b = linearize(b);

    return 0.2126 * r + 0.7152 * g + 0.0722 * b;
}

double ThemeManager::colorContrast(const QColor& color1, const QColor& color2) {
    double l1 = getRelativeLuminance(color1);
    double l2 = getRelativeLuminance(color2);

    double lighter = (l1 > l2) ? l1 : l2;
    double darker = (l1 > l2) ? l2 : l1;

    return (lighter + 0.05) / (darker + 0.05);
}

// ---------------------------------------------------------------------------
// System theme detection
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
            return value == 0;
        }
    } catch (...) {
    }
#endif
    return false;
}

void ThemeManager::setupSystemThemeMonitoring() {
    // This can be expanded to monitor Windows theme changes
}
