#pragma once

#include "core/Identifiers.h"

#include <QObject>

class MindMapScene;
class QMenu;
class TabManager;

// Owns the Template menu's content + the "apply this template" action. The
// menu is rebuilt every time it's shown so it picks up templates loaded
// after startup (e.g. dropped in through the Start Page) and reflects the
// current tab's templateId.
class TemplateMenuController : public QObject {
    Q_OBJECT

public:
    TemplateMenuController(QMenu* menu, TabManager* tabManager, QObject* parent = nullptr);

private:
    void rebuild();
    void apply(const TemplateId& id);

    QMenu* m_menu;
    TabManager* m_tabManager;
};
