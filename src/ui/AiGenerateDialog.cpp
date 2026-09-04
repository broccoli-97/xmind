#include "ui/AiGenerateDialog.h"
#include "core/AiClient.h"
#include "core/AppSettings.h"
#include "core/SettingsDialog.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>

AiGenerateDialog::AiGenerateDialog(QWidget* parent)
    : QDialog(parent), m_aiClient(new AiClient(this)) {
    setWindowTitle(tr("Generate Mind Map from Text (AI)"));
    resize(540, 420);
    setupUI();

    connect(m_aiClient, &AiClient::started, this, &AiGenerateDialog::onAiStarted);
    connect(m_aiClient, &AiClient::finished, this, &AiGenerateDialog::onAiFinished);
    connect(m_aiClient, &AiClient::errorOccurred, this, &AiGenerateDialog::onAiError);
}

AiGenerateDialog::~AiGenerateDialog() = default;

void AiGenerateDialog::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);

    auto* headerLabel =
        new QLabel(tr("Enter a topic, notes, or outline to generate a mind map:"), this);
    mainLayout->addWidget(headerLabel);

    m_inputText = new QPlainTextEdit(this);
    m_inputText->setPlaceholderText(
        tr("e.g. Distributed system consensus algorithms\n"
           "- Paxos and Raft\n"
           "- Leader election\n"
           "- Log replication\n\n"
           "Or paste any meeting notes, article fragments, or topic ideas here..."));
    mainLayout->addWidget(m_inputText, 1);

    // Model info and quick settings bar
    auto* infoBar = new QHBoxLayout();
    m_modelInfoLabel = new QLabel(this);
    m_modelInfoLabel->setObjectName("settingsHint");
    infoBar->addWidget(m_modelInfoLabel, 1);

    m_settingsBtn = new QPushButton(tr("Settings..."), this);
    connect(m_settingsBtn, &QPushButton::clicked, this, &AiGenerateDialog::onOpenSettings);
    infoBar->addWidget(m_settingsBtn);
    mainLayout->addLayout(infoBar);

    // Progress bar and status
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 0); // marquee
    m_progressBar->setVisible(false);
    m_progressBar->setFixedHeight(6);
    m_progressBar->setTextVisible(false);
    mainLayout->addWidget(m_progressBar);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setWordWrap(true);
    mainLayout->addWidget(m_statusLabel);

    // Action buttons
    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_cancelBtn = new QPushButton(tr("Cancel"), this);
    connect(m_cancelBtn, &QPushButton::clicked, this, &AiGenerateDialog::onCancelClicked);
    buttonLayout->addWidget(m_cancelBtn);

    m_generateBtn = new QPushButton(tr("Generate"), this);
    m_generateBtn->setDefault(true);
    connect(m_generateBtn, &QPushButton::clicked, this, &AiGenerateDialog::onGenerateClicked);
    buttonLayout->addWidget(m_generateBtn);

    mainLayout->addLayout(buttonLayout);

    updateModelInfo();
}

void AiGenerateDialog::updateModelInfo() {
    auto& s = AppSettings::instance();
    if (s.aiApiKey().trimmed().isEmpty()) {
        m_modelInfoLabel->setText(tr("API Key not set. Click Settings to configure."));
    } else {
        m_modelInfoLabel->setText(tr("Provider: %1 | Model: %2").arg(s.aiProvider(), s.aiModel()));
    }
}

void AiGenerateDialog::onOpenSettings() {
    SettingsDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        updateModelInfo();
        m_statusLabel->clear();
    }
}

void AiGenerateDialog::onGenerateClicked() {
    QString prompt = m_inputText->toPlainText().trimmed();
    if (prompt.isEmpty()) {
        m_statusLabel->setText(tr("Please enter text or a topic first."));
        m_inputText->setFocus();
        return;
    }

    auto& s = AppSettings::instance();
    QString apiKey = s.aiApiKey().trimmed();
    if (apiKey.isEmpty()) {
        auto reply = QMessageBox::information(
            this, tr("API Key Required"),
            tr("Please configure your AI API Key before generating mind maps.\n"
               "Would you like to open Settings now?"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (reply == QMessageBox::Yes) {
            onOpenSettings();
        }
        return;
    }

    QString endpoint =
        (s.aiProvider() == QStringLiteral("Custom")) ? s.aiCustomEndpoint() : QString();
    m_statusLabel->clear();
    m_aiClient->generateMindMapOutline(prompt, apiKey, s.aiModel(), endpoint);
}

void AiGenerateDialog::onCancelClicked() {
    if (m_aiClient->isBusy()) {
        m_aiClient->cancel();
        m_inputText->setEnabled(true);
        m_generateBtn->setEnabled(true);
        m_progressBar->setVisible(false);
        m_statusLabel->setText(tr("Generation canceled."));
    } else {
        reject();
    }
}

void AiGenerateDialog::onAiStarted() {
    m_inputText->setEnabled(false);
    m_generateBtn->setEnabled(false);
    m_progressBar->setVisible(true);
    m_statusLabel->setText(tr("Generating mind map with AI..."));
}

void AiGenerateDialog::onAiFinished(const QString& markdownOutline) {
    emit outlineGenerated(markdownOutline);
    accept();
}

void AiGenerateDialog::onAiError(const QString& errorMessage) {
    m_inputText->setEnabled(true);
    m_generateBtn->setEnabled(true);
    m_progressBar->setVisible(false);
    m_statusLabel->setText(errorMessage);
}
