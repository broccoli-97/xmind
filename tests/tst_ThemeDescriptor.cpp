#include "core/ThemeDescriptor.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QTest>

class tst_ThemeDescriptor : public QObject {
    Q_OBJECT

private slots:
    void colorSchemeRoundTrip();
    void colorSchemeInheritsFromBase();
    void nodeStyleRoundTrip();
    void nodeStyleDefaults();
    void nodeStyleFillModeRoundTrip();
    void nodeStyleInheritsFromBase();
    void edgeStyleRoundTrip();
    void edgeStyleDefaults();
    void sketchFieldsRoundTrip();
    void sketchFieldsDefaultToClean();
    void descriptorRootStyleSynthesizedFromRootShape();
    void descriptorRootStyleExplicitOverlay();
    void descriptorRoundTrip();
};

void tst_ThemeDescriptor::colorSchemeRoundTrip() {
    ThemeColorScheme cs;
    cs.canvasBackground = QColor("#112233");
    cs.canvasGridDot = QColor("#445566");
    cs.nodePalette[0] = QColor("#AA0000");
    cs.nodePalette[5] = QColor("#0000FF");
    cs.nodeShadow = QColor(10, 20, 30, 40);
    cs.nodeSelectionBorder = QColor("#FF6600");
    cs.nodeText = QColor("#FFFFFF");
    cs.edgeLightenFactor = 130;
    cs.exportBackground = QColor("#FAFAFA");

    QJsonObject json = cs.toJson();
    ThemeColorScheme cs2 = ThemeColorScheme::fromJson(json);

    QCOMPARE(cs2.canvasBackground, cs.canvasBackground);
    QCOMPARE(cs2.canvasGridDot, cs.canvasGridDot);
    QCOMPARE(cs2.nodePalette[0], cs.nodePalette[0]);
    QCOMPARE(cs2.nodePalette[5], cs.nodePalette[5]);
    QCOMPARE(cs2.nodeText, cs.nodeText);
    QCOMPARE(cs2.edgeLightenFactor, 130);
    QCOMPARE(cs2.exportBackground, cs.exportBackground);
}

void tst_ThemeDescriptor::colorSchemeInheritsFromBase() {
    ThemeColorScheme base;
    base.canvasBackground = QColor("#F8F9FA");
    base.nodeText = QColor("#FFFFFF");
    base.edgeLightenFactor = 120;

    ThemeColorScheme inherited = ThemeColorScheme::fromJson(QJsonObject(), base);
    QCOMPARE(inherited.canvasBackground, QColor("#F8F9FA"));
    QCOMPARE(inherited.nodeText, QColor("#FFFFFF"));
    QCOMPARE(inherited.edgeLightenFactor, 120);

    QJsonObject partial;
    partial["canvasBackground"] = "#000000";
    ThemeColorScheme overlaid = ThemeColorScheme::fromJson(partial, base);
    QCOMPARE(overlaid.canvasBackground, QColor("#000000"));
    QCOMPARE(overlaid.nodeText, QColor("#FFFFFF"));        // inherited
    QCOMPARE(overlaid.edgeLightenFactor, 120);             // inherited
}

void tst_ThemeDescriptor::nodeStyleRoundTrip() {
    ThemeNodeStyle s;
    s.borderRadius = 8.0;
    s.padding = 12.0;
    s.minWidth = 80.0;
    s.maxWidth = 250.0;
    s.shape = "none";
    s.rootShape = "underline";
    s.drawShadow = false;
    s.paletteSource = "branch";

    QJsonObject json = s.toJson();
    ThemeNodeStyle s2 = ThemeNodeStyle::fromJson(json);

    QCOMPARE(s2.borderRadius, 8.0);
    QCOMPARE(s2.padding, 12.0);
    QCOMPARE(s2.minWidth, 80.0);
    QCOMPARE(s2.maxWidth, 250.0);
    QCOMPARE(s2.shape, QString("none"));
    QCOMPARE(s2.rootShape, QString("underline"));
    QCOMPARE(s2.drawShadow, false);
    QCOMPARE(s2.paletteSource, QString("branch"));
}

