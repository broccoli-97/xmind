#pragma once

#include <QDialog>

class QComboBox;
class QLineEdit;

class AiSettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit AiSettingsDialog(QWidget* parent = nullptr);

private:
    void loadCurrentSettings();
    void apply();

    QComboBox* m_providerCombo;
    QComboBox* m_modelCombo;
    QLineEdit* m_apiKeyEdit;
    QLineEdit* m_endpointEdit;
};
