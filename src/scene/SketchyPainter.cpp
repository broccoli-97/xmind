#include "scene/SketchyPainter.h"

#include <QPainter>
#include <QPainterPath>
#include <QVector>
#include <QtMath>

namespace SketchyPainter {
namespace {

// Tiny xorshift PRNG. Deterministic from the seed so a given node/edge wobbles
// the same way every repaint — anything else flickers as the user hovers or
// scrolls because the cached pixmap is rebuilt with fresh randomness.
class Rng {
public:
    explicit Rng(quint32 seed) : m_state(seed ? seed : 0x9E3779B9u) {}
    qreal signed1() {
        m_state ^= m_state << 13;
        m_state ^= m_state >> 17;
        m_state ^= m_state << 5;
        return (qint32(m_state & 0xFFFFFFu) / qreal(0x800000u)) - 1.0;
    }
private:
    quint32 m_state;
};

} // namespace

void drawRoughRoundedRect(QPainter* painter, const QRectF& rect, qreal radius,
                          qreal roughness, int passes, quint32 seed) {
    if (roughness <= 0.0 || passes <= 0) {
        painter->drawRoundedRect(rect, radius, radius);
        return;
    }

    QPainterPath base;
    base.addRoundedRect(rect, radius, radius);

    // Sampling density: enough to keep arcs round, sparse enough that each
    // perturbed segment is comfortably longer than the jitter amplitude (so
    // the outline doesn't fold over itself).
    constexpr int kSamples = 56;
    QVector<QPointF> pts;
    pts.reserve(kSamples);
    for (int i = 0; i < kSamples; ++i) {
        pts.append(base.pointAtPercent(qreal(i) / kSamples));
    }

    const qreal shortEdge = qMin(rect.width(), rect.height());
    const qreal amplitude = qBound<qreal>(0.6, roughness * shortEdge * 0.035, 5.0);

    for (int p = 0; p < passes; ++p) {
        Rng rng(seed + quint32(p) * 0x9E3779B9u);
        QPainterPath path;
        QPointF first;
        for (int i = 0; i < kSamples; ++i) {
            QPointF pt = pts[i];
            pt += QPointF(rng.signed1() * amplitude, rng.signed1() * amplitude);
            if (i == 0) {
                first = pt;
                path.moveTo(pt);
            } else {
                path.lineTo(pt);
            }
        }
        path.lineTo(first);
        painter->drawPath(path);
    }
}

void drawRoughCubicBezier(QPainter* painter, const QPointF& p0, const QPointF& c1,
                          const QPointF& c2, const QPointF& p1, qreal roughness,
                          int passes, quint32 seed) {
    if (roughness <= 0.0 || passes <= 0) {
        QPainterPath path;
        path.moveTo(p0);
        path.cubicTo(c1, c2, p1);
        painter->drawPath(path);
        return;
    }

    // Endpoints stay exactly on the source/target anchors — only control
    // points wiggle. This keeps the edge visually attached to both nodes
    // while letting the curve breathe between them.
    const qreal span = QLineF(p0, p1).length();
    const qreal amplitude = qBound<qreal>(1.2, roughness * span * 0.05, 16.0);

    for (int p = 0; p < passes; ++p) {
        Rng rng(seed + quint32(p) * 0x85EBCA77u);
        const QPointF dc1(rng.signed1() * amplitude, rng.signed1() * amplitude);
        const QPointF dc2(rng.signed1() * amplitude, rng.signed1() * amplitude);
        QPainterPath path;
        path.moveTo(p0);
        path.cubicTo(c1 + dc1, c2 + dc2, p1);
        painter->drawPath(path);
    }
}

} // namespace SketchyPainter
