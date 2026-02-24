#include "MainWindow.h"
#include "AppSettings.h"
#include "MindMapScene.h"
#include "MindMapView.h"
#include "NodeItem.h"
#include "SettingsDialog.h"

#include <QAction>
#include <QCloseEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QKeySequence>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QTimer>
#include <QToolBar>
#include <QUndoStack>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    resize(1280, 800);

    m_scene = new MindMapScene(this);
    m_view = new MindMapView(this);
    m_view->setScene(m_scene);
    setCentralWidget(m_view);

    setupActions();
    setupToolBar();
    setupMenuBar();

    connect(m_scene, &MindMapScene::modifiedChanged, this,
            [this](bool) { updateWindowTitle(); });
    connect(m_scene, &MindMapScene::fileLoaded, this, [this](const QString& path) {
        m_currentFile = path;
        updateWindowTitle();
    });

    connectUndoStack();
    updateWindowTitle();

    // Auto-save timer
    m_autoSaveTimer = new QTimer(this);
    connect(m_autoSaveTimer, &QTimer::timeout, this, &MainWindow::onAutoSaveTimeout);
    setupAutoSaveTimer();

    // Settings signals
    connect(&AppSettings::instance(), &AppSettings::autoSaveSettingsChanged,
            this, &MainWindow::onAutoSaveSettingsChanged);
    connect(&AppSettings::instance(), &AppSettings::themeChanged,
            this, &MainWindow::applyTheme);

    restoreWindowState();
    applyTheme();

    statusBar()->showMessage("Enter: Add Child  |  Ctrl+Enter: Add Sibling  |  Del: Delete  |  "
                             "F2/Double-click: Edit  |  Ctrl+L: Auto Layout  |  Scroll: Zoom  |  "
                             "Middle/Right-drag: Pan");
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (maybeSave()) {
        saveWindowState();
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::updateWindowTitle() {
    QString title = "XMind - Mind Map Editor";
    if (!m_currentFile.isEmpty()) {
        title = QFileInfo(m_currentFile).fileName() + " - XMind";
    }
    if (m_scene->isModified()) {
        title.prepend("* ");
    }
    setWindowTitle(title);
}

bool MainWindow::maybeSave() {
    if (!m_scene->isModified())
        return true;

    auto ret = QMessageBox::warning(this, "XMind", "The mind map has been modified.\n"
                                                    "Do you want to save your changes?",
                                    QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (ret == QMessageBox::Save) {
        saveFile();
        return !m_scene->isModified();
    }
    if (ret == QMessageBox::Cancel)
        return false;

    return true; // Discard
}

void MainWindow::newFile() {
    if (!maybeSave())
        return;

    m_scene->clearScene();

    // Create a fresh root node by reloading default state
    m_currentFile.clear();
    // Re-create scene with default root
    delete m_scene;
    m_scene = new MindMapScene(this);
    m_view->setScene(m_scene);

    connect(m_scene, &MindMapScene::modifiedChanged, this,
            [this](bool) { updateWindowTitle(); });
    connect(m_scene, &MindMapScene::fileLoaded, this, [this](const QString& path) {
        m_currentFile = path;
        updateWindowTitle();
    });

    connectUndoStack();
    updateWindowTitle();
    applyTheme();
}

void MainWindow::openFile() {
    if (!maybeSave())
        return;

    QString filePath =
        QFileDialog::getOpenFileName(this, "Open Mind Map", QString(),
                                     "XMind Files (*.xmind);;JSON Files (*.json);;All Files (*)");
    if (filePath.isEmpty())
        return;

    if (!m_scene->loadFromFile(filePath)) {
        QMessageBox::warning(this, "XMind",
                             "Could not open file:\n" + filePath);
        return;
    }

    m_currentFile = filePath;
    updateWindowTitle();
    m_view->zoomToFit();
}

void MainWindow::saveFile() {
    if (m_currentFile.isEmpty()) {
        saveFileAs();
        return;
    }

    if (!m_scene->saveToFile(m_currentFile)) {
        QMessageBox::warning(this, "XMind",
                             "Could not save file:\n" + m_currentFile);
    }
    updateWindowTitle();
}

void MainWindow::saveFileAs() {
    QString filePath =
        QFileDialog::getSaveFileName(this, "Save Mind Map", QString(),
                                     "XMind Files (*.xmind);;JSON Files (*.json);;All Files (*)");
    if (filePath.isEmpty())
        return;

    // Ensure .xmind extension if none provided
    if (!filePath.contains('.'))
        filePath += ".xmind";

    if (!m_scene->saveToFile(filePath)) {
        QMessageBox::warning(this, "XMind",
                             "Could not save file:\n" + filePath);
        return;
    }

    m_currentFile = filePath;
    updateWindowTitle();
}

void MainWindow::exportAsText() {
    QString filePath =
        QFileDialog::getSaveFileName(this, "Export as Text", QString(),
                                     "Text Files (*.txt);;All Files (*)");
    if (filePath.isEmpty())
        return;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "XMind", "Could not write file:\n" + filePath);
        return;
    }
    file.write(m_scene->exportToText().toUtf8());
    file.close();

    statusBar()->showMessage("Exported to " + filePath, 3000);
}

