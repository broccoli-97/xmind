#include "scene/MindMapView.h"
#include "core/ThemeDescriptor.h"
#include "scene/MindMapScene.h"
#include "scene/NodeItem.h"
#include "ui/ThemeManager.h"

#include <QMouseEvent>
#include <QPainter>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QResizeEvent>
#include <QScrollBar>
#include <QTimer>
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
    setSceneRect(kInitialSceneX, kInitialSceneY, kInitialSceneW, kInitialSceneH);
    setAttribute(Qt::WA_InputMethodEnabled, true);

    // Coalesce the bursts of resizeEvents during a window drag into a single
    // zoomToFit after the user stops resizing.
    m_resizeFitTimer = new QTimer(this);
    m_resizeFitTimer->setSingleShot(true);
    m_resizeFitTimer->setInterval(120);
    connect(m_resizeFitTimer, &QTimer::timeout, this, &MindMapView::zoomToFit);

    // QGraphicsScene::changed fires for every item-rect change (potentially
    // many per frame), so debounce sceneRect recalculation to once per
    // 200ms. Single-shot timer restarted on each change.
    m_sceneRectGrowTimer = new QTimer(this);
    m_sceneRectGrowTimer->setSingleShot(true);
    m_sceneRectGrowTimer->setInterval(200);
    connect(m_sceneRectGrowTimer, &QTimer::timeout, this, &MindMapView::recomputeSceneRect);
}

void MindMapView::setScene(QGraphicsScene* newScene) {
    if (auto* old = scene()) {
        disconnect(old, &QGraphicsScene::changed, this, nullptr);
    }
    QGraphicsView::setScene(newScene);
    if (newScene) {
        connect(newScene, &QGraphicsScene::changed, this, [this]() {
            if (m_sceneRectGrowTimer)
                m_sceneRectGrowTimer->start();
        });
        recomputeSceneRect();
    }
}

void MindMapView::recomputeSceneRect() {
    if (!scene())
        return;
    const QRectF items = scene()->itemsBoundingRect();
    QRectF target(kInitialSceneX, kInitialSceneY, kInitialSceneW, kInitialSceneH);
    if (!items.isEmpty()) {
        // Union the initial floor with the items rect plus a margin, so a
        // dragged-out node never immediately hits the scroll wall.
        target = target.united(items.adjusted(-kSceneGrowMargin, -kSceneGrowMargin,
                                              kSceneGrowMargin, kSceneGrowMargin));
    }
    if (target != sceneRect())
        setSceneRect(target);
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

void MindMapView::resizeEvent(QResizeEvent* event) {
    QGraphicsView::resizeEvent(event);
    // Refit content when the window stops changing size. Debounced so a drag
    // doesn't fire dozens of fitInView calls and animations.
    if (scene() && !scene()->items().isEmpty())
        m_resizeFitTimer->start();
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

bool MindMapView::rectFullyVisibleIn(const QRectF& items, const QRectF& viewport, qreal inset) {
    if (items.isEmpty())
        return true;
    // Shrinking the items rect by `inset` makes the predicate tolerant of
    // tiny overflows along the viewport edge, so we don't fire on
    // essentially-on-screen layouts.
    const QRectF tightened = items.adjusted(inset, inset, -inset, -inset);
    return viewport.contains(tightened);
}

void MindMapView::zoomToFitIfOffscreen() {
    if (!scene())
        return;
    const QRectF items = scene()->itemsBoundingRect();
    const QRectF viewportInScene = mapToScene(viewport()->rect()).boundingRect();
    // Positive inset shrinks `items` before the containment check — i.e. a
    // sliver of overflow up to 10px is tolerated as "still on-screen". Without
    // it, a one-pixel rounding mismatch would yank the user's zoom away.
    if (rectFullyVisibleIn(items, viewportInScene, /*inset=*/10.0))
        return;
    zoomToFit();
}

qreal MindMapView::fitMarginForNodeWidth(qreal nodeWidth) {
    return qBound(kMinFitMargin, nodeWidth, kMaxFitMargin);
}

qreal MindMapView::fitMargin() const {
    qreal nodeWidth = 0.0;
    if (auto* mindMapScene = dynamic_cast<MindMapScene*>(scene())) {
        if (auto* root = mindMapScene->rootNode())
            nodeWidth = root->nodeRect().width();
    }
    return fitMarginForNodeWidth(nodeWidth);
}

void MindMapView::zoomToFit() {
    if (!scene())
        return;

    stopAnimations();

    const qreal margin = fitMargin();
    QRectF bounds = scene()->itemsBoundingRect().adjusted(-margin, -margin, margin, margin);

    // Snapshot current state
    QTransform oldTransform = transform();
    QPointF oldCenter = mapToScene(viewport()->rect().center());

    // Let Qt compute the target
    fitInView(bounds, Qt::KeepAspectRatio);
    QTransform newTransform = transform();

    // Cap zoom-in: fitInView happily scales to ~10x for a tiny one-node
    // scene, which makes the node fill the entire viewport. Clamp to
    // kFitMaxScale so few-node maps stay readable instead of gigantic.
    qreal fitScale = newTransform.m11();
    if (fitScale > kFitMaxScale) {
        setTransform(QTransform::fromScale(kFitMaxScale, kFitMaxScale));
        centerOn(bounds.center());
        newTransform = transform();
    }

    QPointF newCenter = mapToScene(viewport()->rect().center());

    qreal oldScale = oldTransform.m11();
    qreal newScale = newTransform.m11();

    // Already at target — nothing to animate
    if (qFuzzyCompare(oldScale, newScale) && (oldCenter - newCenter).manhattanLength() < 0.5) {
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
    QString pattern = QStringLiteral("dots");

    auto* mindMapScene = dynamic_cast<MindMapScene*>(scene());
    if (mindMapScene) {
        const auto* th = mindMapScene->themeDescriptor();
        if (th) {
            bgColor = th->activeColors().canvasBackground;
            dotColor = th->activeColors().canvasGridDot;
            pattern = th->backgroundPattern;
        }
    }

    painter->fillRect(rect, bgColor);

    if (pattern == QLatin1String("none"))
        return;

    const qreal gridSize = 40.0;
    qreal left = qFloor(rect.left() / gridSize) * gridSize;
    qreal top = qFloor(rect.top() / gridSize) * gridSize;

    if (pattern == QLatin1String("lines")) {
        QPen linePen(dotColor, 1);
        painter->setPen(linePen);
        for (qreal x = left; x <= rect.right(); x += gridSize)
            painter->drawLine(QPointF(x, rect.top()), QPointF(x, rect.bottom()));
        for (qreal y = top; y <= rect.bottom(); y += gridSize)
            painter->drawLine(QPointF(rect.left(), y), QPointF(rect.right(), y));
        return;
    }

    // "dots" (default)
    QPen dotPen(dotColor, 2);
    dotPen.setCapStyle(Qt::RoundCap);
    painter->setPen(dotPen);
    for (qreal x = left; x <= rect.right(); x += gridSize) {
        for (qreal y = top; y <= rect.bottom(); y += gridSize) {
            painter->drawPoint(QPointF(x, y));
        }
    }
}
