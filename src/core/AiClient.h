#pragma once

#include <QObject>
#include <QPointer>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;

class AiClient : public QObject {
    Q_OBJECT

public:
    // Placeholders for OrcaRouter partnership & OSS program
    // TODO: Update these once registered in OrcaRouter's OSS plan
    static constexpr const char* kDefaultOrcaEndpoint = "https://api.orcarouter.ai/v1";
    static constexpr const char* kDefaultOrcaModel = "deepseek/deepseek-chat:free";
    static constexpr const char* kProjectReferer = "https://github.com/broccoli-97/xmind";
    static constexpr const char* kProjectTitle = "YMind";
    static constexpr const char* kOrcaPartnerUrl = "https://www.orcarouter.ai/zh-CN/built-with";

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
