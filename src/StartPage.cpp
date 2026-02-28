#include "StartPage.h"
#include "IconFactory.h"
#include "MindMapScene.h"
#include "NodeItem.h"

#include <QUndoStack>

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

QWidget* StartPage::create(QObject* /*receiver*/, std::function<void(int)> onTemplate,
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

    QStringList templateNames = {"Mind Map", "Org Chart", "Project Plan"};
    for (int i = 0; i < 3; ++i) {
        auto* card = new QPushButton();
        card->setObjectName("templateCard");
        card->setFixedSize(180, 140);
        card->setIconSize(QSize(160, 106));
        card->setIcon(QIcon(IconFactory::makeTemplatePreview(i, 160, 106)));
        card->setText(templateNames[i]);
        card->setToolTip(templateNames[i]);
        QObject::connect(card, &QPushButton::clicked, page, [onTemplate, i]() { onTemplate(i); });
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

void StartPage::loadTemplate(int index, MindMapScene* scene) {
    auto* root = scene->rootNode();

    if (index == 0) {
        // Mind Map — central topic with 4 branches
        root->setText("Central Topic");
        scene->addNode("Branch 1", root);
        scene->addNode("Branch 2", root);
        scene->addNode("Branch 3", root);
        scene->addNode("Branch 4", root);
        scene->setLayoutStyle(LayoutStyle::Bilateral);
    } else if (index == 1) {
        // Org Chart — CEO -> 3 departments
        root->setText("CEO");
        scene->setLayoutStyle(LayoutStyle::TopDown);
        scene->addNode("Engineering", root);
        scene->addNode("Marketing", root);
        scene->addNode("Sales", root);
    } else if (index == 2) {
        // Project Plan — root -> phases -> tasks
        root->setText("Project");
        scene->setLayoutStyle(LayoutStyle::RightTree);
        auto* phase1 = scene->addNode("Phase 1", root);
        auto* phase2 = scene->addNode("Phase 2", root);
        scene->addNode("Task 1.1", phase1);
        scene->addNode("Task 1.2", phase1);
        scene->addNode("Task 2.1", phase2);
        scene->addNode("Task 2.2", phase2);
    }

    scene->autoLayout();
    scene->undoStack()->clear();
    scene->setModified(false);
}
