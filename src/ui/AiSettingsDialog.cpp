#include "ui/AiSettingsDialog.h"
#include "core/AiClient.h"
#include "core/AppSettings.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

AiSettingsDialog::AiSettingsDialog(QWidget* parent) : QDialog(parent) {
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

    m_endpointEdit = new QLineEdit;
    m_endpointEdit->setPlaceholderText(QString::fromLatin1(AiClient::kDefaultOrcaEndpoint));
    aiLayout->addRow(tr("Endpoint URL:"), m_endpointEdit);

    auto* getKeyLabel = new QLabel;
    getKeyLabel->setOpenExternalLinks(true);
    getKeyLabel->setText(QStringLiteral("<a href=\"%1\">%2</a>")
                             .arg(QString::fromLatin1(AiClient::kOrcaPartnerUrl),
                                  tr("Get free OrcaRouter API Key")));
    getKeyLabel->setObjectName("settingsHint");
    aiLayout->addRow("", getKeyLabel);

    auto updateProviderUI = [this]() {
        bool isCustom = (m_providerCombo->currentData().toString() == QLatin1String("Custom"));
        m_endpointEdit->setEnabled(isCustom);
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

    loadCurrentSettings();
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
}

void AiSettingsDialog::apply() {
    auto& s = AppSettings::instance();
    s.setAiProvider(m_providerCombo->currentData().toString());
    s.setAiModel(m_modelCombo->currentText().trimmed());
    s.setAiApiKey(m_apiKeyEdit->text().trimmed());
    s.setAiCustomEndpoint(m_endpointEdit->text().trimmed());
}
