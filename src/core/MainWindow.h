#pragma once

#include <QMainWindow>

class AutoSaveManager;
class FileManager;
class MindMapToolBar;
class OutlineWidget;
class QFrame;
class QLabel;
class QMenu;
class QSplitter;
class QToolButton;
class RecoveryManager;
class TabManager;
class TemplateMenuController;
class ThemeMenuController;
class UpdateNotifier;

struct Services;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(const Services& services, QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void setupCentralLayout();
    void setupActions();
    void setupMenuBar();

    // Run `f(scene)` / `f(view)` on the active tab's scene/view if one exists.
    // Kills the swarm of `[this](){ if (auto* s = ...->currentScene()) s->...; }`
    // lambdas that menu wiring would otherwise need.
    template <typename F> void withCurrentScene(F&& f);
    template <typename F> void withCurrentView(F&& f);

    void updateWindowTitle();
    void updateContentVisibility();

    void openSettings();
    void openAbout();
    void saveWindowState();
    void restoreWindowState();
    void applyTheme();
    void refreshOutline();
    void setupStatusBar();
    // Refresh the status bar's contextual hint based on current state.
    // Three messages: start page, in-edit, idle.
    void updateStatusHint();
    // Connect the current scene's editingStarted/editingFinished signals to
    // updateStatusHint. Called on every tab change so the wiring follows the
    // active scene.
    void connectCurrentSceneToStatusHint();

    // Returns true if the user accepted a restore (so MainWindow should skip
    // creating the default Untitled tab). Pops a single Yes/No prompt covering
    // all orphan snapshots; either restores them all or discards them all.
    bool maybeRestoreOrphanSessions();

    // Managers
    const Services* m_services = nullptr;
    TabManager* m_tabManager = nullptr;
    FileManager* m_fileManager = nullptr;
    AutoSaveManager* m_autoSave = nullptr;
    RecoveryManager* m_recovery = nullptr;
    UpdateNotifier* m_updateNotifier = nullptr;
    TemplateMenuController* m_templateMenuController = nullptr;
    ThemeMenuController* m_themeMenuController = nullptr;

    // Widgets
    OutlineWidget* m_outlineWidget = nullptr;
    MindMapToolBar* m_toolbar = nullptr;
    QSplitter* m_contentSplitter = nullptr;
    QWidget* m_rightPanel = nullptr;

    QToolButton* m_toggleOutlineBtn = nullptr;
    QToolButton* m_toggleToolbarBtn = nullptr;
    QLabel* m_statusHelpLabel = nullptr;

    QAction* m_toggleToolbarAct = nullptr;
    QAction* m_toggleOutlineAct = nullptr;
    QAction* m_undoAct = nullptr;
    QAction* m_redoAct = nullptr;
    QAction* m_addChildAct = nullptr;
    QAction* m_addSiblingAct = nullptr;
};
