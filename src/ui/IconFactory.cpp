#include "ui/IconFactory.h"
#include "core/ThemeDescriptor.h"
#include "core/ThemeRegistry.h"
#include "ui/ThemeManager.h"

#include <QGuiApplication>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>

#include <algorithm>
#include <cmath>
#include <iterator>

QIcon IconFactory::makeToolIcon(const QString& name) {
    QPixmap pix(32, 32);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);

    QColor baseColor = ThemeManager::colors().iconBaseColor;
    QPen pen(baseColor, 2.0);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);

    if (name == "add-child") {
        p.drawLine(16, 8, 16, 24);
        p.drawLine(8, 16, 24, 16);
    } else if (name == "add-sibling") {
        p.drawLine(4, 10, 14, 10);
        p.drawLine(4, 22, 14, 22);
        p.drawLine(24, 14, 24, 22);
        p.drawLine(20, 18, 28, 18);
    } else if (name == "delete") {
        p.drawLine(10, 10, 10, 26);
        p.drawLine(22, 10, 22, 26);
        p.drawLine(10, 26, 22, 26);
        p.drawLine(8, 10, 24, 10);
        p.drawLine(13, 6, 19, 6);
        p.drawLine(13, 6, 13, 10);
        p.drawLine(19, 6, 19, 10);
        p.drawLine(14, 13, 14, 23);
        p.drawLine(18, 13, 18, 23);
    } else if (name == "auto-layout") {
        p.drawRect(12, 2, 8, 6);
        p.drawRect(2, 22, 8, 6);
        p.drawRect(22, 22, 8, 6);
        p.drawLine(16, 8, 16, 14);
        p.drawLine(6, 14, 26, 14);
        p.drawLine(6, 14, 6, 22);
        p.drawLine(26, 14, 26, 22);
    } else if (name == "zoom") {
        p.drawEllipse(8, 6, 16, 16);
        QPen thickPen(ThemeManager::colors().iconBaseColor, 3.0);
        p.setPen(thickPen);
        p.drawLine(21, 20, 27, 27);
    } else if (name == "zoom-in") {
        p.drawEllipse(6, 4, 18, 18);
        p.drawLine(15, 9, 15, 17);
        p.drawLine(11, 13, 19, 13);
        QPen thickPen(ThemeManager::colors().iconBaseColor, 3.0);
        p.setPen(thickPen);
        p.drawLine(22, 21, 28, 27);
    } else if (name == "zoom-out") {
        p.drawEllipse(6, 4, 18, 18);
        p.drawLine(11, 13, 19, 13);
        QPen thickPen(ThemeManager::colors().iconBaseColor, 3.0);
        p.setPen(thickPen);
        p.drawLine(22, 21, 28, 27);
    } else if (name == "undo") {
        QPen curvePen(baseColor, 2.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        p.setPen(curvePen);
        // Smooth swooping curve from bottom-right up and to the left
        QPainterPath curve;
        curve.moveTo(25, 23);
        curve.cubicTo(27, 5, 14, 12, 7, 12);
        p.drawPath(curve);
        // Chevron arrowhead pointing left
        p.drawLine(7, 12, 12, 7);
        p.drawLine(7, 12, 12, 17);
    } else if (name == "redo") {
        QPen curvePen(baseColor, 2.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        p.setPen(curvePen);
        // Smooth swooping curve from bottom-left up and to the right
        QPainterPath curve;
        curve.moveTo(7, 23);
        curve.cubicTo(5, 5, 18, 12, 25, 12);
        p.drawPath(curve);
        // Chevron arrowhead pointing right
        p.drawLine(25, 12, 20, 7);
        p.drawLine(25, 12, 20, 17);
    } else if (name == "fit-view") {
        p.drawLine(4, 10, 4, 4);
        p.drawLine(4, 4, 10, 4);
        p.drawLine(22, 4, 28, 4);
        p.drawLine(28, 4, 28, 10);
        p.drawLine(4, 22, 4, 28);
        p.drawLine(4, 28, 10, 28);
        p.drawLine(28, 22, 28, 28);
        p.drawLine(22, 28, 28, 28);
    } else if (name == "export") {
        p.drawRect(8, 14, 16, 14);
        p.drawLine(16, 16, 16, 4);
        p.drawLine(12, 8, 16, 4);
        p.drawLine(20, 8, 16, 4);
    } else if (name == "sidebar") {
        p.drawRect(4, 4, 24, 24);
        p.drawLine(14, 4, 14, 28);
        p.drawLine(17, 10, 25, 10);
        p.drawLine(17, 16, 25, 16);
        p.drawLine(17, 22, 25, 22);
    } else if (name == "toolbar") {
        p.drawRect(4, 10, 24, 12);
        p.drawLine(10, 10, 10, 22);
        p.drawLine(16, 10, 16, 22);
        p.drawLine(22, 10, 22, 22);
    } else if (name == "close-panel") {
        p.drawLine(10, 10, 22, 22);
        p.drawLine(22, 10, 10, 22);
    } else if (name == "ai-generate") {
        p.setBrush(baseColor);
        p.setPen(Qt::NoPen);
        auto drawStar = [&](qreal cx, qreal cy, qreal r) {
            QPainterPath star;
            star.moveTo(cx, cy - r);
            star.quadTo(cx, cy, cx + r, cy);
            star.quadTo(cx, cy, cx, cy + r);
            star.quadTo(cx, cy, cx - r, cy);
            star.quadTo(cx, cy, cx, cy - r);
            p.drawPath(star);
        };
        drawStar(14.0, 16.0, 9.0);
        drawStar(23.0, 8.0, 5.0);
    } else if (name == "update" || name == "update-available") {
        // Rounded-square package with a download arrow — matches the line-art
        // weight of the other toolbar icons and reads clearly at small sizes.
        bool available = (name == "update-available");
        QColor strokeColor = available ? QColor("#2E9E5B") : baseColor;
        QPen strokePen(strokeColor, 2.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        p.setPen(strokePen);
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(QRectF(4, 4, 24, 24), 5.0, 5.0);
        // Down arrow
        p.drawLine(16, 9, 16, 22);
        p.drawLine(16, 22, 10, 16);
        p.drawLine(16, 22, 22, 16);

        // Notification badge for the available state, overlapping the corner.
        if (available) {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor("#E53935"));
            p.drawEllipse(QPointF(25.5, 6.5), 4.5, 4.5);
        }
    }

    p.end();

    // Build a disabled pixmap with reduced opacity
    QPixmap disabledPix(32, 32);
    disabledPix.fill(Qt::transparent);
    QPainter dp(&disabledPix);
    dp.setOpacity(0.3);
    dp.drawPixmap(0, 0, pix);
    dp.end();

    QIcon icon;
    icon.addPixmap(pix, QIcon::Normal);
    icon.addPixmap(disabledPix, QIcon::Disabled);
    return icon;
}

QIcon IconFactory::makeTabIcon(int layoutStyleIndex) {
    QPixmap pix(16, 16);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);

    QColor base = ThemeManager::colors().iconBaseColor;
    QPen pen(base, 1.2);
    p.setPen(pen);
    p.setBrush(base);

    if (layoutStyleIndex == 0) {
        // Bilateral / Mind Map: central rect + 4 radiating lines
        p.drawRect(6, 6, 4, 4);
        p.setBrush(Qt::NoBrush);
        p.drawLine(6, 8, 2, 4);
        p.drawLine(6, 8, 2, 12);
        p.drawLine(10, 8, 14, 4);
        p.drawLine(10, 8, 14, 12);
    } else if (layoutStyleIndex == 1) {
        // TopDown / Org Chart: top node + connector + 3 child nodes
        p.drawRect(6, 1, 4, 3);
        p.drawRect(1, 11, 3, 3);
        p.drawRect(6, 11, 3, 3);
        p.drawRect(12, 11, 3, 3);
        p.setBrush(Qt::NoBrush);
        p.drawLine(8, 4, 8, 7);
        p.drawLine(3, 7, 13, 7);
        p.drawLine(3, 7, 3, 11);
        p.drawLine(8, 7, 8, 11);
        p.drawLine(13, 7, 13, 11);
    } else if (layoutStyleIndex == 2) {
        // RightTree / Project Plan: left node + branches right to sub-nodes
        p.drawRect(1, 6, 4, 4);
        p.drawRect(9, 2, 4, 3);
        p.drawRect(9, 11, 4, 3);
        p.setBrush(Qt::NoBrush);
        p.drawLine(5, 8, 7, 8);
        p.drawLine(7, 4, 7, 12);
        p.drawLine(7, 4, 9, 4);
        p.drawLine(7, 12, 9, 12);
    } else {
        // -1 / Start Page: document outline with fold corner + text lines
        p.setBrush(Qt::NoBrush);
        // Document outline
        p.drawLine(3, 1, 10, 1);
        p.drawLine(10, 1, 13, 4);
        p.drawLine(13, 4, 13, 14);
        p.drawLine(13, 14, 3, 14);
        p.drawLine(3, 14, 3, 1);
        // Fold corner
        p.drawLine(10, 1, 10, 4);
        p.drawLine(10, 4, 13, 4);
        // Text lines
        p.drawLine(5, 7, 11, 7);
        p.drawLine(5, 9, 11, 9);
        p.drawLine(5, 11, 9, 11);
    }

    p.end();
    return QIcon(pix);
}

