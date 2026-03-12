#include "core/UpdateChecker.h"

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

static const char* kReleasesUrl =
    "https://api.github.com/repos/broccoli-97/xmind/releases/latest";

UpdateChecker::UpdateChecker(QObject* parent)
    : QObject(parent), m_nam(new QNetworkAccessManager(this)) {}

void UpdateChecker::checkForUpdates(bool manual) {
    m_manual = manual;

    QNetworkRequest req{QUrl(kReleasesUrl)};
    req.setHeader(QNetworkRequest::UserAgentHeader, "YMind-UpdateChecker");
    req.setRawHeader("Accept", "application/vnd.github+json");

    QNetworkReply* reply = m_nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onReplyFinished(reply);
        reply->deleteLater();
    });
}

void UpdateChecker::onReplyFinished(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError) {
        if (m_manual)
            emit checkFailed(reply->errorString());
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    if (!doc.isObject()) {
        if (m_manual)
            emit checkFailed("Invalid response from GitHub.");
        return;
    }

    QJsonObject obj = doc.object();
    QString tagName = obj.value("tag_name").toString();
    QString htmlUrl = obj.value("html_url").toString();

    if (tagName.isEmpty()) {
        if (m_manual)
            emit checkFailed("No release tag found.");
        return;
    }

    // Strip leading 'v' or 'V'
    QString latestVersion = tagName;
    if (latestVersion.startsWith('v') || latestVersion.startsWith('V'))
        latestVersion = latestVersion.mid(1);

    QString currentVersion = QCoreApplication::applicationVersion();

    if (isNewerVersion(currentVersion, latestVersion)) {
        emit updateAvailable(latestVersion, htmlUrl);
    } else {
        if (m_manual)
            emit upToDate();
    }
}

bool UpdateChecker::isNewerVersion(const QString& current, const QString& latest) {
    QStringList curParts = current.split('.');
    QStringList latParts = latest.split('.');

    int count = qMax(curParts.size(), latParts.size());
    for (int i = 0; i < count; ++i) {
        int c = (i < curParts.size()) ? curParts[i].toInt() : 0;
        int l = (i < latParts.size()) ? latParts[i].toInt() : 0;
        if (l > c)
            return true;
        if (l < c)
            return false;
    }
    return false;
}
