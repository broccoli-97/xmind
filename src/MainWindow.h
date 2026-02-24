#pragma once

#include "AppSettings.h"

#include <QMainWindow>

class MindMapView;
class MindMapScene;
class QTimer;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void setupActions();
    void setupToolBar();
    void setupMenuBar();
    void updateWindowTitle();
    void connectUndoStack();
    bool maybeSave();

    void newFile();
    void openFile();
    void saveFile();
    void saveFileAs();
    void exportAsText();
    void exportAsMarkdown();
    void importFromText();

    void openSettings();
    void saveWindowState();
    void restoreWindowState();
    void setupAutoSaveTimer();
    void onAutoSaveTimeout();
    void onAutoSaveSettingsChanged();
    void applyTheme();

    MindMapView* m_view;
    MindMapScene* m_scene;
    QString m_currentFile;

    QAction* m_undoAct = nullptr;
    QAction* m_redoAct = nullptr;

    QTimer* m_autoSaveTimer = nullptr;
};
