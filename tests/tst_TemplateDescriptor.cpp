#include "core/TemplateDescriptor.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QTest>

class tst_TemplateDescriptor : public QObject {
    Q_OBJECT

private slots:
    // ---- TemplateColorScheme ----
    void colorSchemeRoundTrip();
    void colorSchemeDefaults();

    // ---- TemplateNodeStyle ----
    void nodeStyleRoundTrip();
    void nodeStyleDefaults();

    // ---- TemplateEdgeStyle ----
    void edgeStyleRoundTrip();
    void edgeStyleDefaults();

    // ---- TemplateLayoutConfig ----
    void layoutConfigRoundTrip();
    void layoutConfigDefaults();

    // ---- TemplateContentNode ----
    void contentNodeRoundTrip();
    void contentNodeEmpty();
    void contentNodeNested();

    // ---- TemplateDescriptor (full) ----
    void descriptorRoundTrip();
    void descriptorRootStyleSynthesizedFromRootShape();
    void descriptorRootStyleExplicitOverlay();

    // ---- New style knobs ----
    void nodeStyleFillModeRoundTrip();
    void nodeStyleInheritsFromBase();
    void edgeStyleNewFieldsRoundTrip();
};

// ---------------------------------------------------------------------------
// TemplateColorScheme
// ---------------------------------------------------------------------------

void tst_TemplateDescriptor::colorSchemeRoundTrip() {
    TemplateColorScheme cs;
    cs.canvasBackground = QColor("#112233");
    cs.canvasGridDot = QColor("#445566");
    cs.nodePalette[0] = QColor("#AA0000");
    cs.nodePalette[1] = QColor("#00BB00");
    cs.nodePalette[2] = QColor("#0000CC");
    cs.nodePalette[3] = QColor("#DD0000");
    cs.nodePalette[4] = QColor("#00EE00");
    cs.nodePalette[5] = QColor("#0000FF");
    cs.nodeShadow = QColor(10, 20, 30, 40);
    cs.nodeSelectionBorder = QColor("#FF6600");
    cs.nodeText = QColor("#FFFFFF");
    cs.edgeLightenFactor = 130;
    cs.exportBackground = QColor("#FAFAFA");

    QJsonObject json = cs.toJson();
    TemplateColorScheme cs2 = TemplateColorScheme::fromJson(json);

    QCOMPARE(cs2.canvasBackground, cs.canvasBackground);
    QCOMPARE(cs2.canvasGridDot, cs.canvasGridDot);
    for (int i = 0; i < 6; ++i)
        QCOMPARE(cs2.nodePalette[i], cs.nodePalette[i]);
    QCOMPARE(cs2.nodeShadow, cs.nodeShadow);
    QCOMPARE(cs2.nodeSelectionBorder, cs.nodeSelectionBorder);
    QCOMPARE(cs2.nodeText, cs.nodeText);
    QCOMPARE(cs2.edgeLightenFactor, 130);
    QCOMPARE(cs2.exportBackground, cs.exportBackground);
}

void tst_TemplateDescriptor::colorSchemeDefaults() {
    // No base: missing keys leave the struct's natural defaults
    // (uninitialized QColors and the edgeLightenFactor=140 from the header).
    QJsonObject empty;
    TemplateColorScheme cs = TemplateColorScheme::fromJson(empty);
    QCOMPARE(cs.edgeLightenFactor, 140);
    QVERIFY(!cs.canvasBackground.isValid());

    // With a base, missing keys fall back to the base's values.
    TemplateColorScheme base;
    base.canvasBackground = QColor("#F8F9FA");
    base.nodeText = QColor("#FFFFFF");
    base.edgeLightenFactor = 120;
    TemplateColorScheme inherited = TemplateColorScheme::fromJson(empty, base);
    QCOMPARE(inherited.canvasBackground, QColor("#F8F9FA"));
    QCOMPARE(inherited.nodeText, QColor("#FFFFFF"));
    QCOMPARE(inherited.edgeLightenFactor, 120);
}

// ---------------------------------------------------------------------------
// TemplateNodeStyle
// ---------------------------------------------------------------------------

void tst_TemplateDescriptor::nodeStyleRoundTrip() {
    TemplateNodeStyle s;
    s.borderRadius = 8.0;
    s.padding = 12.0;
    s.minWidth = 80.0;
    s.maxWidth = 250.0;
    s.shape = "none";
    s.rootShape = "underline";
    s.drawShadow = false;
    s.paletteSource = "branch";

    QJsonObject json = s.toJson();
    TemplateNodeStyle s2 = TemplateNodeStyle::fromJson(json);

    QCOMPARE(s2.borderRadius, 8.0);
    QCOMPARE(s2.padding, 12.0);
    QCOMPARE(s2.minWidth, 80.0);
    QCOMPARE(s2.maxWidth, 250.0);
    QCOMPARE(s2.shape, QString("none"));
    QCOMPARE(s2.rootShape, QString("underline"));
    QCOMPARE(s2.drawShadow, false);
    QCOMPARE(s2.paletteSource, QString("branch"));
}

