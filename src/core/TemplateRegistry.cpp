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

    // ---- Template 3: Lined (text floats above curved colored lines) ----
    // Demonstrates the data-driven shape + per-branch coloring + baseline edge
    // anchor. Each top-level branch gets its own color; descendants inherit.
    {
        TemplateDescriptor td;
        td.id = "builtin.lined";
        td.name = tr("Lined");
        td.description = tr("Curved colored lines, text floats above the line");
        td.layout = {"bilateral", 120.0, 22.0};

        // Node style: every node carries an underline so text appears to float
        // on a continuous line that flows from the parent's baseline-anchored edge.
        td.nodeStyle.shape = "underline";
        td.nodeStyle.rootShape = "underline";
        td.nodeStyle.drawShadow = false;
        td.nodeStyle.padding = 6.0;
        td.nodeStyle.minWidth = 60.0;
        td.nodeStyle.maxWidth = 240.0;
        td.nodeStyle.paletteSource = "branch";

        // Edge style: branch colors, line meets node baseline.
        td.edgeStyle.width = 2.5;
        td.edgeStyle.colorSource = "branch";
        td.edgeStyle.anchor = "baseline";

        // Light palette — the figure's saturated branch hues; dark text on
        // a near-white canvas (no fill behind text means nodeText must be
        // legible on the canvas itself).
        TemplateColorScheme lined;
        lined.canvasBackground = QColor("#FAFAFA");
        lined.canvasGridDot = QColor("#E0E0E0");
        lined.nodePalette[0] = QColor("#E53935"); // red
        lined.nodePalette[1] = QColor("#FB8C00"); // orange
        lined.nodePalette[2] = QColor("#1E88E5"); // blue
        lined.nodePalette[3] = QColor("#FDD835"); // yellow
        lined.nodePalette[4] = QColor("#8E24AA"); // purple
        lined.nodePalette[5] = QColor("#43A047"); // green
        lined.nodeShadow = QColor(0, 0, 0, 0); // unused (drawShadow=false)
        lined.nodeSelectionBorder = QColor("#FF6F00");
        lined.nodeText = QColor("#2C2C2C");
        lined.edgeLightenFactor = 100; // keep edge as-saturated as the branch
        lined.exportBackground = QColor("#FFFFFF");
        td.lightColors = lined;

        // Dark palette — same hues at ~88% lightness, light text.
        TemplateColorScheme linedDark;
        linedDark.canvasBackground = QColor("#1A1A2E");
        linedDark.canvasGridDot = QColor("#2A2A4A");
        linedDark.nodePalette[0] = QColor("#EF5350");
        linedDark.nodePalette[1] = QColor("#FFA726");
        linedDark.nodePalette[2] = QColor("#42A5F5");
        linedDark.nodePalette[3] = QColor("#FFEE58");
        linedDark.nodePalette[4] = QColor("#AB47BC");
        linedDark.nodePalette[5] = QColor("#66BB6A");
        linedDark.nodeShadow = QColor(0, 0, 0, 0);
        linedDark.nodeSelectionBorder = QColor("#FFB300");
        linedDark.nodeText = QColor("#E8E8E8");
        linedDark.edgeLightenFactor = 100;
        linedDark.exportBackground = QColor("#1A1A2E");
        td.darkColors = linedDark;

        td.content.text = tr("Mind Mapping");
        td.content.children = {
            {tr("Why Mind Mapping?"), {
                {tr("Disrupting linear thinking"), {}},
                {tr("Allows easy reorganization"), {}},
                {tr("Helps us think \"radiantly\""), {}}}},
            {tr("Main benefits"), {
                {tr("Enables creative thinking"), {}},
                {tr("Combats perfectionism"), {}},
                {tr("Iterative thinking"), {}}}},
            {tr("Visual thinking"), {}},
            {tr("Helpful for pre-writing"), {
                {tr("Escape the blinking cursor"), {}},
                {tr("Healthy ideation strategies"), {}}}},
            {tr("How to Mind Map"), {
                {tr("Begin with an idea"), {}},
                {tr("Build connections outward"), {}},
                {tr("Reorganize as needed"), {}}}}
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
