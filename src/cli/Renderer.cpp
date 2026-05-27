#include "cli/Renderer.h"

#include "core/ThemeDescriptor.h"
#include "core/ThemeRegistry.h"
#include "layout/LayoutStyle.h"
#include "scene/MindMapScene.h"

#include <QBuffer>
#include <QImage>
#include <QPainter>
#include <QtSvg/QSvgGenerator>

namespace Renderer {

Format formatFromString(const QString& s, bool* ok) {
    QString v = s.trimmed().toLower();
    if (ok)
        *ok = true;
    if (v == QLatin1String("svg"))
        return Format::Svg;
    if (v == QLatin1String("png"))
        return Format::Png;
    if (ok)
        *ok = false;
    return Format::Svg;
}

const char* formatToMime(Format f) {
    switch (f) {
    case Format::Svg: return "image/svg+xml";
    case Format::Png: return "image/png";
    }
    return "application/octet-stream";
}

const char* formatToExt(Format f) {
    switch (f) {
    case Format::Svg: return "svg";
    case Format::Png: return "png";
    }
    return "bin";
}

static QByteArray renderSvg(MindMapScene* scene) {
    QRectF rect = scene->itemsBoundingRect().adjusted(-40, -40, 40, 40);
    QByteArray bytes;
    QBuffer buffer(&bytes);
    if (!buffer.open(QIODevice::WriteOnly))
        return {};

    QSvgGenerator gen;
    gen.setOutputDevice(&buffer);
    gen.setSize(rect.size().toSize());
    gen.setViewBox(QRectF(QPointF(0, 0), rect.size()));
    gen.setTitle(QStringLiteral("YMind Export"));

    QPainter p(&gen);
    if (!p.isActive())
        return {};
    p.setRenderHint(QPainter::Antialiasing);
    scene->render(&p, QRectF(), rect);
    p.end();
    buffer.close();
    return bytes;
}

static QByteArray renderPng(MindMapScene* scene, int scale) {
    QRectF rect = scene->itemsBoundingRect().adjusted(-40, -40, 40, 40);
    QSize sz(static_cast<int>(rect.width() * scale),
             static_cast<int>(rect.height() * scale));
    QImage image(sz, QImage::Format_ARGB32_Premultiplied);
    const auto* th = scene->themeDescriptor();
    QColor bg = th ? th->activeColors().exportBackground : QColor(255, 255, 255);
    image.fill(bg);

    QPainter p(&image);
    p.setRenderHint(QPainter::Antialiasing);
    scene->render(&p, QRectF(), rect);
    p.end();

    QByteArray bytes;
    QBuffer buffer(&bytes);
    if (!buffer.open(QIODevice::WriteOnly))
        return {};
    image.save(&buffer, "PNG");
    buffer.close();
    return bytes;
}

QByteArray renderToBytes(MindMapScene* scene, Format format, int scaleFactor) {
    if (!scene)
        return {};
    switch (format) {
    case Format::Svg: return renderSvg(scene);
    case Format::Png: return renderPng(scene, scaleFactor);
    }
    return {};
}

bool renderMarkdown(MindMapScene* scene,
                    const QString& markdown,
                    const QString& layout,
                    const QString& themeId,
                    Format format,
                    int scaleFactor,
                    QByteArray& out,
                    QString* error) {
    if (!scene) {
        if (error)
            *error = QStringLiteral("scene is null");
        return false;
    }

    if (!layout.isEmpty())
        scene->setLayoutStyle(algorithmNameToLayoutStyle(layout));

    if (!themeId.isEmpty()) {
        if (!ThemeRegistry::instance().themeById(themeId)) {
            if (error)
                *error = QStringLiteral("unknown theme id: %1").arg(themeId);
            return false;
        }
        scene->setThemeId(ThemeId(themeId));
    }

    if (!scene->importFromMarkdown(markdown, /*animate=*/false)) {
        if (error)
            *error = QStringLiteral("markdown produced no nodes");
        return false;
    }

    scene->layoutWithoutAnimation();
    // addNode() leaves the most recently added node selected, which would
    // draw a selection ring in the exported image.
    scene->clearSelection();

    out = renderToBytes(scene, format, scaleFactor);
    if (out.isEmpty()) {
        if (error)
            *error = QStringLiteral("renderer returned empty bytes");
        return false;
    }
    return true;
}

} // namespace Renderer
