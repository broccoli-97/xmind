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

QTEST_APPLESS_MAIN(tst_ViewGeometry)
#include "tst_ViewGeometry.moc"
