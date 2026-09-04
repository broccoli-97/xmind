#include "core/SettingsDialog.h"
#include "core/AiClient.h"
#include "core/AppSettings.h"
#include "ui/ThemeManager.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFontComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QSpinBox>
#include <QTimer>
#include <QVBoxLayout>

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("Settings"));
    setMinimumWidth(360);

    auto* mainLayout = new QVBoxLayout(this);

    // Appearance group
    auto* appearanceGroup = new QGroupBox(tr("Appearance"));
    auto* appearanceLayout = new QFormLayout(appearanceGroup);
    m_themeCombo = new QComboBox;
    m_themeCombo->addItem(tr("Light"), 0);
    m_themeCombo->addItem(tr("Dark"), 1);
    appearanceLayout->addRow(tr("Theme:"), m_themeCombo);

    m_syncSystemThemeBtn = new QPushButton(tr("Sync with System Theme"));
    connect(m_syncSystemThemeBtn, &QPushButton::clicked, this, &SettingsDialog::onSyncSystemTheme);
    appearanceLayout->addRow("", m_syncSystemThemeBtn);

    m_languageCombo = new QComboBox;
    m_languageCombo->addItem("English", "en");
    m_languageCombo->addItem(QString::fromUtf8("简体中文"), "zh_CN");
    appearanceLayout->addRow(tr("Language:"), m_languageCombo);

    auto* langHint = new QLabel(tr("Restart required to apply language change"));
    langHint->setObjectName("settingsHint");
    appearanceLayout->addRow(langHint);

    mainLayout->addWidget(appearanceGroup);

    // Auto-save group
    auto* autoSaveGroup = new QGroupBox(tr("Auto-save"));
    auto* autoSaveLayout = new QFormLayout(autoSaveGroup);
    m_autoSaveCheck = new QCheckBox(tr("Enable auto-save"));
    autoSaveLayout->addRow(m_autoSaveCheck);
    m_autoSaveIntervalSpin = new QSpinBox;
    m_autoSaveIntervalSpin->setRange(1, 5);
    m_autoSaveIntervalSpin->setSuffix(tr(" min"));
    autoSaveLayout->addRow(tr("Interval:"), m_autoSaveIntervalSpin);
    connect(m_autoSaveCheck, &QCheckBox::toggled, m_autoSaveIntervalSpin, &QWidget::setEnabled);
    mainLayout->addWidget(autoSaveGroup);

    // Editor group
    auto* editorGroup = new QGroupBox(tr("Editor"));
    auto* editorLayout = new QFormLayout(editorGroup);
    m_fontFamilyCombo = new QFontComboBox;
    editorLayout->addRow(tr("Default font:"), m_fontFamilyCombo);
    m_fontSizeSpin = new QSpinBox;
    m_fontSizeSpin->setRange(8, 24);
    m_fontSizeSpin->setSuffix(tr(" pt"));
    editorLayout->addRow(tr("Default font size:"), m_fontSizeSpin);
    auto* hint = new QLabel(tr("Applies to newly created nodes only"));
    hint->setObjectName("settingsHint");
    editorLayout->addRow(hint);
    mainLayout->addWidget(editorGroup);

    // Updates group
    auto* updatesGroup = new QGroupBox(tr("Updates"));
    auto* updatesLayout = new QFormLayout(updatesGroup);
    m_checkUpdatesCheck = new QCheckBox(tr("Check for updates on startup"));
    updatesLayout->addRow(m_checkUpdatesCheck);
    mainLayout->addWidget(updatesGroup);

    // AI Service group
    auto* aiGroup = new QGroupBox(tr("AI Service"));
    auto* aiLayout = new QFormLayout(aiGroup);

    m_aiProviderCombo = new QComboBox;
    m_aiProviderCombo->addItem(tr("OrcaRouter (Recommended)"), QStringLiteral("OrcaRouter"));
    m_aiProviderCombo->addItem(tr("Custom (OpenAI-compatible)"), QStringLiteral("Custom"));
    aiLayout->addRow(tr("Provider:"), m_aiProviderCombo);

    m_aiModelCombo = new QComboBox;
    m_aiModelCombo->setEditable(true);
    m_aiModelCombo->addItem(QStringLiteral("deepseek/deepseek-chat:free"));
    m_aiModelCombo->addItem(QStringLiteral("qwen/qwen-2.5-72b-instruct:free"));
    m_aiModelCombo->addItem(QStringLiteral("orcarouter/auto"));
    aiLayout->addRow(tr("Model:"), m_aiModelCombo);

    m_aiApiKeyEdit = new QLineEdit;
    m_aiApiKeyEdit->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    m_aiApiKeyEdit->setPlaceholderText(tr("Enter API Key"));
    aiLayout->addRow(tr("API Key:"), m_aiApiKeyEdit);

    m_aiEndpointEdit = new QLineEdit;
    m_aiEndpointEdit->setPlaceholderText(QString::fromLatin1(AiClient::kDefaultOrcaEndpoint));
    aiLayout->addRow(tr("Endpoint URL:"), m_aiEndpointEdit);

    auto* getKeyLabel = new QLabel;
    getKeyLabel->setOpenExternalLinks(true);
    getKeyLabel->setText(QStringLiteral("<a href=\"%1\">%2</a>")
                             .arg(QString::fromLatin1(AiClient::kOrcaPartnerUrl),
                                  tr("Get free OrcaRouter API Key")));
    getKeyLabel->setObjectName("settingsHint");
    aiLayout->addRow("", getKeyLabel);

    auto updateProviderUI = [this]() {
        bool isCustom = (m_aiProviderCombo->currentData().toString() == QLatin1String("Custom"));
        m_aiEndpointEdit->setEnabled(isCustom);
        if (!isCustom) {
            m_aiEndpointEdit->setText(QString::fromLatin1(AiClient::kDefaultOrcaEndpoint));
        }
    };
    connect(m_aiProviderCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
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

void SettingsDialog::loadCurrentSettings() {
    auto& s = AppSettings::instance();
    m_themeCombo->setCurrentIndex(s.theme() == AppTheme::Dark ? 1 : 0);
    m_autoSaveCheck->setChecked(s.autoSaveEnabled());
    m_autoSaveIntervalSpin->setValue(s.autoSaveIntervalMinutes());
    m_autoSaveIntervalSpin->setEnabled(s.autoSaveEnabled());
    m_fontSizeSpin->setValue(s.defaultFontSize());
    m_fontFamilyCombo->setCurrentFont(QFont(s.defaultFontFamily()));
    m_checkUpdatesCheck->setChecked(s.checkForUpdatesEnabled());

    int langIdx = m_languageCombo->findData(s.language());
    if (langIdx >= 0)
        m_languageCombo->setCurrentIndex(langIdx);

    int provIdx = m_aiProviderCombo->findData(s.aiProvider());
    if (provIdx >= 0)
        m_aiProviderCombo->setCurrentIndex(provIdx);
    else
        m_aiProviderCombo->setCurrentIndex(0);

    m_aiModelCombo->setEditText(s.aiModel());
    m_aiApiKeyEdit->setText(s.aiApiKey());
    m_aiEndpointEdit->setText(s.aiCustomEndpoint());
    bool isCustom = (m_aiProviderCombo->currentData().toString() == QLatin1String("Custom"));
    m_aiEndpointEdit->setEnabled(isCustom);
}

void SettingsDialog::onSyncSystemTheme() {
    // Detect system theme and update combo box
    bool isDarkMode = ThemeManager::isSystemDarkMode();
    m_themeCombo->setCurrentIndex(isDarkMode ? 1 : 0);
}

void SettingsDialog::apply() {
    auto& s = AppSettings::instance();
    s.setTheme(m_themeCombo->currentIndex() == 1 ? AppTheme::Dark : AppTheme::Light);
    s.setAutoSaveEnabled(m_autoSaveCheck->isChecked());
    s.setAutoSaveIntervalMinutes(m_autoSaveIntervalSpin->value());
    s.setDefaultFontSize(m_fontSizeSpin->value());
    s.setDefaultFontFamily(m_fontFamilyCombo->currentFont().family());
    s.setCheckForUpdatesEnabled(m_checkUpdatesCheck->isChecked());

    s.setAiProvider(m_aiProviderCombo->currentData().toString());
    s.setAiModel(m_aiModelCombo->currentText().trimmed());
    s.setAiApiKey(m_aiApiKeyEdit->text().trimmed());
    s.setAiCustomEndpoint(m_aiEndpointEdit->text().trimmed());

    const QString oldLang = s.language();
    const QString newLang = m_languageCombo->currentData().toString();
    s.setLanguage(newLang);

    if (oldLang != newLang) {
        auto reply = QMessageBox::question(
            this, tr("Restart Required"),
            tr("The language change will take effect after restarting YMind. Restart now?"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (reply == QMessageBox::Yes) {
            const QString program = QApplication::applicationFilePath();
            const QStringList args = QApplication::arguments().mid(1);
            const QString workingDir = QDir::currentPath();
            QObject::connect(qApp, &QCoreApplication::aboutToQuit, qApp,
                             [program, args, workingDir]() {
                                 QProcess::startDetached(program, args, workingDir);
                             });
            // Defer so this dialog's accept() can return first. closeAllWindows()
            // routes through MainWindow::closeEvent -> maybeSave(), so cancelling
            // an unsaved-changes prompt aborts the restart cleanly.
            QTimer::singleShot(0, qApp, []() { qApp->closeAllWindows(); });
        }
    }
}