// ---------------------------------------------------------------------------
// Template preview
// ---------------------------------------------------------------------------
//
// Goals for the preview thumbnails on the Start Page:
//   1. Crisp on high-DPI displays. Pixmaps are allocated at native pixel size
//      and the device pixel ratio is set so QPainter draws once at the right
//      resolution (no downscaling blur).
//   2. Consistent z-order at edge/node junctions. A two-pass paint draws ALL
//      connector curves first, then ALL nodes (fill + border together) — so
//      every node border lies on top of every curve, and every curve endpoint
//      hides cleanly under the node fill (we extend lines 1.5px into the
//      target node).
//   3. Each card uses ITS OWN template's palette so the preview previews the
//      real visual flavor of that template instead of a generic blue/gray.
//   4. No text inside the pixmap. The template name is rendered by a native
//      QLabel below the card (StartPage.cpp), which is always pixel-perfect.
namespace {

// Lighten a saturated brand color into a soft tint suitable as a node fill,
// so the border stays the visual emphasis. Mirrors the modern card aesthetic
// (light tinted body + colored accent border).
QColor softFill(const QColor& border, bool dark) {
    QColor hsl = border.toHsl();
    int h = hsl.hslHue();
    int s = std::max(0, hsl.hslSaturation() - 40);
    int l = dark ? 70 : 232; // pastel in light mode, deep tint in dark mode
    QColor out;
    out.setHsl(qMax(0, h), s, l);
    return out;
}

// A connector tone that harmonizes with the palette without competing for
// attention. We desaturate and lift the lightness of the palette's primary
// color so curves read as "background" but still belong to the same family
// as the nodes.
QColor connectorTone(const QColor& accent, bool dark) {
    QColor hsl = accent.toHsl();
    int h = hsl.hslHue();
    int s = std::max(0, hsl.hslSaturation() - 90);
    int l = dark ? 110 : 195;
    QColor out;
    out.setHsl(qMax(0, h), s, l);
    return out;
}

QPixmap createHiDpiPixmap(int width, int height) {
    qreal dpr = 1.0;
    if (auto* screen = QGuiApplication::primaryScreen())
        dpr = screen->devicePixelRatio();
    QPixmap pix(int(std::lround(width * dpr)), int(std::lround(height * dpr)));
    pix.setDevicePixelRatio(dpr);
    pix.fill(Qt::transparent);
    return pix;
}

void drawCurve(QPainter& p, qreal x1, qreal y1, qreal x2, qreal y2, bool horizontal) {
    QPainterPath path;
    path.moveTo(x1, y1);
    if (horizontal) {
        qreal mx = (x1 + x2) * 0.5;
        path.cubicTo(mx, y1, mx, y2, x2, y2);
    } else {
        qreal my = (y1 + y2) * 0.5;
        path.cubicTo(x1, my, x2, my, x2, y2);
    }
    p.drawPath(path);
}

// Draw a node = (very subtle drop shadow, light mode only) + tinted fill +
// colored border. Border is painted last so it always sits on top of any
// curves that pass underneath.
void drawNode(QPainter& p, const QRectF& r, qreal radius, const QColor& accent, qreal strokeW,
              bool dark) {
    if (!dark) {
        // 1-pixel-Y drop shadow gives just enough lift without looking heavy.
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0, 0, 0, 22));
        p.drawRoundedRect(r.translated(0, 0.7), radius, radius);
    }
    p.setPen(Qt::NoPen);
    p.setBrush(softFill(accent, dark));
    p.drawRoundedRect(r, radius, radius);
    p.setPen(QPen(accent, strokeW));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(r, radius, radius);
}

