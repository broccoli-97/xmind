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

    // What the view does once the auto-layout animation lands.
    //   IfOffscreen — refit only when content runs off-screen, so a
    //                 deliberate user zoom survives a layout that stays
    //                 within the viewport (the Ctrl+L contract).
    //   Always      — unconditional refit; for template switches, where the
    //                 algorithm/spacing change rearranges the whole map and
    //                 the previous zoom/pan no longer frames anything useful.
    enum class PostLayoutFit { IfOffscreen, Always };

    // Recompute every node position with the active layout algorithm and
    // animate nodes there. This is the ONLY full re-layout entry point, and
    // it runs exclusively on explicit gestures:
    //   - the user invokes Auto Layout (Ctrl+L / toolbar / Edit menu),
    //   - the user switches template (algorithm or spacing changed),
    //   - the user collapses/expands a subtree (visible structure changed:
    //     collapsing reclaims the folded branch's space, expanding makes
    //     room for it) — see toggleNodeCollapsed(),
    //   - initial arrangement of fresh content (new-from-template, imports).
    // Editing must never trigger it: confirming a node's text, adding a node
    // (placed via LayoutEngine::initialChildPosition), or undo/redo keep all
    // other nodes where they are, so a user zoomed into a large map is not
    // disturbed. Window resize refits the *view* (MindMapView::zoomToFit)
    // but never moves nodes.
    void autoLayout(PostLayoutFit fit = PostLayoutFit::IfOffscreen);

    // Fold/unfold `node`'s subtree from an explicit user gesture (the Space
    // shortcut, the collapse badge/chevron on the canvas, an outline fold).
    // Owns the full dance: flips the state, dirties the document, notifies
    // observers (nodeCollapseChanged), and re-layouts so the map tightens
    // around a folded branch / makes room for an unfolded one. No-op for
    // null or childless nodes. NodeItem::setCollapsed remains the raw state
    // setter for deserialization and tests (no layout side effects).
    void toggleNodeCollapsed(NodeItem* node);

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

    // Keyboard navigation: arrow + Tab semantics. The mapping is layout-aware
    // because siblings stack vertically in Bilateral/RightTree and horizontally
    // in TopDown, and the "toward children" direction depends on which side of
    // the root a node sits on (for Bilateral).
    //   Up/Down/Left/Right - directional move; semantic depends on layout
    //   NextSibling/PrevSibling - cyclic, layout-independent (used for Tab)
    enum class NavDirection { Up, Down, Left, Right, NextSibling, PrevSibling };
    NodeItem* findNeighbor(NodeItem* node, NavDirection dir) const;

    // Walk the tree pre-order and collect every node whose text contains the
    // needle. Pure function; the in-map find bar uses this for match
    // navigation. Returns an empty list when needle is empty or the scene
    // has no root.
    QList<NodeItem*> findMatches(const QString& needle,
                                 Qt::CaseSensitivity cs = Qt::CaseInsensitive) const;

    // Clear the search highlight on every node (no-op for nodes that weren't
    // highlighted). The find bar calls this when it closes.
    void clearSearchHighlights();

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
    bool importFromMarkdownStrict(const QString& text, MarkdownImportReport* report,
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
    // Re-emitted from InlineEditController so external observers (e.g.
    // MainWindow's status bar) don't have to peer inside the controller.
    void editingStarted(NodeItem* node);
    void editingFinished();
    // Emitted when a node's collapse state is toggled from any user gesture
    // (Space shortcut, collapse badge/chevron, outline fold — all routed
    // through toggleNodeCollapsed) so other views — the outline panel in
    // particular — can mirror the fold state without polling.
    void nodeCollapseChanged(NodeItem* node);

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
