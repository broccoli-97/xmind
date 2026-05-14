#pragma once

#include <QByteArray>
#include <QString>

class MindMapScene;

// Headless rendering helpers — produce in-memory bytes for SVG or PNG so the
// CLI can pipe to stdout or attach to an MCP response without writing a file.
namespace Renderer {

enum class Format { Svg, Png };

Format formatFromString(const QString& s, bool* ok = nullptr);
const char* formatToMime(Format f);
const char* formatToExt(Format f);

QByteArray renderToBytes(MindMapScene* scene, Format format, int scaleFactor = 2);

// Loads a markdown document into the given scene, applies the layout/theme,
// and renders it. The scene must already be constructed by the caller. On
// success, populates `out` with the encoded bytes.
bool renderMarkdown(MindMapScene* scene,
                    const QString& markdown,
                    const QString& layout,
                    const QString& themeId,
                    Format format,
                    int scaleFactor,
                    QByteArray& out,
                    QString* error = nullptr);

} // namespace Renderer
