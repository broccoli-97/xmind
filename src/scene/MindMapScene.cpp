#include "scene/MindMapScene.h"
#include "core/Commands.h"
#include "core/TemplateDescriptor.h"
#include "core/ThemeDescriptor.h"
#include "scene/EdgeItem.h"
#include "scene/InlineEditController.h"
#include "scene/MindMapExporter.h"
#include "scene/MindMapSerializer.h"
#include "scene/MindMapView.h"
#include "scene/NodeItem.h"
#include "scene/StyleProvider.h"

#include <QEasingCurve>
#include <QGraphicsSceneMouseEvent>
#include <QJsonObject>
#include <QKeyEvent>
#include <QParallelAnimationGroup>
#include <QPointer>
#include <QPropertyAnimation>
#include <QTimer>
#include <QUndoStack>

// Global fallback set by app/tests at startup so existing default-constructed
// scenes keep working without their callsites needing to inject a provider.
static StyleProvider* s_defaultStyleProvider = nullptr;

StyleProvider* MindMapScene::defaultStyleProvider() {
    return s_defaultStyleProvider;
}

void MindMapScene::setDefaultStyleProvider(StyleProvider* provider) {
    s_defaultStyleProvider = provider;
}

void MindMapScene::setStyleProvider(StyleProvider* provider) {
    m_styleProvider = provider;
}

StyleProvider* MindMapScene::styleProvider() const {
    return m_styleProvider ? m_styleProvider : s_defaultStyleProvider;
}

MindMapScene::MindMapScene(QObject* parent) : QGraphicsScene(parent) {
    m_undoStack = new QUndoStack(this);
    connect(m_undoStack, &QUndoStack::cleanChanged, this,
            [this](bool clean) { setModified(!clean); });

    m_editController = new InlineEditController(this, this);
    // Re-emit editing signals so MainWindow (or any other observer) doesn't
    // need to know about the controller.
    connect(m_editController, &InlineEditController::editingStarted, this,
            &MindMapScene::editingStarted);
    connect(m_editController, &InlineEditController::editingFinished, this,
            &MindMapScene::editingFinished);

    m_rootNode = createRootNode(tr("Central Topic"));
}

NodeItem* MindMapScene::rootNode() const {
    return m_rootNode;
}

NodeItem* MindMapScene::createRootNode(const QString& text) {
    auto* node = new NodeItem(text);
    addItem(node);
    node->setPos(0, 0);
    connect(node, &NodeItem::doubleClicked, this, &MindMapScene::startEditing);
    return node;
}

void MindMapScene::registerEdge(EdgeItem* edge) {
    m_edges.append(edge);
}

void MindMapScene::unregisterEdge(EdgeItem* edge) {
    m_edges.removeOne(edge);
}

NodeItem* MindMapScene::addNode(const QString& text, NodeItem* parent) {
    if (!parent)
        return nullptr;

    auto* node = new NodeItem(text);
    addItem(node);
    parent->addChild(node);

    // Create edge
    auto* edge = new EdgeItem(parent, node);
    addItem(edge);
    parent->addEdge(edge);
    node->addEdge(edge);
    m_edges.append(edge);

    // Position avoiding overlap with existing nodes
    const auto* td = templateDescriptor();
    if (td) {
        LayoutParams params{td->layout.depthSpacing, td->layout.spreadSpacing};
        node->setPos(LayoutEngine::initialChildPosition(node, parent, m_rootNode,
                                                        td->layout.algorithm, params));
    } else {
        node->setPos(LayoutEngine::initialChildPosition(node, parent, m_rootNode, m_layoutStyle));
    }

    connect(node, &NodeItem::doubleClicked, this, &MindMapScene::startEditing);

    // Select the new node
    clearSelection();
    node->setSelected(true);

    markModified();
    return node;
}

void MindMapScene::removeNode(NodeItem* node) {
    if (!node || node == m_rootNode)
        return;

    // Recursively remove children first
    auto children = node->childNodes();
    for (auto* child : children) {
        removeNode(child);
    }

    // Drop edges incident to this node. The node already tracks them on its
    // own m_edges, so we don't need to scan the scene's full edge list.
    const auto incident = node->edges();
    for (auto* edge : incident) {
        NodeItem* other = (edge->sourceNode() == node) ? edge->targetNode() : edge->sourceNode();
        if (other)
            other->removeEdge(edge);
        m_edges.removeOne(edge);
        removeItem(edge);
        delete edge;
    }

    // Remove from parent
    if (node->parentNode()) {
        node->parentNode()->removeChild(node);
    }

    removeItem(node);
    delete node;

    markModified();
}

