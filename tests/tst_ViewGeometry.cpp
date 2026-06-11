#include "scene/MindMapView.h"

#include <QRectF>
#include <QTest>

class tst_ViewGeometry : public QObject {
    Q_OBJECT

private slots:
    void emptyItemsRectAlwaysVisible();
    void itemsFullyInsideViewportIsVisible();
    void itemsLargerThanViewportIsNotVisible();
    void smallOverflowIsTolerated();
    void itemsExactlyAtViewportEdgeIsVisible();
    void fitMarginTracksNodeWidth();
    void fitMarginFloorsAtMinimum();
    void fitMarginCapsAtMaximum();
};

void tst_ViewGeometry::emptyItemsRectAlwaysVisible() {
    QCOMPARE(MindMapView::rectFullyVisibleIn(QRectF(), QRectF(0, 0, 800, 600), 10.0), true);
}

void tst_ViewGeometry::itemsFullyInsideViewportIsVisible() {
    QRectF items(100, 100, 200, 150);
    QRectF viewport(0, 0, 800, 600);
    QCOMPARE(MindMapView::rectFullyVisibleIn(items, viewport, 0.0), true);
}

void tst_ViewGeometry::itemsLargerThanViewportIsNotVisible() {
    QRectF items(-1000, -1000, 4000, 4000);
    QRectF viewport(0, 0, 800, 600);
    QCOMPARE(MindMapView::rectFullyVisibleIn(items, viewport, 10.0), false);
}

void tst_ViewGeometry::smallOverflowIsTolerated() {
    // items overflow viewport by ~5px on the right; the 10px tolerant inset
    // should swallow it as "essentially on-screen".
    QRectF items(0, 0, 805, 595);
    QRectF viewport(0, 0, 800, 600);
    QCOMPARE(MindMapView::rectFullyVisibleIn(items, viewport, 10.0), true);

    // With zero tolerance the same case should report off-screen.
    QCOMPARE(MindMapView::rectFullyVisibleIn(items, viewport, 0.0), false);
}

void tst_ViewGeometry::itemsExactlyAtViewportEdgeIsVisible() {
    QRectF items(0, 0, 800, 600);
    QRectF viewport(0, 0, 800, 600);
    QCOMPARE(MindMapView::rectFullyVisibleIn(items, viewport, 0.0), true);
}

void tst_ViewGeometry::fitMarginTracksNodeWidth() {
    // Within the clamp range the fit margin IS the node width, so the fitted
    // view keeps one node's width of breathing room around the content.
    QCOMPARE(MindMapView::fitMarginForNodeWidth(150.0), 150.0);
    QCOMPARE(MindMapView::fitMarginForNodeWidth(80.0), 80.0);
    QCOMPARE(MindMapView::fitMarginForNodeWidth(400.0), 400.0);
}

void tst_ViewGeometry::fitMarginFloorsAtMinimum() {
    // No scene / no root yet (width 0), or a degenerate tiny node, still
    // gets the legacy 80px of breathing room.
    QCOMPARE(MindMapView::fitMarginForNodeWidth(0.0), 80.0);
    QCOMPARE(MindMapView::fitMarginForNodeWidth(12.0), 80.0);
    QCOMPARE(MindMapView::fitMarginForNodeWidth(-5.0), 80.0);
}

void tst_ViewGeometry::fitMarginCapsAtMaximum() {
    // A single very long root topic must not surround the map with a huge
    // whitespace frame.
    QCOMPARE(MindMapView::fitMarginForNodeWidth(1200.0), 400.0);
}

QTEST_APPLESS_MAIN(tst_ViewGeometry)
#include "tst_ViewGeometry.moc"
