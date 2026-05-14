#include "scene/MindMapExporter.h"
#include "core/ThemeDescriptor.h"
#include "scene/MindMapScene.h"
#include "scene/NodeItem.h"
#include "ui/ThemeManager.h"

#include <QImage>
#include <QPageSize>
#include <QPainter>
#include <QRegularExpression>
#include <QtPrintSupport/QPrinter>
#include <QtSvg/QSvgGenerator>

MindMapExporter::MindMapExporter(MindMapScene* scene) : m_scene(scene) {}

void MindMapExporter::exportNodeToText(NodeItem* node, int indent, QString& output) const {
    output += QString(indent, '\t') + node->text() + '\n';
    for (auto* child : node->childNodes()) {
        exportNodeToText(child, indent + 1, output);
    }
}

QString MindMapExporter::exportToText() const {
    QString output;
    if (m_scene->m_rootNode)
        exportNodeToText(m_scene->m_rootNode, 0, output);
    return output;
}

void MindMapExporter::exportNodeToMarkdown(NodeItem* node, int level, QString& output) const {
    if (level == 0) {
        output += "# " + node->text() + "\n\n";
    } else if (level == 1) {
        output += "## " + node->text() + "\n\n";
    } else {
        output += QString((level - 2) * 2, ' ') + "- " + node->text() + '\n';
    }
    for (auto* child : node->childNodes()) {
        exportNodeToMarkdown(child, level + 1, output);
    }
    if (level <= 1)
        output += '\n';
}

QString MindMapExporter::exportToMarkdown() const {
    QString output;
    if (m_scene->m_rootNode)
        exportNodeToMarkdown(m_scene->m_rootNode, 0, output);
    return output;
}

bool MindMapExporter::exportToPng(const QString& filePath, int scaleFactor) {
    QRectF contentRect = m_scene->itemsBoundingRect().adjusted(-40, -40, 40, 40);
    QSize imageSize(static_cast<int>(contentRect.width() * scaleFactor),
                    static_cast<int>(contentRect.height() * scaleFactor));

    QImage image(imageSize, QImage::Format_ARGB32_Premultiplied);
    const auto* th = m_scene->themeDescriptor();
    QColor bgColor =
        th ? th->activeColors().exportBackground : ThemeManager::colors().exportBackground;
    image.fill(bgColor);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    m_scene->render(&painter, QRectF(), contentRect);
    painter.end();

    return image.save(filePath, "PNG");
}

bool MindMapExporter::exportToSvg(const QString& filePath) {
    QRectF contentRect = m_scene->itemsBoundingRect().adjusted(-40, -40, 40, 40);

    QSvgGenerator generator;
    generator.setFileName(filePath);
    generator.setSize(contentRect.size().toSize());
    generator.setViewBox(QRectF(QPointF(0, 0), contentRect.size()));
    generator.setTitle("YMind Export");

    QPainter painter(&generator);
    if (!painter.isActive())
        return false;
    painter.setRenderHint(QPainter::Antialiasing);
    m_scene->render(&painter, QRectF(), contentRect);
    painter.end();

    return true;
}

bool MindMapExporter::exportToPdf(const QString& filePath) {
    QRectF contentRect = m_scene->itemsBoundingRect().adjusted(-40, -40, 40, 40);

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageSize(QPageSize(contentRect.size().toSize(), QPageSize::Point));
    printer.setPageMargins(QMarginsF(0, 0, 0, 0));

    QPainter painter(&printer);
    if (!painter.isActive())
        return false;
    painter.setRenderHint(QPainter::Antialiasing);
    m_scene->render(&painter, QRectF(), contentRect);
    painter.end();

    return true;
}