NodeItem* MindMapScene::selectedNode() const {
    auto sel = selectedItems();
    for (auto* item : sel) {
        auto* node = dynamic_cast<NodeItem*>(item);
        if (node)
            return node;
    }
    return nullptr;
}

QUndoStack* MindMapScene::undoStack() const {
    return m_undoStack;
}

bool MindMapScene::isEditing() const {
    return m_editController->isEditing();
}

LayoutStyle MindMapScene::layoutStyle() const {
    // Template wins when set — m_layoutStyle is the user's fallback for
    // template-less scenes (legacy v1 files, CLI without --template). Single
    // source of truth at read-time replaces the old setTemplateId sync.
    if (const auto* td = templateDescriptor())
        return algorithmNameToLayoutStyle(td->layout.algorithm);
    return m_layoutStyle;
}

void MindMapScene::setLayoutStyle(LayoutStyle style) {
    if (m_layoutStyle == style)
        return;
    const LayoutStyle oldEffective = layoutStyle();
    m_layoutStyle = style;
    if (layoutStyle() != oldEffective)
        emit layoutStyleChanged();
}

TemplateId MindMapScene::templateId() const {
    return m_templateId;
}

void MindMapScene::setTemplateId(const TemplateId& id) {
    if (m_templateId == id)
        return;
    const LayoutStyle oldEffective = layoutStyle();
    m_templateId = id;
    if (layoutStyle() != oldEffective)
        emit layoutStyleChanged();
    invalidateStyle();
}

const TemplateDescriptor* MindMapScene::templateDescriptor() const {
    if (!m_templateId.isValid())
        return nullptr;
    auto* sp = styleProvider();
    return sp ? sp->templateById(m_templateId.toString()) : nullptr;
}

ThemeId MindMapScene::themeId() const {
    return m_themeId;
}

void MindMapScene::setThemeId(const ThemeId& id) {
    m_themeId = id;
    invalidateStyle();
}

void MindMapScene::invalidateStyle() {
    // Drop device caches so items repaint with the new theme/template's
    // fill/border/edge style instead of a stale cached pixmap. Re-measure
    // each node — themes/templates differ in padding/font/min-width, and
    // without a refresh m_rect stays at the previous style's dimensions.
    const auto allItems = this->items();
    for (auto* it : allItems) {
        it->setCacheMode(QGraphicsItem::NoCache);
        if (auto* node = dynamic_cast<NodeItem*>(it))
            node->refreshGeometry();
        it->update();
    }
    for (auto* e : m_edges)
        e->updatePath();
    for (auto* v : views())
        v->viewport()->update();

    // Restore device caches on the next tick so the no-cache repaint above
    // completes first. Only NodeItem uses DeviceCoordinateCache; leave edges
    // and overlays alone.
    QPointer<MindMapScene> guard(this);
    QTimer::singleShot(0, this, [guard]() {
        if (!guard)
            return;
        const auto its = guard->items();
        for (auto* item : its) {
            if (dynamic_cast<NodeItem*>(item))
                item->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
        }
    });
}

const ThemeDescriptor* MindMapScene::themeDescriptor() const {
    auto* sp = styleProvider();
    if (!sp)
        return nullptr;
    const QString id = m_themeId.isValid() ? m_themeId.toString() : sp->defaultThemeId();
    return sp->themeById(id);
}

EdgeItem* MindMapScene::findEdge(NodeItem* parent, NodeItem* child) const {
    for (auto* edge : m_edges) {
        if (edge->sourceNode() == parent && edge->targetNode() == child)
            return edge;
    }
    return nullptr;
}

QList<NodeItem*> MindMapScene::findMatches(const QString& needle, Qt::CaseSensitivity cs) const {
    QList<NodeItem*> result;
    if (needle.isEmpty() || !m_rootNode)
        return result;
    std::function<void(NodeItem*)> walk = [&](NodeItem* n) {
        if (n->text().contains(needle, cs))
            result.append(n);
        for (auto* c : n->childNodes())
            walk(c);
    };
    walk(m_rootNode);
    return result;
}

