#pragma once

#include <QObject>

class QTimer;
class RecoveryManager;
class TabManager;

// Owns the auto-save timer + the loop that walks open tabs. Listens for
// AppSettings changes (interval, on/off) and restarts itself; MainWindow only
// constructs one and wires the `autoSaved` signal to its status bar.
//
// Untitled tabs are routed to RecoveryManager (when one is supplied) so a
// crash before "Save As" doesn't lose the user's work.
class AutoSaveManager : public QObject {
    Q_OBJECT

public:
    // |recovery| may be null in tests / CLI contexts that don't want recovery
    // snapshotting.
    AutoSaveManager(TabManager* tabManager, RecoveryManager* recovery = nullptr,
                    QObject* parent = nullptr);

signals:
    void autoSaved();

private:
    void applySettings();
    void onTimeout();

    TabManager* m_tabManager;
    RecoveryManager* m_recovery;
    QTimer* m_timer;
};
