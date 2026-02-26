#include "StartPage.h"
#include "MindMapScene.h"
#include "NodeItem.h"
#include "ThemeManager.h"

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
    title->setStyleSheet("font-size: 24px; font-weight: bold; margin-bottom: 4px;"
                         " background: transparent; border: none;");
    outer->addWidget(title);

    // Subtitle
    auto* subtitle = new QLabel("Choose a template to get started");
    subtitle->setObjectName("startPageSubtitle");
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setStyleSheet("font-size: 14px; color: #888888; margin-bottom: 24px;"
                            " background: transparent; border: none;");
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
        card->setIcon(QIcon(ThemeManager::makeTemplatePreview(i, 160, 106)));
        card->setText(templateNames[i]);
        card->setToolTip(templateNames[i]);
        card->setStyleSheet("QPushButton#templateCard {"
                            "  background-color: #F0F0F0;"
                            "  border: 2px solid #D0D0D0;"
                            "  border-radius: 8px;"
                            "  padding: 8px;"
                            "  font-size: 13px;"
                            "  color: #333333;"
                            "  text-align: bottom;"
                            "}"
                            "QPushButton#templateCard:hover {"
                            "  border-color: #007ACC;"
                            "  background-color: #E8E8E8;"
                            "}"
                            "QPushButton#templateCard:pressed {"
                            "  background-color: #D0E8FF;"
                            "}");
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
    blankBtn->setStyleSheet("QPushButton#blankCanvasBtn {"
                            "  background-color: transparent;"
                            "  border: 1px solid #D0D0D0;"
                            "  border-radius: 6px;"
                            "  font-size: 13px;"
                            "  color: #333333;"
                            "}"
                            "QPushButton#blankCanvasBtn:hover {"
                            "  border-color: #007ACC;"
                            "  background-color: #F0F0F0;"
                            "}"
                            "QPushButton#blankCanvasBtn:pressed {"
                            "  background-color: #D0E8FF;"
                            "}");
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
