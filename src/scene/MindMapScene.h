#pragma once

#include "core/Identifiers.h"
#include "layout/LayoutEngine.h"
#include "scene/MindMapExporter.h"

#include <QGraphicsScene>
#include <QMap>

class NodeItem;
class EdgeItem;
class QJsonObject;
class QJsonArray;
class QUndoStack;
class TemplateDescriptor;
class ThemeDescriptor;
class InlineEditController;
class StyleProvider;

class MindMapScene : public QGraphicsScene {
    Q_OBJECT

public:
    explicit MindMapScene(QObject* parent = nullptr);

    // Style lookup. Injection is global because constructing a scene happens
    // in dozens of test cases — making every site explicit was more churn
    // than the win warranted. App startup (MainWindow, tests, CLI) sets the
    // default once; per-scene overrides via setStyleProvider() are still
    // available for callers that need them.
    static StyleProvider* defaultStyleProvider();
    static void setDefaultStyleProvider(StyleProvider* provider);
    void setStyleProvider(StyleProvider* provider);
    StyleProvider* styleProvider() const;

    NodeItem* rootNode() const;
    NodeItem* addNode(const QString& text, NodeItem* parent);
    void removeNode(NodeItem* node);
    void autoLayout();

    NodeItem* selectedNode() const;

    QUndoStack* undoStack() const;
    bool isEditing() const;

    LayoutStyle layoutStyle() const;
    void setLayoutStyle(LayoutStyle style);

    // Template — layout + starter content (picked when the map was created)
    TemplateId templateId() const;
    void setTemplateId(const TemplateId& id);
    const TemplateDescriptor* templateDescriptor() const;

    // Theme — visual style, swappable any time without affecting layout/content
    ThemeId themeId() const;
    void setThemeId(const ThemeId& id);
    const ThemeDescriptor* themeDescriptor() const;

    EdgeItem* findEdge(NodeItem* parent, NodeItem* child) const;

    // Serialization
    QJsonObject toJson() const;
    bool fromJson(const QJsonObject& json);
    bool saveToFile(const QString& filePath);
    bool loadFromFile(const QString& filePath);

    // Export/Import
    QString exportToText() const;
    QString exportToMarkdown() const;
    bool exportToPng(const QString& filePath, int scaleFactor = 2);
    bool exportToSvg(const QString& filePath);
    bool exportToPdf(const QString& filePath);
    bool importFromText(const QString& text);
    bool importFromMarkdown(const QString& text, bool animate = true);
    // Strict variant: see MindMapExporter::importFromMarkdownStrict. Returns
    // false and leaves the scene untouched when `report->errors` is non-empty.
    bool importFromMarkdownStrict(const QString& text,
                                  MarkdownImportReport* report,
                                  bool animate = true);

    // Immediate (non-animated) layout — required for headless/CLI rendering
    // where no event loop is available to drive QPropertyAnimation.
    void layoutWithoutAnimation();

    // Scene management
    void clearScene();
    bool isModified() const;
    void setModified(bool modified);

    // Root node creation (consolidates 4 duplicated patterns)
    NodeItem* createRootNode(const QString& text);

    // Edge registration (public API for Commands)
    void registerEdge(EdgeItem* edge);
    void unregisterEdge(EdgeItem* edge);

signals:
    void modifiedChanged(bool modified);
    void fileLoaded(const QString& filePath);
    void layoutStyleChanged();

public slots:
    void addChildToSelected();
    void addSiblingToSelected();
    void deleteSelected();
    void startEditing(NodeItem* node);

    void cancelEditing();

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    friend class MindMapSerializer;
    friend class MindMapExporter;

    void finishEditing();
    void markModified();

    // Theme/template swap invalidation. Drops device-coord caches, re-measures
    // nodes (padding/font/min-width can change), repaints edges, and schedules
    // cache restoration on the next event-loop tick so the no-cache repaint
    // completes first. Owns the entire dance so callers don't have to.
    void invalidateStyle();

    NodeItem* m_rootNode = nullptr;
    QList<EdgeItem*> m_edges;
    QUndoStack* m_undoStack;
    bool m_modified = false;
    bool m_batchLoading = false;
    LayoutStyle m_layoutStyle = LayoutStyle::Bilateral;
    TemplateId m_templateId;
    ThemeId m_themeId;
    StyleProvider* m_styleProvider = nullptr;

    // Editing
    InlineEditController* m_editController;
};
