#pragma once

#include <QMainWindow>

class MindMapView;
class MindMapScene;

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
    bool maybeSave();

    void newFile();
    void openFile();
    void saveFile();
    void saveFileAs();
    void exportAsText();
    void exportAsMarkdown();
    void importFromText();

    MindMapView* m_view;
    MindMapScene* m_scene;
    QString m_currentFile;
};