void tst_TemplateDescriptor::nodeStyleDefaults() {
    TemplateNodeStyle s = TemplateNodeStyle::fromJson(QJsonObject());

    QCOMPARE(s.borderRadius, 10.0);
    QCOMPARE(s.padding, 16.0);
    QCOMPARE(s.minWidth, 120.0);
    QCOMPARE(s.maxWidth, 300.0);
    QCOMPARE(s.shape, QString("roundedRect"));
    QCOMPARE(s.rootShape, QString());
    QCOMPARE(s.drawShadow, true);
    QCOMPARE(s.paletteSource, QString("level"));
}

// ---------------------------------------------------------------------------
// TemplateEdgeStyle
// ---------------------------------------------------------------------------

void tst_TemplateDescriptor::edgeStyleRoundTrip() {
    TemplateEdgeStyle e;
    e.width = 3.5;
    e.colorSource = "branch";
    e.anchor = "baseline";

    QJsonObject json = e.toJson();
    TemplateEdgeStyle e2 = TemplateEdgeStyle::fromJson(json);

    QCOMPARE(e2.width, 3.5);
    QCOMPARE(e2.colorSource, QString("branch"));
    QCOMPARE(e2.anchor, QString("baseline"));
}

void tst_TemplateDescriptor::edgeStyleDefaults() {
    TemplateEdgeStyle e = TemplateEdgeStyle::fromJson(QJsonObject());
    QCOMPARE(e.width, 2.5);
    QCOMPARE(e.colorSource, QString("target"));
    QCOMPARE(e.anchor, QString("center"));
}

// ---------------------------------------------------------------------------
// TemplateLayoutConfig
// ---------------------------------------------------------------------------

void tst_TemplateDescriptor::layoutConfigRoundTrip() {
    TemplateLayoutConfig c;
    c.algorithm = "topdown";
    c.depthSpacing = 80.0;
    c.spreadSpacing = 20.0;

    QJsonObject json = c.toJson();
    TemplateLayoutConfig c2 = TemplateLayoutConfig::fromJson(json);

    QCOMPARE(c2.algorithm, QString("topdown"));
    QCOMPARE(c2.depthSpacing, 80.0);
    QCOMPARE(c2.spreadSpacing, 20.0);
}

void tst_TemplateDescriptor::layoutConfigDefaults() {
    TemplateLayoutConfig c = TemplateLayoutConfig::fromJson(QJsonObject());

    QCOMPARE(c.algorithm, QString("bilateral"));
    QCOMPARE(c.depthSpacing, 100.0);
    QCOMPARE(c.spreadSpacing, 16.0);
}

// ---------------------------------------------------------------------------
// TemplateContentNode
// ---------------------------------------------------------------------------

void tst_TemplateDescriptor::contentNodeRoundTrip() {
    TemplateContentNode n;
    n.text = "Root";
    n.children = {{"Child 1", {}}, {"Child 2", {}}};

    QJsonObject json = n.toJson();
    TemplateContentNode n2 = TemplateContentNode::fromJson(json);

    QCOMPARE(n2.text, QString("Root"));
    QCOMPARE(n2.children.size(), 2);
    QCOMPARE(n2.children[0].text, QString("Child 1"));
    QCOMPARE(n2.children[1].text, QString("Child 2"));
}

void tst_TemplateDescriptor::contentNodeEmpty() {
    QJsonObject empty;
    TemplateContentNode n = TemplateContentNode::fromJson(empty);
    QCOMPARE(n.text, QString("Topic"));
    QVERIFY(n.children.isEmpty());
}

void tst_TemplateDescriptor::contentNodeNested() {
    TemplateContentNode n;
    n.text = "Root";
    n.children = {{"A", {{"A1", {}}, {"A2", {}}}}};

    QJsonObject json = n.toJson();
    TemplateContentNode n2 = TemplateContentNode::fromJson(json);

    QCOMPARE(n2.children.size(), 1);
    QCOMPARE(n2.children[0].text, QString("A"));
    QCOMPARE(n2.children[0].children.size(), 2);
    QCOMPARE(n2.children[0].children[0].text, QString("A1"));
    QCOMPARE(n2.children[0].children[1].text, QString("A2"));
}

// ---------------------------------------------------------------------------
// TemplateDescriptor (full)
// ---------------------------------------------------------------------------

