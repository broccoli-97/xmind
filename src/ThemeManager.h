#pragma once

#include <QIcon>
#include <QPixmap>

struct TabState; // forward declare

class ThemeManager {
public:
    static const char* darkStyleSheet();
    static const char* lightStyleSheet();
    static QIcon makeToolIcon(const QString& name);
    static QPixmap makeTemplatePreview(int index, int width = 120, int height = 80);
    static void applyTheme(const QList<TabState>& tabs);
};
