#include "MindMapView.h"

#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QWheelEvent>
#include <QtMath>

MindMapView::MindMapView(QWidget* parent) : QGraphicsView(parent) {
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setViewportUpdateMode(FullViewportUpdate);
    setDragMode(NoDrag);
    setTransformationAnchor(AnchorUnderMouse);
    setResizeAnchor(AnchorViewCenter);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setSceneRect(-5000, -5000, 10000, 10000);
    setAttribute(Qt::WA_InputMethodEnabled, true);
}

void MindMapView::wheelEvent(QWheelEvent* event) {
    qreal factor = (event->angleDelta().y() > 0) ? 1.15 : 1.0 / 1.15;
    scale(factor, factor);
    event->accept();
}

void MindMapView::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton ||
        (event->button() == Qt::RightButton && event->modifiers() == Qt::NoModifier)) {
        m_panning = true;
        m_lastPanPoint = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }
    QGraphicsView::mousePressEvent(event);
}

void MindMapView::mouseMoveEvent(QMouseEvent* event) {
    if (m_panning) {
        QPoint delta = event->pos() - m_lastPanPoint;
        m_lastPanPoint = event->pos();
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        event->accept();
        return;
    }
    QGraphicsView::mouseMoveEvent(event);
}

void MindMapView::mouseReleaseEvent(QMouseEvent* event) {
    if (m_panning) {
        m_panning = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
        return;
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void MindMapView::zoomIn() {
    scale(1.2, 1.2);
}

void MindMapView::zoomOut() {
    scale(1.0 / 1.2, 1.0 / 1.2);
}

void MindMapView::zoomToFit() {
    if (!scene())
        return;
    QRectF bounds = scene()->itemsBoundingRect().adjusted(-80, -80, 80, 80);
    fitInView(bounds, Qt::KeepAspectRatio);
}

void MindMapView::drawBackground(QPainter* painter, const QRectF& rect) {
    painter->fillRect(rect, QColor("#F8F9FA"));

    QPen dotPen(QColor("#D8D8D8"), 2);
    dotPen.setCapStyle(Qt::RoundCap);
    painter->setPen(dotPen);

    const qreal gridSize = 40.0;
    qreal left = qFloor(rect.left() / gridSize) * gridSize;
    qreal top = qFloor(rect.top() / gridSize) * gridSize;

    for (qreal x = left; x <= rect.right(); x += gridSize) {
        for (qreal y = top; y <= rect.bottom(); y += gridSize) {
            painter->drawPoint(QPointF(x, y));
        }
    }
}
