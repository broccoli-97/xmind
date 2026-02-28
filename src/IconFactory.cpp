#include "IconFactory.h"
#include "ThemeManager.h"

#include <QPainter>
#include <QPainterPath>

QIcon IconFactory::makeToolIcon(const QString& name) {
    QPixmap pix(32, 32);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);

    QColor baseColor = ThemeManager::colors().iconBaseColor;
    QPen pen(baseColor, 2.0);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);

    if (name == "add-child") {
        p.drawLine(16, 8, 16, 24);
        p.drawLine(8, 16, 24, 16);
    } else if (name == "add-sibling") {
        p.drawLine(4, 10, 14, 10);
        p.drawLine(4, 22, 14, 22);
        p.drawLine(24, 14, 24, 22);
        p.drawLine(20, 18, 28, 18);
    } else if (name == "delete") {
        p.drawLine(10, 10, 10, 26);
        p.drawLine(22, 10, 22, 26);
        p.drawLine(10, 26, 22, 26);
        p.drawLine(8, 10, 24, 10);
        p.drawLine(13, 6, 19, 6);
        p.drawLine(13, 6, 13, 10);
        p.drawLine(19, 6, 19, 10);
        p.drawLine(14, 13, 14, 23);
        p.drawLine(18, 13, 18, 23);
    } else if (name == "auto-layout") {
        p.drawRect(12, 2, 8, 6);
        p.drawRect(2, 22, 8, 6);
        p.drawRect(22, 22, 8, 6);
        p.drawLine(16, 8, 16, 14);
        p.drawLine(6, 14, 26, 14);
        p.drawLine(6, 14, 6, 22);
        p.drawLine(26, 14, 26, 22);
    } else if (name == "zoom") {
        p.drawEllipse(8, 6, 16, 16);
        QPen thickPen(ThemeManager::colors().iconBaseColor, 3.0);
        p.setPen(thickPen);
        p.drawLine(21, 20, 27, 27);
    } else if (name == "zoom-in") {
        p.drawEllipse(6, 4, 18, 18);
        p.drawLine(15, 9, 15, 17);
        p.drawLine(11, 13, 19, 13);
        QPen thickPen(ThemeManager::colors().iconBaseColor, 3.0);
        p.setPen(thickPen);
        p.drawLine(22, 21, 28, 27);
    } else if (name == "zoom-out") {
        p.drawEllipse(6, 4, 18, 18);
        p.drawLine(11, 13, 19, 13);
        QPen thickPen(ThemeManager::colors().iconBaseColor, 3.0);
        p.setPen(thickPen);
        p.drawLine(22, 21, 28, 27);
    } else if (name == "undo") {
        QPen curvePen(baseColor, 2.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        p.setPen(curvePen);
        // Smooth swooping curve from bottom-right up and to the left
        QPainterPath curve;
        curve.moveTo(25, 23);
        curve.cubicTo(27, 5, 14, 12, 7, 12);
        p.drawPath(curve);
        // Chevron arrowhead pointing left
        p.drawLine(7, 12, 12, 7);
        p.drawLine(7, 12, 12, 17);
    } else if (name == "redo") {
        QPen curvePen(baseColor, 2.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        p.setPen(curvePen);
        // Smooth swooping curve from bottom-left up and to the right
        QPainterPath curve;
        curve.moveTo(7, 23);
        curve.cubicTo(5, 5, 18, 12, 25, 12);
        p.drawPath(curve);
        // Chevron arrowhead pointing right
        p.drawLine(25, 12, 20, 7);
        p.drawLine(25, 12, 20, 17);
    } else if (name == "fit-view") {
        p.drawLine(4, 10, 4, 4);
        p.drawLine(4, 4, 10, 4);
        p.drawLine(22, 4, 28, 4);
        p.drawLine(28, 4, 28, 10);
        p.drawLine(4, 22, 4, 28);
        p.drawLine(4, 28, 10, 28);
        p.drawLine(28, 22, 28, 28);
        p.drawLine(22, 28, 28, 28);
    } else if (name == "export") {
        p.drawRect(8, 14, 16, 14);
        p.drawLine(16, 16, 16, 4);
        p.drawLine(12, 8, 16, 4);
        p.drawLine(20, 8, 16, 4);
    } else if (name == "sidebar") {
        p.drawRect(4, 4, 24, 24);
        p.drawLine(14, 4, 14, 28);
        p.drawLine(17, 10, 25, 10);
        p.drawLine(17, 16, 25, 16);
        p.drawLine(17, 22, 25, 22);
    } else if (name == "toolbar") {
        p.drawRect(4, 10, 24, 12);
        p.drawLine(10, 10, 10, 22);
        p.drawLine(16, 10, 16, 22);
        p.drawLine(22, 10, 22, 22);
    } else if (name == "close-panel") {
        p.drawLine(10, 10, 22, 22);
        p.drawLine(22, 10, 10, 22);
    }

    p.end();

    // Build a disabled pixmap with reduced opacity
    QPixmap disabledPix(32, 32);
    disabledPix.fill(Qt::transparent);
    QPainter dp(&disabledPix);
    dp.setOpacity(0.3);
    dp.drawPixmap(0, 0, pix);
    dp.end();

    QIcon icon;
    icon.addPixmap(pix, QIcon::Normal);
    icon.addPixmap(disabledPix, QIcon::Disabled);
    return icon;
}

QIcon IconFactory::makeTabIcon(int layoutStyleIndex) {
    QPixmap pix(16, 16);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);

    QColor base = ThemeManager::colors().iconBaseColor;
    QPen pen(base, 1.2);
    p.setPen(pen);
    p.setBrush(base);

    if (layoutStyleIndex == 0) {
        // Bilateral / Mind Map: central rect + 4 radiating lines
        p.drawRect(6, 6, 4, 4);
        p.setBrush(Qt::NoBrush);
        p.drawLine(6, 8, 2, 4);
        p.drawLine(6, 8, 2, 12);
        p.drawLine(10, 8, 14, 4);
        p.drawLine(10, 8, 14, 12);
    } else if (layoutStyleIndex == 1) {
        // TopDown / Org Chart: top node + connector + 3 child nodes
        p.drawRect(6, 1, 4, 3);
        p.drawRect(1, 11, 3, 3);
        p.drawRect(6, 11, 3, 3);
        p.drawRect(12, 11, 3, 3);
        p.setBrush(Qt::NoBrush);
        p.drawLine(8, 4, 8, 7);
        p.drawLine(3, 7, 13, 7);
        p.drawLine(3, 7, 3, 11);
        p.drawLine(8, 7, 8, 11);
        p.drawLine(13, 7, 13, 11);
    } else if (layoutStyleIndex == 2) {
        // RightTree / Project Plan: left node + branches right to sub-nodes
        p.drawRect(1, 6, 4, 4);
        p.drawRect(9, 2, 4, 3);
        p.drawRect(9, 11, 4, 3);
        p.setBrush(Qt::NoBrush);
        p.drawLine(5, 8, 7, 8);
        p.drawLine(7, 4, 7, 12);
        p.drawLine(7, 4, 9, 4);
        p.drawLine(7, 12, 9, 12);
    } else {
        // -1 / Start Page: document outline with fold corner + text lines
        p.setBrush(Qt::NoBrush);
        // Document outline
        p.drawLine(3, 1, 10, 1);
        p.drawLine(10, 1, 13, 4);
        p.drawLine(13, 4, 13, 14);
        p.drawLine(13, 14, 3, 14);
        p.drawLine(3, 14, 3, 1);
        // Fold corner
        p.drawLine(10, 1, 10, 4);
        p.drawLine(10, 4, 13, 4);
        // Text lines
        p.drawLine(5, 7, 11, 7);
        p.drawLine(5, 9, 11, 9);
        p.drawLine(5, 11, 9, 11);
    }

    p.end();
    return QIcon(pix);
}

