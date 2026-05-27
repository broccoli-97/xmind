#include "scene/MindMapExporter.h"
#include "core/ThemeDescriptor.h"
#include "scene/MindMapScene.h"
#include "scene/NodeItem.h"
#include "ui/ThemeManager.h"

#include <QCoreApplication>
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
    generator.setTitle(QCoreApplication::translate("MindMapExporter", "YMind Export"));

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

namespace {

// One parsed entry produced by the strict parser. Depth is 0-based:
// 0 = root, 1 = direct child of root, etc.
struct StrictItem {
    int line;
    int depth;
    QString text;
};

QString stripStrictInline(QString s) {
    static const QRegularExpression linkRx(
        QStringLiteral("\\[([^\\]]+)\\]\\([^\\)]*\\)"));
    s.replace(linkRx, QStringLiteral("\\1"));
    static const QRegularExpression emphRx(
        QStringLiteral("(\\*\\*|__|\\*|_|`)(.+?)\\1"));
    s.replace(emphRx, QStringLiteral("\\2"));
    return s.trimmed();
}

bool isHorizontalRuleOrSetext(const QString& trimmed) {
    if (trimmed.size() < 3)
        return false;
    QChar c0 = trimmed[0];
    if (c0 != '=' && c0 != '-' && c0 != '*' && c0 != '_')
        return false;
    int significant = 0;
    for (QChar ch : trimmed) {
        if (ch == c0)
            significant++;
        else if (ch != ' ' && ch != '\t')
            return false;
    }
    return significant >= 3;
}

} // namespace

