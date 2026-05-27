#include "core/UpdateNotifier.h"
#include "core/AppSettings.h"
#include "core/UpdateChecker.h"
#include "ui/IconFactory.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QToolButton>
#include <QUrl>

UpdateNotifier::UpdateNotifier(QWidget* parentWidget)
    : QObject(parentWidget),
      m_parentWidget(parentWidget),
      m_checker(new UpdateChecker(this)) {
    m_banner = buildBanner(parentWidget);

    m_statusBtn = new QToolButton(parentWidget);
    m_statusBtn->setObjectName("statusBarUpdateBtn");
    m_statusBtn->setAutoRaise(true);
    m_statusBtn->setFixedSize(24, 24);
    m_statusBtn->setIconSize(QSize(18, 18));
    connect(m_statusBtn, &QToolButton::clicked, this, &UpdateNotifier::onStatusButtonClicked);

    m_versionLabel = new QLabel(QString("v%1").arg(QCoreApplication::applicationVersion()),
                                parentWidget);
    m_versionLabel->setObjectName("statusBarVersionLabel");

    connect(m_checker, &UpdateChecker::updateAvailable, this, &UpdateNotifier::onUpdateAvailable);
    connect(m_checker, &UpdateChecker::upToDate, this, [this]() {
        emit upToDateMessage(tr("Check for Updates"),
                             tr("You are running the latest version of YMind."));
    });
    connect(m_checker, &UpdateChecker::checkFailed, this, [this](const QString& msg) {
        emit checkFailedMessage(tr("Check for Updates"),
                                tr("Could not check for updates:\n%1").arg(msg));
    });

    refreshStatusIcon();

    if (AppSettings::instance().checkForUpdatesEnabled()) {
        QTimer::singleShot(3000, m_checker, [this]() { m_checker->checkForUpdates(false); });
    }
}

void UpdateNotifier::checkNow() {
    m_checker->checkForUpdates(true);
}

QFrame* UpdateNotifier::buildBanner(QWidget* parent) {
    auto* banner = new QFrame(parent);
    banner->setObjectName("updateBanner");
    banner->setVisible(false);
    auto* layout = new QHBoxLayout(banner);
    layout->setContentsMargins(12, 6, 6, 6);
    layout->setSpacing(8);

    auto* icon = new QLabel(banner);
    icon->setPixmap(IconFactory::makeToolIcon("update-available").pixmap(18, 18));
    layout->addWidget(icon);

    m_bannerLabel = new QLabel(banner);
    m_bannerLabel->setObjectName("updateBannerLabel");
    m_bannerLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    m_bannerLabel->setOpenExternalLinks(false);
    connect(m_bannerLabel, &QLabel::linkActivated, this,
            [](const QString& link) { QDesktopServices::openUrl(QUrl(link)); });
    layout->addWidget(m_bannerLabel, 1);

    auto* close = new QToolButton(banner);
    close->setObjectName("updateBannerClose");
    close->setIcon(IconFactory::makeToolIcon("close-panel"));
    close->setIconSize(QSize(14, 14));
    close->setAutoRaise(true);
    close->setFixedSize(22, 22);
    close->setToolTip(tr("Dismiss"));
    connect(close, &QToolButton::clicked, banner, &QFrame::hide);
    layout->addWidget(close);

    return banner;
}

void UpdateNotifier::onUpdateAvailable(const QString& latestVersion, const QString& releaseUrl) {
    m_pendingVersion = latestVersion;
    m_pendingUrl = releaseUrl;

    if (m_bannerLabel && m_banner) {
        m_bannerLabel->setText(
            tr("A new version <b>v%1</b> of YMind is available. "
               "<a href=\"%2\" style=\"color: inherit; text-decoration: underline;\">Download</a>")
                .arg(latestVersion, releaseUrl));
        m_banner->setVisible(true);
    }

    refreshStatusIcon();
}

void UpdateNotifier::onStatusButtonClicked() {
    if (!m_pendingUrl.isEmpty()) {
        QDesktopServices::openUrl(QUrl(m_pendingUrl));
        return;
    }
    m_checker->checkForUpdates(true);
}

void UpdateNotifier::refreshStatusIcon() {
    if (!m_statusBtn)
        return;

    const bool available = !m_pendingVersion.isEmpty();
    m_statusBtn->setIcon(IconFactory::makeToolIcon(available ? "update-available" : "update"));
    m_statusBtn->setToolTip(
        available ? tr("Update available: v%1 — click to open download page").arg(m_pendingVersion)
                  : tr("Check for updates"));
}
