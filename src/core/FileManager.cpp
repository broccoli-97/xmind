#include "core/FileManager.h"
#include "scene/MindMapScene.h"
#include "scene/MindMapView.h"
#include "ui/TabManager.h"

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
    QString filePath = QFileDialog::getOpenFileName(
        m_window, tr("Open Mind Map"), QString(),
        tr("YMind Files (*.ymind);;JSON Files (*.json);;All Files (*)"));
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
            QMessageBox::warning(m_window, "YMind", tr("Could not open file:\n%1").arg(filePath));
            return;
        }
        m_tabManager->setCurrentFilePath(filePath);
        auto& tab = m_tabManager->tab(cur);
        if (tab.stack)
            tab.stack->setCurrentIndex(1);
        m_tabManager->updateTabText(cur);
        view->zoomToFit();
        m_tabManager->notifyTabChanged(cur);
    } else {
        auto* scene = new MindMapScene(m_window);
        auto* view = new MindMapView(m_window);
        view->setScene(scene);

        if (!scene->loadFromFile(filePath)) {
            QMessageBox::warning(m_window, "YMind", tr("Could not open file:\n%1").arg(filePath));
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
        QMessageBox::warning(m_window, "YMind", tr("Could not save file:\n%1").arg(path));
    }

    int cur = m_tabManager->currentIndex();
    if (cur >= 0) {
        m_tabManager->updateTabText(cur);
    }
    m_tabManager->notifyTabChanged(cur);
}

void FileManager::saveFileAs() {
    QString filePath = QFileDialog::getSaveFileName(
        m_window, tr("Save Mind Map"), QString(),
        tr("YMind Files (*.ymind);;JSON Files (*.json);;All Files (*)"));
    if (filePath.isEmpty())
        return;

    if (!filePath.contains('.'))
        filePath += ".ymind";

    auto* scene = m_tabManager->currentScene();
    if (!scene->saveToFile(filePath)) {
        QMessageBox::warning(m_window, "YMind", tr("Could not save file:\n%1").arg(filePath));
        return;
    }

    m_tabManager->setCurrentFilePath(filePath);
    int cur = m_tabManager->currentIndex();
    if (cur >= 0) {
        m_tabManager->updateTabText(cur);
    }
    m_tabManager->notifyTabChanged(cur);
}

// ---------------------------------------------------------------------------
// Common export helper
// ---------------------------------------------------------------------------
void FileManager::doExport(const QString& dialogTitle, const QString& filter,
                           const QString& defaultExt, std::function<bool(const QString&)> exporter,
                           const QString& errorLabel) {
    QString filePath = QFileDialog::getSaveFileName(m_window, dialogTitle, QString(), filter);
    if (filePath.isEmpty())
        return;

    if (!filePath.endsWith(defaultExt, Qt::CaseInsensitive))
        filePath += defaultExt;

    if (!exporter(filePath)) {
        QMessageBox::warning(m_window, "YMind",
                             tr("Could not export %1:\n%2").arg(errorLabel, filePath));
        return;
    }
    if (auto* mw = qobject_cast<QMainWindow*>(m_window))
        mw->statusBar()->showMessage(tr("Exported to %1").arg(filePath), 3000);
}

void FileManager::exportAsText() {
    doExport(
        tr("Export as Text"), tr("Text Files (*.txt);;All Files (*)"), ".txt",
        [this](const QString& path) {
            QFile file(path);
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
                return false;
            file.write(m_tabManager->currentScene()->exportToText().toUtf8());
            file.close();
            return true;
        },
        tr("file"));
}

void FileManager::exportAsMarkdown() {
    doExport(
        tr("Export as Markdown"), tr("Markdown Files (*.md);;All Files (*)"), ".md",
        [this](const QString& path) {
            QFile file(path);
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
                return false;
            file.write(m_tabManager->currentScene()->exportToMarkdown().toUtf8());
            file.close();
            return true;
        },
        tr("file"));
}

void FileManager::exportAsPng() {
    doExport(
        tr("Export as PNG"), tr("PNG Images (*.png);;All Files (*)"), ".png",
        [this](const QString& path) { return m_tabManager->currentScene()->exportToPng(path); },
        "PNG");
}

void FileManager::exportAsSvg() {
    doExport(
        tr("Export as SVG"), tr("SVG Files (*.svg);;All Files (*)"), ".svg",
        [this](const QString& path) { return m_tabManager->currentScene()->exportToSvg(path); },
        "SVG");
}

