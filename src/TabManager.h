#pragma once

#include <QList>
#include <QObject>


class MindMapScene;
class MindMapView;
class QTabBar;
class QStackedWidget;
class QToolButton;
class QAction;

struct TabState {
    MindMapScene* scene = nullptr;
    MindMapView* view = nullptr;
    QStackedWidget* stack = nullptr;
    QString filePath;
};

class TabManager : public QObject {
    Q_OBJECT
public:
    explicit TabManager(QWidget* parent);

    void init(QAction* undoAct, QAction* redoAct);

    void addNewTab();
    void addTab(MindMapScene* scene, MindMapView* view, QStackedWidget* stack,
                const QString& filePath);
    void closeTab(int index);
    void switchToTab(int index);
    void updateTabText(int index);
    bool isTabEmpty(int index) const;
    int findTabByFilePath(const QString& filePath) const;
    bool maybeSaveTab(int index);
    bool maybeSave();
    void onTabBarContextMenu(const QPoint& pos);

    // Accessors
    int currentIndex() const;
    int tabCount() const;
    const QList<TabState>& tabs() const;
    TabState& tab(int index);
    MindMapScene* currentScene() const;
    MindMapView* currentView() const;
    QString currentFilePath() const;
    void setCurrentFilePath(const QString& path);

    QTabBar* tabBar() const;
    QToolButton* newTabButton() const;
    QStackedWidget* contentStack() const;

signals:
    void currentTabChanged(int index);
    void tabTextUpdated(int index);
    void saveRequested();

private slots:
    void onTabMoved(int from, int to);

private:
    void connectSceneSignals(MindMapScene* scene);
    void connectUndoStack();
    void disconnectUndoStack();

    QWidget* m_parentWidget;
    QTabBar* m_tabBar = nullptr;
    QStackedWidget* m_contentStack = nullptr;
    QToolButton* m_newTabBtn = nullptr;
    QAction* m_undoAct = nullptr;
    QAction* m_redoAct = nullptr;

    QList<TabState> m_tabs;
    MindMapScene* m_scene = nullptr;
    MindMapView* m_view = nullptr;
    QString m_currentFile;
};
