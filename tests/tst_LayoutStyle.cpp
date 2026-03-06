#include "layout/LayoutStyle.h"

#include <QTest>

class tst_LayoutStyle : public QObject {
    Q_OBJECT

private slots:
    void styleToName();
    void nameToStyle();
    void unknownNameFallsToBilateral();
};

void tst_LayoutStyle::styleToName() {
    QCOMPARE(layoutStyleToAlgorithmName(LayoutStyle::Bilateral), QString("bilateral"));
    QCOMPARE(layoutStyleToAlgorithmName(LayoutStyle::TopDown), QString("topdown"));
    QCOMPARE(layoutStyleToAlgorithmName(LayoutStyle::RightTree), QString("righttree"));
}

void tst_LayoutStyle::nameToStyle() {
    QCOMPARE(algorithmNameToLayoutStyle("bilateral"), LayoutStyle::Bilateral);
    QCOMPARE(algorithmNameToLayoutStyle("topdown"), LayoutStyle::TopDown);
    QCOMPARE(algorithmNameToLayoutStyle("righttree"), LayoutStyle::RightTree);
}

void tst_LayoutStyle::unknownNameFallsToBilateral() {
    QCOMPARE(algorithmNameToLayoutStyle("nonexistent"), LayoutStyle::Bilateral);
    QCOMPARE(algorithmNameToLayoutStyle(""), LayoutStyle::Bilateral);
}

QTEST_APPLESS_MAIN(tst_LayoutStyle)
#include "tst_LayoutStyle.moc"