void FileManager::exportAsPdf() {
    doExport(
        tr("Export as PDF"), tr("PDF Files (*.pdf);;All Files (*)"), ".pdf",
        [this](const QString& path) { return m_tabManager->currentScene()->exportToPdf(path); },
        "PDF");
}

namespace {

// Render the first N issues from the strict markdown parser as a single
// translated block, with a "...and X more" tail if we truncate.
QString formatMarkdownIssues(const QList<MarkdownImportIssue>& issues, int maxToShow = 10) {
    QStringList lines;
    const int n = std::min<int>(issues.size(), maxToShow);
    for (int i = 0; i < n; ++i) {
        const auto& iss = issues[i];
        if (iss.line > 0)
            lines << QObject::tr("Line %1: %2").arg(iss.line).arg(iss.message);
        else
            lines << iss.message;
    }
    if (issues.size() > maxToShow)
        lines << QObject::tr("... and %1 more issue(s).").arg(issues.size() - maxToShow);
    return lines.join('\n');
}

void showMarkdownImportError(QWidget* window, const QString& filePath,
                             const MarkdownImportReport& report) {
    QMessageBox box(window);
    box.setIcon(QMessageBox::Warning);
    box.setWindowTitle(QStringLiteral("YMind"));
    box.setText(QObject::tr("Could not import %1.\n\n"
                            "The file must be a simple Markdown outline "
                            "(optional `# Title` followed by an unordered "
                            "list with 2-space indentation). See "
                            "docs/markdown-import-format.md for the full "
                            "format.")
                    .arg(QFileInfo(filePath).fileName()));
    box.setDetailedText(formatMarkdownIssues(report.errors));
    box.exec();
}

} // namespace

void FileManager::importFromMarkdown() {
    QString filePath =
        QFileDialog::getOpenFileName(m_window, tr("Import from Markdown"), QString(),
                                     tr("Markdown Files (*.md *.markdown);;All Files (*)"));
    if (filePath.isEmpty())
        return;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(m_window, "YMind", tr("Could not read file:\n%1").arg(filePath));
        return;
    }
    QString text = QString::fromUtf8(file.readAll());
    file.close();

    int cur = m_tabManager->currentIndex();
    if (cur >= 0 && m_tabManager->isTabEmpty(cur)) {
        auto* scene = m_tabManager->currentScene();
        auto* view = m_tabManager->currentView();
        MarkdownImportReport report;
        if (!scene->importFromMarkdownStrict(text, &report)) {
            showMarkdownImportError(m_window, filePath, report);
            return;
        }
        m_tabManager->setCurrentFilePath(QString());
        auto& tab = m_tabManager->tab(cur);
        if (tab.stack)
            tab.stack->setCurrentIndex(1);
        view->zoomToFit();
        m_tabManager->notifyTabChanged(cur);
    } else {
        auto* scene = new MindMapScene(m_window);
        auto* view = new MindMapView(m_window);
        view->setScene(scene);

        MarkdownImportReport report;
        if (!scene->importFromMarkdownStrict(text, &report)) {
            showMarkdownImportError(m_window, filePath, report);
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
        mw->statusBar()->showMessage(tr("Imported from %1").arg(filePath), 3000);
}

void FileManager::importMarkdownContent(const QString& markdown) {
    if (markdown.trimmed().isEmpty())
        return;

    int cur = m_tabManager->currentIndex();
    if (cur >= 0 && m_tabManager->isTabEmpty(cur)) {
        auto* scene = m_tabManager->currentScene();
        auto* view = m_tabManager->currentView();
        if (!scene->importFromMarkdown(markdown, /*animate=*/true)) {
            QMessageBox::warning(m_window, QStringLiteral("YMind"),
                                 tr("Failed to parse generated Markdown into a mind map."));
            return;
        }
        m_tabManager->setCurrentFilePath(QString());
        auto& tab = m_tabManager->tab(cur);
        if (tab.stack)
            tab.stack->setCurrentIndex(1);
        view->zoomToFit();
        m_tabManager->notifyTabChanged(cur);
    } else {
        auto* scene = new MindMapScene(m_window);
        auto* view = new MindMapView(m_window);
        view->setScene(scene);

        if (!scene->importFromMarkdown(markdown, /*animate=*/true)) {
            QMessageBox::warning(m_window, QStringLiteral("YMind"),
                                 tr("Failed to parse generated Markdown into a mind map."));
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
        mw->statusBar()->showMessage(tr("Mind map generated successfully"), 3000);
}
