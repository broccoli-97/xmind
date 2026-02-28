#include "FileManager.h"
#include "MindMapScene.h"
#include "MindMapView.h"
#include "TabManager.h"

#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMainWindow>
#include <QMessageBox>
#include <QStackedWidget>
#include <QStatusBar>

FileManager::FileManager(QWidget* parentWindow, TabManager* tabManager, QObject* parent)
    : QObject(parent), m_window(parentWindow), m_tabManager(tabManager) {}

void FileManager::newFile() {
    int cur = m_tabManager->currentIndex();
    if (cur >= 0 && m_tabManager->isTabEmpty(cur))
        return;
    m_tabManager->addNewTab();
}

void FileManager::openFile() {
    QString filePath =
        QFileDialog::getOpenFileName(m_window, "Open Mind Map", QString(),
                                     "XMind Files (*.xmind);;JSON Files (*.json);;All Files (*)");
    if (filePath.isEmpty())
        return;

    int existing = m_tabManager->findTabByFilePath(filePath);
    if (existing >= 0) {
        m_tabManager->switchToTab(existing);
        return;
    }

    int cur = m_tabManager->currentIndex();
    if (cur >= 0 && m_tabManager->isTabEmpty(cur)) {
        auto* scene = m_tabManager->currentScene();
        auto* view = m_tabManager->currentView();
        if (!scene->loadFromFile(filePath)) {
            QMessageBox::warning(m_window, "XMind", "Could not open file:\n" + filePath);
            return;
        }
        m_tabManager->setCurrentFilePath(filePath);
        auto& tab = m_tabManager->tab(cur);
        if (tab.stack)
            tab.stack->setCurrentIndex(1);
        m_tabManager->updateTabText(cur);
        view->zoomToFit();
        emit m_tabManager->currentTabChanged(cur);
    } else {
        auto* scene = new MindMapScene(m_window);
        auto* view = new MindMapView(m_window);
        view->setScene(scene);

        if (!scene->loadFromFile(filePath)) {
            QMessageBox::warning(m_window, "XMind", "Could not open file:\n" + filePath);
            delete scene;
            delete view;
            return;
        }

        auto* stack = new QStackedWidget(m_window);
        stack->addWidget(view);
        stack->setCurrentIndex(0);

        m_tabManager->addTab(scene, view, stack, filePath);
        m_tabManager->currentView()->zoomToFit();
    }
}

void FileManager::saveFile() {
    if (m_tabManager->currentFilePath().isEmpty()) {
        saveFileAs();
        return;
    }

    auto* scene = m_tabManager->currentScene();
    QString path = m_tabManager->currentFilePath();

    if (!scene->saveToFile(path)) {
        QMessageBox::warning(m_window, "XMind", "Could not save file:\n" + path);
    }

    int cur = m_tabManager->currentIndex();
    if (cur >= 0) {
        m_tabManager->updateTabText(cur);
    }
    emit m_tabManager->currentTabChanged(cur);
}

void FileManager::saveFileAs() {
    QString filePath =
        QFileDialog::getSaveFileName(m_window, "Save Mind Map", QString(),
                                     "XMind Files (*.xmind);;JSON Files (*.json);;All Files (*)");
    if (filePath.isEmpty())
        return;

    if (!filePath.contains('.'))
        filePath += ".xmind";

    auto* scene = m_tabManager->currentScene();
    if (!scene->saveToFile(filePath)) {
        QMessageBox::warning(m_window, "XMind", "Could not save file:\n" + filePath);
        return;
    }

    m_tabManager->setCurrentFilePath(filePath);
    int cur = m_tabManager->currentIndex();
    if (cur >= 0) {
        m_tabManager->updateTabText(cur);
    }
    emit m_tabManager->currentTabChanged(cur);
}

