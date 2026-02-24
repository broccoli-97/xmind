#pragma once

#include "AppSettings.h"

#include <QMainWindow>

class MindMapView;
class MindMapScene;
class NodeItem;
class QTimer;
class QDockWidget;
class QTreeWidget;
class QTreeWidgetItem;
class QListWidget;

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
    void setupSidebar();
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

    void refreshOutline();
    void buildOutlineSubtree(NodeItem* node, QTreeWidgetItem* parentItem);
    void onOutlineItemClicked(QTreeWidgetItem* item, int column);
    void loadTemplate(int index);

    static QIcon makeToolIcon(const QString& name);
    static QPixmap makeTemplatePreview(int index);

    MindMapView* m_view;
    MindMapScene* m_scene;
    QString m_currentFile;

    QAction* m_undoAct = nullptr;
    QAction* m_redoAct = nullptr;

    QDockWidget* m_sidebarDock = nullptr;
    QTreeWidget* m_outlineTree = nullptr;
    QListWidget* m_templateList = nullptr;

    QTimer* m_autoSaveTimer = nullptr;
};
