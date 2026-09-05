#pragma once

#include <QDialog>

class AiClient;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTcpServer;

class AiSettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit AiSettingsDialog(QWidget* parent = nullptr);

private slots:
    void onGetApiKey();
    void onAuthCallback();

private:
    void loadCurrentSettings();
    void apply();

    AiClient* m_aiClient;
    QComboBox* m_providerCombo;
    QComboBox* m_modelCombo;
    QLineEdit* m_apiKeyEdit;
    QLineEdit* m_endpointEdit;
    QPushButton* m_getKeyBtn;
    QLabel* m_authStatusLabel;

    // PKCE OAuth state
    QTcpServer* m_callbackServer = nullptr;
    QByteArray m_codeVerifier;
    QString m_oauthState;
};
