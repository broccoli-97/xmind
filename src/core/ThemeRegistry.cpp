#include "core/ThemeRegistry.h"
#include "core/BuiltinTemplateStrings.h" // keeps lupdate picking up builtin strings
#include "ui/ThemeManager.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

ThemeRegistry& ThemeRegistry::instance() {
    static ThemeRegistry s_instance;
    return s_instance;
}

QString ThemeRegistry::defaultThemeId() {
    return QStringLiteral("builtin.default");
}

namespace {

ThemeColorScheme colorSchemeFromTheme(const ThemeColors& tc) {
    ThemeColorScheme cs;
    cs.canvasBackground = tc.canvasBackground;
    cs.canvasGridDot = tc.canvasGridDot;
    for (int i = 0; i < 6; ++i)
        cs.nodePalette[i] = tc.nodePalette[i];
    cs.nodeShadow = tc.nodeShadow;
    cs.nodeSelectionBorder = tc.nodeSelectionBorder;
    cs.nodeText = tc.nodeText;
    cs.edgeLightenFactor = tc.edgeLightenFactor;
    cs.exportBackground = tc.exportBackground;
    return cs;
}

QString translateBuiltin(const QString& s) {
    if (s.isEmpty())
        return s;
    return QCoreApplication::translate("ThemeRegistry", s.toUtf8().constData());
}

} // namespace

void ThemeRegistry::loadBuiltins() {
    const ThemeColorScheme lightDefaults = colorSchemeFromTheme(ThemeManager::lightColors());
    const ThemeColorScheme darkDefaults = colorSchemeFromTheme(ThemeManager::darkColors());

    // Order here drives the order shown in the Switch Theme picker.
    const QStringList kBuiltinPaths = {
        ":/themes/default.json",
        ":/themes/morandi.json",
        ":/themes/outlined.json",
        ":/themes/tinted.json",
    };

    for (const QString& path : kBuiltinPaths) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "ThemeRegistry: failed to open builtin" << path;
            continue;
        }

        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
        if (err.error != QJsonParseError::NoError) {
            qWarning() << "ThemeRegistry: invalid JSON in" << path << err.errorString();
            continue;
        }

        ThemeDescriptor td =
            ThemeDescriptor::fromJson(doc.object(), lightDefaults, darkDefaults);
        if (td.id.isEmpty()) {
            qWarning() << "ThemeRegistry: builtin" << path << "missing id";
            continue;
        }

        td.name = translateBuiltin(td.name);
        td.description = translateBuiltin(td.description);

        m_themes[td.id] = td;
        m_orderedIds.append(td.id);
    }
}

void ThemeRegistry::loadFromFile(const QString& filePath,
                                 const ThemeColorScheme& lightDefaults,
                                 const ThemeColorScheme& darkDefaults) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return;

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    if (err.error != QJsonParseError::NoError)
        return;

    QJsonObject obj = doc.object();
    if (obj["$schema"].toString() != QLatin1String("ymind-theme-v1"))
        return;

    ThemeDescriptor td = ThemeDescriptor::fromJson(obj, lightDefaults, darkDefaults);
    if (td.id.isEmpty())
        return;

    if (!m_themes.contains(td.id)) {
        m_themes[td.id] = td;
        m_orderedIds.append(td.id);
    }
}

void ThemeRegistry::loadFromDirectory(const QString& dirPath) {
    QDir dir(dirPath);
    if (!dir.exists())
        return;

    const ThemeColorScheme lightDefaults = colorSchemeFromTheme(ThemeManager::lightColors());
    const ThemeColorScheme darkDefaults = colorSchemeFromTheme(ThemeManager::darkColors());

    // Flat .json files at the top level.
    const auto files = dir.entryList({"*.json"}, QDir::Files);
    for (const QString& fileName : files)
        loadFromFile(dir.absoluteFilePath(fileName), lightDefaults, darkDefaults);

    // Folder-packaged themes: each subdirectory may contain a theme.json
    // alongside preview images. Drop a downloaded folder in directly.
    const auto subdirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString& sub : subdirs) {
        QDir subDir(dir.absoluteFilePath(sub));
        QString themeFile = subDir.absoluteFilePath("theme.json");
        if (QFile::exists(themeFile)) {
            loadFromFile(themeFile, lightDefaults, darkDefaults);
            continue;
        }
        const auto subFiles = subDir.entryList({"*.json"}, QDir::Files);
        for (const QString& fileName : subFiles)
            loadFromFile(subDir.absoluteFilePath(fileName), lightDefaults, darkDefaults);
    }
}

void ThemeRegistry::registerTheme(const ThemeDescriptor& td) {
    if (td.id.isEmpty())
        return;
    if (!m_themes.contains(td.id))
        m_orderedIds.append(td.id);
    m_themes[td.id] = td;
}

const ThemeDescriptor* ThemeRegistry::themeById(const QString& id) const {
    auto it = m_themes.find(id);
    if (it != m_themes.end())
        return &it.value();
    return nullptr;
}

QList<const ThemeDescriptor*> ThemeRegistry::allThemes() const {
    QList<const ThemeDescriptor*> result;
    for (const auto& id : m_orderedIds) {
        auto it = m_themes.find(id);
        if (it != m_themes.end())
            result.append(&it.value());
    }
    return result;
}
