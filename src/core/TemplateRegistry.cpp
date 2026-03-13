#include "core/TemplateRegistry.h"

#include <QColor>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

TemplateRegistry& TemplateRegistry::instance() {
    static TemplateRegistry s_instance;
    return s_instance;
}

// ---------------------------------------------------------------------------
// Helper: build a TemplateColorScheme from raw values (matches ThemeManager)
// ---------------------------------------------------------------------------
static TemplateColorScheme makeColorScheme(
    const QColor& canvasBg, const QColor& gridDot,
    const QColor (&palette)[6],
    const QColor& shadow, const QColor& selBorder, const QColor& text,
    int edgeLighten, const QColor& exportBg)
{
    TemplateColorScheme cs;
    cs.canvasBackground = canvasBg;
    cs.canvasGridDot = gridDot;
    for (int i = 0; i < 6; ++i) cs.nodePalette[i] = palette[i];
    cs.nodeShadow = shadow;
    cs.nodeSelectionBorder = selBorder;
    cs.nodeText = text;
    cs.edgeLightenFactor = edgeLighten;
    cs.exportBackground = exportBg;
    return cs;
}

void TemplateRegistry::loadBuiltins() {
    // ----- Light / Dark palettes (copied from ThemeManager.cpp) -----
    const QColor lightPalette[6] = {
        QColor("#1565C0"), QColor("#2E7D32"), QColor("#E65100"),
        QColor("#6A1B9A"), QColor("#C62828"), QColor("#00838F")};
    const QColor darkPalette[6] = {
        QColor("#42A5F5"), QColor("#66BB6A"), QColor("#FFA726"),
        QColor("#AB47BC"), QColor("#EF5350"), QColor("#26C6DA")};

    auto lightCS = makeColorScheme(
        QColor("#F8F9FA"), QColor("#D8D8D8"), lightPalette,
        QColor(0,0,0,30), QColor("#FF6F00"), QColor("#FFFFFF"),
        140, QColor("#FFFFFF"));

    auto darkCS = makeColorScheme(
        QColor("#1A1A2E"), QColor("#2A2A4A"), darkPalette,
        QColor(0,0,0,50), QColor("#FFB300"), QColor("#FFFFFF"),
        120, QColor("#1A1A2E"));

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
