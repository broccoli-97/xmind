#include "scene/InlineEditController.h"
#include "core/Commands.h"
#include "scene/MindMapScene.h"
#include "scene/NodeItem.h"

#include <QGraphicsProxyWidget>
#include <QKeyEvent>
#include <QLineEdit>
#include <QUndoStack>

InlineEditController::InlineEditController(MindMapScene* scene, QObject* parent)
    : QObject(parent), m_scene(scene) {}

bool InlineEditController::isEditing() const {
    return m_editingNode != nullptr;
}

NodeItem* InlineEditController::editingNode() const {
    return m_editingNode;
}

void InlineEditController::startEditing(NodeItem* node) {
    if (m_editingNode)
        finishEditing();

    m_editingNode = node;
    m_editLineEdit = new QLineEdit(node->text());
    m_editLineEdit->setAlignment(Qt::AlignCenter);
    m_editLineEdit->selectAll();
    m_editLineEdit->setAttribute(Qt::WA_InputMethodEnabled, true);

    // Use theme-controlled global stylesheet; apply node's font locally
    m_editLineEdit->setFont(node->font());

    m_editProxy = m_scene->addWidget(m_editLineEdit);
    m_editProxy->setZValue(100);
    m_editProxy->setFlag(QGraphicsItem::ItemAcceptsInputMethod, true);

    QRectF rect = node->nodeRect();
    QPointF nodePos = node->pos();
    qreal w = qMax(rect.width() + 20, 180.0);
    qreal h = rect.height();
    m_editProxy->setPos(nodePos.x() - w / 2, nodePos.y() - h / 2);
    m_editLineEdit->setFixedWidth(static_cast<int>(w));
    m_editLineEdit->setFixedHeight(static_cast<int>(h));
    m_editLineEdit->setFocus();

    m_editLineEdit->installEventFilter(this);
}

void InlineEditController::finishEditing() {
    if (!m_editingNode || !m_editLineEdit)
        return;

    QString newText = m_editLineEdit->text().trimmed();
    QString oldText = m_editingNode->text();
    NodeItem* node = m_editingNode;

    // Cleanly remove event filter before deleting UI
    m_editLineEdit->removeEventFilter(this);

    m_editingNode = nullptr;
    m_editLineEdit = nullptr;

    // Removing the proxy from the scene immediately is fine, but defer deletion
    // to avoid destroying widgets while they are still handling events.
    m_scene->removeItem(m_editProxy);
    m_editProxy->deleteLater();
    m_editProxy = nullptr;

    if (!newText.isEmpty() && newText != oldText) {
        m_scene->undoStack()->push(new EditTextCommand(m_scene, node, oldText, newText));
    }
    m_scene->clearSelection();
    node->setSelected(true);
}

void InlineEditController::cancelEditing() {
    if (!m_editingNode)
        return;

    // Cleanly remove event filter if present
    if (m_editLineEdit) {
        m_editLineEdit->removeEventFilter(this);
    }

    m_editingNode = nullptr;
    m_editLineEdit = nullptr;

    m_scene->removeItem(m_editProxy);
    m_editProxy->deleteLater();
    m_editProxy = nullptr;
}

bool InlineEditController::handleMousePress(const QPointF& scenePos) {
    if (m_editingNode && m_editProxy) {
        QRectF proxyRect = m_editProxy->sceneBoundingRect();
        if (!proxyRect.contains(scenePos)) {
            finishEditing();
            return true;
        }
    }
    return false;
}

bool InlineEditController::eventFilter(QObject* obj, QEvent* event) {
    if (obj == m_editLineEdit && event->type() == QEvent::KeyPress) {
        auto* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Escape) {
            cancelEditing();
            return true;
        }
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            finishEditing();
            return true;
        }
    }
    return QObject::eventFilter(obj, event);
}