// Resolve the active 6-color palette for a given templateId, with a sensible
// fallback if the template is unknown.
struct PreviewPalette {
    QColor accent[6];
    bool dark;
};

PreviewPalette resolvePalette(const QString& /*templateId*/) {
    // Template previews show the *default* theme's palette — every built-in
    // template card on the Start Page renders in the same neutral palette so
    // the cards differ by layout shape, not color. Themes are picked
    // separately from the menu and don't affect these card previews.
    PreviewPalette pal{};
    pal.dark = ThemeManager::isDark();
    const auto* th = ThemeRegistry::instance().themeById(ThemeRegistry::defaultThemeId());
    if (th) {
        const auto& cs = th->activeColors();
        for (int i = 0; i < 6; ++i)
            pal.accent[i] = cs.nodePalette[i];
    } else {
        const auto& tc = ThemeManager::colors();
        for (int i = 0; i < 6; ++i)
            pal.accent[i] = tc.nodePalette[i];
    }
    return pal;
}

// Common stroke / radius constants tuned for the 120×80 logical canvas.
constexpr qreal kStrokeW = 1.6;
constexpr qreal kCurveW = 1.8;
constexpr qreal kInset = 1.5; // amount each curve extends past the node edge

// ---- Template-specific drawers -------------------------------------------
// All four follow the same pattern: pass 1 draws every connector curve, pass
// 2 draws every node. Curves are colored by their *target* node's accent
// (matching the real EdgeItem behavior) and extended kInset units into that
// node so the endpoint hides under the fill.

