#pragma once

#include <QDialog>

class QComboBox;
class QCheckBox;
class QSpinBox;

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);

private:
    void loadCurrentSettings();
    void apply();

    QComboBox* m_themeCombo;
    QCheckBox* m_autoSaveCheck;
    QSpinBox* m_autoSaveIntervalSpin;
    QSpinBox* m_fontSizeSpin;
};
