#pragma once

#include <QWidget>
#include <functional>

class MindMapScene;

namespace StartPage {
QWidget* create(QObject* receiver, std::function<void(const QString&)> onTemplate,
                std::function<void()> onBlankCanvas);
void loadTemplate(const QString& templateId, MindMapScene* scene);
bool loadTemplateFromFile(const QString& filePath, MindMapScene* scene);
} // namespace StartPage
