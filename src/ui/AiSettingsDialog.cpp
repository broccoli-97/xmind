#include "ui/AiSettingsDialog.h"
#include "core/AiClient.h"
#include "core/AppSettings.h"

#include <QComboBox>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUrl>
#include <QUrlQuery>
#include <QUuid>
#include <QVBoxLayout>

AiSettingsDialog::AiSettingsDialog(QWidget* parent)
    : QDialog(parent), m_aiClient(new AiClient(this)) {
    setWindowTitle(tr("AI Settings"));
    setMinimumWidth(400);

    auto* mainLayout = new QVBoxLayout(this);

    // AI Service group
    auto* aiGroup = new QGroupBox(tr("AI Service"));
    auto* aiLayout = new QFormLayout(aiGroup);

    m_providerCombo = new QComboBox;
    m_providerCombo->addItem(tr("OrcaRouter (Recommended)"), QStringLiteral("OrcaRouter"));
    m_providerCombo->addItem(tr("Custom (OpenAI-compatible)"), QStringLiteral("Custom"));
    aiLayout->addRow(tr("Provider:"), m_providerCombo);

    m_modelCombo = new QComboBox;
    m_modelCombo->setEditable(true);
    m_modelCombo->addItem(QStringLiteral("deepseek/deepseek-chat:free"));
    m_modelCombo->addItem(QStringLiteral("qwen/qwen-2.5-72b-instruct:free"));
    m_modelCombo->addItem(QStringLiteral("orcarouter/auto"));
    aiLayout->addRow(tr("Model:"), m_modelCombo);

    m_apiKeyEdit = new QLineEdit;
    m_apiKeyEdit->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    m_apiKeyEdit->setPlaceholderText(tr("Enter API Key"));
    aiLayout->addRow(tr("API Key:"), m_apiKeyEdit);

    // One-click get API Key button
    auto* keyBtnLayout = new QHBoxLayout();
    m_getKeyBtn = new QPushButton(tr("Get API Key from OrcaRouter..."));
    connect(m_getKeyBtn, &QPushButton::clicked, this, &AiSettingsDialog::onGetApiKey);
    keyBtnLayout->addWidget(m_getKeyBtn);
    keyBtnLayout->addStretch();
    aiLayout->addRow("", keyBtnLayout);

    m_authStatusLabel = new QLabel;
    m_authStatusLabel->setObjectName("settingsHint");
    m_authStatusLabel->setWordWrap(true);
    m_authStatusLabel->setVisible(false);
    aiLayout->addRow(m_authStatusLabel);

    m_endpointEdit = new QLineEdit;
    m_endpointEdit->setPlaceholderText(QString::fromLatin1(AiClient::kDefaultOrcaEndpoint));
    aiLayout->addRow(tr("Endpoint URL:"), m_endpointEdit);

    auto updateProviderUI = [this]() {
        bool isCustom = (m_providerCombo->currentData().toString() == QLatin1String("Custom"));
        m_endpointEdit->setEnabled(isCustom);
        m_getKeyBtn->setVisible(!isCustom);
        if (!isCustom) {
            m_endpointEdit->setText(QString::fromLatin1(AiClient::kDefaultOrcaEndpoint));
        }
    };
    connect(m_providerCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            updateProviderUI);

    mainLayout->addWidget(aiGroup);

    mainLayout->addStretch();

    // Button box
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
        apply();
        accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttons);

    // Connect API key exchange signals
    connect(m_aiClient, &AiClient::apiKeyReceived, this, [this](const QString& key) {
        m_apiKeyEdit->setText(key);
        m_authStatusLabel->setText(tr("API Key obtained successfully!"));
        m_authStatusLabel->setVisible(true);
        m_getKeyBtn->setEnabled(true);
        m_getKeyBtn->setText(tr("Get API Key from OrcaRouter..."));
        if (m_callbackServer) {
            m_callbackServer->close();
            m_callbackServer->deleteLater();
            m_callbackServer = nullptr;
        }
    });
    connect(m_aiClient, &AiClient::apiKeyError, this, [this](const QString& err) {
        m_authStatusLabel->setText(err);
        m_authStatusLabel->setVisible(true);
        m_getKeyBtn->setEnabled(true);
        m_getKeyBtn->setText(tr("Get API Key from OrcaRouter..."));
        if (m_callbackServer) {
            m_callbackServer->close();
            m_callbackServer->deleteLater();
            m_callbackServer = nullptr;
        }
    });

    loadCurrentSettings();
}