void MainWindow::exportAsMarkdown() {
    QString filePath =
        QFileDialog::getSaveFileName(this, "Export as Markdown", QString(),
                                     "Markdown Files (*.md);;All Files (*)");
    if (filePath.isEmpty())
        return;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "XMind", "Could not write file:\n" + filePath);
        return;
    }
    file.write(m_scene->exportToMarkdown().toUtf8());
    file.close();

    statusBar()->showMessage("Exported to " + filePath, 3000);
}

void MainWindow::importFromText() {
    if (!maybeSave())
        return;

    QString filePath =
        QFileDialog::getOpenFileName(this, "Import from Text", QString(),
                                     "Text Files (*.txt);;All Files (*)");
    if (filePath.isEmpty())
        return;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "XMind", "Could not read file:\n" + filePath);
        return;
    }
    QString text = QString::fromUtf8(file.readAll());
    file.close();

    if (!m_scene->importFromText(text)) {
        QMessageBox::warning(this, "XMind", "Could not parse text file:\n" + filePath);
        return;
    }

    m_currentFile.clear();
    updateWindowTitle();
    m_view->zoomToFit();
    statusBar()->showMessage("Imported from " + filePath, 3000);
}

void MainWindow::openSettings() {
    SettingsDialog dlg(this);
    dlg.exec();
}

void MainWindow::saveWindowState() {
    auto& s = AppSettings::instance();
    s.setWindowGeometry(saveGeometry());
    s.setWindowState(QMainWindow::saveState());
}

void MainWindow::restoreWindowState() {
    auto& s = AppSettings::instance();
    QByteArray geo = s.windowGeometry();
    if (!geo.isEmpty())
        restoreGeometry(geo);
    QByteArray state = s.windowState();
    if (!state.isEmpty())
        QMainWindow::restoreState(state);
}

void MainWindow::setupAutoSaveTimer() {
    auto& s = AppSettings::instance();
    if (s.autoSaveEnabled()) {
        m_autoSaveTimer->start(s.autoSaveIntervalMinutes() * 60 * 1000);
    } else {
        m_autoSaveTimer->stop();
    }
}

void MainWindow::onAutoSaveTimeout() {
    if (!m_currentFile.isEmpty() && m_scene->isModified()) {
        if (m_scene->saveToFile(m_currentFile)) {
            updateWindowTitle();
            statusBar()->showMessage("Auto-saved", 3000);
        }
    }
}

void MainWindow::onAutoSaveSettingsChanged() {
    setupAutoSaveTimer();
}

