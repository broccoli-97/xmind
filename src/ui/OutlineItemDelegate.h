#pragma once

#include <QStyledItemDelegate>

// Custom delegate for the OutlineWidget's tree.
//
// Replaces the default flat selection rectangle with a rounded "pill" shape.
// Hover gets the same rounded shape for visual consistency.
class OutlineItemDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit OutlineItemDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
};
