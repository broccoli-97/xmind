#include "SettingsDialog.h"
#include "AppSettings.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Settings");
    setMinimumWidth(360);

    auto* mainLayout = new QVBoxLayout(this);

    // Appearance group
    auto* appearanceGroup = new QGroupBox("Appearance");
    auto* appearanceLayout = new QFormLayout(appearanceGroup);
    m_themeCombo = new QComboBox;
    m_themeCombo->addItem("Light", 0);
    m_themeCombo->addItem("Dark", 1);
    appearanceLayout->addRow("Theme:", m_themeCombo);
    mainLayout->addWidget(appearanceGroup);

    // Auto-save group
    auto* autoSaveGroup = new QGroupBox("Auto-save");
    auto* autoSaveLayout = new QFormLayout(autoSaveGroup);
    m_autoSaveCheck = new QCheckBox("Enable auto-save");
    autoSaveLayout->addRow(m_autoSaveCheck);
    m_autoSaveIntervalSpin = new QSpinBox;
    m_autoSaveIntervalSpin->setRange(1, 5);
    m_autoSaveIntervalSpin->setSuffix(" min");
    autoSaveLayout->addRow("Interval:", m_autoSaveIntervalSpin);
    connect(m_autoSaveCheck, &QCheckBox::toggled, m_autoSaveIntervalSpin, &QWidget::setEnabled);
    mainLayout->addWidget(autoSaveGroup);

    // Editor group
    auto* editorGroup = new QGroupBox("Editor");
    auto* editorLayout = new QFormLayout(editorGroup);
    m_fontSizeSpin = new QSpinBox;
    m_fontSizeSpin->setRange(8, 24);
    m_fontSizeSpin->setSuffix(" pt");
    editorLayout->addRow("Default font size:", m_fontSizeSpin);
    auto* hint = new QLabel("Applies to newly created nodes only");
    hint->setStyleSheet("color: gray; font-size: 9pt;");
    editorLayout->addRow(hint);
    mainLayout->addWidget(editorGroup);

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
}

void SettingsDialog::apply() {
    auto& s = AppSettings::instance();
    s.setTheme(m_themeCombo->currentIndex() == 1 ? AppTheme::Dark : AppTheme::Light);
    s.setAutoSaveEnabled(m_autoSaveCheck->isChecked());
    s.setAutoSaveIntervalMinutes(m_autoSaveIntervalSpin->value());
    s.setDefaultFontSize(m_fontSizeSpin->value());
}
