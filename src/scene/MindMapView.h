#pragma once

#include <QGraphicsView>

class QParallelAnimationGroup;
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
    void drawBackground(QPainter* painter, const QRectF& rect) override;

private:
    void stopAnimations();

    bool m_panning = false;
    QPoint m_lastPanPoint;
    QParallelAnimationGroup* m_scrollAnimation = nullptr;
    QVariantAnimation* m_zoomAnimation = nullptr;
};
