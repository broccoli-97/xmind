#include "Commands.h"
#include "EdgeItem.h"
#include "LayoutEngine.h"
#include "MindMapScene.h"
#include "NodeItem.h"

// ===========================================================================
// AddNodeCommand
// ===========================================================================

AddNodeCommand::AddNodeCommand(MindMapScene* scene, NodeItem* parent, const QString& text,
                               QUndoCommand* parentCmd)
    : QUndoCommand("Add Node", parentCmd), m_scene(scene), m_parent(parent), m_text(text) {}

AddNodeCommand::~AddNodeCommand() {
    if (m_ownsObjects) {
        delete m_edge;
        delete m_node;
    }
}

void AddNodeCommand::redo() {
    if (!m_node) {
        // First time: create the node and edge
        m_node = new NodeItem(m_text);
        m_scene->addItem(m_node);
        m_parent->addChild(m_node);

        auto* edge = new EdgeItem(m_parent, m_node);
        m_scene->addItem(edge);
        m_parent->addEdge(edge);
        m_node->addEdge(edge);
        m_scene->registerEdge(edge);
        m_edge = edge;

        // Position near parent
        m_node->setPos(LayoutEngine::initialChildPosition(m_parent, m_scene->rootNode(),
                                                          m_scene->layoutStyle()));

        QObject::connect(m_node, &NodeItem::doubleClicked, m_scene, &MindMapScene::startEditing);
    } else {
        // Re-do: re-attach existing objects
        m_scene->addItem(m_node);
        m_scene->addItem(m_edge);
        m_parent->addChild(m_node);
        m_parent->addEdge(m_edge);
        m_node->addEdge(m_edge);
        m_scene->registerEdge(m_edge);
        m_edge->updatePath();
    }

    m_scene->clearSelection();
    m_node->setSelected(true);
    m_ownsObjects = false;
}

void AddNodeCommand::undo() {
    // Detach from scene without deleting
    m_parent->removeChild(m_node);
    m_parent->removeEdge(m_edge);
    m_node->removeEdge(m_edge);
    m_scene->unregisterEdge(m_edge);
    m_scene->removeItem(m_edge);
    m_scene->removeItem(m_node);

    m_ownsObjects = true;
}

// ===========================================================================
// RemoveNodeCommand
// ===========================================================================

RemoveNodeCommand::RemoveNodeCommand(MindMapScene* scene, NodeItem* node, QUndoCommand* parentCmd)
    : QUndoCommand("Delete Node", parentCmd), m_scene(scene) {
    m_snapshot = captureSubtree(node);
}

RemoveNodeCommand::~RemoveNodeCommand() {
    if (m_ownsObjects) {
        // We own the objects — delete them. Walk bottom-up.
        std::function<void(const NodeSnapshot&)> cleanup = [&](const NodeSnapshot& snap) {
            for (const auto& child : snap.children)
                cleanup(child);
            delete snap.edge;
            delete snap.node;
        };
        cleanup(m_snapshot);
    }
}

RemoveNodeCommand::NodeSnapshot RemoveNodeCommand::captureSubtree(NodeItem* node) const {
    NodeSnapshot snap;
    snap.node = node;
    snap.parent = node->parentNode();
    snap.position = node->pos();

    // Find the edge connecting this node to its parent
    snap.edge = nullptr;
    if (snap.parent) {
        snap.edge = m_scene->findEdge(snap.parent, node);
    }

    // Record index in parent's child list
    snap.childIndex = 0;
    if (snap.parent) {
        snap.childIndex = snap.parent->childNodes().indexOf(node);
    }

    // Recursively capture children
    for (auto* child : node->childNodes()) {
        snap.children.append(captureSubtree(child));
    }

    return snap;
}

void RemoveNodeCommand::removeSubtree(const NodeSnapshot& snap) {
    // Remove children bottom-up
    for (const auto& child : snap.children) {
        removeSubtree(child);
    }

    // Remove edge
    if (snap.edge) {
        snap.edge->sourceNode()->removeEdge(snap.edge);
        snap.edge->targetNode()->removeEdge(snap.edge);
        m_scene->unregisterEdge(snap.edge);
        m_scene->removeItem(snap.edge);
    }

    // Remove from parent
    if (snap.parent) {
        snap.parent->removeChild(snap.node);
    }

    m_scene->removeItem(snap.node);
}

void RemoveNodeCommand::restoreSubtree(const NodeSnapshot& snap) {
    // Restore this node
    m_scene->addItem(snap.node);
    snap.node->setPos(snap.position);

    if (snap.parent) {
        snap.parent->insertChild(snap.childIndex, snap.node);
    }

    // Restore edge
    if (snap.edge) {
        m_scene->addItem(snap.edge);
        snap.parent->addEdge(snap.edge);
        snap.node->addEdge(snap.edge);
        m_scene->registerEdge(snap.edge);
        snap.edge->updatePath();
    }

    // Restore children top-down
    for (const auto& child : snap.children) {
        restoreSubtree(child);
    }
}

void RemoveNodeCommand::redo() {
    removeSubtree(m_snapshot);
    m_ownsObjects = true;
}

void RemoveNodeCommand::undo() {
    restoreSubtree(m_snapshot);
    m_ownsObjects = false;

    m_scene->clearSelection();
    m_snapshot.node->setSelected(true);
}

// ===========================================================================
// EditTextCommand
// ===========================================================================

EditTextCommand::EditTextCommand(NodeItem* node, const QString& oldText, const QString& newText,
                                 QUndoCommand* parentCmd)
    : QUndoCommand("Edit Text", parentCmd), m_node(node), m_oldText(oldText), m_newText(newText) {}

void EditTextCommand::undo() {
    m_node->setText(m_oldText);
}

void EditTextCommand::redo() {
    m_node->setText(m_newText);
}

// ===========================================================================
// MoveNodeCommand
// ===========================================================================

MoveNodeCommand::MoveNodeCommand(NodeItem* node, const QPointF& oldPos, const QPointF& newPos,
                                 QUndoCommand* parentCmd)
    : QUndoCommand("Move Node", parentCmd), m_node(node), m_oldPos(oldPos), m_newPos(newPos) {}

void MoveNodeCommand::undo() {
    QPointF delta = m_oldPos - m_node->pos();
    m_node->moveSubtree(delta);
}

void MoveNodeCommand::redo() {
    if (m_firstRedo) {
        // Skip first redo — drag already moved the node
        m_firstRedo = false;
        return;
    }
    QPointF delta = m_newPos - m_node->pos();
    m_node->moveSubtree(delta);
}

// ===========================================================================
// ToggleEdgeLockCommand
// ===========================================================================

ToggleEdgeLockCommand::ToggleEdgeLockCommand(EdgeItem* edge, QUndoCommand* parentCmd)
    : QUndoCommand("Toggle Edge Lock", parentCmd), m_edge(edge) {}

void ToggleEdgeLockCommand::undo() {
    m_edge->setLocked(!m_edge->isLocked());
}

void ToggleEdgeLockCommand::redo() {
    m_edge->setLocked(!m_edge->isLocked());
}