void MindMapScene::clearSearchHighlights() {
    for (auto* it : items()) {
        if (auto* n = dynamic_cast<NodeItem*>(it)) {
            n->setSearchMatch(false);
            n->setSearchCurrent(false);
        }
    }
}

NodeItem* MindMapScene::findNeighbor(NodeItem* node, NavDirection dir) const {
    if (!node)
        return nullptr;

    NodeItem* parent = node->parentNode();
    QList<NodeItem*> siblings = parent ? parent->childNodes() : QList<NodeItem*>{};

    auto firstChild = [&]() -> NodeItem* {
        const auto children = node->childNodes();
        return children.isEmpty() ? nullptr : children.first();
    };
    auto prevSibling = [&]() -> NodeItem* {
        if (siblings.isEmpty())
            return nullptr;
        int idx = siblings.indexOf(node);
        if (idx <= 0)
            return nullptr;
        return siblings[idx - 1];
    };
    auto nextSibling = [&]() -> NodeItem* {
        if (siblings.isEmpty())
            return nullptr;
        int idx = siblings.indexOf(node);
        if (idx < 0 || idx == siblings.size() - 1)
            return nullptr;
        return siblings[idx + 1];
    };
    auto cyclicNext = [&]() -> NodeItem* {
        if (siblings.size() < 2)
            return nullptr;
        int idx = siblings.indexOf(node);
        if (idx < 0)
            return nullptr;
        return siblings[(idx + 1) % siblings.size()];
    };
    auto cyclicPrev = [&]() -> NodeItem* {
        if (siblings.size() < 2)
            return nullptr;
        int idx = siblings.indexOf(node);
        if (idx < 0)
            return nullptr;
        return siblings[(idx - 1 + siblings.size()) % siblings.size()];
    };

    if (dir == NavDirection::NextSibling)
        return cyclicNext();
    if (dir == NavDirection::PrevSibling)
        return cyclicPrev();

    // Layout-aware arrow mapping. "Toward children" axis varies by layout
    // (and, for bilateral, by which side of the root the node sits on);
    // siblings stack on the perpendicular axis.
    enum class TowardChildren { Right, Left, Down };
    TowardChildren toChildren;
    const LayoutStyle style = layoutStyle();
    if (style == LayoutStyle::TopDown) {
        toChildren = TowardChildren::Down;
    } else if (style == LayoutStyle::RightTree) {
        toChildren = TowardChildren::Right;
    } else {
        // Bilateral: root has no preferred side, so default to Right; otherwise
        // mirror by x-position (children grow outward from the root).
        toChildren =
            (!parent || node->pos().x() >= 0) ? TowardChildren::Right : TowardChildren::Left;
    }

    switch (dir) {
    case NavDirection::Up:
        return (toChildren == TowardChildren::Down) ? parent : prevSibling();
    case NavDirection::Down:
        return (toChildren == TowardChildren::Down) ? firstChild() : nextSibling();
    case NavDirection::Left:
        if (toChildren == TowardChildren::Right)
            return parent;
        if (toChildren == TowardChildren::Left)
            return firstChild();
        return prevSibling(); // TopDown
    case NavDirection::Right:
        if (toChildren == TowardChildren::Right)
            return firstChild();
        if (toChildren == TowardChildren::Left)
            return parent;
        return nextSibling(); // TopDown
    case NavDirection::NextSibling:
    case NavDirection::PrevSibling:
        break; // handled above
    }
    return nullptr;
}

void MindMapScene::addChildToSelected() {
    if (m_editController->isEditing())
        finishEditing();
    NodeItem* node = selectedNode();
    if (!node)
        node = m_rootNode;

    auto* cmd = new AddNodeCommand(this, node, tr("New Topic"));
    m_undoStack->push(cmd);

    for (auto* view : views()) {
        if (auto* mv = qobject_cast<MindMapView*>(view))
            mv->ensureNodeVisible(cmd->createdNode());
    }

    startEditing(cmd->createdNode());
}

void MindMapScene::addSiblingToSelected() {
    if (m_editController->isEditing())
        finishEditing();
    NodeItem* node = selectedNode();
    if (!node || node == m_rootNode) {
        addChildToSelected();
        return;
    }

    auto* cmd = new AddNodeCommand(this, node->parentNode(), tr("New Topic"));
    m_undoStack->push(cmd);

    for (auto* view : views()) {
        if (auto* mv = qobject_cast<MindMapView*>(view))
            mv->ensureNodeVisible(cmd->createdNode());
    }

    startEditing(cmd->createdNode());
}

