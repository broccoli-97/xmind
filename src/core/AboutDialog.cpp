#include "core/AboutDialog.h"
#include "ui/ThemeManager.h"

#include <QCoreApplication>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("About YMind"));
    setFixedWidth(420);

    QString linkColor = ThemeManager::isDark() ? "#5cacee" : "#0563C1";

    auto* layout = new QVBoxLayout(this);
    layout->setSpacing(12);
    layout->setContentsMargins(24, 24, 24, 24);

    auto makeLink = [&](const QString& url, const QString& text) {
        return QString("<a href=\"%1\" style=\"color:%2;\">%3</a>").arg(url, linkColor, text);
    };

    // App info
    auto* appLabel = new QLabel(this);
    appLabel->setTextFormat(Qt::RichText);
    appLabel->setWordWrap(true);
    appLabel->setText(
        QString("<h2>YMind</h2>"
                "<p>%1</p>"
                "<p>%2</p>")
            .arg(tr("Version %1").arg(QCoreApplication::applicationVersion()),
                 tr("A desktop mind map editor.")));
    layout->addWidget(appLabel);

    // License
    auto* licenseLabel = new QLabel(this);
    licenseLabel->setTextFormat(Qt::RichText);
    licenseLabel->setWordWrap(true);
    licenseLabel->setText(QString("<p>%1</p>")
                              .arg(tr("Licensed under the %1.")
                                       .arg(makeLink("https://www.apache.org/licenses/LICENSE-2.0",
                                                     "Apache License 2.0"))));
    licenseLabel->setOpenExternalLinks(true);
    layout->addWidget(licenseLabel);

    // Qt / LGPL notice
    auto* qtLabel = new QLabel(this);
    qtLabel->setTextFormat(Qt::RichText);
    qtLabel->setWordWrap(true);
    QString qtOpenSource = "https://www.qt.io/download/open-source";
    qtLabel->setText(
        QString("<p>%1</p>")
            .arg(tr("This application uses Qt %1, licensed under the %2. "
                     "Qt source code and re-linking instructions are available at: %3.")
                     .arg(qVersion(),
                          makeLink("https://www.gnu.org/licenses/lgpl-3.0.html", "GNU LGPL v3"),
                          makeLink(qtOpenSource, qtOpenSource))));
    qtLabel->setOpenExternalLinks(true);
    layout->addWidget(qtLabel);

    layout->addStretch();

    // Close button
    auto* closeBtn = new QPushButton(tr("Close"), this);
    closeBtn->setDefault(true);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    auto* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);
}