QPixmap IconFactory::makeTemplatePreview(int index, int width, int height) {
    const auto& c = ThemeManager::colors();
    QPixmap pix(width, height);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);

    qreal sx = width / 120.0;
    qreal sy = height / 80.0;
    p.scale(sx, sy);

    QPen linePen(c.previewLine, 1.5);
    QPen nodePen(c.previewNodeBorder, 1.5);
    QBrush nodeBrush(c.previewNodeFill);

    if (index == 0) {
        p.setPen(nodePen);
        p.setBrush(nodeBrush);
        p.drawRoundedRect(42, 26, 36, 18, 4, 4);
        p.setPen(linePen);
        p.drawLine(60, 26, 90, 10);
        p.drawLine(60, 44, 90, 58);
        p.drawLine(42, 26, 18, 10);
        p.drawLine(42, 44, 18, 58);
        p.setPen(nodePen);
        p.drawRoundedRect(84, 4, 28, 12, 3, 3);
        p.drawRoundedRect(84, 52, 28, 12, 3, 3);
        p.drawRoundedRect(4, 4, 28, 12, 3, 3);
        p.drawRoundedRect(4, 52, 28, 12, 3, 3);
    } else if (index == 1) {
        p.setPen(nodePen);
        p.setBrush(nodeBrush);
        p.drawRoundedRect(42, 4, 36, 14, 3, 3);
        p.drawRoundedRect(8, 46, 28, 14, 3, 3);
        p.drawRoundedRect(46, 46, 28, 14, 3, 3);
        p.drawRoundedRect(84, 46, 28, 14, 3, 3);
        p.setPen(linePen);
        p.drawLine(60, 18, 60, 28);
        p.drawLine(22, 28, 98, 28);
        p.drawLine(22, 28, 22, 46);
        p.drawLine(60, 28, 60, 46);
        p.drawLine(98, 28, 98, 46);
    } else if (index == 2) {
        p.setPen(nodePen);
        p.setBrush(nodeBrush);
        p.drawRoundedRect(4, 26, 28, 14, 3, 3);
        p.drawRoundedRect(44, 8, 28, 12, 3, 3);
        p.drawRoundedRect(44, 46, 28, 12, 3, 3);
        p.drawRoundedRect(84, 2, 28, 10, 2, 2);
        p.drawRoundedRect(84, 18, 28, 10, 2, 2);
        p.drawRoundedRect(84, 40, 28, 10, 2, 2);
        p.drawRoundedRect(84, 54, 28, 10, 2, 2);
        p.setPen(linePen);
        p.drawLine(32, 33, 44, 14);
        p.drawLine(32, 33, 44, 52);
        p.drawLine(72, 14, 84, 7);
        p.drawLine(72, 14, 84, 23);
        p.drawLine(72, 52, 84, 45);
        p.drawLine(72, 52, 84, 59);
    }

    p.setPen(c.previewText);
    p.setFont(QFont("sans-serif", 7));
    QStringList names = {"Mind Map", "Org Chart", "Project Plan"};
    if (index >= 0 && index < names.size()) {
        p.drawText(QRect(0, 68, 120, 12), Qt::AlignCenter, names[index]);
    }

    p.end();
    return pix;
}
