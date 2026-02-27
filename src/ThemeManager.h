#pragma once

#include <QColor>
#include <QIcon>
#include <QPixmap>

struct TabState; // forward declare

struct ThemeColors {
    // Canvas
    QColor canvasBackground;
    QColor canvasGridDot;

    // Node palette (6 levels, wraps around)
    QColor nodePalette[6];

    // Edge
    int edgeLightenFactor;

    // Inline editor
    QColor editorBackground;
    QColor editorBorder;
    QColor editorText;

    // Icons
    QColor iconBaseColor;
};

class ThemeManager {
public:
    static const char* darkStyleSheet();
    static const char* lightStyleSheet();
    static QIcon makeToolIcon(const QString& name);
    static QPixmap makeTemplatePreview(int index, int width = 120, int height = 80);
    static void applyTheme(const QList<TabState>& tabs);

    // Centralized color access
    static bool isDark();
    static const ThemeColors& colors();

    // System theme detection and contrast checking
    static bool isSystemDarkMode();
    static double colorContrast(const QColor& color1, const QColor& color2);
    static void setupSystemThemeMonitoring();
};
