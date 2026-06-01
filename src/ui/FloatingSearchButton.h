#pragma once

#include <QAbstractButton>

class QVariantAnimation;

// A circular, line-style "search" affordance that floats over the canvas
// (top-right corner). Its clicked() signal is wired by MainWindow to
// openFindBar(), so it is simply a visible second entry point to the same
// Ctrl+F find bar.
//
// Custom-painted (rather than QSS + an IconFactory pixmap) so the
// magnifying-glass glyph and the smooth accent-fill hover transition stay
// crisp at any DPI and match the app's line-art icon set. Colors are read live
// from ThemeManager on every paint, so it follows theme changes — call
// refreshTheme() after a theme switch to force the repaint.
class FloatingSearchButton : public QAbstractButton {
    Q_OBJECT

public:
    explicit FloatingSearchButton(QWidget* parent = nullptr);

    // Repaint with the current theme's colors. Call after a theme switch.
    void refreshTheme();

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    void animateHover(double to);

    double m_hover = 0.0; // 0 = idle, 1 = fully hovered; animated between the two
    QVariantAnimation* m_hoverAnim = nullptr;
};