void AiSettingsDialog::onGetApiKey() {
    // 1. Generate PKCE verifier and challenge
    m_codeVerifier = AiClient::generateCodeVerifier();
    QString challenge = AiClient::computeCodeChallenge(m_codeVerifier);
    m_oauthState = QUuid::createUuid().toString(QUuid::WithoutBraces);

    // 2. Start local TCP server for callback
    if (m_callbackServer) {
        m_callbackServer->close();
        m_callbackServer->deleteLater();
    }
    m_callbackServer = new QTcpServer(this);
    if (!m_callbackServer->listen(QHostAddress::LocalHost, 0)) {
        m_authStatusLabel->setText(tr("Failed to start local auth server."));
        m_authStatusLabel->setVisible(true);
        return;
    }
    connect(m_callbackServer, &QTcpServer::newConnection, this, &AiSettingsDialog::onAuthCallback);

    quint16 port = m_callbackServer->serverPort();
    QString callbackUrl = QStringLiteral("http://127.0.0.1:%1/cb").arg(port);

    // 3. Build auth URL
    QUrl authUrl(QString::fromLatin1(AiClient::kOrcaAuthBase));
    QUrlQuery query;
    query.addQueryItem("callback_url", callbackUrl);
    query.addQueryItem("code_challenge", challenge);
    query.addQueryItem("code_challenge_method", "S256");
    query.addQueryItem("state", m_oauthState);
    query.addQueryItem("app_name", QString::fromLatin1(AiClient::kProjectTitle));
    query.addQueryItem("ref", QString::fromLatin1(AiClient::kOrcaRefCode));
    authUrl.setQuery(query);

    // 4. Open browser and update UI
    QDesktopServices::openUrl(authUrl);
    m_getKeyBtn->setEnabled(false);
    m_getKeyBtn->setText(tr("Waiting for authorization..."));
    m_authStatusLabel->setText(
        tr("A browser window has been opened. Please authorize YMind on OrcaRouter, "
           "then return here."));
    m_authStatusLabel->setVisible(true);
}

void AiSettingsDialog::onAuthCallback() {
    if (!m_callbackServer)
        return;

    QTcpSocket* socket = m_callbackServer->nextPendingConnection();
    if (!socket)
        return;

    connect(socket, &QTcpSocket::readyRead, this, [this, socket]() {
        QByteArray data = socket->readAll();

        // Parse the HTTP GET request line: "GET /cb?code=...&state=... HTTP/1.1"
        QString requestLine = QString::fromUtf8(data).section('\n', 0, 0).trimmed();
        QString path = requestLine.section(' ', 1, 1); // "/cb?code=xxx&state=yyy"

        QUrl requestUrl(QStringLiteral("http://localhost") + path);
        QUrlQuery params(requestUrl.query());
        QString code = params.queryItemValue("code");
        QString state = params.queryItemValue("state");

        // Send response to browser
        QByteArray response;
        if (!code.isEmpty() && state == m_oauthState) {
            response = "HTTP/1.1 200 OK\r\n"
                       "Content-Type: text/html; charset=utf-8\r\n"
                       "Connection: close\r\n\r\n"
                       "<html><body style='font-family:system-ui;text-align:center;"
                       "padding:60px'>"
                       "<h2>&#10003; Authorization successful!</h2>"
                       "<p>You can close this window and return to YMind.</p>"
                       "</body></html>";
        } else {
            response = "HTTP/1.1 400 Bad Request\r\n"
                       "Content-Type: text/html; charset=utf-8\r\n"
                       "Connection: close\r\n\r\n"
                       "<html><body style='font-family:system-ui;text-align:center;"
                       "padding:60px'>"
                       "<h2>Authorization failed</h2>"
                       "<p>Please try again from YMind.</p>"
                       "</body></html>";
        }
        socket->write(response);
        socket->flush();
        socket->disconnectFromHost();

        // Close the server immediately — we only need one callback
        m_callbackServer->close();

        if (!code.isEmpty() && state == m_oauthState) {
            m_authStatusLabel->setText(tr("Exchanging authorization code for API Key..."));
            m_aiClient->exchangeCodeForKey(code, m_codeVerifier);
        } else {
            m_authStatusLabel->setText(tr("Authorization failed: invalid state or missing code."));
            m_getKeyBtn->setEnabled(true);
            m_getKeyBtn->setText(tr("Get API Key from OrcaRouter..."));
        }
    });
}

void AiSettingsDialog::loadCurrentSettings() {
    auto& s = AppSettings::instance();

    int provIdx = m_providerCombo->findData(s.aiProvider());
    if (provIdx >= 0)
        m_providerCombo->setCurrentIndex(provIdx);
    else
        m_providerCombo->setCurrentIndex(0);

    m_modelCombo->setEditText(s.aiModel());
    m_apiKeyEdit->setText(s.aiApiKey());
    m_endpointEdit->setText(s.aiCustomEndpoint());
    bool isCustom = (m_providerCombo->currentData().toString() == QLatin1String("Custom"));
    m_endpointEdit->setEnabled(isCustom);
    m_getKeyBtn->setVisible(!isCustom);
}

void AiSettingsDialog::apply() {
    auto& s = AppSettings::instance();
    s.setAiProvider(m_providerCombo->currentData().toString());
    s.setAiModel(m_modelCombo->currentText().trimmed());
    s.setAiApiKey(m_apiKeyEdit->text().trimmed());
    s.setAiCustomEndpoint(m_endpointEdit->text().trimmed());
}