void MainWindow::applyTheme() {
    // Invalidate caches so items repaint with new theme colors
    const auto items = m_scene->items();
    for (auto* item : items) {
        item->setCacheMode(QGraphicsItem::NoCache);
    }
    m_view->viewport()->update();
    // Re-enable cache after repaint
    QTimer::singleShot(0, this, [this]() {
        const auto items = m_scene->items();
        for (auto* item : items) {
            if (dynamic_cast<NodeItem*>(item))
                item->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
        }
    });
}

void MainWindow::setupActions() {
    m_undoAct = new QAction("&Undo", this);
    m_undoAct->setShortcut(QKeySequence::Undo);
    m_undoAct->setEnabled(false);
    connect(m_undoAct, &QAction::triggered, this, [this]() {
        if (!m_scene->isEditing())
            m_scene->undoStack()->undo();
    });

    m_redoAct = new QAction("&Redo", this);
    m_redoAct->setShortcuts({QKeySequence::Redo, QKeySequence("Ctrl+Y")});
    m_redoAct->setEnabled(false);
    connect(m_redoAct, &QAction::triggered, this, [this]() {
        if (!m_scene->isEditing())
            m_scene->undoStack()->redo();
    });
}

void MainWindow::connectUndoStack() {
    auto* stack = m_scene->undoStack();
    connect(stack, &QUndoStack::canUndoChanged, m_undoAct, &QAction::setEnabled);
    connect(stack, &QUndoStack::canRedoChanged, m_redoAct, &QAction::setEnabled);
    connect(stack, &QUndoStack::undoTextChanged, this, [this](const QString& text) {
        m_undoAct->setText(text.isEmpty() ? "&Undo" : "&Undo " + text);
    });
    connect(stack, &QUndoStack::redoTextChanged, this, [this](const QString& text) {
        m_redoAct->setText(text.isEmpty() ? "&Redo" : "&Redo " + text);
    });
    m_undoAct->setEnabled(stack->canUndo());
    m_redoAct->setEnabled(stack->canRedo());
}

void MainWindow::setupToolBar() {
    auto* toolbar = addToolBar("Main");
    toolbar->setMovable(false);
    toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    toolbar->addAction(m_undoAct);
    toolbar->addAction(m_redoAct);
    toolbar->addSeparator();

    auto* addChildAct = toolbar->addAction("Add Child");
    addChildAct->setToolTip("Add a child node (Enter)");
    connect(addChildAct, &QAction::triggered, m_scene, &MindMapScene::addChildToSelected);

    auto* addSiblingAct = toolbar->addAction("Add Sibling");
    addSiblingAct->setToolTip("Add a sibling node (Ctrl+Enter)");
    connect(addSiblingAct, &QAction::triggered, m_scene, &MindMapScene::addSiblingToSelected);

    auto* deleteAct = toolbar->addAction("Delete");
    deleteAct->setToolTip("Delete selected node (Del)");
    connect(deleteAct, &QAction::triggered, m_scene, &MindMapScene::deleteSelected);

    toolbar->addSeparator();

    auto* layoutAct = toolbar->addAction("Auto Layout");
    layoutAct->setToolTip("Automatically arrange all nodes (Ctrl+L)");
    connect(layoutAct, &QAction::triggered, m_scene, &MindMapScene::autoLayout);

    toolbar->addSeparator();

    auto* zoomInAct = toolbar->addAction("Zoom +");
    zoomInAct->setToolTip("Zoom in (Ctrl++)");
    connect(zoomInAct, &QAction::triggered, m_view, &MindMapView::zoomIn);

    auto* zoomOutAct = toolbar->addAction("Zoom -");
    zoomOutAct->setToolTip("Zoom out (Ctrl+-)");
    connect(zoomOutAct, &QAction::triggered, m_view, &MindMapView::zoomOut);

    auto* fitAct = toolbar->addAction("Fit View");
    fitAct->setToolTip("Fit all nodes in view (Ctrl+0)");
    connect(fitAct, &QAction::triggered, m_view, &MindMapView::zoomToFit);
}

