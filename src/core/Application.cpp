#include "core/Application.h"

#include <QFileOpenEvent>
#include <QTimer>
#include <QUrl>
#include <utility>

bool Application::event(QEvent* event) {
    if (event->type() == QEvent::FileOpen) {
        auto* openEvent = static_cast<QFileOpenEvent*>(event);
        const QString path =
            openEvent->file().isEmpty() ? openEvent->url().toLocalFile() : openEvent->file();
        if (!path.isEmpty()) {
            m_pendingFiles.append(path);
            if (m_fileOpenHandling)
                QTimer::singleShot(0, this, &Application::dispatchPendingFiles);
            event->accept();
            return true;
        }
    }
    return QApplication::event(event);
}

void Application::startFileOpenHandling() {
    m_fileOpenHandling = true;
    QTimer::singleShot(0, this, &Application::dispatchPendingFiles);
}

void Application::dispatchPendingFiles() {
    // Opening a file can enter a native dialog's nested event loop. Remove the
    // current batch before emitting so new requests remain in their own batch.
    const QStringList files = std::exchange(m_pendingFiles, {});
    for (const QString& path : files)
        emit fileOpenRequested(path);
}