namespace {

// One parsed item from the markdown stream: a (depth, text) pair.
// Depth uses the convention: heading level for #/##/..., or
// (last-heading-depth + 1 + list-nesting) for - / * / + list items.
struct MarkdownItem {
    int depth;
    QString text;
};

QList<MarkdownItem> parseMarkdownItems(const QString& md) {
    QList<MarkdownItem> items;
    const QStringList lines = md.split('\n');

    bool inFence = false;
    int lastHeadingDepth = 0;

    static const QRegularExpression headingRx(QStringLiteral("^(#{1,6})\\s+(.+?)\\s*#*\\s*$"));
    static const QRegularExpression listRx(QStringLiteral("^(\\s*)([-*+]|\\d+\\.)\\s+(.+)$"));
    static const QRegularExpression fenceRx(QStringLiteral("^\\s*```"));

    auto stripInlineMd = [](QString s) {
        // Drop link wrappers: [text](url) -> text
        static const QRegularExpression linkRx(
            QStringLiteral("\\[([^\\]]+)\\]\\([^\\)]*\\)"));
        s.replace(linkRx, QStringLiteral("\\1"));
        // Strip surrounding emphasis markers (** __ * _ ` )
        static const QRegularExpression emphRx(
            QStringLiteral("(\\*\\*|__|\\*|_|`)(.+?)\\1"));
        s.replace(emphRx, QStringLiteral("\\2"));
        return s.trimmed();
    };

    for (const QString& raw : lines) {
        if (fenceRx.match(raw).hasMatch()) {
            inFence = !inFence;
            continue;
        }
        if (inFence)
            continue;

        QString line = raw;
        // Trim trailing whitespace but preserve leading indent for list detection.
        while (!line.isEmpty() && (line.back() == ' ' || line.back() == '\t' || line.back() == '\r'))
            line.chop(1);

        if (line.trimmed().isEmpty())
            continue;

        QRegularExpressionMatch h = headingRx.match(line.trimmed());
        if (h.hasMatch()) {
            int level = h.captured(1).length();
            QString txt = stripInlineMd(h.captured(2));
            if (txt.isEmpty())
                continue;
            items.append({level, txt});
            lastHeadingDepth = level;
            continue;
        }

        QRegularExpressionMatch l = listRx.match(line);
        if (l.hasMatch()) {
            QString indent = l.captured(1);
            QString txt = stripInlineMd(l.captured(3));
            if (txt.isEmpty())
                continue;
            // Tabs count as one indent level; spaces use 2-per-level rounding.
            int spaces = 0;
            for (QChar c : indent) {
                if (c == '\t')
                    spaces += 2;
                else
                    spaces += 1;
            }
            int nesting = spaces / 2;
            int depth = lastHeadingDepth + 1 + nesting;
            items.append({depth, txt});
            continue;
        }

        // Plain paragraph line — only meaningful if we have no items yet
        // (use it as a root) to avoid surprising "extra leaf nodes" later on.
        if (items.isEmpty()) {
            QString txt = stripInlineMd(line.trimmed());
            if (!txt.isEmpty())
                items.append({1, txt});
        }
    }

    return items;
}

} // namespace

bool MindMapExporter::importFromMarkdown(const QString& text, bool animate) {
    QList<MarkdownItem> items = parseMarkdownItems(text);
    if (items.isEmpty())
        return false;

    m_scene->clearScene();
    m_scene->m_batchLoading = true;

    // Stack of (depth, node) pairs walked left-to-right as we read items.
    QList<QPair<int, NodeItem*>> stack;
    int rootDepth = 0;

    for (const MarkdownItem& it : items) {
        if (stack.isEmpty()) {
            m_scene->m_rootNode = m_scene->createRootNode(it.text);
            rootDepth = it.depth;
            stack.append({it.depth, m_scene->m_rootNode});
            continue;
        }

        // Anything as shallow as the root nests directly under root —
        // a mindmap has exactly one root, so "sibling-of-root" collapses
        // to "child-of-root".
        int effective = qMax(it.depth, rootDepth + 1);

        while (stack.size() > 1 && stack.last().first >= effective)
            stack.removeLast();

        NodeItem* parent = stack.last().second;
        NodeItem* node = m_scene->addNode(it.text, parent);
        if (!node)
            continue;
        stack.append({effective, node});
    }

    m_scene->m_batchLoading = false;
    if (animate)
        m_scene->autoLayout();
    m_scene->setModified(false);
    return true;
}

bool MindMapExporter::importFromText(const QString& text) {
    QStringList lines = text.split('\n', Qt::SkipEmptyParts);
    if (lines.isEmpty())
        return false;

    m_scene->clearScene();
    m_scene->m_batchLoading = true;

    // Parse indented text into tree
    // Stack tracks (indent_level, node) pairs
    QList<QPair<int, NodeItem*>> stack;

    for (const QString& line : lines) {
        // Count leading tabs
        int indent = 0;
        while (indent < line.size() && line[indent] == '\t')
            indent++;

        QString nodeText = line.mid(indent).trimmed();
        if (nodeText.isEmpty())
            continue;

        if (stack.isEmpty()) {
            // First node becomes root
            m_scene->m_rootNode = m_scene->createRootNode(nodeText);
            stack.append({indent, m_scene->m_rootNode});
        } else {
            // Find the parent: walk back up the stack to find the most recent
            // node with a smaller indent
            while (stack.size() > 1 && stack.last().first >= indent)
                stack.removeLast();

            NodeItem* parent = stack.last().second;
            NodeItem* node = m_scene->addNode(nodeText, parent);
            stack.append({indent, node});
        }
    }

    if (!m_scene->m_rootNode) {
        m_scene->m_rootNode = m_scene->createRootNode(MindMapScene::tr("Central Topic"));
    }

    m_scene->m_batchLoading = false;
    m_scene->autoLayout();
    m_scene->setModified(false);
    return true;
}
