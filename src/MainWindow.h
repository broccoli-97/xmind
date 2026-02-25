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
class QTabWidget;
class QToolButton;

struct TabState {
    MindMapScene* scene = nullptr;
    MindMapView* view = nullptr;
    QString filePath;
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    // Tab management
    void setupTabWidget();
    void addNewTab();
    void addTab(MindMapScene* scene, MindMapView* view, const QString& filePath);
    void closeTab(int index);
    void switchToTab(int index);
    void updateTabText(int index);
    void disconnectUndoStack();
    void connectSceneSignals(MindMapScene* scene);
    bool maybeSaveTab(int index);
    bool isTabEmpty(int index) const;
    int findTabByFilePath(const QString& filePath) const;
    void onTabBarContextMenu(const QPoint& pos);

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
    void exportAsPng();
    void exportAsSvg();
    void exportAsPdf();
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

    // Cached active-tab pointers
    MindMapView* m_view = nullptr;
    MindMapScene* m_scene = nullptr;
    QString m_currentFile;

    // Tab infrastructure
    QTabWidget* m_tabWidget = nullptr;
    QList<TabState> m_tabs;

    QAction* m_undoAct = nullptr;
    QAction* m_redoAct = nullptr;

    QDockWidget* m_sidebarDock = nullptr;
    QTreeWidget* m_outlineTree = nullptr;
    QListWidget* m_templateList = nullptr;

    QTimer* m_autoSaveTimer = nullptr;
};
