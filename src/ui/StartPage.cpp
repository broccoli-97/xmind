#include "ui/StartPage.h"
#include "core/AppSettings.h"
#include "core/TemplateDescriptor.h"
#include "core/TemplateRegistry.h"
#include "core/ThemeDescriptor.h"
#include "core/ThemeRegistry.h"
#include "ui/IconFactory.h"
#include "ui/ThemeManager.h"
#include "scene/MindMapScene.h"
#include "scene/NodeItem.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QUndoStack>
#include <QUrl>
#include <QVBoxLayout>

QWidget* StartPage::create(QObject* /*receiver*/, std::function<void(const QString&)> onTemplate,
                           std::function<void()> onBlankCanvas) {
    auto* page = new QWidget();
    page->setObjectName("startPage");

    auto* outer = new QVBoxLayout(page);
    outer->setAlignment(Qt::AlignCenter);

    // Title
    auto* title = new QLabel(QCoreApplication::translate("StartPage", "Create a New Mind Map"));
    title->setObjectName("startPageTitle");
    title->setAlignment(Qt::AlignCenter);
    outer->addWidget(title);

    // Subtitle
    auto* subtitle =
        new QLabel(QCoreApplication::translate("StartPage", "Choose a template to get started"));
    subtitle->setObjectName("startPageSubtitle");
    subtitle->setAlignment(Qt::AlignCenter);
    outer->addWidget(subtitle);

    // Template cards row — the 4 built-in templates. Mind Map / Org Chart /
    // Project Plan differ in their layout algorithm; Lined is also a template
    // (not a theme) because its underline-shaped nodes and baseline-anchored
    // connectors are structural, not just decoration. Visual styling (colors,
    // fills, borders, shadows) lives on themes and is picked separately from
    // the Theme menu after a template is selected.
    auto* cardRow = new QWidget();
    auto* cardLayout = new QHBoxLayout(cardRow);
    cardLayout->setAlignment(Qt::AlignCenter);
    cardLayout->setSpacing(24);

    QStringList builtinIds = {"builtin.mindmap", "builtin.orgchart",
                              "builtin.projectplan", "builtin.lined"};
    for (const auto& id : builtinIds) {
        const auto* td = TemplateRegistry::instance().templateById(id);
        if (!td) continue;

        // Each card is a vertical tile: clickable thumbnail on top, native
        // text label below. Pulling text out of the pixmap eliminates the
        // blurry-when-scaled text and lets the title use crisp system text.
        auto* tile = new QWidget();
        auto* tileLayout = new QVBoxLayout(tile);
        tileLayout->setContentsMargins(0, 0, 0, 0);
        tileLayout->setSpacing(8);
        tileLayout->setAlignment(Qt::AlignHCenter);

        auto* card = new QPushButton();
        card->setObjectName("templateCard");
        card->setFixedSize(180, 124);
        card->setIconSize(QSize(160, 100));
        card->setIcon(QIcon(IconFactory::makeTemplatePreview(td->id, 160, 100)));
        card->setToolTip(td->description.isEmpty() ? td->name : td->description);
        card->setProperty("templateId", td->id);
        QString templateId = td->id;
        QObject::connect(card, &QPushButton::clicked, page,
                         [onTemplate, templateId]() { onTemplate(templateId); });

        auto* nameLabel = new QLabel(td->name);
        nameLabel->setObjectName("templateCardName");
        nameLabel->setAlignment(Qt::AlignCenter);
        // The card thumbnail above is the click target; keep the label as a
        // plain caption (default cursor) so we don't promise clickability we
        // don't deliver.

        tileLayout->addWidget(card);
        tileLayout->addWidget(nameLabel);
        cardLayout->addWidget(tile);
    }
    outer->addWidget(cardRow);

    // Spacing
    outer->addSpacing(16);

    // Blank Canvas button + Load Theme link
    auto* blankBtn =
        new QPushButton(QCoreApplication::translate("StartPage", "Blank Canvas"));
    blankBtn->setObjectName("blankCanvasBtn");
    blankBtn->setFixedSize(160, 36);
    QObject::connect(blankBtn, &QPushButton::clicked, page, [onBlankCanvas]() { onBlankCanvas(); });

    auto* blankRow = new QHBoxLayout();
    blankRow->setAlignment(Qt::AlignCenter);
    blankRow->addWidget(blankBtn);
    outer->addLayout(blankRow);

    // Link color is baked into the inline HTML because Qt's rich-text engine
    // ignores `color: inherit` and QSS descendant rules for anchor tags. The
    // helper re-emits the HTML so we can refresh it when the theme changes.
    auto applyLinkHtml = [](QLabel* label, const QString& text) {
        const QString color = ThemeManager::isDark() ? QStringLiteral("#FFFFFF")
                                                     : QStringLiteral("#000000");
        label->setText(QString("<a href=\"#\" style=\"color:%1;\">%2</a>").arg(color, text));
    };

    const QString loadText = QCoreApplication::translate("StartPage", "Load Theme...");
    outer->addSpacing(8);
    auto* loadLink = new QLabel();
    applyLinkHtml(loadLink, loadText);
    loadLink->setObjectName("loadTemplateLink");
    loadLink->setAlignment(Qt::AlignCenter);
    loadLink->setCursor(Qt::PointingHandCursor);
    QObject::connect(loadLink, &QLabel::linkActivated, page, [page, onTemplate]() {
        QString filePath = QFileDialog::getOpenFileName(
            page,
            QCoreApplication::translate("StartPage", "Load Theme"),
            QString(),
            QCoreApplication::translate("StartPage", "Theme Files (*.json)"));
        if (filePath.isEmpty())
            return;

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly))
            return;

        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
        if (err.error != QJsonParseError::NoError)
            return;

        QJsonObject obj = doc.object();
        QString schema = obj["$schema"].toString();
        // Accept both template and theme files. Templates create a new doc
        // with their layout + content; themes register so the new doc can
        // apply them (the caller starts with a Mind Map template).
        if (schema == QLatin1String("ymind-template-v1")) {
            TemplateDescriptor td = TemplateDescriptor::fromJson(obj);
            if (td.id.isEmpty())
                return;
            TemplateRegistry::instance().registerTemplate(td);
            onTemplate(td.id);
        } else if (schema == QLatin1String("ymind-theme-v1")) {
            ThemeDescriptor th = ThemeDescriptor::fromJson(obj);
            if (th.id.isEmpty())
                return;
            ThemeRegistry::instance().registerTheme(th);
            // Start a default Mind Map doc; the theme will be applied below
            // via StartPage::loadTemplate when the scene picks it up.
            // The applied theme is tracked via the scene's pendingThemeId
            // mechanism — kept simple here: theme application is left to
            // the menu after the doc opens. Just open Mind Map.
            onTemplate(QStringLiteral("builtin.mindmap"));
        }
    });
    auto* linkRow = new QHBoxLayout();
    linkRow->setAlignment(Qt::AlignCenter);
    linkRow->addWidget(loadLink);
    outer->addLayout(linkRow);

    // "Browse themes online…" link — opens the GitHub Pages themes page where
    // users can grab additional .json styles to drop in their templates folder.
    const QString browseText =
        QCoreApplication::translate("StartPage", "Browse themes online...");
    auto* browseLink = new QLabel();
    applyLinkHtml(browseLink, browseText);
    browseLink->setObjectName("loadTemplateLink");
    browseLink->setAlignment(Qt::AlignCenter);
    browseLink->setCursor(Qt::PointingHandCursor);
    QObject::connect(browseLink, &QLabel::linkActivated, page, []() {
        QDesktopServices::openUrl(QUrl("https://broccoli-97.github.io/xmind/#themes"));
    });
    auto* browseRow = new QHBoxLayout();
    browseRow->setAlignment(Qt::AlignCenter);
    browseRow->addWidget(browseLink);
    outer->addLayout(browseRow);

    // Refresh both link colors live when the user toggles the app theme.
    // Receiver context is `page` so the connection drops automatically when
    // the start page widget is destroyed.
    QObject::connect(&AppSettings::instance(), &AppSettings::themeChanged, page,
                     [applyLinkHtml, loadLink, loadText, browseLink, browseText]() {
                         applyLinkHtml(loadLink, loadText);
                         applyLinkHtml(browseLink, browseText);
                     });

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
    // New documents start on the default theme. The user can switch to any
    // other theme afterwards from the Theme menu without affecting layout.
    if (scene->themeId().isEmpty())
        scene->setThemeId(ThemeRegistry::defaultThemeId());

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