void MainWindow::setupMenuBar() {
    // File menu
    auto* fileMenu = menuBar()->addMenu("&File");

    auto* newAct = fileMenu->addAction("&New");
    newAct->setShortcut(QKeySequence::New);
    connect(newAct, &QAction::triggered, this, &MainWindow::newFile);

    auto* openAct = fileMenu->addAction("&Open...");
    openAct->setShortcut(QKeySequence::Open);
    connect(openAct, &QAction::triggered, this, &MainWindow::openFile);

    fileMenu->addSeparator();

    auto* saveAct = fileMenu->addAction("&Save");
    saveAct->setShortcut(QKeySequence::Save);
    connect(saveAct, &QAction::triggered, this, &MainWindow::saveFile);

    auto* saveAsAct = fileMenu->addAction("Save &As...");
    saveAsAct->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAct, &QAction::triggered, this, &MainWindow::saveFileAs);

    fileMenu->addSeparator();

    auto* exportMenu = fileMenu->addMenu("&Export");

    auto* exportTextAct = exportMenu->addAction("As &Text...");
    connect(exportTextAct, &QAction::triggered, this, &MainWindow::exportAsText);

    auto* exportMdAct = exportMenu->addAction("As &Markdown...");
    connect(exportMdAct, &QAction::triggered, this, &MainWindow::exportAsMarkdown);

    auto* importAct = fileMenu->addAction("&Import from Text...");
    connect(importAct, &QAction::triggered, this, &MainWindow::importFromText);

    fileMenu->addSeparator();

    auto* exitAct = fileMenu->addAction("E&xit");
    exitAct->setShortcut(QKeySequence::Quit);
    connect(exitAct, &QAction::triggered, this, &QWidget::close);

    // Edit menu
    auto* editMenu = menuBar()->addMenu("&Edit");

    editMenu->addAction(m_undoAct);
    editMenu->addAction(m_redoAct);
    editMenu->addSeparator();

    auto* addChildAct = editMenu->addAction("Add &Child");
    addChildAct->setToolTip("Add a child node (Enter)");
    connect(addChildAct, &QAction::triggered, m_scene, &MindMapScene::addChildToSelected);

    auto* addSiblingAct = editMenu->addAction("Add &Sibling");
    addSiblingAct->setToolTip("Add a sibling node (Ctrl+Enter)");
    connect(addSiblingAct, &QAction::triggered, m_scene, &MindMapScene::addSiblingToSelected);

    auto* deleteAct = editMenu->addAction("&Delete");
    deleteAct->setToolTip("Delete selected node (Del)");
    connect(deleteAct, &QAction::triggered, m_scene, &MindMapScene::deleteSelected);

    editMenu->addSeparator();

    auto* layoutAct = editMenu->addAction("Auto &Layout");
    layoutAct->setShortcut(QKeySequence("Ctrl+L"));
    connect(layoutAct, &QAction::triggered, m_scene, &MindMapScene::autoLayout);

    editMenu->addSeparator();

    auto* settingsAct = editMenu->addAction("&Settings...");
    settingsAct->setShortcut(QKeySequence("Ctrl+,"));
    connect(settingsAct, &QAction::triggered, this, &MainWindow::openSettings);

    // View menu
    auto* viewMenu = menuBar()->addMenu("&View");

    auto* zoomInAct = viewMenu->addAction("Zoom &In");
    zoomInAct->setShortcut(QKeySequence::ZoomIn);
    connect(zoomInAct, &QAction::triggered, m_view, &MindMapView::zoomIn);

    auto* zoomOutAct = viewMenu->addAction("Zoom &Out");
    zoomOutAct->setShortcut(QKeySequence::ZoomOut);
    connect(zoomOutAct, &QAction::triggered, m_view, &MindMapView::zoomOut);

    auto* fitAct = viewMenu->addAction("&Fit to View");
    fitAct->setShortcut(QKeySequence("Ctrl+0"));
    connect(fitAct, &QAction::triggered, m_view, &MindMapView::zoomToFit);
}