// ---------------------------------------------------------------------------
// Common export helper
// ---------------------------------------------------------------------------
void FileManager::doExport(const QString& dialogTitle, const QString& filter,
                           const QString& defaultExt,
                           std::function<bool(const QString&)> exporter,
                           const QString& errorLabel) {
    QString filePath = QFileDialog::getSaveFileName(m_window, dialogTitle, QString(), filter);
    if (filePath.isEmpty())
        return;

    if (!filePath.endsWith(defaultExt, Qt::CaseInsensitive))
        filePath += defaultExt;

    if (!exporter(filePath)) {
        QMessageBox::warning(m_window, "XMind",
                             QString("Could not export %1:\n%2").arg(errorLabel, filePath));
        return;
    }
    if (auto* mw = qobject_cast<QMainWindow*>(m_window))
        mw->statusBar()->showMessage("Exported to " + filePath, 3000);
}

void FileManager::exportAsText() {
    doExport("Export as Text", "Text Files (*.txt);;All Files (*)", ".txt",
             [this](const QString& path) {
                 QFile file(path);
                 if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
                     return false;
                 file.write(m_tabManager->currentScene()->exportToText().toUtf8());
                 file.close();
                 return true;
             },
             "file");
}

void FileManager::exportAsMarkdown() {
    doExport("Export as Markdown", "Markdown Files (*.md);;All Files (*)", ".md",
             [this](const QString& path) {
                 QFile file(path);
                 if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
                     return false;
                 file.write(m_tabManager->currentScene()->exportToMarkdown().toUtf8());
                 file.close();
                 return true;
             },
             "file");
}

void FileManager::exportAsPng() {
    doExport("Export as PNG", "PNG Images (*.png);;All Files (*)", ".png",
             [this](const QString& path) {
                 return m_tabManager->currentScene()->exportToPng(path);
             },
             "PNG");
}

void FileManager::exportAsSvg() {
    doExport("Export as SVG", "SVG Files (*.svg);;All Files (*)", ".svg",
             [this](const QString& path) {
                 return m_tabManager->currentScene()->exportToSvg(path);
             },
             "SVG");
}

void FileManager::exportAsPdf() {
    doExport("Export as PDF", "PDF Files (*.pdf);;All Files (*)", ".pdf",
             [this](const QString& path) {
                 return m_tabManager->currentScene()->exportToPdf(path);
             },
             "PDF");
}

void FileManager::importFromText() {
    QString filePath = QFileDialog::getOpenFileName(m_window, "Import from Text", QString(),
                                                    "Text Files (*.txt);;All Files (*)");
    if (filePath.isEmpty())
        return;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(m_window, "XMind", "Could not read file:\n" + filePath);
        return;
    }
    QString text = QString::fromUtf8(file.readAll());
    file.close();

    int cur = m_tabManager->currentIndex();
    if (cur >= 0 && m_tabManager->isTabEmpty(cur)) {
        auto* scene = m_tabManager->currentScene();
        auto* view = m_tabManager->currentView();
        if (!scene->importFromText(text)) {
            QMessageBox::warning(m_window, "XMind", "Could not parse text file:\n" + filePath);
            return;
        }
        m_tabManager->setCurrentFilePath(QString());
        auto& tab = m_tabManager->tab(cur);
        if (tab.stack)
            tab.stack->setCurrentIndex(1);
        view->zoomToFit();
        emit m_tabManager->currentTabChanged(cur);
    } else {
        auto* scene = new MindMapScene(m_window);
        auto* view = new MindMapView(m_window);
        view->setScene(scene);

        if (!scene->importFromText(text)) {
            QMessageBox::warning(m_window, "XMind", "Could not parse text file:\n" + filePath);
            delete scene;
            delete view;
            return;
        }

        auto* stack = new QStackedWidget(m_window);
        stack->addWidget(view);
        stack->setCurrentIndex(0);

        m_tabManager->addTab(scene, view, stack, QString());
        m_tabManager->currentView()->zoomToFit();
    }

    if (auto* mw = qobject_cast<QMainWindow*>(m_window))
        mw->statusBar()->showMessage("Imported from " + filePath, 3000);
}
