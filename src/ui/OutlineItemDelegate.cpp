#include "ui/OutlineItemDelegate.h"
#include "ui/ThemeManager.h"

#include <QAbstractItemView>
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
    // selection/hover rectangle on top of our own fill.
    opt.state &= ~QStyle::State_Selected;
    opt.state &= ~QStyle::State_MouseOver;
    opt.state &= ~QStyle::State_HasFocus;
    opt.backgroundBrush = Qt::NoBrush;

    if (selected || hovered) {
        // Span the highlight across the full row width so every selected row
        // shares the same bar regardless of indent depth, with a small inset
        // so the rounded corners read cleanly against the panel background.
        QRectF bar = opt.rect;
        if (const auto* view = qobject_cast<const QAbstractItemView*>(opt.widget)) {
            const QRect vp = view->viewport()->rect();
            bar.setLeft(vp.left());
            bar.setRight(vp.right());
        }
        bar.adjust(4, 1, -4, -1);
        constexpr qreal radius = 5.0;

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);
        painter->setBrush(selected ? selBg : hoverBg);
        painter->drawRoundedRect(bar, radius, radius);
        painter->restore();
    }

    // Draw the icon and text on top of our background. State flags have been
    // stripped above so the QSS won't repaint a flat selection rectangle.
    QStyledItemDelegate::paint(painter, opt, index);
}
