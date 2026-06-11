#include "ui/FindBar.h"
#include "ui/ThemeManager.h"

#include <QAbstractButton>
#include <QEnterEvent>
#include <QGraphicsDropShadowEffect>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QVariantAnimation>

#include <cmath>

namespace {

// Transparent margin around the visible panel so the drop shadow can render
// inside the widget's own bounds (graphics effects on child widgets are
// clipped to the widget rect — same trick as FloatingSearchButton).
constexpr int kShadowPad = 14;
constexpr int kPanelHeight = 44;
constexpr qreal kPanelRadius = 10.0;
constexpr int kPreferredPanelWidth = 400;
constexpr int kCanvasMargin = 14; // visible gap between panel and canvas edges
constexpr int kGlyphZone = 34;    // left strip reserved for the magnifier glyph

// App-wide interaction accent (matches FloatingSearchButton / the QSS hover
// color) — used for the field's selection highlight.
const QColor kAccent("#007ACC");

} // namespace

// ---------------------------------------------------------------------------
// FindBarButton — borderless icon button with a painted glyph. The glyphs are
// drawn at a deliberate size (9–10 px strokes at 1.8 px width) so the
// chevrons read clearly, unlike font arrows. Hover/press show the macOS-style
// soft rounded backplate.
// ---------------------------------------------------------------------------
class FindBarButton : public QAbstractButton {
public:
    enum class Glyph { ChevronUp, ChevronDown, Close };

    explicit FindBarButton(Glyph glyph, QWidget* parent = nullptr)
        : QAbstractButton(parent), m_glyph(glyph) {
        setFixedSize(26, 26);
        // Clicks must not pull focus off the query field — Esc/F3 handling
        // lives there and on the panel.
        setFocusPolicy(Qt::NoFocus);
    }

protected:
    void enterEvent(QEnterEvent*) override { update(); }
    void leaveEvent(QEvent*) override { update(); }

