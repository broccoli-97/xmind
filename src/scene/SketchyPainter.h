#pragma once

#include <QPointF>
#include <QRectF>
#include <QtGlobal>

class QPainter;

// Hand-drawn / Excalidraw-style stroke helpers. The two functions here mirror
// the painter calls used by NodeItem (rounded rect outline) and EdgeItem
// (cubic bezier), but trace each shape `passes` times with deterministic
// perpendicular jitter so the result reads as a sketched line rather than a
// CAD-perfect one.
//
// Determinism is important — the jitter is keyed off the caller-supplied seed
// so a node's wobble does not change between repaints (no flicker on hover,
// selection, or scroll). Pass the address of the item (cast to quint32) for
// a stable per-item seed.
//
// roughness == 0 short-circuits to the clean drawRoundedRect / cubicTo path.
namespace SketchyPainter {

void drawRoughRoundedRect(QPainter* painter, const QRectF& rect, qreal radius,
                          qreal roughness, int passes, quint32 seed);

void drawRoughCubicBezier(QPainter* painter, const QPointF& p0, const QPointF& c1,
                          const QPointF& c2, const QPointF& p1, qreal roughness,
                          int passes, quint32 seed);

} // namespace SketchyPainter
