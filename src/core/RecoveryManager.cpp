#include "core/RecoveryManager.h"
#include "scene/MindMapScene.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QUuid>
#include <algorithm>

namespace {

constexpr auto kSnapshotPrefix = "tab-";
constexpr auto kSnapshotExt = ".ymind";

// Pull the tab index out of "tab-<n>.ymind" filenames. Returns -1 if the
// filename doesn't match the expected shape, so callers can skip stray files.
int parseTabIdx(const QString& fileName) {
    if (!fileName.startsWith(kSnapshotPrefix) || !fileName.endsWith(kSnapshotExt))
        return -1;
    const QString digits = fileName.mid(QString(kSnapshotPrefix).size(),
                                        fileName.size() - QString(kSnapshotPrefix).size() -
                                            QString(kSnapshotExt).size());
    bool ok = false;
    int n = digits.toInt(&ok);
    return ok ? n : -1;
}

} // namespace

RecoveryManager::RecoveryManager(QString recoveryRoot, QObject* parent)
    : QObject(parent), m_recoveryRoot(std::move(recoveryRoot)) {}

void RecoveryManager::initialize() {
    if (m_initialized)
        return;
    // Strip braces from QUuid::toString() so the dir name is filesystem-clean
    // across platforms (Windows is fussy about `{}` in some contexts).
    m_sessionId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QDir().mkpath(sessionDir().absolutePath());
    m_initialized = true;
}

QDir RecoveryManager::sessionDir() const {
    return QDir(m_recoveryRoot + "/" + m_sessionId);
}

QString RecoveryManager::snapshotPath(int tabIdx) const {
    return sessionDir().filePath(
        QString("%1%2%3").arg(kSnapshotPrefix).arg(tabIdx).arg(kSnapshotExt));
}

bool RecoveryManager::saveSnapshot(int tabIdx, MindMapScene* scene) {
    if (!m_initialized || !scene)
        return false;

    QSaveFile file(snapshotPath(tabIdx));
    if (!file.open(QIODevice::WriteOnly))
        return false;

    const QJsonObject json = scene->toJson();
    file.write(QJsonDocument(json).toJson(QJsonDocument::Indented));
    return file.commit();
}

void RecoveryManager::discardSnapshot(int tabIdx) {
    if (!m_initialized)
        return;
    QFile::remove(snapshotPath(tabIdx));
}

void RecoveryManager::clearCurrentSession() {
    if (!m_initialized)
        return;
    QDir dir = sessionDir();
    if (dir.exists())
        dir.removeRecursively();
}

QList<RecoveryManager::OrphanSession> RecoveryManager::findOrphanSessions() const {
    QList<OrphanSession> result;
    QDir root(m_recoveryRoot);
    if (!root.exists())
        return result;

    const auto subdirs = root.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const auto& info : subdirs) {
        const QString name = info.fileName();
        if (name == m_sessionId)
            continue;
        result.append({name, info.absoluteFilePath()});
    }
    return result;
}

QList<QJsonObject> RecoveryManager::loadSnapshots(const QString& sessionDirPath) const {
    QDir dir(sessionDirPath);
    if (!dir.exists())
        return {};

    // Collect (tabIdx, filePath) pairs so we can return snapshots in index
    // order. A user who had tabs 0, 1, 2 open shouldn't see them restored
    // alphabetically (tab-1, tab-10, tab-2).
    QList<QPair<int, QString>> entries;
    const auto fileInfos = dir.entryInfoList({"tab-*.ymind"}, QDir::Files);
    for (const auto& info : fileInfos) {
        int idx = parseTabIdx(info.fileName());
        if (idx >= 0)
            entries.append({idx, info.absoluteFilePath()});
    }
    std::sort(entries.begin(), entries.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });

    QList<QJsonObject> result;
    for (const auto& [_, path] : entries) {
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly))
            continue;
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &err);
        if (err.error == QJsonParseError::NoError && doc.isObject())
            result.append(doc.object());
    }
    return result;
}

void RecoveryManager::discardSession(const QString& sessionDirPath) {
    QDir dir(sessionDirPath);
    if (dir.exists())
        dir.removeRecursively();
}
