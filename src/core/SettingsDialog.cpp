#include "core/SettingsDialog.h"
#include "core/AppSettings.h"
#include "ui/ThemeManager.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFontComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
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
    m_fontSizeSpin->setSuffix(" pt");
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
    s.setLanguage(m_languageCombo->currentData().toString());
}
