#pragma once

#include <QPoint>
#include <QWidget>

class QLabel;
class QLineEdit;
class QVariantAnimation;

class FindBarButton;

// Floating find panel overlaid on the canvas (top-right), macOS-style:
// rounded surface with a hairline border and soft drop shadow, a painted
// magnifier glyph, a borderless query field, a "N of M" counter, and
// custom-painted chevron / close buttons. Toggled by Ctrl+F from MainWindow.
//
// Emits queryChanged on every keystroke, stepNext/stepPrev on Enter / F3 /
// the chevron buttons (Shift+Enter / Shift+F3 step backwards), and closed
// when the user dismisses it (Esc, click on close, or Ctrl+F again).
//
// The widget owns no model state — MainWindow drives the match list and
// pushes the count back via `setMatchStatus`. Keeps the bar reusable across
// tabs without rebinding signals on every tab switch.
class FindBar : public QWidget {
    Q_OBJECT

public:
    explicit FindBar(QWidget* parent = nullptr);

    void setQuery(const QString& q);
    QString query() const;
    // Display "N of M" or "No matches" / empty string in the count label.
    // currentIdx is 1-based; pass 0/0 for "no current match".
    void setMatchStatus(int currentIdx, int total);

    // Place the panel in the top-right corner of a canvas of `canvasSize`,
    // shrinking to fit narrow windows. Owns the margin/size math so the
    // caller only forwards resize events.
    void reposition(const QSize& canvasSize);

    // Horizontal shake (macOS "not found" gesture). Called when the user
    // tries to step through matches and there are none.
    void indicateNoMatch();

    // Re-apply palette-derived colors after a light/dark theme switch.
    void refreshTheme();

public slots:
    // Show + focus the line edit; selects existing text so a fresh search
    // overwrites cleanly on the next keystroke. Slides in on first show.
    void activate();

signals:
    void queryChanged(const QString& q);
    void stepNext();
    void stepPrev();
    void closed();

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    // The visible rounded panel, inset from the widget rect so the drop
    // shadow has room to render inside the widget's own bounds.
    QRect panelRect() const;
    void animateTo(const QPoint& target, bool fromAbove);

    QLineEdit* m_edit = nullptr;
    QLabel* m_status = nullptr;
    FindBarButton* m_prevBtn = nullptr;
    FindBarButton* m_nextBtn = nullptr;
    FindBarButton* m_closeBtn = nullptr;
    QVariantAnimation* m_slideAnim = nullptr;
    QVariantAnimation* m_shakeAnim = nullptr;
    QPoint m_basePos;       // resting position set by reposition()
    bool m_noMatch = false; // tints the counter + field underline state
};
