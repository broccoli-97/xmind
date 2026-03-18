#include "scene/MindMapView.h"
#include "core/TemplateDescriptor.h"
#include "scene/MindMapScene.h"
#include "ui/ThemeManager.h"

#include <QMouseEvent>
#include <QPainter>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QScrollBar>
#include <QVariantAnimation>
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
    if (event->angleDelta().y() > 0) {
        if (canZoomIn())
            scale(1.15, 1.15);
    } else {
        if (canZoomOut())
            scale(1.0 / 1.15, 1.0 / 1.15);
    }
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
    if (canZoomIn())
        scale(1.2, 1.2);
}

void MindMapView::zoomOut() {
    if (canZoomOut())
        scale(1.0 / 1.2, 1.0 / 1.2);
}

bool MindMapView::canZoomIn() const {
    return transform().m11() < kMaxScale;
}

bool MindMapView::canZoomOut() const {
    return transform().m11() > kMinScale;
}

void MindMapView::zoomToFit() {
    if (!scene())
        return;

    stopAnimations();

    QRectF bounds = scene()->itemsBoundingRect().adjusted(-80, -80, 80, 80);

    // Snapshot current state
    QTransform oldTransform = transform();
    QPointF oldCenter = mapToScene(viewport()->rect().center());

    // Let Qt compute the target
    fitInView(bounds, Qt::KeepAspectRatio);
    QTransform newTransform = transform();
    QPointF newCenter = mapToScene(viewport()->rect().center());

    qreal oldScale = oldTransform.m11();
    qreal newScale = newTransform.m11();

    // Already at target — nothing to animate
    if (qFuzzyCompare(oldScale, newScale) &&
        (oldCenter - newCenter).manhattanLength() < 0.5) {
        return;
    }

    // Restore old state, then animate
    setTransform(oldTransform);
    centerOn(oldCenter);

    m_zoomAnimation = new QVariantAnimation(this);
    m_zoomAnimation->setDuration(400);
    m_zoomAnimation->setStartValue(0.0);
    m_zoomAnimation->setEndValue(1.0);
    m_zoomAnimation->setEasingCurve(QEasingCurve::OutCubic);

    connect(m_zoomAnimation, &QVariantAnimation::valueChanged, this,
            [this, oldScale, newScale, oldCenter, newCenter](const QVariant& value) {
                qreal t = value.toReal();
                qreal s = oldScale + (newScale - oldScale) * t;
                QPointF c = oldCenter + (newCenter - oldCenter) * t;
                setTransform(QTransform::fromScale(s, s));
                centerOn(c);
            });

    connect(m_zoomAnimation, &QAbstractAnimation::finished, this, [this]() {
        m_zoomAnimation->deleteLater();
        m_zoomAnimation = nullptr;
    });

    m_zoomAnimation->start();
}

void MindMapView::ensureNodeVisible(QGraphicsItem* item) {
    if (!item)
        return;

    stopAnimations();

    // Snapshot current scrollbar positions
    int oldH = horizontalScrollBar()->value();
    int oldV = verticalScrollBar()->value();

    // Let Qt compute the target scroll position
    ensureVisible(item, 80, 80);

    // Capture the target positions
    int newH = horizontalScrollBar()->value();
    int newV = verticalScrollBar()->value();

    // Already visible — nothing to animate
    if (oldH == newH && oldV == newV)
        return;

    // Restore original positions before animating
    horizontalScrollBar()->setValue(oldH);
    verticalScrollBar()->setValue(oldV);

    // Animate horizontal scrollbar
    auto* hAnim = new QPropertyAnimation(horizontalScrollBar(), "value");
    hAnim->setDuration(300);
    hAnim->setStartValue(oldH);
    hAnim->setEndValue(newH);
    hAnim->setEasingCurve(QEasingCurve::OutCubic);

    // Animate vertical scrollbar
    auto* vAnim = new QPropertyAnimation(verticalScrollBar(), "value");
    vAnim->setDuration(300);
    vAnim->setStartValue(oldV);
    vAnim->setEndValue(newV);
    vAnim->setEasingCurve(QEasingCurve::OutCubic);

    m_scrollAnimation = new QParallelAnimationGroup(this);
    m_scrollAnimation->addAnimation(hAnim);
    m_scrollAnimation->addAnimation(vAnim);
    connect(m_scrollAnimation, &QAbstractAnimation::finished, this, [this]() {
        m_scrollAnimation->deleteLater();
        m_scrollAnimation = nullptr;
    });
    m_scrollAnimation->start();
}

void MindMapView::stopAnimations() {
    if (m_scrollAnimation) {
        m_scrollAnimation->stop();
        m_scrollAnimation->deleteLater();
        m_scrollAnimation = nullptr;
    }
    if (m_zoomAnimation) {
        m_zoomAnimation->stop();
        m_zoomAnimation->deleteLater();
        m_zoomAnimation = nullptr;
    }
}

void MindMapView::drawBackground(QPainter* painter, const QRectF& rect) {
    QColor bgColor = ThemeManager::colors().canvasBackground;
    QColor dotColor = ThemeManager::colors().canvasGridDot;

    auto* mindMapScene = dynamic_cast<MindMapScene*>(scene());
    if (mindMapScene) {
        const auto* td = mindMapScene->templateDescriptor();
        if (td) {
            bgColor = td->activeColors().canvasBackground;
            dotColor = td->activeColors().canvasGridDot;
        }
    }

    painter->fillRect(rect, bgColor);

    QPen dotPen(dotColor, 2);
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