bool MindMapExporter::importFromMarkdownStrict(const QString& text,
                                               MarkdownImportReport* report,
                                               bool animate) {
    MarkdownImportReport localReport;
    if (!report)
        report = &localReport;
    report->errors.clear();

    QString cleaned = text;
    if (!cleaned.isEmpty() && cleaned[0] == QChar(0xFEFF))
        cleaned.remove(0, 1);
    const QStringList rawLines = cleaned.split('\n');
    QList<StrictItem> items;

    bool seenAnyContent = false;
    bool rootIsHeading = false;
    int previousDepth = -1;

    static const QRegularExpression atxHeadingRx(
        QStringLiteral("^(#{1,6})\\s+(.+?)\\s*#*\\s*$"));
    static const QRegularExpression atxNoSpaceRx(QStringLiteral("^#{1,6}[^\\s#]"));
    static const QRegularExpression orderedRx(QStringLiteral("^\\s*\\d+[.)]\\s+"));
    static const QRegularExpression fenceRx(QStringLiteral("^\\s*(```|~~~)"));
    static const QRegularExpression blockquoteRx(QStringLiteral("^\\s*>"));
    static const QRegularExpression htmlCommentRx(QStringLiteral("^\\s*<!--"));
    static const QRegularExpression htmlRx(QStringLiteral("^\\s*</?[a-zA-Z!]"));

    auto pushErr = [&](int line, const QString& msg) {
        report->errors.append({line, msg});
    };

    QString prevNonBlank;

    for (int i = 0; i < rawLines.size(); ++i) {
        const int lineNo = i + 1;
        QString line = rawLines[i];
        while (!line.isEmpty()
               && (line.back() == ' ' || line.back() == '\t' || line.back() == '\r'))
            line.chop(1);

        if (line.isEmpty()) {
            prevNonBlank.clear();
            continue;
        }

        const QString trimmed = line.trimmed();

        // Setext underline / horizontal rule. Reject either way — neither is
        // expressible in the strict format.
        if (isHorizontalRuleOrSetext(trimmed)) {
            if (!prevNonBlank.isEmpty()) {
                pushErr(lineNo,
                        MindMapScene::tr("Underlined headings (setext) are not "
                                         "supported. Use `# Title` on a single "
                                         "line."));
            } else {
                pushErr(lineNo,
                        MindMapScene::tr("Horizontal rules are not supported."));
            }
            prevNonBlank = line;
            continue;
        }

        // HTML comments are silently ignored (useful for metadata stubs).
        if (htmlCommentRx.match(line).hasMatch()) {
            prevNonBlank.clear();
            continue;
        }

        if (fenceRx.match(line).hasMatch()) {
            pushErr(lineNo,
                    MindMapScene::tr("Fenced code blocks are not supported."));
            prevNonBlank = line;
            continue;
        }

        if (htmlRx.match(line).hasMatch()) {
            pushErr(lineNo, MindMapScene::tr("Raw HTML is not supported."));
            prevNonBlank = line;
            continue;
        }

        if (blockquoteRx.match(line).hasMatch()) {
            pushErr(lineNo,
                    MindMapScene::tr("Blockquotes (>) are not supported."));
            prevNonBlank = line;
            continue;
        }

        if (orderedRx.match(line).hasMatch()) {
            pushErr(lineNo, MindMapScene::tr("Ordered lists are not supported. "
                                             "Use `-`, `*` or `+` bullets."));
            prevNonBlank = line;
            continue;
        }

        if (line.contains('|')) {
            pushErr(lineNo, MindMapScene::tr("Tables are not supported."));
            prevNonBlank = line;
            continue;
        }

        if (atxNoSpaceRx.match(line).hasMatch()) {
            pushErr(lineNo, MindMapScene::tr("Heading must have a space after "
                                             "`#` (e.g. `# Title`)."));
            prevNonBlank = line;
            continue;
        }

        QRegularExpressionMatch hm = atxHeadingRx.match(trimmed);
        if (hm.hasMatch()) {
            const int level = hm.captured(1).length();
            const QString txt = stripStrictInline(hm.captured(2));
            if (level != 1) {
                pushErr(lineNo,
                        MindMapScene::tr("Only `# Title` (H1) is supported. "
                                         "Use list nesting instead of `%1`.")
                            .arg(QString(level, QChar('#'))));
                prevNonBlank = line;
                continue;
            }
            if (seenAnyContent) {
                pushErr(lineNo,
                        MindMapScene::tr("`# Title` must be the very first "
                                         "content (only one root is allowed)."));
                prevNonBlank = line;
                continue;
            }
            if (txt.isEmpty()) {
                pushErr(lineNo, MindMapScene::tr("Heading text is empty."));
                prevNonBlank = line;
                continue;
            }
            items.append({lineNo, 0, txt});
            seenAnyContent = true;
            rootIsHeading = true;
            previousDepth = 0;
            prevNonBlank = line;
            continue;
        }

        // Bullet detection: split off leading whitespace, then check for
        // `-`/`*`/`+` followed by a space. Doing this manually (rather than
        // via regex) lets us produce a precise error for tab-indented bullets.
        int prefixLen = 0;
        while (prefixLen < line.size()
               && (line[prefixLen] == ' ' || line[prefixLen] == '\t'))
            prefixLen++;
        const QString prefix = line.left(prefixLen);
        const QString body = line.mid(prefixLen);
        const bool isBullet = body.size() >= 2
                              && (body[0] == '-' || body[0] == '*' || body[0] == '+')
                              && (body[1] == ' ' || body[1] == '\t');
        if (isBullet) {
            if (prefix.contains('\t')) {
                pushErr(lineNo, MindMapScene::tr(
                                    "Indentation must use spaces, not tabs "
                                    "(2 spaces per level)."));
                prevNonBlank = line;
                continue;
            }
            const int spaces = prefix.size();
            if (spaces % 2 != 0) {
                pushErr(lineNo,
                        MindMapScene::tr("Indentation must be a multiple of 2 "
                                         "spaces (got %1).")
                            .arg(spaces));
                prevNonBlank = line;
                continue;
            }
            const int nesting = spaces / 2;
            const QString txt = stripStrictInline(body.mid(2).trimmed());
            if (txt.isEmpty()) {
                pushErr(lineNo, MindMapScene::tr("List item text is empty."));
                prevNonBlank = line;
                continue;
            }

            int depth;
            if (rootIsHeading) {
                depth = 1 + nesting;
            } else if (!seenAnyContent) {
                if (spaces != 0) {
                    pushErr(lineNo, MindMapScene::tr(
                                        "First list item must not be indented "
                                        "(it becomes the root)."));
                    prevNonBlank = line;
                    continue;
                }
                depth = 0;
            } else {
                depth = nesting;
            }

            if (previousDepth >= 0 && depth > previousDepth + 1) {
                pushErr(lineNo,
                        MindMapScene::tr("Indentation skipped a level: jumped "
                                         "from depth %1 to %2.")
                            .arg(previousDepth)
                            .arg(depth));
                prevNonBlank = line;
                continue;
            }

            items.append({lineNo, depth, txt});
            seenAnyContent = true;
            previousDepth = depth;
            prevNonBlank = line;
            continue;
        }

        // Plain paragraph text — only accepted as a fallback root when there's
        // no content yet, otherwise it's stray text.
        if (!seenAnyContent) {
            const QString txt = stripStrictInline(trimmed);
            if (!txt.isEmpty()) {
                items.append({lineNo, 0, txt});
                seenAnyContent = true;
                rootIsHeading = true;
                previousDepth = 0;
            }
            prevNonBlank = line;
            continue;
        }

        pushErr(lineNo,
                MindMapScene::tr("Stray text outside of the unordered list "
                                 "structure: \"%1\".")
                    .arg(trimmed.left(60)));
        prevNonBlank = line;
    }

    if (items.isEmpty() && report->errors.isEmpty()) {
        pushErr(0, MindMapScene::tr("The file is empty — nothing to import."));
    }

    if (!report->ok())
        return false;

    m_scene->clearScene();
    m_scene->m_batchLoading = true;

    QList<QPair<int, NodeItem*>> stack;
    for (const StrictItem& it : items) {
        if (stack.isEmpty()) {
            m_scene->m_rootNode = m_scene->createRootNode(it.text);
            stack.append({it.depth, m_scene->m_rootNode});
            continue;
        }
        while (stack.size() > 1 && stack.last().first >= it.depth)
            stack.removeLast();
        NodeItem* parent = stack.last().second;
        NodeItem* node = m_scene->addNode(it.text, parent);
        if (!node)
            continue;
        stack.append({it.depth, node});
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