void MindMapScene::deleteSelected() {
    if (m_editController->isEditing())
        cancelEditing();
    NodeItem* node = selectedNode();
    if (node && node != m_rootNode) {
        m_undoStack->push(new RemoveNodeCommand(this, node));
    }
}

void MindMapScene::startEditing(NodeItem* node) {
    m_editController->startEditing(node);
}

void MindMapScene::finishEditing() {
    m_editController->finishEditing();
}

void MindMapScene::cancelEditing() {
    m_editController->cancelEditing();
}

void MindMapScene::keyPressEvent(QKeyEvent* event) {
    if (m_editController->isEditing()) {
        QGraphicsScene::keyPressEvent(event);
        return;
    }

    switch (event->key()) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (event->modifiers() & Qt::ControlModifier) {
            addSiblingToSelected();
        } else {
            addChildToSelected();
        }
        event->accept();
        break;
    case Qt::Key_Delete:
        deleteSelected();
        event->accept();
        break;
    case Qt::Key_Backspace:
        // Bare Backspace is hostile: a user tapping it to correct a typo on a
        // selected node would lose the whole node. Require Ctrl/Cmd to delete
        // via Backspace — keeps macOS muscle memory (the physical key labelled
        // "delete" on a Mac keyboard *is* Backspace) without the footgun.
        if (event->modifiers() & (Qt::ControlModifier | Qt::MetaModifier)) {
            deleteSelected();
            event->accept();
        } else {
            QGraphicsScene::keyPressEvent(event);
        }
        break;
    case Qt::Key_F2:
        if (auto* node = selectedNode()) {
            startEditing(node);
        }
        event->accept();
        break;
    case Qt::Key_Space:
        // Space toggles collapse on the selected non-root node (root has no
        // siblings or hidden state worth toggling). Leaves with no children
        // also opt out since the indicator wouldn't make sense.
        if (auto* node = selectedNode();
            node && node != m_rootNode && !node->childNodes().isEmpty()) {
            node->toggleCollapsed();
            markModified();
            emit nodeCollapseChanged(node);
            event->accept();
            break;
        }
        QGraphicsScene::keyPressEvent(event);
        break;
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Tab:
    case Qt::Key_Backtab: {
        // Pick the navigation source: the current selection, or fall back to
        // the root so a fresh map without any selection still responds.
        NodeItem* current = selectedNode();
        if (!current)
            current = m_rootNode;
        if (!current) {
            QGraphicsScene::keyPressEvent(event);
            return;
        }

        NavDirection dir;
        switch (event->key()) {
        case Qt::Key_Up:
            dir = NavDirection::Up;
            break;
        case Qt::Key_Down:
            dir = NavDirection::Down;
            break;
        case Qt::Key_Left:
            dir = NavDirection::Left;
            break;
        case Qt::Key_Right:
            dir = NavDirection::Right;
            break;
        case Qt::Key_Tab:
            dir = NavDirection::NextSibling;
            break;
        case Qt::Key_Backtab:
            dir = NavDirection::PrevSibling;
            break;
        default:
            dir = NavDirection::Up;
            break; // unreachable
        }

        if (auto* target = findNeighbor(current, dir)) {
            // Don't try to land on a hidden node — collapsed parents
            // shouldn't be navigable into via arrow keys.
            if (target->isVisible()) {
                clearSelection();
                target->setSelected(true);
                for (auto* view : views()) {
                    if (auto* mv = qobject_cast<MindMapView*>(view))
                        mv->ensureNodeVisible(target);
                }
            }
        }
        event->accept();
        break;
    }
    default:
        QGraphicsScene::keyPressEvent(event);
    }
}

void MindMapScene::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    m_editController->handleMousePress(event->scenePos());
    QGraphicsScene::mousePressEvent(event);
}

// --- Modified state ---

bool MindMapScene::isModified() const {
    return m_modified;
}

void MindMapScene::setModified(bool modified) {
    if (m_modified != modified) {
        m_modified = modified;
        emit modifiedChanged(m_modified);
    }
}

void MindMapScene::markModified() {
    if (!m_batchLoading)
        setModified(true);
}

// --- Serialization (delegates to MindMapSerializer) ---

QJsonObject MindMapScene::toJson() const {
    return MindMapSerializer(const_cast<MindMapScene*>(this)).toJson();
}

