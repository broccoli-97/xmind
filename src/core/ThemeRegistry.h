#pragma once

#include "core/ThemeDescriptor.h"

#include <QCoreApplication>
#include <QList>
#include <QMap>
#include <QString>

class ThemeRegistry {
    Q_DECLARE_TR_FUNCTIONS(ThemeRegistry)
public:
    static ThemeRegistry& instance();

    void loadBuiltins();
    void loadFromDirectory(const QString& dirPath);

    void registerTheme(const ThemeDescriptor& td);

    const ThemeDescriptor* themeById(const QString& id) const;
    QList<const ThemeDescriptor*> allThemes() const;

    // ID of the always-present default theme that ships with the app.
    static QString defaultThemeId();

private:
    void loadFromFile(const QString& filePath,
                      const ThemeColorScheme& lightDefaults,
                      const ThemeColorScheme& darkDefaults);

    ThemeRegistry() = default;
    QMap<QString, ThemeDescriptor> m_themes;
    QList<QString> m_orderedIds; // insertion order
};
