#pragma once

#include <QList>
#include <QPointF>
#include <QString>
#include <QUndoCommand>


class MindMapScene;
class NodeItem;
class EdgeItem;

// ---------------------------------------------------------------------------
// AddNodeCommand
// ---------------------------------------------------------------------------
class AddNodeCommand : public QUndoCommand {
public:
    AddNodeCommand(MindMapScene* scene, NodeItem* parent, const QString& text,
                   QUndoCommand* parentCmd = nullptr);
    ~AddNodeCommand() override;

    void undo() override;
    void redo() override;

    NodeItem* createdNode() const { return m_node; }

private:
    MindMapScene* m_scene;
    NodeItem* m_parent;
    NodeItem* m_node = nullptr;
    EdgeItem* m_edge = nullptr;
    QString m_text;
    bool m_ownsObjects = false; // true when objects are detached from scene
};

// ---------------------------------------------------------------------------
// RemoveNodeCommand
// ---------------------------------------------------------------------------
class RemoveNodeCommand : public QUndoCommand {
public:
    RemoveNodeCommand(MindMapScene* scene, NodeItem* node, QUndoCommand* parentCmd = nullptr);
    ~RemoveNodeCommand() override;

    void undo() override;
    void redo() override;

private:
    struct NodeSnapshot {
        NodeItem* node;
        EdgeItem* edge; // edge connecting to parent (nullptr for root)
        NodeItem* parent;
        QPointF position;
        int childIndex; // index in parent's child list
        QList<NodeSnapshot> children;
    };

    NodeSnapshot captureSubtree(NodeItem* node) const;
    void removeSubtree(const NodeSnapshot& snap);
    void restoreSubtree(const NodeSnapshot& snap);

    MindMapScene* m_scene;
    NodeSnapshot m_snapshot;
    bool m_ownsObjects = false; // true when objects are detached from scene
};

// ---------------------------------------------------------------------------
// EditTextCommand
// ---------------------------------------------------------------------------
class EditTextCommand : public QUndoCommand {
public:
    EditTextCommand(NodeItem* node, const QString& oldText, const QString& newText,
                    QUndoCommand* parentCmd = nullptr);

    void undo() override;
    void redo() override;

private:
    NodeItem* m_node;
    QString m_oldText;
    QString m_newText;
};

// ---------------------------------------------------------------------------
// MoveNodeCommand
// ---------------------------------------------------------------------------
class MoveNodeCommand : public QUndoCommand {
public:
    MoveNodeCommand(NodeItem* node, const QPointF& oldPos, const QPointF& newPos,
                    QUndoCommand* parentCmd = nullptr);

    void undo() override;
    void redo() override;

private:
    NodeItem* m_node;
    QPointF m_oldPos;
    QPointF m_newPos;
    bool m_firstRedo = true;
};

// ---------------------------------------------------------------------------
// ToggleEdgeLockCommand
// ---------------------------------------------------------------------------
class ToggleEdgeLockCommand : public QUndoCommand {
public:
    ToggleEdgeLockCommand(EdgeItem* edge, QUndoCommand* parentCmd = nullptr);

    void undo() override;
    void redo() override;

private:
    EdgeItem* m_edge;
};
