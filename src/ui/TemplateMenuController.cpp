#include "ui/TemplateMenuController.h"
#include "core/TemplateDescriptor.h"
#include "core/TemplateRegistry.h"
#include "scene/MindMapScene.h"
#include "ui/TabManager.h"

#include <QAction>
#include <QActionGroup>
#include <QMenu>

TemplateMenuController::TemplateMenuController(QMenu* menu, TabManager* tabManager, QObject* parent)
    : QObject(parent), m_menu(menu), m_tabManager(tabManager) {
    connect(m_menu, &QMenu::aboutToShow, this, &TemplateMenuController::rebuild);
    rebuild();
}

void TemplateMenuController::rebuild() {
    m_menu->clear();

    auto* scene = m_tabManager ? m_tabManager->currentScene() : nullptr;
    const TemplateId activeId = scene ? scene->templateId() : TemplateId{};

    auto* group = new QActionGroup(m_menu);
    group->setExclusive(true);

    const auto templates = TemplateRegistry::instance().allTemplates();
    for (const auto* td : templates) {
        const TemplateId id(td->id);
        QAction* act = m_menu->addAction(td->name);
        act->setCheckable(true);
        act->setChecked(id == activeId);
        group->addAction(act);
        connect(act, &QAction::triggered, this, [this, id]() { apply(id); });
    }
}

void TemplateMenuController::apply(const TemplateId& id) {
    auto* scene = m_tabManager->currentScene();
    if (!scene || !id.isValid())
        return;
    if (scene->templateId() == id)
        return;

    // Setter owns cache invalidation + re-measure + modified flag; re-layout
    // because the new algorithm and spacing change positions. Fit
    // unconditionally: the switch rearranges the whole map, so whatever the
    // user had zoomed/panned to no longer frames anything meaningful.
    scene->setTemplateId(id);
    scene->autoLayout(MindMapScene::PostLayoutFit::Always);
}
