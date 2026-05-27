#pragma once

#include <QObject>

class QTimer;
class TabManager;

// Owns the auto-save timer + the loop that walks open tabs. Listens for
// AppSettings changes (interval, on/off) and restarts itself; MainWindow only
// constructs one and wires the `autoSaved` signal to its status bar.
class AutoSaveManager : public QObject {
    Q_OBJECT

public:
    AutoSaveManager(TabManager* tabManager, QObject* parent = nullptr);

signals:
    void autoSaved();

private:
    void applySettings();
    void onTimeout();

    TabManager* m_tabManager;
    QTimer* m_timer;
};