void tst_TemplateDescriptor::descriptorRoundTrip() {
    TemplateDescriptor td;
    td.id = "test.template";
    td.name = "Test Template";
    td.description = "A test template";
    td.layout = {"righttree", 90.0, 12.0};
    td.nodeStyle.borderRadius = 5.0;
    td.edgeStyle.width = 1.5;
    td.content = {"Root", {{"A", {}}, {"B", {}}}};

    // Set some distinguishing colors
    td.lightColors.canvasBackground = QColor("#AABBCC");
    td.darkColors.canvasBackground = QColor("#112233");

    QJsonObject json = td.toJson();
    TemplateDescriptor td2 = TemplateDescriptor::fromJson(json);

    QCOMPARE(td2.id, td.id);
    QCOMPARE(td2.name, td.name);
    QCOMPARE(td2.description, td.description);
    QCOMPARE(td2.layout.algorithm, QString("righttree"));
    QCOMPARE(td2.layout.depthSpacing, 90.0);
    QCOMPARE(td2.nodeStyle.borderRadius, 5.0);
    QCOMPARE(td2.edgeStyle.width, 1.5);
    QCOMPARE(td2.content.text, QString("Root"));
    QCOMPARE(td2.content.children.size(), 2);
    QCOMPARE(td2.lightColors.canvasBackground, QColor("#AABBCC"));
    QCOMPARE(td2.darkColors.canvasBackground, QColor("#112233"));
}

void tst_TemplateDescriptor::descriptorRootStyleSynthesizedFromRootShape() {
    // Legacy templates declare `rootShape` directly on nodeStyle; the loader
    // must synthesize a hasRootStyle override with only the shape changed.
    QJsonObject json;
    json["id"] = "test.legacy";
    QJsonObject ns;
    ns["shape"] = "roundedRect";
    ns["rootShape"] = "underline";
    json["nodeStyle"] = ns;

    TemplateDescriptor td = TemplateDescriptor::fromJson(json);
    QVERIFY(td.hasRootStyle);
    QCOMPARE(td.nodeStyle.shape, QString("roundedRect"));
    QCOMPARE(td.rootStyle.shape, QString("underline"));
    // Sibling fields inherit from the base nodeStyle.
    QCOMPARE(td.rootStyle.padding, td.nodeStyle.padding);
}

void tst_TemplateDescriptor::descriptorRootStyleExplicitOverlay() {
    // Explicit rootStyle block overlays only the keys it sets.
    QJsonObject json;
    json["id"] = "test.root";
    QJsonObject ns;
    ns["padding"] = 10.0;
    ns["borderWidth"] = 1.5;
    ns["fillMode"] = "tinted";
    json["nodeStyle"] = ns;

    QJsonObject root;
    root["padding"] = 20.0;          // override
    root["borderWidth"] = 3.0;       // override
    // fillMode omitted → should inherit from base nodeStyle ("tinted")
    json["rootStyle"] = root;

    TemplateDescriptor td = TemplateDescriptor::fromJson(json);
    QVERIFY(td.hasRootStyle);
    QCOMPARE(td.rootStyle.padding, 20.0);
    QCOMPARE(td.rootStyle.borderWidth, 3.0);
    QCOMPARE(td.rootStyle.fillMode, QString("tinted"));
}

void tst_TemplateDescriptor::nodeStyleFillModeRoundTrip() {
    TemplateNodeStyle s;
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
    TemplateNodeStyle s2 = TemplateNodeStyle::fromJson(json);

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

void tst_TemplateDescriptor::nodeStyleInheritsFromBase() {
    // Empty JSON with a populated base → every field comes from base.
    TemplateNodeStyle base;
    base.borderWidth = 1.7;
    base.fillMode = "tinted";
    base.fillAlpha = 0.3;
    base.padding = 14.0;

    TemplateNodeStyle s = TemplateNodeStyle::fromJson(QJsonObject(), base);
    QCOMPARE(s.borderWidth, 1.7);
    QCOMPARE(s.fillMode, QString("tinted"));
    QCOMPARE(s.fillAlpha, 0.3);
    QCOMPARE(s.padding, 14.0);

    // Partial JSON → only specified keys overlay; rest inherits.
    QJsonObject partial;
    partial["padding"] = 22.0;
    TemplateNodeStyle s2 = TemplateNodeStyle::fromJson(partial, base);
    QCOMPARE(s2.padding, 22.0);             // overridden
    QCOMPARE(s2.fillMode, QString("tinted")); // inherited
    QCOMPARE(s2.fillAlpha, 0.3);             // inherited
}

void tst_TemplateDescriptor::edgeStyleNewFieldsRoundTrip() {
    TemplateEdgeStyle e;
    e.curvature = 0.3;
    e.dashStyle = "dashed";
    e.colorModifier = "same";
    e.lineCap = "flat";

    QJsonObject json = e.toJson();
    TemplateEdgeStyle e2 = TemplateEdgeStyle::fromJson(json);

    QCOMPARE(e2.curvature, 0.3);
    QCOMPARE(e2.dashStyle, QString("dashed"));
    QCOMPARE(e2.colorModifier, QString("same"));
    QCOMPARE(e2.lineCap, QString("flat"));
}

QTEST_APPLESS_MAIN(tst_TemplateDescriptor)
#include "tst_TemplateDescriptor.moc"
