#pragma once

#include <QIcon>
#include <QPixmap>

class IconFactory {
public:
    static QIcon makeToolIcon(const QString& name);
    static QIcon makeTabIcon(int layoutStyleIndex);
    static QPixmap makeTemplatePreview(int index, int width = 120, int height = 80);
};
