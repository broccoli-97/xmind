#include "scene/MindMapScene.h"
#include "core/Commands.h"
#include "core/TemplateDescriptor.h"
#include "core/TemplateRegistry.h"
#include "scene/EdgeItem.h"
#include "scene/InlineEditController.h"
#include "scene/MindMapExporter.h"
#include "scene/MindMapSerializer.h"
#include "scene/MindMapView.h"
#include "scene/NodeItem.h"

#include <QEasingCurve>
#include <QGraphicsSceneMouseEvent>
#include <QJsonObject>
#include <QKeyEvent>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QUndoStack>

MindMapScene::MindMapScene(QObject* parent) : QGraphicsScene(parent) {
    m_undoStack = new QUndoStack(this);
    connect(m_undoStack, &QUndoStack::cleanChanged, this,
            [this](bool clean) { setModified(!clean); });

    m_editController = new InlineEditController(this, this);

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

    // Remove edges connected to this node
    QList<EdgeItem*> edgesToRemove;
    for (auto* edge : m_edges) {
        if (edge->sourceNode() == node || edge->targetNode() == node) {
            edgesToRemove.append(edge);
        }
    }
    for (auto* edge : edgesToRemove) {
        edge->sourceNode()->removeEdge(edge);
        edge->targetNode()->removeEdge(edge);
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
    return m_layoutStyle;
}

void MindMapScene::setLayoutStyle(LayoutStyle style) {
    if (m_layoutStyle != style) {
        m_layoutStyle = style;
        emit layoutStyleChanged();
    }
}

QString MindMapScene::templateId() const {
    return m_templateId;
}

void MindMapScene::setTemplateId(const QString& id) {
    m_templateId = id;
    // Sync layout style from template
    const auto* td = templateDescriptor();
    if (td) {
        LayoutStyle ls = algorithmNameToLayoutStyle(td->layout.algorithm);
        if (m_layoutStyle != ls) {
            m_layoutStyle = ls;
            emit layoutStyleChanged();
        }
    }
}

const TemplateDescriptor* MindMapScene::templateDescriptor() const {
    if (m_templateId.isEmpty())
        return nullptr;
    return TemplateRegistry::instance().templateById(m_templateId);
}

EdgeItem* MindMapScene::findEdge(NodeItem* parent, NodeItem* child) const {
    for (auto* edge : m_edges) {
        if (edge->sourceNode() == parent && edge->targetNode() == child)
            return edge;
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
    case Qt::Key_Backspace:
        deleteSelected();
        event->accept();
        break;
    case Qt::Key_F2:
        if (auto* node = selectedNode()) {
            startEditing(node);
        }
        event->accept();
        break;
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
            if (auto* mv = qobject_cast<MindMapView*>(view))
                mv->zoomToFit();
        }
    });
    group->start();
}
