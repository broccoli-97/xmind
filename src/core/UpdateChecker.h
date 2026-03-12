#pragma once

#include <QObject>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;

class UpdateChecker : public QObject {
    Q_OBJECT

public:
    explicit UpdateChecker(QObject* parent = nullptr);

    void checkForUpdates(bool manual);

    static bool isNewerVersion(const QString& current, const QString& latest);

signals:
    void updateAvailable(const QString& version, const QString& url);
    void upToDate();
    void checkFailed(const QString& errorMessage);

private:
    void onReplyFinished(QNetworkReply* reply);

    QNetworkAccessManager* m_nam;
    bool m_manual = false;
};