void drawMindMapPreview(QPainter& p, const PreviewPalette& pal) {
    constexpr qreal radius = 4.0;
    const QRectF center(42, 26, 36, 18);
    const QRectF leaves[4] = {
        QRectF(84, 4, 28, 12),
        QRectF(84, 52, 28, 12),
        QRectF(4, 4, 28, 12),
        QRectF(4, 52, 28, 12),
    };
    const int leafColorIdx[4] = {1, 2, 3, 4};

    p.setBrush(Qt::NoBrush);
    // Right-side curves
    p.setPen(QPen(connectorTone(pal.accent[leafColorIdx[0]], pal.dark), kCurveW, Qt::SolidLine,
                  Qt::RoundCap));
    drawCurve(p, 78 - kInset, 35, 84 + kInset, 10, true);
    p.setPen(QPen(connectorTone(pal.accent[leafColorIdx[1]], pal.dark), kCurveW, Qt::SolidLine,
                  Qt::RoundCap));
    drawCurve(p, 78 - kInset, 35, 84 + kInset, 58, true);
    // Left-side curves
    p.setPen(QPen(connectorTone(pal.accent[leafColorIdx[2]], pal.dark), kCurveW, Qt::SolidLine,
                  Qt::RoundCap));
    drawCurve(p, 42 + kInset, 35, 32 - kInset, 10, true);
    p.setPen(QPen(connectorTone(pal.accent[leafColorIdx[3]], pal.dark), kCurveW, Qt::SolidLine,
                  Qt::RoundCap));
    drawCurve(p, 42 + kInset, 35, 32 - kInset, 58, true);

    drawNode(p, center, radius, pal.accent[0], kStrokeW, pal.dark);
    for (int i = 0; i < 4; ++i)
        drawNode(p, leaves[i], 3.0, pal.accent[leafColorIdx[i]], kStrokeW, pal.dark);
}

void drawOrgChartPreview(QPainter& p, const PreviewPalette& pal) {
    const QRectF top(42, 4, 36, 14);
    const QRectF leaves[3] = {
        QRectF(8, 46, 28, 14),
        QRectF(46, 46, 28, 14),
        QRectF(84, 46, 28, 14),
    };
    const int leafColorIdx[3] = {1, 2, 3};

    p.setBrush(Qt::NoBrush);
    for (int i = 0; i < 3; ++i) {
        p.setPen(QPen(connectorTone(pal.accent[leafColorIdx[i]], pal.dark), kCurveW, Qt::SolidLine,
                      Qt::RoundCap));
        const qreal targetX = leaves[i].center().x();
        drawCurve(p, 60, 18 - kInset, targetX, 46 + kInset, false);
    }

    drawNode(p, top, 3.0, pal.accent[0], kStrokeW, pal.dark);
    for (int i = 0; i < 3; ++i)
        drawNode(p, leaves[i], 3.0, pal.accent[leafColorIdx[i]], kStrokeW, pal.dark);
}