void tst_ThemeDescriptor::nodeStyleDefaults() {
    ThemeNodeStyle s = ThemeNodeStyle::fromJson(QJsonObject());
    QCOMPARE(s.borderRadius, 10.0);
    QCOMPARE(s.padding, 16.0);
    QCOMPARE(s.minWidth, 120.0);
    QCOMPARE(s.maxWidth, 300.0);
    QCOMPARE(s.shape, QString("roundedRect"));
    QCOMPARE(s.fillMode, QString("solid"));
    QCOMPARE(s.drawShadow, true);
    QCOMPARE(s.paletteSource, QString("level"));
}

void tst_ThemeDescriptor::nodeStyleFillModeRoundTrip() {
    ThemeNodeStyle s;
    s.fillMode = "outlined";
    s.fillAlpha = 0.22;
    s.borderWidth = 2.0;
    s.borderColorSource = "fixed";
    s.shadowLayers = 3;
    s.shadowSpread = 6.0;
    s.shadowOffsetY = 2.0;
    s.shadowOpacity = 0.5;
    s.selectionWidth = 4.0;

    QJsonObject json = s.toJson();
    ThemeNodeStyle s2 = ThemeNodeStyle::fromJson(json);

    QCOMPARE(s2.fillMode, QString("outlined"));
    QCOMPARE(s2.fillAlpha, 0.22);
    QCOMPARE(s2.borderWidth, 2.0);
    QCOMPARE(s2.borderColorSource, QString("fixed"));
    QCOMPARE(s2.shadowLayers, 3);
    QCOMPARE(s2.shadowSpread, 6.0);
    QCOMPARE(s2.shadowOffsetY, 2.0);
    QCOMPARE(s2.shadowOpacity, 0.5);
    QCOMPARE(s2.selectionWidth, 4.0);
}

void tst_ThemeDescriptor::nodeStyleInheritsFromBase() {
    ThemeNodeStyle base;
    base.borderWidth = 1.7;
    base.fillMode = "tinted";
    base.fillAlpha = 0.3;
    base.padding = 14.0;

    ThemeNodeStyle s = ThemeNodeStyle::fromJson(QJsonObject(), base);
    QCOMPARE(s.borderWidth, 1.7);
    QCOMPARE(s.fillMode, QString("tinted"));
    QCOMPARE(s.fillAlpha, 0.3);

    QJsonObject partial;
    partial["padding"] = 22.0;
    ThemeNodeStyle s2 = ThemeNodeStyle::fromJson(partial, base);
    QCOMPARE(s2.padding, 22.0);
    QCOMPARE(s2.fillMode, QString("tinted"));
    QCOMPARE(s2.fillAlpha, 0.3);
}

void tst_ThemeDescriptor::edgeStyleRoundTrip() {
    ThemeEdgeStyle e;
    e.width = 3.5;
    e.colorSource = "branch";
    e.anchor = "baseline";
    e.curvature = 0.3;
    e.dashStyle = "dashed";
    e.colorModifier = "same";
    e.lineCap = "flat";

    QJsonObject json = e.toJson();
    ThemeEdgeStyle e2 = ThemeEdgeStyle::fromJson(json);

    QCOMPARE(e2.width, 3.5);
    QCOMPARE(e2.colorSource, QString("branch"));
    QCOMPARE(e2.anchor, QString("baseline"));
    QCOMPARE(e2.curvature, 0.3);
    QCOMPARE(e2.dashStyle, QString("dashed"));
    QCOMPARE(e2.colorModifier, QString("same"));
    QCOMPARE(e2.lineCap, QString("flat"));
}

void tst_ThemeDescriptor::edgeStyleDefaults() {
    ThemeEdgeStyle e = ThemeEdgeStyle::fromJson(QJsonObject());
    QCOMPARE(e.width, 2.5);
    QCOMPARE(e.colorSource, QString("target"));
    QCOMPARE(e.anchor, QString("center"));
    QCOMPARE(e.curvature, 0.5);
    QCOMPARE(e.dashStyle, QString("solid"));
    QCOMPARE(e.colorModifier, QString("lighten"));
    QCOMPARE(e.lineCap, QString("round"));
}

