#pragma once

#include <QMainWindow>

class TabManager;
class FileManager;
class OutlineWidget;
class QLabel;
class QTimer;
class QSplitter;
class QToolButton;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void setupCentralLayout();
    void setupActions();
    void setupToolBar();
    void setupMenuBar();

    void updateWindowTitle();
    void updateContentVisibility();

    void openSettings();
    void saveWindowState();
    void restoreWindowState();
    void setupAutoSaveTimer();
    void onAutoSaveTimeout();
    void onAutoSaveSettingsChanged();
    void applyTheme();
    void refreshOutline();

    // Managers
    TabManager* m_tabManager = nullptr;
    FileManager* m_fileManager = nullptr;

    // Widgets
    OutlineWidget* m_outlineWidget = nullptr;
    QWidget* m_toolbarWidget = nullptr;
    QSplitter* m_contentSplitter = nullptr;
    QWidget* m_rightPanel = nullptr;

    QToolButton* m_toggleOutlineBtn = nullptr;
    QToolButton* m_toggleToolbarBtn = nullptr;
    QToolButton* m_undoBtn = nullptr;
    QToolButton* m_redoBtn = nullptr;
    QLabel* m_statusHelpLabel = nullptr;

    QAction* m_toggleToolbarAct = nullptr;
    QAction* m_toggleOutlineAct = nullptr;
    QAction* m_undoAct = nullptr;
    QAction* m_redoAct = nullptr;
    QAction* m_addChildAct = nullptr;
    QAction* m_addSiblingAct = nullptr;

    QTimer* m_autoSaveTimer = nullptr;
};
