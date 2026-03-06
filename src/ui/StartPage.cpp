#include "ui/StartPage.h"
#include "core/TemplateDescriptor.h"
#include "core/TemplateRegistry.h"
#include "ui/IconFactory.h"
#include "scene/MindMapScene.h"
#include "scene/NodeItem.h"

#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QUndoStack>
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

    // Template cards row — only the 3 builtins
    auto* cardRow = new QWidget();
    auto* cardLayout = new QHBoxLayout(cardRow);
    cardLayout->setAlignment(Qt::AlignCenter);
    cardLayout->setSpacing(24);

    QStringList builtinIds = {"builtin.mindmap", "builtin.orgchart", "builtin.projectplan"};
    for (const auto& id : builtinIds) {
        const auto* td = TemplateRegistry::instance().templateById(id);
        if (!td) continue;

        auto* card = new QPushButton();
        card->setObjectName("templateCard");
        card->setFixedSize(180, 140);
        card->setIconSize(QSize(160, 106));
        card->setIcon(QIcon(IconFactory::makeTemplatePreview(td->id, 160, 106)));
        card->setToolTip(td->name);
        card->setProperty("templateId", td->id);
        QString templateId = td->id;
        QObject::connect(card, &QPushButton::clicked, page,
                         [onTemplate, templateId]() { onTemplate(templateId); });
        cardLayout->addWidget(card);
    }
    outer->addWidget(cardRow);

    // Spacing
    outer->addSpacing(16);

    // Blank Canvas button + Load Template link
    auto* blankBtn = new QPushButton("Blank Canvas");
    blankBtn->setObjectName("blankCanvasBtn");
    blankBtn->setFixedSize(160, 36);
    QObject::connect(blankBtn, &QPushButton::clicked, page, [onBlankCanvas]() { onBlankCanvas(); });

    auto* blankRow = new QHBoxLayout();
    blankRow->setAlignment(Qt::AlignCenter);
    blankRow->addWidget(blankBtn);
    outer->addLayout(blankRow);

    // "Load Template..." underlined link
    outer->addSpacing(8);
    auto* loadLink = new QLabel("<a href=\"#\" style=\"color: inherit;\">Load Template...</a>");
    loadLink->setObjectName("loadTemplateLink");
    loadLink->setAlignment(Qt::AlignCenter);
    loadLink->setCursor(Qt::PointingHandCursor);
    QObject::connect(loadLink, &QLabel::linkActivated, page, [page, onTemplate]() {
        QString filePath = QFileDialog::getOpenFileName(
            page, "Load Template", QString(), "Template Files (*.json)");
        if (filePath.isEmpty())
            return;

        // Load the template file into the registry so loadTemplate() can find it
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly))
            return;

        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
        if (err.error != QJsonParseError::NoError)
            return;

        QJsonObject obj = doc.object();
        if (obj["$schema"].toString() != "ymind-template-v1")
            return;

        TemplateDescriptor td = TemplateDescriptor::fromJson(obj);
        if (td.id.isEmpty())
            return;

        // Register it temporarily so loadTemplate() can look it up
        TemplateRegistry::instance().registerTemplate(td);
        onTemplate(td.id);
    });
    auto* linkRow = new QHBoxLayout();
    linkRow->setAlignment(Qt::AlignCenter);
    linkRow->addWidget(loadLink);
    outer->addLayout(linkRow);

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

bool StartPage::loadTemplateFromFile(const QString& filePath, MindMapScene* scene) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    if (err.error != QJsonParseError::NoError)
        return false;

    QJsonObject obj = doc.object();
    if (obj["$schema"].toString() != "ymind-template-v1")
        return false;

    TemplateDescriptor td = TemplateDescriptor::fromJson(obj);
    if (td.id.isEmpty())
        return false;

    TemplateRegistry::instance().registerTemplate(td);
    loadTemplate(td.id, scene);
    return true;
}
