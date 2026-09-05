#pragma once

#include <QObject>
#include <QPointer>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;

class AiClient : public QObject {
    Q_OBJECT

public:
    // OrcaRouter OSS program – attribution & referral
    static constexpr const char* kDefaultOrcaEndpoint = "https://api.orcarouter.ai/v1";
    static constexpr const char* kDefaultOrcaModel = "deepseek/deepseek-chat:free";
    static constexpr const char* kProjectReferer =
        "https://www.orcarouter.ai/ref/ref_dc5e5ce5b8727aef463e";
    static constexpr const char* kProjectTitle = "YMind";
    static constexpr const char* kOrcaPartnerUrl =
        "https://www.orcarouter.ai/ref/ref_dc5e5ce5b8727aef463e";

    explicit AiClient(QObject* parent = nullptr);
    ~AiClient() override;

    bool isBusy() const;
    void generateMindMapOutline(const QString& prompt, const QString& apiKey,
                                const QString& model = QString(),
                                const QString& endpoint = QString());
    void cancel();

    // Utility: strips markdown code fences (```markdown ... ```) and excess whitespace
    static QString cleanMarkdownOutline(const QString& rawOutput);

signals:
    void started();
    void finished(const QString& markdownOutline);
    void errorOccurred(const QString& errorMessage);

private slots:
    void onReplyFinished();

private:
    QNetworkAccessManager* m_nam;
    QPointer<QNetworkReply> m_currentReply;
};
