#include "ui/OutlineItemDelegate.h"
#include "ui/ThemeManager.h"

#include <QAbstractItemView>
#include <QPainter>
#include <QTreeView>

OutlineItemDelegate::OutlineItemDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

// Mirror of StyleSheetGenerator's generateBranchIcon(): same 12px chevron,
// stroke, and color as the QSS ::branch images, centered in the branch cell.
static void drawBranchChevron(QPainter* painter, const QRect& cell, bool open) {
    constexpr int sz = 12;
    const QPointF origin(cell.left() + (cell.width() - sz) / 2.0,
                         cell.top() + (cell.height() - sz) / 2.0);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(QColor("#B0B0B0"), 1.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->setBrush(Qt::NoBrush);
    if (open) {
        painter->drawLine(origin + QPointF(2, 4), origin + QPointF(6, 8));
        painter->drawLine(origin + QPointF(6, 8), origin + QPointF(10, 4));
    } else {
        painter->drawLine(origin + QPointF(4, 2), origin + QPointF(8, 6));
        painter->drawLine(origin + QPointF(8, 6), origin + QPointF(4, 10));
    }
    painter->restore();
}

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

        // The tree view paints the QSS ::branch chevron before the delegate
        // runs, so the full-width bar above just covered it. Repaint it on top
        // for foldable rows; the cell is the indentation slot left of the item.
        if (index.model()->hasChildren(index)) {
            if (const auto* tree = qobject_cast<const QTreeView*>(opt.widget)) {
                const QRect cell(opt.rect.left() - tree->indentation(), opt.rect.top(),
                                 tree->indentation(), opt.rect.height());
                drawBranchChevron(painter, cell, tree->isExpanded(index));
            }
        }
    }

    // Draw the icon and text on top of our background. State flags have been
    // stripped above so the QSS won't repaint a flat selection rectangle.
    QStyledItemDelegate::paint(painter, opt, index);
}
