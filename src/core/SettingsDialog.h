#pragma once

#include <QDialog>

class QComboBox;
class QCheckBox;
class QFontComboBox;
class QSpinBox;
class QPushButton;

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);

private slots:
    void onSyncSystemTheme();

private:
    void loadCurrentSettings();
    void apply();

    QComboBox* m_themeCombo;
    QPushButton* m_syncSystemThemeBtn;
    QCheckBox* m_autoSaveCheck;
    QSpinBox* m_autoSaveIntervalSpin;
    QFontComboBox* m_fontFamilyCombo;
    QSpinBox* m_fontSizeSpin;
};
