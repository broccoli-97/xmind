#include "core/AiClient.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

AiClient::AiClient(QObject* parent) : QObject(parent), m_nam(new QNetworkAccessManager(this)) {}

AiClient::~AiClient() {
    cancel();
}

bool AiClient::isBusy() const {
    return m_currentReply != nullptr && m_currentReply->isRunning();
}

void AiClient::cancel() {
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }
}

QString AiClient::cleanMarkdownOutline(const QString& rawOutput) {
    QString text = rawOutput.trimmed();

    // Strip leading markdown code fences if wrapped in ``` or ```markdown
    if (text.startsWith(QLatin1String("```"))) {
        int firstNewline = text.indexOf('\n');
        if (firstNewline != -1) {
            text = text.mid(firstNewline + 1);
        }
    }
    if (text.endsWith(QLatin1String("```"))) {
        text.chop(3);
    }
    return text.trimmed();
}

void AiClient::generateMindMapOutline(const QString& prompt, const QString& apiKey,
                                      const QString& model, const QString& endpoint) {
    if (isBusy()) {
        cancel();
    }

    QString actualEndpoint = endpoint.trimmed().isEmpty()
                                 ? QString::fromLatin1(kDefaultOrcaEndpoint)
                                 : endpoint.trimmed();
    if (actualEndpoint.endsWith('/')) {
        actualEndpoint.chop(1);
    }
    QUrl url(actualEndpoint + QStringLiteral("/chat/completions"));

    QString actualModel =
        model.trimmed().isEmpty() ? QString::fromLatin1(kDefaultOrcaModel) : model.trimmed();

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setRawHeader("Authorization", "Bearer " + apiKey.trimmed().toUtf8());

    // Project attribution headers for OrcaRouter (and OpenRouter-compatible gateways)
    request.setRawHeader("HTTP-Referer", kProjectReferer);
    request.setRawHeader("X-Title", kProjectTitle);

    QJsonObject root;
    root["model"] = actualModel;
    root["temperature"] = 0.3;

    QJsonArray messages;

    QJsonObject sysMsg;
    sysMsg["role"] = "system";
    sysMsg["content"] =
        QStringLiteral("You are a specialized mind map outline generator.\n"
                       "Convert the user's text or topic into a structured Markdown outline.\n"
                       "Rules:\n"
                       "1. The first line must be a single top-level heading (# Topic Title) "
                       "representing the root topic.\n"
                       "2. Use subsequent headings (##, ###, etc.) or indented list items (- or *) "
                       "to represent subtopics and branch nodes.\n"
                       "3. Keep each node text concise and expressive (phrases or key concepts, "
                       "avoid long paragraphs).\n"
                       "4. Output ONLY the raw Markdown outline. Do NOT wrap output in ``` code "
                       "blocks. Do NOT include any intro, commentary, or outro.");
    messages.append(sysMsg);

    QJsonObject userMsg;
    userMsg["role"] = "user";
    userMsg["content"] = prompt.trimmed();
    messages.append(userMsg);

    root["messages"] = messages;

    QByteArray postData = QJsonDocument(root).toJson(QJsonDocument::Compact);

    emit started();

    m_currentReply = m_nam->post(request, postData);
    connect(m_currentReply, &QNetworkReply::finished, this, &AiClient::onReplyFinished);
}

void AiClient::onReplyFinished() {
    if (!m_currentReply)
        return;

    QNetworkReply* reply = m_currentReply;
    m_currentReply = nullptr;
    reply->deleteLater();

    if (reply->error() == QNetworkReply::OperationCanceledError) {
        // Canceled by user intentionally
        return;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray data = reply->readAll();

    if (reply->error() != QNetworkReply::NoError) {
        if (statusCode == 401) {
            emit errorOccurred(tr("Invalid API Key. Please verify your API Key in Settings."));
            return;
        } else if (statusCode == 429) {
            emit errorOccurred(
                tr("Rate limit reached. Please wait a moment or try another model."));
            return;
        }

        // Try extracting error message from JSON response
        QJsonParseError jsonErr;
        QJsonDocument errDoc = QJsonDocument::fromJson(data, &jsonErr);
        if (jsonErr.error == QJsonParseError::NoError && errDoc.isObject()) {
            QJsonObject errObj = errDoc.object().value("error").toObject();
            QString msg = errObj.value("message").toString();
            if (!msg.isEmpty()) {
                emit errorOccurred(tr("API Error (%1): %2").arg(statusCode).arg(msg));
                return;
            }
        }

        emit errorOccurred(
            tr("Network request failed: %1 (HTTP %2)").arg(reply->errorString()).arg(statusCode));
        return;
    }

    QJsonParseError jsonErr;
    QJsonDocument doc = QJsonDocument::fromJson(data, &jsonErr);
    if (jsonErr.error != QJsonParseError::NoError || !doc.isObject()) {
        emit errorOccurred(tr("Failed to parse API response JSON."));
        return;
    }

    QJsonObject resp = doc.object();
    QJsonArray choices = resp.value("choices").toArray();
    if (choices.isEmpty()) {
        emit errorOccurred(tr("API returned no choices."));
        return;
    }

    QJsonObject firstChoice = choices.first().toObject();
    QJsonObject message = firstChoice.value("message").toObject();
    QString content = message.value("content").toString();

    QString cleaned = cleanMarkdownOutline(content);
    if (cleaned.isEmpty()) {
        emit errorOccurred(tr("Model returned an empty outline."));
        return;
    }

    emit finished(cleaned);
}
