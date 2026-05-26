#pragma once

#include <QGraphicsView>

class QParallelAnimationGroup;
class QTimer;
class QVariantAnimation;

class MindMapView : public QGraphicsView {
    Q_OBJECT

public:
    explicit MindMapView(QWidget* parent = nullptr);

public slots:
    void zoomIn();
    void zoomOut();
    void zoomToFit();
    void ensureNodeVisible(QGraphicsItem* item);

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
};