bool MindMapScene::fromJson(const QJsonObject& json) {
    return MindMapSerializer(this).fromJson(json);
}

bool MindMapScene::saveToFile(const QString& filePath) {
    return MindMapSerializer(this).saveToFile(filePath);
}

bool MindMapScene::loadFromFile(const QString& filePath) {
    return MindMapSerializer(this).loadFromFile(filePath);
}

void MindMapScene::clearScene() {
    if (m_editController->isEditing())
        cancelEditing();

    m_undoStack->clear();

    // Remove all edges
    for (auto* edge : m_edges) {
        removeItem(edge);
        delete edge;
    }
    m_edges.clear();

    // Remove all nodes directly (edges are already cleared above)
    if (m_rootNode) {
        QList<NodeItem*> allNodes;
        std::function<void(NodeItem*)> collectNodes = [&](NodeItem* node) {
            for (auto* child : node->childNodes())
                collectNodes(child);
            allNodes.append(node);
        };
        collectNodes(m_rootNode);

        for (auto* node : allNodes) {
            removeItem(node);
            delete node;
        }
        m_rootNode = nullptr;
    }

    setModified(false);
}

// --- Export/Import (delegates to MindMapExporter) ---

QString MindMapScene::exportToText() const {
    return MindMapExporter(const_cast<MindMapScene*>(this)).exportToText();
}

QString MindMapScene::exportToMarkdown() const {
    return MindMapExporter(const_cast<MindMapScene*>(this)).exportToMarkdown();
}

bool MindMapScene::exportToPng(const QString& filePath, int scaleFactor) {
    return MindMapExporter(this).exportToPng(filePath, scaleFactor);
}

bool MindMapScene::exportToSvg(const QString& filePath) {
    return MindMapExporter(this).exportToSvg(filePath);
}

bool MindMapScene::exportToPdf(const QString& filePath) {
    return MindMapExporter(this).exportToPdf(filePath);
}

bool MindMapScene::importFromText(const QString& text) {
    return MindMapExporter(this).importFromText(text);
}

bool MindMapScene::importFromMarkdown(const QString& text, bool animate) {
    return MindMapExporter(this).importFromMarkdown(text, animate);
}

bool MindMapScene::importFromMarkdownStrict(const QString& text, MarkdownImportReport* report,
                                            bool animate) {
    return MindMapExporter(this).importFromMarkdownStrict(text, report, animate);
}

void MindMapScene::layoutWithoutAnimation() {
    if (!m_rootNode)
        return;

    QMap<NodeItem*, QPointF> positions;
    const auto* td = templateDescriptor();
    if (td) {
        LayoutParams params{td->layout.depthSpacing, td->layout.spreadSpacing};
        positions = LayoutEngine::computeLayout(m_rootNode, td->layout.algorithm, params);
    } else {
        positions = LayoutEngine::computeLayout(m_rootNode, m_layoutStyle);
    }

    for (auto it = positions.begin(); it != positions.end(); ++it)
        it.key()->setPos(it.value());
}

// --- Auto-layout ---

void MindMapScene::autoLayout() {
    if (!m_rootNode)
        return;
    if (m_editController->isEditing())
        finishEditing();

    QMap<NodeItem*, QPointF> positions;
    const auto* td = templateDescriptor();
    if (td) {
        LayoutParams params{td->layout.depthSpacing, td->layout.spreadSpacing};
        positions = LayoutEngine::computeLayout(m_rootNode, td->layout.algorithm, params);
    } else {
        positions = LayoutEngine::computeLayout(m_rootNode, m_layoutStyle);
    }

    // Animate to new positions
    auto* group = new QParallelAnimationGroup(this);
    for (auto it = positions.begin(); it != positions.end(); ++it) {
        auto* anim = new QPropertyAnimation(it.key(), "pos");
        anim->setDuration(400);
        anim->setEndValue(it.value());
        anim->setEasingCurve(QEasingCurve::OutCubic);
        group->addAnimation(anim);
    }
    connect(group, &QAbstractAnimation::finished, this, [this, group]() {
        group->deleteLater();
        for (auto* view : views()) {
            // Use the conditional variant so a deliberate user zoom isn't
            // yanked away when auto-layout doesn't move content off-screen.
            if (auto* mv = qobject_cast<MindMapView*>(view))
                mv->zoomToFitIfOffscreen();
        }
    });
    group->start();
}
