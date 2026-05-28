#pragma once

#include <QWidget>

class QLabel;
class QLineEdit;
class QToolButton;

// Inline find bar sitting just above the canvas. Toggled by Ctrl+F from
// MainWindow. Emits queryChanged on every keystroke, stepNext/stepPrev when
// the user hits F3/Shift+F3 or the prev/next buttons, and closed when the
// user dismisses it (Esc, click on close, or Ctrl+F again).
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

public slots:
    // Show + focus the line edit; selects existing text so a fresh search
    // overwrites cleanly on the next keystroke.
    void activate();

signals:
    void queryChanged(const QString& q);
    void stepNext();
    void stepPrev();
    void closed();

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    QLineEdit* m_edit = nullptr;
    QLabel* m_status = nullptr;
    QToolButton* m_prevBtn = nullptr;
    QToolButton* m_nextBtn = nullptr;
    QToolButton* m_closeBtn = nullptr;
};
