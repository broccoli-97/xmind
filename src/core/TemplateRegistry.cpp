#include "core/TemplateRegistry.h"
#include "ui/ThemeManager.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

TemplateRegistry& TemplateRegistry::instance() {
    static TemplateRegistry s_instance;
    return s_instance;
}

// ---------------------------------------------------------------------------
// Helper: convert ThemeColors to TemplateColorScheme (shared fields only)
// ---------------------------------------------------------------------------
static TemplateColorScheme colorSchemeFromTheme(const ThemeColors& tc) {
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

void TemplateRegistry::loadBuiltins() {
    // Derive color schemes from the centralized ThemeManager definitions
    // instead of duplicating color values here.
    auto lightCS = colorSchemeFromTheme(ThemeManager::lightColors());
    auto darkCS = colorSchemeFromTheme(ThemeManager::darkColors());

    // ---- Template 0: Mind Map (Bilateral) ----
    {
        TemplateDescriptor td;
        td.id = "builtin.mindmap";
        td.name = tr("Mind Map");
        td.description = tr("Central topic with bilateral branches");
        td.layout = {"bilateral", 100.0, 16.0};
        td.lightColors = lightCS;
        td.darkColors = darkCS;
        td.content.text = tr("Central Topic");
        td.content.children = {{tr("Branch 1"), {}}, {tr("Branch 2"), {}},
                                {tr("Branch 3"), {}}, {tr("Branch 4"), {}}};
        m_templates[td.id] = td;
        m_orderedIds.append(td.id);
    }

    // ---- Template 1: Org Chart (TopDown) ----
    {
        TemplateDescriptor td;
        td.id = "builtin.orgchart";
        td.name = tr("Org Chart");
        td.description = tr("Top-down organizational chart");
        td.layout = {"topdown", 100.0, 16.0};
        td.lightColors = lightCS;
        td.darkColors = darkCS;
        td.content.text = tr("CEO");
        td.content.children = {{tr("Engineering"), {}}, {tr("Marketing"), {}}, {tr("Sales"), {}}};
        m_templates[td.id] = td;
        m_orderedIds.append(td.id);
    }

    // ---- Template 2: Project Plan (RightTree) ----
    {
        TemplateDescriptor td;
        td.id = "builtin.projectplan";
        td.name = tr("Project Plan");
        td.description = tr("Right-tree project plan with phases and tasks");
        td.layout = {"righttree", 100.0, 16.0};
        td.lightColors = lightCS;
        td.darkColors = darkCS;
        td.content.text = tr("Project");
        td.content.children = {
            {tr("Phase 1"), {{tr("Task 1.1"), {}}, {tr("Task 1.2"), {}}}},
            {tr("Phase 2"), {{tr("Task 2.1"), {}}, {tr("Task 2.2"), {}}}}
        };
        m_templates[td.id] = td;
        m_orderedIds.append(td.id);
    }
}

void TemplateRegistry::loadFromDirectory(const QString& dirPath) {
    QDir dir(dirPath);
    if (!dir.exists())
        return;

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

        TemplateDescriptor td = TemplateDescriptor::fromJson(obj);
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
