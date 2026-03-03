#pragma once

#include <QWidget>
#include <functional>

class MindMapScene;

namespace StartPage {
QWidget* create(QObject* receiver, std::function<void(int)> onTemplate,
                std::function<void()> onBlankCanvas);
void loadTemplate(int index, MindMapScene* scene);
} // namespace StartPage
