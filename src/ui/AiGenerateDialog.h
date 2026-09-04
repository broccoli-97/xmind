#pragma once

#include <QDialog>

class AiClient;
class QPlainTextEdit;
class QPushButton;
class QProgressBar;
class QLabel;

class AiGenerateDialog : public QDialog {
    Q_OBJECT

public:
    explicit AiGenerateDialog(QWidget* parent = nullptr);
    ~AiGenerateDialog() override;

signals:
    void outlineGenerated(const QString& markdownOutline);

private slots:
    void onGenerateClicked();
    void onCancelClicked();
    void onAiStarted();
    void onAiFinished(const QString& markdownOutline);
    void onAiError(const QString& errorMessage);
    void onOpenSettings();

private:
    void setupUI();
    void updateModelInfo();

    AiClient* m_aiClient;
    QPlainTextEdit* m_inputText;
    QLabel* m_modelInfoLabel;
    QProgressBar* m_progressBar;
    QLabel* m_statusLabel;
    QPushButton* m_generateBtn;
    QPushButton* m_cancelBtn;
    QPushButton* m_settingsBtn;
};
