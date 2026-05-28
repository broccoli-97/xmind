#pragma once

#include <QDir>
#include <QList>
#include <QObject>
#include <QString>

class MindMapScene;
class QJsonObject;

// Crash-recovery for untitled tabs.
//
// On each launch, RecoveryManager stamps a per-launch UUID and owns a dir at
// `<recoveryRoot>/<sessionId>/`. Every auto-save tick, AutoSaveManager writes
// an atomic snapshot of each untitled-modified tab into that dir. On a clean
// exit, the dir is deleted. On the next launch, any session dirs left behind
// (whose UUIDs differ from ours) are *orphans* — leftovers from a crashed
// prior session — and the user is offered the chance to restore them.
//
// `recoveryRoot` is a constructor parameter so tests can point at a temp dir
// instead of `QStandardPaths::AppLocalDataLocation/recovery`.
class RecoveryManager : public QObject {
    Q_OBJECT

public:
    RecoveryManager(QString recoveryRoot, QObject* parent = nullptr);

    // Generate the per-launch UUID, ensure the session dir exists. Must be
    // called before any save/discard call. Idempotent.
    void initialize();

    // Atomically write `scene->toJson()` into `<sessionDir>/tab-<idx>.ymind`
    // via QSaveFile (rename-on-commit). Does NOT touch the scene's modified
    // state — recovery snapshots are parallel to user saves, not a substitute.
    bool saveSnapshot(int tabIdx, MindMapScene* scene);

    // Remove `tab-<idx>.ymind` from the current session. Used when a user
    // save covers the same tab, so the recovery copy doesn't shadow the real
    // file on a later restore prompt.
    void discardSnapshot(int tabIdx);

    // Remove the current session's dir. Called on graceful shutdown so the
    // next launch doesn't see our dir as an orphan.
    void clearCurrentSession();

    struct OrphanSession {
        QString sessionId; // The directory name (UUID).
        QString dirPath;   // Absolute path to the orphan session dir.
    };

    // List session dirs under `recoveryRoot` whose name is NOT our UUID.
    // These are crashed-prior-session leftovers.
    QList<OrphanSession> findOrphanSessions() const;

    // Read every `tab-*.ymind` snapshot in `sessionDirPath` and return their
    // parsed top-level JSON objects, sorted by tab index ascending. Files
    // that fail to parse are skipped silently.
    QList<QJsonObject> loadSnapshots(const QString& sessionDirPath) const;

    // Remove `sessionDirPath` and everything in it. Used after restore or
    // discard so the orphan doesn't re-appear on the next launch.
    void discardSession(const QString& sessionDirPath);

    QString currentSessionId() const { return m_sessionId; }
    QString recoveryRoot() const { return m_recoveryRoot; }

private:
    QString snapshotPath(int tabIdx) const;
    QDir sessionDir() const;

    QString m_recoveryRoot;
    QString m_sessionId;
    bool m_initialized = false;
};
