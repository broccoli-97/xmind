#include "core/AutoSaveManager.h"
#include "core/AppSettings.h"
#include "scene/MindMapScene.h"
#include "ui/TabManager.h"

#include <QTimer>

AutoSaveManager::AutoSaveManager(TabManager* tabManager, QObject* parent)
    : QObject(parent), m_tabManager(tabManager), m_timer(new QTimer(this)) {
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
        if (!tab.filePath.isEmpty() && tab.scene->isModified()) {
            if (tab.scene->saveToFile(tab.filePath)) {
                m_tabManager->updateTabText(i);
                saved = true;
            }
        }
    }
    if (saved)
        emit autoSaved();
}
