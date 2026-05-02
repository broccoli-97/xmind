#include "ui/OutlineItemDelegate.h"
#include "ui/ThemeManager.h"

#include <QPainter>

OutlineItemDelegate::OutlineItemDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

void OutlineItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                const QModelIndex& index) const {
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    const bool selected = (opt.state & QStyle::State_Selected);
    const bool hovered = (opt.state & QStyle::State_MouseOver) && !selected;
    const bool dark = ThemeManager::isDark();

    // Match the QSS palette in StyleSheetGenerator so the delegate stays in
    // sync with the rest of the tree styling.
    const QColor selBg = dark ? QColor(0x09, 0x47, 0x71) : QColor(0xCC, 0xE4, 0xF7);
    const QColor hoverBg = dark ? QColor(0x2A, 0x2D, 0x2E) : QColor(0xF0, 0xF0, 0xF0);

    // Strip the state flags so the QSS-styled base paint does NOT draw a flat
    // selection/hover rectangle on top of (or behind) our rounded pill. The
    // text color falls back to the regular tree foreground, which has good
    // contrast on both light and dark selection backgrounds.
    opt.state &= ~QStyle::State_Selected;
    opt.state &= ~QStyle::State_MouseOver;
    opt.state &= ~QStyle::State_HasFocus;
    opt.backgroundBrush = Qt::NoBrush;

    if (selected || hovered) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        // Pill rect: inset horizontally and vertically so the highlight reads
        // as a floating capsule rather than a row-spanning bar.
        const QRectF pill = QRectF(opt.rect).adjusted(3, 2, -4, -2);
        constexpr qreal radius = 6.0;

        painter->setPen(Qt::NoPen);
        painter->setBrush(selected ? selBg : hoverBg);
        painter->drawRoundedRect(pill, radius, radius);

        painter->restore();
    }

    // Draw the icon and text on top of our background. State flags have been
    // stripped above so the QSS won't repaint a flat selection rectangle.
    QStyledItemDelegate::paint(painter, opt, index);
}
