#pragma once

#include <QGraphicsView>

class QParallelAnimationGroup;
class QTimer;
class QVariantAnimation;

class MindMapView : public QGraphicsView {
    Q_OBJECT

public:
    explicit MindMapView(QWidget* parent = nullptr);

    // Override so MindMapView can attach / detach scene-change signals when
    // the underlying scene swaps. Necessary for the auto-growing sceneRect.
    void setScene(QGraphicsScene* scene);

public slots:
    void zoomIn();
    void zoomOut();
    void zoomToFit();
    // Fits the view to the scene only when content currently runs off-screen
    // (i.e. some part of `itemsBoundingRect()` lies outside the viewport).
    // No-op otherwise so a deliberate zoom-in isn't yanked away by Ctrl+L.
    void zoomToFitIfOffscreen();
    // Expand the QGraphicsView's sceneRect to encompass the current items
    // bounding rect (plus a margin), but never shrink below the initial
    // 10000x10000 floor that preserves pan headroom on empty scenes. Called
    // (debounced) whenever the underlying scene reports a change.
    void recomputeSceneRect();
    void ensureNodeVisible(QGraphicsItem* item);

public:
    // Pure geometry predicate, lifted out so it can be unit-tested without a
    // live QGraphicsView. Returns true iff `items` (shrunk by `inset` on every
    // side, so the predicate is forgiving at edges) is fully contained in
    // `viewport`. The inset matches the 80px breathing room used by zoomToFit.
    static bool rectFullyVisibleIn(const QRectF& items, const QRectF& viewport, qreal inset);

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void drawBackground(QPainter* painter, const QRectF& rect) override;

private:
    void stopAnimations();
    bool canZoomIn() const;
    bool canZoomOut() const;

    static constexpr qreal kMinScale = 0.1;
    static constexpr qreal kMaxScale = 10.0;
    // Cap zoom-to-fit at 1:1 so a single small node (e.g. the root after
    // creating a new map) doesn't get scaled up to fill the viewport. The
    // user can still zoom in manually past this with the wheel.
    static constexpr qreal kFitMaxScale = 1.0;

    bool m_panning = false;
    QPoint m_lastPanPoint;
    QParallelAnimationGroup* m_scrollAnimation = nullptr;
    QVariantAnimation* m_zoomAnimation = nullptr;
    QTimer* m_resizeFitTimer = nullptr;
    QTimer* m_sceneRectGrowTimer = nullptr;

    // Initial sceneRect — also the minimum the auto-grow logic will ever
    // set, so a freshly-cleared scene keeps the original pan headroom.
    static constexpr qreal kInitialSceneX = -5000.0;
    static constexpr qreal kInitialSceneY = -5000.0;
    static constexpr qreal kInitialSceneW = 10000.0;
    static constexpr qreal kInitialSceneH = 10000.0;
    // How far past the items rect to extend the sceneRect on growth, so the
    // user has room to drag nodes outward without immediately hitting a wall.
    static constexpr qreal kSceneGrowMargin = 2000.0;
};
