#pragma once

#include "AppSettings.h"

#include <QMainWindow>

class MindMapView;
class MindMapScene;
class NodeItem;
class QTimer;
class QTreeWidget;
class QTreeWidgetItem;
class QTabBar;
class QToolButton;
class QStackedWidget;
class QSplitter;

struct TabState {
    MindMapScene* scene = nullptr;
    MindMapView* view = nullptr;
    QStackedWidget* stack = nullptr;
    QString filePath;
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    // Layout and tab management
    void setupCentralLayout();
    void addNewTab();
    void addTab(MindMapScene* scene, MindMapView* view, QStackedWidget* stack,
                const QString& filePath);
    void closeTab(int index);
    void switchToTab(int index);
    void updateTabText(int index);
    void disconnectUndoStack();
    void connectSceneSignals(MindMapScene* scene);
    bool maybeSaveTab(int index);
    bool isTabEmpty(int index) const;
    int findTabByFilePath(const QString& filePath) const;
    void onTabBarContextMenu(const QPoint& pos);
    void updateContentVisibility();

    void setupActions();
    void setupToolBar();
    void setupMenuBar();
    void setupOutlinePanel();
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
    QWidget* createStartPage();
    void activateBlankCanvas();

    static QIcon makeToolIcon(const QString& name);
    static QPixmap makeTemplatePreview(int index, int width = 120, int height = 80);

    // Cached active-tab pointers
    MindMapView* m_view = nullptr;
    MindMapScene* m_scene = nullptr;
    QString m_currentFile;

    // Tab infrastructure
    QTabBar* m_tabBar = nullptr;
    QStackedWidget* m_contentStack = nullptr;
    QToolButton* m_newTabBtn = nullptr;
    QList<TabState> m_tabs;

    // Toolbar, outline, content area
    QWidget* m_toolbarWidget = nullptr;
    QSplitter* m_contentSplitter = nullptr;
    QWidget* m_outlinePanel = nullptr;
    QTreeWidget* m_outlineTree = nullptr;
    QWidget* m_rightPanel = nullptr;

    QToolButton* m_toggleOutlineBtn = nullptr;
    QToolButton* m_toggleToolbarBtn = nullptr;

    QAction* m_toggleToolbarAct = nullptr;
    QAction* m_toggleOutlineAct = nullptr;
    QAction* m_undoAct = nullptr;
    QAction* m_redoAct = nullptr;
    QAction* m_addChildAct = nullptr;
    QAction* m_addSiblingAct = nullptr;

    QTimer* m_autoSaveTimer = nullptr;
};
