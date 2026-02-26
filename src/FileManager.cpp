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

void FileManager::exportAsText() {
    QString filePath = QFileDialog::getSaveFileName(m_window, "Export as Text", QString(),
                                                    "Text Files (*.txt);;All Files (*)");
    if (filePath.isEmpty())
        return;

    if (!filePath.endsWith(".txt", Qt::CaseInsensitive))
        filePath += ".txt";

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(m_window, "XMind", "Could not write file:\n" + filePath);
        return;
    }
    file.write(m_tabManager->currentScene()->exportToText().toUtf8());
    file.close();

    if (auto* mw = qobject_cast<QMainWindow*>(m_window))
        mw->statusBar()->showMessage("Exported to " + filePath, 3000);
}

void FileManager::exportAsMarkdown() {
    QString filePath = QFileDialog::getSaveFileName(m_window, "Export as Markdown", QString(),
                                                    "Markdown Files (*.md);;All Files (*)");
    if (filePath.isEmpty())
        return;

    if (!filePath.endsWith(".md", Qt::CaseInsensitive))
        filePath += ".md";

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(m_window, "XMind", "Could not write file:\n" + filePath);
        return;
    }
    file.write(m_tabManager->currentScene()->exportToMarkdown().toUtf8());
    file.close();

    if (auto* mw = qobject_cast<QMainWindow*>(m_window))
        mw->statusBar()->showMessage("Exported to " + filePath, 3000);
}

void FileManager::exportAsPng() {
    QString filePath = QFileDialog::getSaveFileName(m_window, "Export as PNG", QString(),
                                                    "PNG Images (*.png);;All Files (*)");
    if (filePath.isEmpty())
        return;

    if (!filePath.endsWith(".png", Qt::CaseInsensitive))
        filePath += ".png";

    if (!m_tabManager->currentScene()->exportToPng(filePath)) {
        QMessageBox::warning(m_window, "XMind", "Could not export PNG:\n" + filePath);
        return;
    }
    if (auto* mw = qobject_cast<QMainWindow*>(m_window))
        mw->statusBar()->showMessage("Exported to " + filePath, 3000);
}

void FileManager::exportAsSvg() {
    QString filePath = QFileDialog::getSaveFileName(m_window, "Export as SVG", QString(),
                                                    "SVG Files (*.svg);;All Files (*)");
    if (filePath.isEmpty())
        return;

    if (!filePath.endsWith(".svg", Qt::CaseInsensitive))
        filePath += ".svg";

    if (!m_tabManager->currentScene()->exportToSvg(filePath)) {
        QMessageBox::warning(m_window, "XMind", "Could not export SVG:\n" + filePath);
        return;
    }
    if (auto* mw = qobject_cast<QMainWindow*>(m_window))
        mw->statusBar()->showMessage("Exported to " + filePath, 3000);
}

void FileManager::exportAsPdf() {
    QString filePath = QFileDialog::getSaveFileName(m_window, "Export as PDF", QString(),
                                                    "PDF Files (*.pdf);;All Files (*)");
    if (filePath.isEmpty())
        return;

    if (!filePath.endsWith(".pdf", Qt::CaseInsensitive))
        filePath += ".pdf";

    if (!m_tabManager->currentScene()->exportToPdf(filePath)) {
        QMessageBox::warning(m_window, "XMind", "Could not export PDF:\n" + filePath);
        return;
    }
    if (auto* mw = qobject_cast<QMainWindow*>(m_window))
        mw->statusBar()->showMessage("Exported to " + filePath, 3000);
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
