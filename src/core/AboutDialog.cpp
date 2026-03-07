#include "core/AboutDialog.h"
#include "ui/ThemeManager.h"

#include <QCoreApplication>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("About YMind");
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
                "<p>Version %1</p>"
                "<p>A desktop mind map editor.</p>")
            .arg(QCoreApplication::applicationVersion()));
    layout->addWidget(appLabel);

    // License
    auto* licenseLabel = new QLabel(this);
    licenseLabel->setTextFormat(Qt::RichText);
    licenseLabel->setWordWrap(true);
    licenseLabel->setText(QString("<p>Licensed under the %1.</p>")
                              .arg(makeLink("https://www.apache.org/licenses/LICENSE-2.0",
                                            "Apache License 2.0")));
    licenseLabel->setOpenExternalLinks(true);
    layout->addWidget(licenseLabel);

    // Qt / LGPL notice
    auto* qtLabel = new QLabel(this);
    qtLabel->setTextFormat(Qt::RichText);
    qtLabel->setWordWrap(true);
    QString qtOpenSource = "https://www.qt.io/download/open-source";
    qtLabel->setText(
        QString("<p>This application uses Qt %1, licensed under the %2. "
                "Qt source code and re-linking instructions are available at: %3.</p>")
            .arg(qVersion(),
                 makeLink("https://www.gnu.org/licenses/lgpl-3.0.html", "GNU LGPL v3"),
                 makeLink(qtOpenSource, qtOpenSource)));
    qtLabel->setOpenExternalLinks(true);
    layout->addWidget(qtLabel);

    layout->addStretch();

    // Close button
    auto* closeBtn = new QPushButton("Close", this);
    closeBtn->setDefault(true);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    auto* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);
}
