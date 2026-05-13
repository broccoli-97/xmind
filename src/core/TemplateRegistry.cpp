#include "core/TemplateRegistry.h"
#include "core/BuiltinTemplateStrings.h" // keeps lupdate picking up built-in strings

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
    // Each built-in is a layout + starter content. Visual styling is a
    // separate concern handled by ThemeRegistry.
    const QStringList kBuiltinPaths = {
        ":/templates/mindmap.json",
        ":/templates/orgchart.json",
        ":/templates/projectplan.json",
        ":/templates/lined.json",
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

        TemplateDescriptor td = TemplateDescriptor::fromJson(doc.object());
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

void TemplateRegistry::loadFromFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return;

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    if (err.error != QJsonParseError::NoError)
        return;

    QJsonObject obj = doc.object();
    if (obj["$schema"].toString() != QLatin1String("ymind-template-v1"))
        return;

    TemplateDescriptor td = TemplateDescriptor::fromJson(obj);
    if (td.id.isEmpty())
        return;

    if (!m_templates.contains(td.id)) {
        m_templates[td.id] = td;
        m_orderedIds.append(td.id);
    }
}

void TemplateRegistry::loadFromDirectory(const QString& dirPath) {
    QDir dir(dirPath);
    if (!dir.exists())
        return;

    const auto files = dir.entryList({"*.json"}, QDir::Files);
    for (const QString& fileName : files)
        loadFromFile(dir.absoluteFilePath(fileName));

    // Folder-packaged templates: each subdir may contain a template.json.
    const auto subdirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString& sub : subdirs) {
        QDir subDir(dir.absoluteFilePath(sub));
        QString tFile = subDir.absoluteFilePath("template.json");
        if (QFile::exists(tFile)) {
            loadFromFile(tFile);
            continue;
        }
        const auto subFiles = subDir.entryList({"*.json"}, QDir::Files);
        for (const QString& fileName : subFiles)
            loadFromFile(subDir.absoluteFilePath(fileName));
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
