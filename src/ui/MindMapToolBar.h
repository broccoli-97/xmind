#pragma once

#include <QWidget>

class FileManager;
class QAction;
class QToolButton;
class TabManager;

// The inline toolbar above the tabbed content stack: undo/redo, add/delete,
// auto-layout, zoom, fit, export. Extracted from MainWindow so the wiring
// (lambdas → TabManager::currentScene/currentView) lives next to the buttons
// rather than buried in MainWindow.
class MindMapToolBar : public QWidget {
    Q_OBJECT

public:
    MindMapToolBar(TabManager* tabManager, FileManager* fileManager, QAction* undoAct,
                   QAction* redoAct, QWidget* parent = nullptr);

signals:
    void closeRequested();
    void aiGenerateRequested();

private:
    void buildContent();
    QToolButton* addButton(const QString& iconName, const QString& text, const QString& tooltip);
    void addSeparator();

    TabManager* m_tabManager;
    FileManager* m_fileManager;
    QAction* m_undoAct;
    QAction* m_redoAct;
    class QHBoxLayout* m_layout = nullptr;
};