void drawProjectPlanPreview(QPainter& p, const PreviewPalette& pal) {
    const QRectF root(4, 26, 28, 14);
    const QRectF mid[2] = {QRectF(44, 8, 28, 12), QRectF(44, 46, 28, 12)};
    const QRectF leaves[4] = {
        QRectF(84, 2, 28, 10),
        QRectF(84, 18, 28, 10),
        QRectF(84, 40, 28, 10),
        QRectF(84, 54, 28, 10),
    };

    p.setBrush(Qt::NoBrush);
    // Root → mid
    p.setPen(QPen(connectorTone(pal.accent[1], pal.dark), kCurveW, Qt::SolidLine, Qt::RoundCap));
    drawCurve(p, 32 - kInset, 33, 44 + kInset, 14, true);
    p.setPen(QPen(connectorTone(pal.accent[2], pal.dark), kCurveW, Qt::SolidLine, Qt::RoundCap));
    drawCurve(p, 32 - kInset, 33, 44 + kInset, 52, true);
    // Mid → leaves (top group uses accent[3], bottom group accent[4] for variety)
    p.setPen(QPen(connectorTone(pal.accent[3], pal.dark), kCurveW, Qt::SolidLine, Qt::RoundCap));
    drawCurve(p, 72 - kInset, 14, 84 + kInset, 7, true);
    drawCurve(p, 72 - kInset, 14, 84 + kInset, 23, true);
    p.setPen(QPen(connectorTone(pal.accent[4], pal.dark), kCurveW, Qt::SolidLine, Qt::RoundCap));
    drawCurve(p, 72 - kInset, 52, 84 + kInset, 45, true);
    drawCurve(p, 72 - kInset, 52, 84 + kInset, 59, true);

    drawNode(p, root, 3.0, pal.accent[0], kStrokeW, pal.dark);
    drawNode(p, mid[0], 3.0, pal.accent[1], kStrokeW, pal.dark);
    drawNode(p, mid[1], 3.0, pal.accent[2], kStrokeW, pal.dark);
    drawNode(p, leaves[0], 2.5, pal.accent[3], kStrokeW, pal.dark);
    drawNode(p, leaves[1], 2.5, pal.accent[3], kStrokeW, pal.dark);
    drawNode(p, leaves[2], 2.5, pal.accent[4], kStrokeW, pal.dark);
    drawNode(p, leaves[3], 2.5, pal.accent[4], kStrokeW, pal.dark);
}

