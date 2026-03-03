#pragma once

#include <QObject>
#include <functional>

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
    // Common export helper: shows save dialog, validates extension, runs exporter, shows status.
    // |dialogTitle|: title of the QFileDialog
    // |filter|: file filter string
    // |defaultExt|: extension to append if missing (e.g. ".txt")
    // |exporter|: callback taking the chosen file path, returns true on success
    // |errorLabel|: format label for error message (e.g. "file", "PNG", "SVG")
    void doExport(const QString& dialogTitle, const QString& filter, const QString& defaultExt,
                  std::function<bool(const QString&)> exporter, const QString& errorLabel);

    QWidget* m_window;
    TabManager* m_tabManager;
};