void tst_ThemeDescriptor::sketchFieldsRoundTrip() {
    ThemeNodeStyle ns;
    ns.roughness = 0.65;
    ns.strokePasses = 2;
    ns.fontFamily = "Caveat";
    ns.fontPointSize = 17.0;
    QJsonObject nsJson = ns.toJson();
    ThemeNodeStyle ns2 = ThemeNodeStyle::fromJson(nsJson);
    QCOMPARE(ns2.roughness, 0.65);
    QCOMPARE(ns2.strokePasses, 2);
    QCOMPARE(ns2.fontFamily, QString("Caveat"));
    QCOMPARE(ns2.fontPointSize, 17.0);

    ThemeEdgeStyle es;
    es.roughness = 0.55;
    es.strokePasses = 3;
    QJsonObject esJson = es.toJson();
    ThemeEdgeStyle es2 = ThemeEdgeStyle::fromJson(esJson);
    QCOMPARE(es2.roughness, 0.55);
    QCOMPARE(es2.strokePasses, 3);
}

void tst_ThemeDescriptor::sketchFieldsDefaultToClean() {
    // Existing themes that don't mention the sketch fields must still render
    // as crisp shapes — guard against accidental default changes.
    ThemeNodeStyle ns = ThemeNodeStyle::fromJson(QJsonObject());
    QCOMPARE(ns.roughness, 0.0);
    QCOMPARE(ns.strokePasses, 1);
    QVERIFY(ns.fontFamily.isEmpty());
    QCOMPARE(ns.fontPointSize, 0.0);

    ThemeEdgeStyle es = ThemeEdgeStyle::fromJson(QJsonObject());
    QCOMPARE(es.roughness, 0.0);
    QCOMPARE(es.strokePasses, 1);
}

void tst_ThemeDescriptor::descriptorRootStyleSynthesizedFromRootShape() {
    QJsonObject json;
    json["id"] = "test.legacy";
    QJsonObject ns;
    ns["shape"] = "roundedRect";
    ns["rootShape"] = "underline";
    json["nodeStyle"] = ns;

    ThemeDescriptor td = ThemeDescriptor::fromJson(json);
    QVERIFY(td.hasRootStyle);
    QCOMPARE(td.nodeStyle.shape, QString("roundedRect"));
    QCOMPARE(td.rootStyle.shape, QString("underline"));
    QCOMPARE(td.rootStyle.padding, td.nodeStyle.padding);
}

void tst_ThemeDescriptor::descriptorRootStyleExplicitOverlay() {
    QJsonObject json;
    json["id"] = "test.root";
    QJsonObject ns;
    ns["padding"] = 10.0;
    ns["borderWidth"] = 1.5;
    ns["fillMode"] = "tinted";
    json["nodeStyle"] = ns;

    QJsonObject root;
    root["padding"] = 20.0;
    root["borderWidth"] = 3.0;
    json["rootStyle"] = root;

    ThemeDescriptor td = ThemeDescriptor::fromJson(json);
    QVERIFY(td.hasRootStyle);
    QCOMPARE(td.rootStyle.padding, 20.0);
    QCOMPARE(td.rootStyle.borderWidth, 3.0);
    QCOMPARE(td.rootStyle.fillMode, QString("tinted"));
}

void tst_ThemeDescriptor::descriptorRoundTrip() {
    ThemeDescriptor td;
    td.id = "test.theme";
    td.name = "Test Theme";
    td.description = "A theme";
    td.nodeStyle.fillMode = "tinted";
    td.nodeStyle.fillAlpha = 0.45;
    td.edgeStyle.width = 1.5;
    td.lightColors.canvasBackground = QColor("#AABBCC");
    td.darkColors.canvasBackground = QColor("#112233");
    td.backgroundPattern = "lines";

    QJsonObject json = td.toJson();
    ThemeDescriptor td2 = ThemeDescriptor::fromJson(json);

    QCOMPARE(td2.id, td.id);
    QCOMPARE(td2.name, td.name);
    QCOMPARE(td2.nodeStyle.fillMode, QString("tinted"));
    QCOMPARE(td2.nodeStyle.fillAlpha, 0.45);
    QCOMPARE(td2.edgeStyle.width, 1.5);
    QCOMPARE(td2.lightColors.canvasBackground, QColor("#AABBCC"));
    QCOMPARE(td2.darkColors.canvasBackground, QColor("#112233"));
    QCOMPARE(td2.backgroundPattern, QString("lines"));
}

QTEST_APPLESS_MAIN(tst_ThemeDescriptor)
#include "tst_ThemeDescriptor.moc"
