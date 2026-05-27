#include "ui/ThemeMenuController.h"
#include "core/ThemeDescriptor.h"
#include "core/ThemeRegistry.h"
#include "scene/MindMapScene.h"
#include "ui/TabManager.h"

#include <QAction>
#include <QActionGroup>
#include <QDesktopServices>
#include <QMenu>
#include <QUrl>

ThemeMenuController::ThemeMenuController(QMenu* menu, TabManager* tabManager, QObject* parent)
    : QObject(parent), m_menu(menu), m_tabManager(tabManager) {
    connect(m_menu, &QMenu::aboutToShow, this, &ThemeMenuController::rebuild);
    rebuild();
}

void ThemeMenuController::rebuild() {
    m_menu->clear();

    auto* scene = m_tabManager ? m_tabManager->currentScene() : nullptr;
    const ThemeId activeId = (scene && scene->themeId().isValid())
                                 ? scene->themeId()
                                 : ThemeId(ThemeRegistry::defaultThemeId());

    auto* group = new QActionGroup(m_menu);
    group->setExclusive(true);

    const auto themes = ThemeRegistry::instance().allThemes();
    for (const auto* th : themes) {
        const ThemeId id(th->id);
        QAction* act = m_menu->addAction(th->name);
        act->setCheckable(true);
        act->setChecked(id == activeId);
        group->addAction(act);
        connect(act, &QAction::triggered, this, [this, id]() { apply(id); });
    }

    m_menu->addSeparator();
    auto* browseThemesAct = m_menu->addAction(tr("&Browse Themes Online..."));
    connect(browseThemesAct, &QAction::triggered, this, []() {
        QDesktopServices::openUrl(QUrl("https://broccoli-97.github.io/xmind/#themes"));
    });
}

void ThemeMenuController::apply(const ThemeId& id) {
    auto* scene = m_tabManager->currentScene();
    if (!scene || !id.isValid())
        return;
    if (scene->themeId() == id)
        return;

    scene->setThemeId(id);
}
