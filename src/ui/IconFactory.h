#pragma once

#include <QIcon>
#include <QPixmap>
#include <QString>

class IconFactory {
public:
    static QIcon makeToolIcon(const QString& name);
    static QIcon makeTabIcon(int layoutStyleIndex);
    static QPixmap makeTemplatePreview(const QString& templateId, int width = 120, int height = 80);

    // Legacy overload for backward compatibility
    static QPixmap makeTemplatePreview(int index, int width = 120, int height = 80);
};
