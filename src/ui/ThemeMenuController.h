#pragma once

#include "core/Identifiers.h"

#include <QObject>

class MindMapScene;
class QMenu;
class TabManager;

// Owns the Theme menu's content + the "apply this theme" action. Same
// rebuild-on-show pattern as TemplateMenuController so newly-loaded themes
// (e.g. dropped in by the user) appear without a restart.
class ThemeMenuController : public QObject {
    Q_OBJECT

public:
    ThemeMenuController(QMenu* menu, TabManager* tabManager, QObject* parent = nullptr);

private:
    void rebuild();
    void apply(const ThemeId& id);

    QMenu* m_menu;
    TabManager* m_tabManager;
};
