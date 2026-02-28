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
        p.drawPath(QPainterPath());
        p.setPen(pen);
        p.drawArc(8, 8, 14, 14, 0, 180 * 16);
        p.drawLine(8, 15, 4, 11);
        p.drawLine(8, 15, 6, 20);
    } else if (name == "redo") {
        p.drawPath(QPainterPath());
        p.setPen(pen);
        p.drawArc(10, 8, 14, 14, 180 * 16, 180 * 16);
        p.drawLine(24, 15, 28, 11);
        p.drawLine(24, 15, 26, 20);
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

QPixmap IconFactory::makeTemplatePreview(int index, int width, int height) {
    const auto& c = ThemeManager::colors();
    QPixmap pix(width, height);
    pix.fill(c.previewBackground);
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
        p.drawRoundedRect(42, 30, 36, 18, 4, 4);
        p.setPen(linePen);
        p.drawLine(60, 30, 90, 12);
        p.drawLine(60, 48, 90, 66);
        p.drawLine(42, 30, 18, 14);
        p.drawLine(42, 48, 18, 66);
        p.setPen(nodePen);
        p.drawRoundedRect(84, 6, 28, 12, 3, 3);
        p.drawRoundedRect(84, 60, 28, 12, 3, 3);
        p.drawRoundedRect(4, 8, 28, 12, 3, 3);
        p.drawRoundedRect(4, 60, 28, 12, 3, 3);
    } else if (index == 1) {
        p.setPen(nodePen);
        p.setBrush(nodeBrush);
        p.drawRoundedRect(42, 6, 36, 14, 3, 3);
        p.drawRoundedRect(8, 50, 28, 14, 3, 3);
        p.drawRoundedRect(46, 50, 28, 14, 3, 3);
        p.drawRoundedRect(84, 50, 28, 14, 3, 3);
        p.setPen(linePen);
        p.drawLine(60, 20, 60, 32);
        p.drawLine(22, 32, 98, 32);
        p.drawLine(22, 32, 22, 50);
        p.drawLine(60, 32, 60, 50);
        p.drawLine(98, 32, 98, 50);
    } else if (index == 2) {
        p.setPen(nodePen);
        p.setBrush(nodeBrush);
        p.drawRoundedRect(4, 30, 28, 14, 3, 3);
        p.drawRoundedRect(44, 10, 28, 12, 3, 3);
        p.drawRoundedRect(44, 52, 28, 12, 3, 3);
        p.drawRoundedRect(84, 4, 28, 10, 2, 2);
        p.drawRoundedRect(84, 20, 28, 10, 2, 2);
        p.drawRoundedRect(84, 46, 28, 10, 2, 2);
        p.drawRoundedRect(84, 62, 28, 10, 2, 2);
        p.setPen(linePen);
        p.drawLine(32, 37, 44, 16);
        p.drawLine(32, 37, 44, 58);
        p.drawLine(72, 16, 84, 9);
        p.drawLine(72, 16, 84, 25);
        p.drawLine(72, 58, 84, 51);
        p.drawLine(72, 58, 84, 67);
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
