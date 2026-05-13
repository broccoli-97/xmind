#include "core/TemplateRegistry.h"
#include "core/BuiltinTemplateStrings.h" // keeps lupdate picking up built-in strings
#include "ui/ThemeManager.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

TemplateRegistry& TemplateRegistry::instance() {
    static TemplateRegistry s_instance;
    return s_instance;
}

namespace {

// Convert ThemeColors → TemplateColorScheme for the inheritance fallback used
// when a built-in JSON omits the "colors" block.
TemplateColorScheme colorSchemeFromTheme(const ThemeColors& tc) {
    TemplateColorScheme cs;
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

// Translate built-in template strings under the "TemplateRegistry" context.
// Source strings are listed in BuiltinTemplateStrings.h so lupdate finds them.
QString translateBuiltin(const QString& s) {
    if (s.isEmpty())
        return s;
    return QCoreApplication::translate("TemplateRegistry", s.toUtf8().constData());
}

void translateContent(TemplateContentNode& n) {
    n.text = translateBuiltin(n.text);
    for (auto& child : n.children)
        translateContent(child);
}

} // namespace

void TemplateRegistry::loadBuiltins() {
    // Built-ins live as JSON files baked into the binary via Qt resources.
    // Missing colors inherit from ThemeManager so a global palette change
    // (canvas, palette, etc.) reaches every built-in automatically.
    const TemplateColorScheme lightDefaults = colorSchemeFromTheme(ThemeManager::lightColors());
    const TemplateColorScheme darkDefaults = colorSchemeFromTheme(ThemeManager::darkColors());

    // Order here drives the order shown on the Start Page.
    const QStringList kBuiltinPaths = {
        ":/templates/mindmap.json",
        ":/templates/orgchart.json",
        ":/templates/projectplan.json",
        ":/templates/lined.json",
        ":/templates/outlined.json",
        ":/templates/tinted.json",
    };

    for (const QString& path : kBuiltinPaths) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "TemplateRegistry: failed to open built-in" << path;
            continue;
        }

        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
        if (err.error != QJsonParseError::NoError) {
            qWarning() << "TemplateRegistry: invalid JSON in" << path << err.errorString();
            continue;
        }

        TemplateDescriptor td =
            TemplateDescriptor::fromJson(doc.object(), lightDefaults, darkDefaults);
        if (td.id.isEmpty()) {
            qWarning() << "TemplateRegistry: built-in" << path << "missing id";
            continue;
        }

        td.name = translateBuiltin(td.name);
        td.description = translateBuiltin(td.description);
        translateContent(td.content);

        m_templates[td.id] = td;
        m_orderedIds.append(td.id);
    }
}

void TemplateRegistry::loadFromDirectory(const QString& dirPath) {
    QDir dir(dirPath);
    if (!dir.exists())
        return;

    const TemplateColorScheme lightDefaults = colorSchemeFromTheme(ThemeManager::lightColors());
    const TemplateColorScheme darkDefaults = colorSchemeFromTheme(ThemeManager::darkColors());

    const auto files = dir.entryList({"*.json"}, QDir::Files);
    for (const QString& fileName : files) {
        QFile file(dir.absoluteFilePath(fileName));
        if (!file.open(QIODevice::ReadOnly))
            continue;

        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
        if (err.error != QJsonParseError::NoError)
            continue;

        QJsonObject obj = doc.object();
        if (obj["$schema"].toString() != "ymind-template-v1")
            continue;

        TemplateDescriptor td =
            TemplateDescriptor::fromJson(obj, lightDefaults, darkDefaults);
        if (td.id.isEmpty())
            continue;

        // Don't overwrite builtins
        if (!m_templates.contains(td.id)) {
            m_templates[td.id] = td;
            m_orderedIds.append(td.id);
        }
    }
}

void TemplateRegistry::registerTemplate(const TemplateDescriptor& td) {
    if (td.id.isEmpty())
        return;
    if (!m_templates.contains(td.id))
        m_orderedIds.append(td.id);
    m_templates[td.id] = td;
}

const TemplateDescriptor* TemplateRegistry::templateById(const QString& id) const {
    auto it = m_templates.find(id);
    if (it != m_templates.end())
        return &it.value();
    return nullptr;
}

QList<const TemplateDescriptor*> TemplateRegistry::allTemplates() const {
    QList<const TemplateDescriptor*> result;
    for (const auto& id : m_orderedIds) {
        auto it = m_templates.find(id);
        if (it != m_templates.end())
            result.append(&it.value());
    }
    return result;
}

QString TemplateRegistry::builtinIdForLayoutStyle(int layoutStyleInt) {
    switch (layoutStyleInt) {
    case 0: return QStringLiteral("builtin.mindmap");
    case 1: return QStringLiteral("builtin.orgchart");
    case 2: return QStringLiteral("builtin.projectplan");
    default: return QStringLiteral("builtin.mindmap");
    }
}
