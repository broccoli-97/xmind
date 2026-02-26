#pragma once

#include <QObject>

class TabManager;
class QWidget;

class FileManager : public QObject {
    Q_OBJECT
public:
    explicit FileManager(QWidget* parentWindow, TabManager* tabManager, QObject* parent = nullptr);

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

private:
    QWidget* m_window;
    TabManager* m_tabManager;
};
