#include "scene/MindMapExporter.h"
#include "core/TemplateDescriptor.h"
#include "scene/MindMapScene.h"
#include "scene/NodeItem.h"
#include "ui/ThemeManager.h"

#include <QImage>
#include <QPageSize>
#include <QPainter>
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
    const auto* td = m_scene->templateDescriptor();
    QColor bgColor =
        td ? td->activeColors().exportBackground : ThemeManager::colors().exportBackground;
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
