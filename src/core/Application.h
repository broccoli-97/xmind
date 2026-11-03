#pragma once

#include <QApplication>
#include <QStringList>

// Finder sends file-open events to the application rather than passing argv
// when an existing instance is used. Deliver them after window initialization.
class Application : public QApplication {
    Q_OBJECT
public:
    using QApplication::QApplication;
    void startFileOpenHandling();

signals:
    void fileOpenRequested(const QString& path);

protected:
    bool event(QEvent* event) override;

private:
    void dispatchPendingFiles();

    QStringList m_pendingFiles;
    bool m_fileOpenHandling = false;
};
