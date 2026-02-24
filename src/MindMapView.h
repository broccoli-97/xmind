#pragma once

#include <QGraphicsView>

class MindMapView : public QGraphicsView {
    Q_OBJECT

public:
    explicit MindMapView(QWidget* parent = nullptr);

public slots:
    void zoomIn();
    void zoomOut();
    void zoomToFit();

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void drawBackground(QPainter* painter, const QRectF& rect) override;

private:
    bool m_panning = false;
    QPoint m_lastPanPoint;
};