    void paintEvent(QPaintEvent*) override {
        const bool dark = ThemeManager::isDark();
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        // Soft rounded backplate on hover/press.
        if (isEnabled() && (underMouse() || isDown())) {
            QColor plate = dark ? QColor(255, 255, 255, isDown() ? 42 : 26)
                                : QColor(0, 0, 0, isDown() ? 34 : 20);
            p.setPen(Qt::NoPen);
            p.setBrush(plate);
            p.drawRoundedRect(QRectF(rect()).adjusted(1, 1, -1, -1), 6, 6);
        }

        QColor ink = ThemeManager::colors().iconBaseColor;
        if (!isEnabled())
            ink.setAlphaF(ink.alphaF() * 0.32);

        const QPointF c = QRectF(rect()).center();
        p.setPen(QPen(ink, 1.9, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.setBrush(Qt::NoBrush);

        switch (m_glyph) {
        case Glyph::ChevronUp: {
            const QPointF pts[3] = {c + QPointF(-4.6, 2.2), c + QPointF(0, -2.4),
                                    c + QPointF(4.6, 2.2)};
            p.drawPolyline(pts, 3);
            break;
        }
        case Glyph::ChevronDown: {
            const QPointF pts[3] = {c + QPointF(-4.6, -2.2), c + QPointF(0, 2.4),
                                    c + QPointF(4.6, -2.2)};
            p.drawPolyline(pts, 3);
            break;
        }
        case Glyph::Close:
            p.setPen(QPen(ink, 1.7, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(c + QPointF(-3.6, -3.6), c + QPointF(3.6, 3.6));
            p.drawLine(c + QPointF(-3.6, 3.6), c + QPointF(3.6, -3.6));
            break;
        }
    }

private:
    Glyph m_glyph;
};

// ---------------------------------------------------------------------------
// FindBar
// ---------------------------------------------------------------------------
FindBar::FindBar(QWidget* parent) : QWidget(parent) {
    setObjectName("findBar");
    setFixedHeight(kPanelHeight + 2 * kShadowPad);

    auto* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(22);
    shadow->setOffset(0, 4);
    shadow->setColor(QColor(0, 0, 0, 70));
    setGraphicsEffect(shadow);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(kShadowPad + kGlyphZone, kShadowPad, kShadowPad + 7, kShadowPad);
    layout->setSpacing(2);

    m_edit = new QLineEdit(this);
    m_edit->setObjectName("findEdit");
    m_edit->setPlaceholderText(tr("Find in map..."));
    m_edit->setClearButtonEnabled(true);
    m_edit->setFrame(false);
    layout->addWidget(m_edit, 1);

    m_status = new QLabel(this);
    m_status->setObjectName("findStatus");
    layout->addWidget(m_status);
    layout->addSpacing(4);

    m_prevBtn = new FindBarButton(FindBarButton::Glyph::ChevronUp, this);
    m_prevBtn->setToolTip(tr("Previous match (Shift+F3)"));
    m_prevBtn->setEnabled(false);
    layout->addWidget(m_prevBtn);

    m_nextBtn = new FindBarButton(FindBarButton::Glyph::ChevronDown, this);
    m_nextBtn->setToolTip(tr("Next match (F3)"));
    m_nextBtn->setEnabled(false);
    layout->addWidget(m_nextBtn);

    layout->addSpacing(4);

    m_closeBtn = new FindBarButton(FindBarButton::Glyph::Close, this);
    m_closeBtn->setToolTip(tr("Close find bar (Esc)"));
    layout->addWidget(m_closeBtn);

    connect(m_edit, &QLineEdit::textChanged, this, &FindBar::queryChanged);
    // Enter steps forward, Shift+Enter steps back (macOS convention).
    connect(m_edit, &QLineEdit::returnPressed, this, [this]() {
        if (QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)
            emit stepPrev();
        else
            emit stepNext();
    });
    connect(m_prevBtn, &QAbstractButton::clicked, this, &FindBar::stepPrev);
    connect(m_nextBtn, &QAbstractButton::clicked, this, &FindBar::stepNext);
    connect(m_closeBtn, &QAbstractButton::clicked, this, &FindBar::closed);

    refreshTheme();
}

QRect FindBar::panelRect() const {
    return rect().adjusted(kShadowPad, kShadowPad, -kShadowPad, -kShadowPad);
}

void FindBar::setQuery(const QString& q) {
    if (m_edit->text() != q)
        m_edit->setText(q);
}

QString FindBar::query() const {
    return m_edit->text();
}

void FindBar::setMatchStatus(int currentIdx, int total) {
    const bool noMatch = !m_edit->text().isEmpty() && total == 0;
    if (noMatch != m_noMatch) {
        m_noMatch = noMatch;
        refreshTheme(); // counter swaps between gray and red
    }
    m_prevBtn->setEnabled(total > 0);
    m_nextBtn->setEnabled(total > 0);

    if (m_edit->text().isEmpty()) {
        m_status->clear();
        return;
    }
    if (total == 0) {
        m_status->setText(tr("No matches"));
        return;
    }
    m_status->setText(tr("%1 of %2").arg(currentIdx).arg(total));
}

void FindBar::reposition(const QSize& canvasSize) {
    const int panelW =
        qBound(220, kPreferredPanelWidth, qMax(220, canvasSize.width() - 2 * kCanvasMargin));
    resize(panelW + 2 * kShadowPad, height());
    // Panel (not widget) edges sit kCanvasMargin from the canvas corner; the
    // shadow pad hangs off into the margin.
    m_basePos = QPoint(canvasSize.width() - width() - kCanvasMargin + kShadowPad,
                       kCanvasMargin - kShadowPad);
    const bool animating = (m_slideAnim && m_slideAnim->state() == QAbstractAnimation::Running) ||
                           (m_shakeAnim && m_shakeAnim->state() == QAbstractAnimation::Running);
    if (!animating)
        move(m_basePos);
}

void FindBar::animateTo(const QPoint& target, bool fromAbove) {
    if (!m_slideAnim) {
        m_slideAnim = new QVariantAnimation(this);
        m_slideAnim->setDuration(170);
        m_slideAnim->setEasingCurve(QEasingCurve::OutCubic);
        connect(m_slideAnim, &QVariantAnimation::valueChanged, this,
                [this](const QVariant& v) { move(v.toPoint()); });
    }
    m_slideAnim->stop();
    m_slideAnim->setStartValue(fromAbove ? target - QPoint(0, 10) : pos());
    m_slideAnim->setEndValue(target);
    m_slideAnim->start();
}

void FindBar::activate() {
    const bool wasHidden = isHidden();
    show();
    raise();
    m_edit->setFocus();
    m_edit->selectAll();
    if (wasHidden)
        animateTo(m_basePos, /*fromAbove=*/true);
}

void FindBar::indicateNoMatch() {
    if (m_slideAnim)
        m_slideAnim->stop();
    if (!m_shakeAnim) {
        m_shakeAnim = new QVariantAnimation(this);
        m_shakeAnim->setDuration(280);
        connect(m_shakeAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant& v) {
            const double t = v.toDouble();
            const double amp = 6.0 * (1.0 - t); // decaying oscillation
            move(m_basePos + QPoint(qRound(std::sin(t * 3.14159 * 5.0) * amp), 0));
        });
        connect(m_shakeAnim, &QVariantAnimation::finished, this, [this]() { move(m_basePos); });
    }
    m_shakeAnim->stop();
    m_shakeAnim->setStartValue(0.0);
    m_shakeAnim->setEndValue(1.0);
    m_shakeAnim->start();
}

void FindBar::refreshTheme() {
    const bool dark = ThemeManager::isDark();
    const QColor text = dark ? QColor("#E8E8E8") : QColor("#1D1D1F");
    const QColor placeholder = dark ? QColor("#7E7E84") : QColor("#9B9BA0");

    // Local sheets out-rank the app-wide QSS, stripping the global QLineEdit
    // chrome (border, padding, opaque background) so the field melts into the
    // panel surface.
    m_edit->setStyleSheet(QString("QLineEdit { background: transparent; border: none;"
                                  " padding: 0; color: %1; selection-background-color: %2;"
                                  " selection-color: white; }")
                              .arg(text.name(), kAccent.name()));
    QPalette pal = m_edit->palette();
    pal.setColor(QPalette::PlaceholderText, placeholder);
    m_edit->setPalette(pal);

    // Counter: quiet gray normally, soft red when nothing matches.
    const QColor counter = m_noMatch ? (dark ? QColor("#FF6B6B") : QColor("#E0383E"))
                                     : (dark ? QColor("#98989D") : QColor("#86868B"));
    m_status->setStyleSheet(
        QString("QLabel { color: %1; background: transparent; }").arg(counter.name()));
    update();
}

void FindBar::paintEvent(QPaintEvent*) {
    const bool dark = ThemeManager::isDark();
    const QColor surface = dark ? QColor("#2D2D30") : QColor("#FFFFFF");
    const QColor border = dark ? QColor("#3F3F46") : QColor("#D6D6DA");

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const QRectF panel = QRectF(panelRect()).adjusted(0.5, 0.5, -0.5, -0.5);
    p.setPen(QPen(border, 1.0));
    p.setBrush(surface);
    p.drawRoundedRect(panel, kPanelRadius, kPanelRadius);

    // Magnifier glyph in the reserved left strip — quiet, like the gray
    // search icon in macOS search fields.
    QColor glyph = ThemeManager::colors().iconBaseColor;
    glyph.setAlphaF(glyph.alphaF() * 0.62);
    const QPointF c(panel.left() + kGlyphZone / 2.0 + 3.0, panel.center().y());
    const qreal r = 5.0;
    const QPointF lensC = c - QPointF(1.6, 1.6);
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(glyph, 1.7, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.drawEllipse(QRectF(lensC.x() - r, lensC.y() - r, 2 * r, 2 * r));
    const qreal k = r / std::sqrt(2.0);
    const QPointF handleStart = lensC + QPointF(k, k);
    p.setPen(QPen(glyph, 2.0, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(handleStart, handleStart + QPointF(4.4, 4.4));
}

void FindBar::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        event->accept();
        emit closed();
        return;
    }
    if (event->key() == Qt::Key_F3) {
        event->accept();
        if (event->modifiers() & Qt::ShiftModifier)
            emit stepPrev();
        else
            emit stepNext();
        return;
    }
    QWidget::keyPressEvent(event);
}
