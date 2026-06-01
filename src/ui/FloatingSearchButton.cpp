#include "ui/FloatingSearchButton.h"
#include "ui/ThemeManager.h"

#include <QEnterEvent>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QVariantAnimation>

#include <algorithm>
#include <cmath>

namespace {
// The widget box is larger than the visible circle so the drop shadow has room
// to render inside the widget's own bounds (graphics effects on child widgets
// are otherwise clipped to the widget rect).
constexpr int kBoxSize = 56;
constexpr int kCircleInset = 10; // transparent padding around the visible circle

// App-wide interaction accent (matches the hover/checked color used throughout
// StyleSheetGenerator.cpp). Hover fades the body toward this, the icon to white.
const QColor kAccent("#007ACC");

QColor lerp(const QColor& a, const QColor& b, double t) {
    t = std::clamp(t, 0.0, 1.0);
    return QColor::fromRgbF(a.redF() + (b.redF() - a.redF()) * t,
                            a.greenF() + (b.greenF() - a.greenF()) * t,
                            a.blueF() + (b.blueF() - a.blueF()) * t,
                            a.alphaF() + (b.alphaF() - a.alphaF()) * t);
}
} // namespace

FloatingSearchButton::FloatingSearchButton(QWidget* parent) : QAbstractButton(parent) {
    setFixedSize(kBoxSize, kBoxSize);
    setCursor(Qt::PointingHandCursor);
    // Clicking must not pull keyboard focus off the canvas — the find bar grabs
    // focus itself when it activates.
    setFocusPolicy(Qt::NoFocus);

    auto* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(16);
    shadow->setOffset(0, 3);
    shadow->setColor(QColor(0, 0, 0, 70));
    setGraphicsEffect(shadow);

    m_hoverAnim = new QVariantAnimation(this);
    m_hoverAnim->setDuration(150);
    m_hoverAnim->setEasingCurve(QEasingCurve::InOutQuad);
    connect(m_hoverAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant& v) {
        m_hover = v.toDouble();
        update();
    });
}

void FloatingSearchButton::refreshTheme() {
    update();
}

void FloatingSearchButton::animateHover(double to) {
    m_hoverAnim->stop();
    m_hoverAnim->setStartValue(m_hover);
    m_hoverAnim->setEndValue(to);
    m_hoverAnim->start();
}

void FloatingSearchButton::enterEvent(QEnterEvent*) {
    animateHover(1.0);
}

void FloatingSearchButton::leaveEvent(QEvent*) {
    animateHover(0.0);
}

void FloatingSearchButton::paintEvent(QPaintEvent*) {
    const bool dark = ThemeManager::isDark();
    // Surface/border track the QSS palette so the button reads as part of the
    // chrome; the idle glyph uses the shared icon color.
    const QColor surface = dark ? QColor("#2D2D30") : QColor("#FFFFFF");
    const QColor border = dark ? QColor("#3F3F46") : QColor("#D0D0D0");
    const QColor iconIdle = ThemeManager::colors().iconBaseColor;

    QColor fill = lerp(surface, kAccent, m_hover);
    const QColor stroke = lerp(border, kAccent, m_hover);
    const QColor icon = lerp(iconIdle, QColor("#FFFFFF"), m_hover);

    if (isDown()) // subtle press feedback
        fill = fill.darker(112);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Circular body.
    const QRectF circle =
        QRectF(rect()).adjusted(kCircleInset, kCircleInset, -kCircleInset, -kCircleInset);
    p.setPen(QPen(stroke, 1.4));
    p.setBrush(fill);
    p.drawEllipse(circle);

    // Line-style magnifying glass centered on the circle — lens ellipse plus a
    // diagonal handle, the same glyph idiom as IconFactory's "zoom" icon.
    const QPointF c = circle.center();
    const double r = 6.5;                              // lens radius
    const QPointF lensC(c.x() - 2.0, c.y() - 2.0);     // nudge up-left so the
    const QRectF lens(lensC.x() - r, lensC.y() - r, 2 * r, 2 * r); // handle stays centered

    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(icon, 2.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.drawEllipse(lens);

    const double k = r / std::sqrt(2.0); // lens edge on the lower-right diagonal
    const QPointF handleStart(lensC.x() + k, lensC.y() + k);
    const QPointF handleEnd(handleStart.x() + 6.0, handleStart.y() + 6.0);
    p.setPen(QPen(icon, 2.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.drawLine(handleStart, handleEnd);
}
