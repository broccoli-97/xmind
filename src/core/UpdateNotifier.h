#pragma once

#include <QObject>

class QFrame;
class QLabel;
class QString;
class QToolButton;
class QWidget;
class UpdateChecker;

// Wraps everything related to "is there a newer release?" — the GitHub-poll
// UpdateChecker plus the two UI affordances it drives (an inline banner at
// the top of the window and a status-bar icon). MainWindow asks for the two
// widgets and connects them into its layout; this class owns the rest.
class UpdateNotifier : public QObject {
    Q_OBJECT

public:
    explicit UpdateNotifier(QWidget* parentWidget);

    QFrame* banner() const { return m_banner; }
    QToolButton* statusButton() const { return m_statusBtn; }
    QLabel* versionLabel() const { return m_versionLabel; }

    // Triggered by Help → Check for Updates...
    void checkNow();

    // Triggered by the status-bar icon click: jump to the download page if
    // we already know of an update, otherwise kick off a fresh user-driven
    // check.
    void onStatusButtonClicked();

signals:
    void upToDateMessage(const QString& title, const QString& message);
    void checkFailedMessage(const QString& title, const QString& message);

private:
    QFrame* buildBanner(QWidget* parent);
    void onUpdateAvailable(const QString& latestVersion, const QString& releaseUrl);
    void refreshStatusIcon();

    QWidget* m_parentWidget;
    UpdateChecker* m_checker;
    QFrame* m_banner = nullptr;
    QLabel* m_bannerLabel = nullptr;
    QToolButton* m_statusBtn = nullptr;
    QLabel* m_versionLabel = nullptr;
    QString m_pendingVersion;
    QString m_pendingUrl;
};
