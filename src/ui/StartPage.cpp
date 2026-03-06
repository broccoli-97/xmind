#include "ui/StartPage.h"
#include "core/TemplateDescriptor.h"
#include "core/TemplateRegistry.h"
#include "ui/IconFactory.h"
#include "scene/MindMapScene.h"
#include "scene/NodeItem.h"

#include <QUndoStack>

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

QWidget* StartPage::create(QObject* /*receiver*/, std::function<void(const QString&)> onTemplate,
                           std::function<void()> onBlankCanvas) {
    auto* page = new QWidget();
    page->setObjectName("startPage");

    auto* outer = new QVBoxLayout(page);
    outer->setAlignment(Qt::AlignCenter);

    // Title
    auto* title = new QLabel("Create a New Mind Map");
    title->setObjectName("startPageTitle");
    title->setAlignment(Qt::AlignCenter);
    outer->addWidget(title);

    // Subtitle
    auto* subtitle = new QLabel("Choose a template to get started");
    subtitle->setObjectName("startPageSubtitle");
    subtitle->setAlignment(Qt::AlignCenter);
    outer->addWidget(subtitle);

    // Template cards row
    auto* cardRow = new QWidget();
    auto* cardLayout = new QHBoxLayout(cardRow);
    cardLayout->setAlignment(Qt::AlignCenter);
    cardLayout->setSpacing(24);

    auto templates = TemplateRegistry::instance().allTemplates();
    for (int i = 0; i < templates.size(); ++i) {
        const auto* td = templates[i];
        auto* card = new QPushButton();
        card->setObjectName("templateCard");
        card->setFixedSize(180, 140);
        card->setIconSize(QSize(160, 106));
        card->setIcon(QIcon(IconFactory::makeTemplatePreview(td->id, 160, 106)));
        card->setToolTip(td->name);
        QString templateId = td->id;
        QObject::connect(card, &QPushButton::clicked, page,
                         [onTemplate, templateId]() { onTemplate(templateId); });
        cardLayout->addWidget(card);
    }
    outer->addWidget(cardRow);

    // Spacing
    outer->addSpacing(16);

    // Blank Canvas button
    auto* blankBtn = new QPushButton("Blank Canvas");
    blankBtn->setObjectName("blankCanvasBtn");
    blankBtn->setFixedSize(160, 36);
    QObject::connect(blankBtn, &QPushButton::clicked, page, [onBlankCanvas]() { onBlankCanvas(); });

    auto* blankRow = new QHBoxLayout();
    blankRow->setAlignment(Qt::AlignCenter);
    blankRow->addWidget(blankBtn);
    outer->addLayout(blankRow);

    return page;
}

static void buildContentTree(MindMapScene* scene, NodeItem* parent,
                              const QList<TemplateContentNode>& children) {
    for (const auto& child : children) {
        auto* node = scene->addNode(child.text, parent);
        if (node && !child.children.isEmpty())
            buildContentTree(scene, node, child.children);
    }
}

void StartPage::loadTemplate(const QString& templateId, MindMapScene* scene) {
    const auto* td = TemplateRegistry::instance().templateById(templateId);
    if (!td)
        return;

    auto* root = scene->rootNode();
    root->setText(td->content.text);

    scene->setTemplateId(templateId);

    buildContentTree(scene, root, td->content.children);

    scene->autoLayout();
    scene->undoStack()->clear();
    scene->setModified(false);
}
