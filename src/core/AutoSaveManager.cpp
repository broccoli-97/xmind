#include "core/AutoSaveManager.h"
#include "core/AppSettings.h"
#include "core/RecoveryManager.h"
#include "scene/MindMapScene.h"
#include "ui/TabManager.h"

#include <QTimer>

AutoSaveManager::AutoSaveManager(TabManager* tabManager, RecoveryManager* recovery, QObject* parent)
    : QObject(parent), m_tabManager(tabManager), m_recovery(recovery), m_timer(new QTimer(this)) {
    connect(m_timer, &QTimer::timeout, this, &AutoSaveManager::onTimeout);
    connect(&AppSettings::instance(), &AppSettings::autoSaveSettingsChanged, this,
            &AutoSaveManager::applySettings);
    applySettings();
}

void AutoSaveManager::applySettings() {
    auto& s = AppSettings::instance();
    if (s.autoSaveEnabled()) {
        m_timer->start(s.autoSaveIntervalMinutes() * 60 * 1000);
    } else {
        m_timer->stop();
    }
}

void AutoSaveManager::onTimeout() {
    bool saved = false;
    for (int i = 0; i < m_tabManager->tabCount(); ++i) {
        const auto& tab = m_tabManager->tabs()[i];
        if (!tab.scene->isModified())
            continue;

        if (!tab.filePath.isEmpty()) {
            // Titled tab — write through to the real file. A successful save
            // also obsoletes any prior recovery snapshot for this index, so
            // drop it so the next launch's restore prompt doesn't shadow the
            // real file.
            if (tab.scene->saveToFile(tab.filePath)) {
                m_tabManager->updateTabText(i);
                saved = true;
                if (m_recovery)
                    m_recovery->discardSnapshot(i);
            }
        } else if (m_recovery) {
            // Untitled tab — no real file to save to, so persist a snapshot
            // into the recovery dir instead. Doesn't touch the scene's
            // modified state (the user still needs to do Save As).
            m_recovery->saveSnapshot(i, tab.scene);
        }
    }
    if (saved)
        emit autoSaved();
}
