#include "ui/FindBar.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>

FindBar::FindBar(QWidget* parent) : QWidget(parent) {
    setObjectName("findBar");

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->setSpacing(6);

    m_edit = new QLineEdit(this);
    m_edit->setPlaceholderText(tr("Find in map..."));
    m_edit->setClearButtonEnabled(true);
    layout->addWidget(m_edit, 1);

    m_status = new QLabel(this);
    m_status->setObjectName("findStatus");
    layout->addWidget(m_status);

    m_prevBtn = new QToolButton(this);
    m_prevBtn->setText("↑"); // up arrow
    m_prevBtn->setToolTip(tr("Previous match (Shift+F3)"));
    m_prevBtn->setAutoRaise(true);
    layout->addWidget(m_prevBtn);

    m_nextBtn = new QToolButton(this);
    m_nextBtn->setText("↓"); // down arrow
    m_nextBtn->setToolTip(tr("Next match (F3)"));
    m_nextBtn->setAutoRaise(true);
    layout->addWidget(m_nextBtn);

    m_closeBtn = new QToolButton(this);
    m_closeBtn->setText("✕"); // multiplication x
    m_closeBtn->setToolTip(tr("Close find bar (Esc)"));
    m_closeBtn->setAutoRaise(true);
    layout->addWidget(m_closeBtn);

    connect(m_edit, &QLineEdit::textChanged, this, &FindBar::queryChanged);
    connect(m_edit, &QLineEdit::returnPressed, this, &FindBar::stepNext);
    connect(m_prevBtn, &QToolButton::clicked, this, &FindBar::stepPrev);
    connect(m_nextBtn, &QToolButton::clicked, this, &FindBar::stepNext);
    connect(m_closeBtn, &QToolButton::clicked, this, &FindBar::closed);
}

void FindBar::setQuery(const QString& q) {
    if (m_edit->text() != q)
        m_edit->setText(q);
}

QString FindBar::query() const {
    return m_edit->text();
}

void FindBar::setMatchStatus(int currentIdx, int total) {
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

void FindBar::activate() {
    show();
    m_edit->setFocus();
    m_edit->selectAll();
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