void drawLinedPreview(QPainter& p, const PreviewPalette& pal) {
    // Mimic the lined template's signature look: short colored "underlines"
    // (each one IS a node) connected by curves at the baseline. Above each
    // underline we draw a tiny gray bar suggesting the floating text.
    //
    // Skip the yellow palette slot (index 3 on the lined template) — yellow
    // has poor contrast on a near-white card surface. Use 0/1/2/4/5 instead.
    constexpr qreal lineW = 2.6;
    constexpr qreal connectorW = 1.8;

    struct Stub {
        qreal x1, x2, y;
        QColor color;
    };
    const Stub root = {48, 72, 42, pal.accent[0]};
    const Stub leftA = {6, 30, 14, pal.accent[1]};
    const Stub leftB = {6, 30, 64, pal.accent[2]};
    const Stub rightA = {90, 114, 14, pal.accent[5]};
    const Stub rightB = {90, 114, 64, pal.accent[4]};

    auto curve = [&p](qreal x1, qreal y1, qreal x2, qreal y2, const QColor& c) {
        p.setPen(QPen(c, connectorW, Qt::SolidLine, Qt::RoundCap));
        p.setBrush(Qt::NoBrush);
        QPainterPath path;
        path.moveTo(x1, y1);
        const qreal mx = (x1 + x2) * 0.5;
        path.cubicTo(mx, y1, mx, y2, x2, y2);
        p.drawPath(path);
    };
    // Pass 1: connector curves (each colored by its target's accent).
    curve(root.x1, root.y, leftA.x2, leftA.y, leftA.color);
    curve(root.x1, root.y, leftB.x2, leftB.y, leftB.color);
    curve(root.x2, root.y, rightA.x1, rightA.y, rightA.color);
    curve(root.x2, root.y, rightB.x1, rightB.y, rightB.color);

    // Pass 2: the underlines themselves (the "nodes").
    auto stub = [&p](const Stub& s, qreal w) {
        p.setPen(QPen(s.color, w, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(s.x1, s.y), QPointF(s.x2, s.y));
    };
    stub(root, lineW + 0.4);
    stub(leftA, lineW);
    stub(leftB, lineW);
    stub(rightA, lineW);
    stub(rightB, lineW);

    // Pass 3: tiny ghost-text bars floating above each underline.
    auto textBar = [&p, &pal](const Stub& s) {
        QColor c = pal.dark ? QColor(220, 220, 220, 170) : QColor(60, 60, 60, 130);
        const qreal w = (s.x2 - s.x1) * 0.6;
        const qreal cx = (s.x1 + s.x2) * 0.5;
        p.setPen(QPen(c, 1.4, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(cx - w * 0.5, s.y - 4.5), QPointF(cx + w * 0.5, s.y - 4.5));
    };
    textBar(root);
    textBar(leftA);
    textBar(leftB);
    textBar(rightA);
    textBar(rightB);
}

void drawGenericPreview(QPainter& p, const PreviewPalette& pal) {
    constexpr qreal radius = 4.0;
    const QRectF center(35, 28, 50, 18);

    p.setBrush(Qt::NoBrush);
    // 4 radiating curves, each colored by a different palette slot for life.
    const int idx[4] = {1, 2, 3, 4};
    p.setPen(
        QPen(connectorTone(pal.accent[idx[0]], pal.dark), kCurveW, Qt::SolidLine, Qt::RoundCap));
    drawCurve(p, 85 - kInset, 37, 90 + kInset, 14, true);
    p.setPen(
        QPen(connectorTone(pal.accent[idx[1]], pal.dark), kCurveW, Qt::SolidLine, Qt::RoundCap));
    drawCurve(p, 85 - kInset, 37, 90 + kInset, 56, true);
    p.setPen(
        QPen(connectorTone(pal.accent[idx[2]], pal.dark), kCurveW, Qt::SolidLine, Qt::RoundCap));
    drawCurve(p, 35 + kInset, 37, 18 - kInset, 14, true);
    p.setPen(
        QPen(connectorTone(pal.accent[idx[3]], pal.dark), kCurveW, Qt::SolidLine, Qt::RoundCap));
    drawCurve(p, 35 + kInset, 37, 18 - kInset, 56, true);

    drawNode(p, center, radius, pal.accent[0], kStrokeW, pal.dark);
}

} // namespace

QPixmap IconFactory::makeTemplatePreview(const QString& templateId, int width, int height) {
    QPixmap pix = createHiDpiPixmap(width, height);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    // All drawers work on a 120×80 logical canvas; scale uniformly so shapes
    // keep their proportions regardless of the requested output size.
    p.scale(width / 120.0, height / 80.0);

    const PreviewPalette pal = resolvePalette(templateId);

    if (templateId == QLatin1String("builtin.mindmap")) {
        drawMindMapPreview(p, pal);
    } else if (templateId == QLatin1String("builtin.orgchart")) {
        drawOrgChartPreview(p, pal);
    } else if (templateId == QLatin1String("builtin.projectplan")) {
        drawProjectPlanPreview(p, pal);
    } else if (templateId == QLatin1String("builtin.lined")) {
        drawLinedPreview(p, pal);
    } else {
        drawGenericPreview(p, pal);
    }

    p.end();
    return pix;
}

QPixmap IconFactory::makeTemplatePreview(int index, int width, int height) {
    // Forward to the template-id-aware path so behavior stays in sync.
    static const QString ids[] = {QStringLiteral("builtin.mindmap"),
                                  QStringLiteral("builtin.orgchart"),
                                  QStringLiteral("builtin.projectplan")};
    const QString id = (index >= 0 && size_t(index) < std::size(ids)) ? ids[index] : QString();
    return makeTemplatePreview(id, width, height);
}
