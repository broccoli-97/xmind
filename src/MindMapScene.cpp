#include "MindMapScene.h"
#include "Commands.h"
#include "EdgeItem.h"
#include "LayoutEngine.h"
#include "NodeItem.h"
#include "ThemeManager.h"

#include <QEasingCurve>
#include <QFile>
#include <QGraphicsProxyWidget>
#include <QGraphicsSceneMouseEvent>
#include <QGuiApplication>
#include <QImage>
#include <QInputMethod>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>
#include <QLineEdit>
#include <QPageSize>
#include <QPainter>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QTextStream>
#include <QUndoStack>
#include <QtPrintSupport/QPrinter>
#include <QtSvg/QSvgGenerator>

MindMapScene::MindMapScene(QObject* parent) : QGraphicsScene(parent) {
    m_undoStack = new QUndoStack(this);
    connect(m_undoStack, &QUndoStack::cleanChanged, this,
            [this](bool clean) { setModified(!clean); });

    m_rootNode = createRootNode("Central Topic");
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

    // Position near parent
    node->setPos(LayoutEngine::initialChildPosition(parent, m_rootNode, m_layoutStyle));

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
    return m_editingNode != nullptr;
}

LayoutStyle MindMapScene::layoutStyle() const {
    return m_layoutStyle;
}

void MindMapScene::setLayoutStyle(LayoutStyle style) {
    m_layoutStyle = style;
}

EdgeItem* MindMapScene::findEdge(NodeItem* parent, NodeItem* child) const {
    for (auto* edge : m_edges) {
        if (edge->sourceNode() == parent && edge->targetNode() == child)
            return edge;
    }
    return nullptr;
}

void MindMapScene::addChildToSelected() {
    if (m_editingNode)
        finishEditing();
    NodeItem* node = selectedNode();
    if (!node)
        node = m_rootNode;

    auto* cmd = new AddNodeCommand(this, node, "New Topic");
    m_undoStack->push(cmd);
    startEditing(cmd->createdNode());
}

void MindMapScene::addSiblingToSelected() {
    if (m_editingNode)
        finishEditing();
    NodeItem* node = selectedNode();
    if (!node || node == m_rootNode) {
        addChildToSelected();
        return;
    }

    auto* cmd = new AddNodeCommand(this, node->parentNode(), "New Topic");
    m_undoStack->push(cmd);
    startEditing(cmd->createdNode());
}

void MindMapScene::deleteSelected() {
    if (m_editingNode)
        cancelEditing();
    NodeItem* node = selectedNode();
    if (node && node != m_rootNode) {
        m_undoStack->push(new RemoveNodeCommand(this, node));
    }
}

void MindMapScene::startEditing(NodeItem* node) {
    if (m_editingNode)
        finishEditing();

    m_editingNode = node;
    m_editLineEdit = new QLineEdit(node->text());
    m_editLineEdit->setAlignment(Qt::AlignCenter);
    m_editLineEdit->selectAll();
    m_editLineEdit->setAttribute(Qt::WA_InputMethodEnabled, true);

    // Use theme-controlled global stylesheet; apply node's font locally
    m_editLineEdit->setFont(node->font());

    m_editProxy = addWidget(m_editLineEdit);
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

void MindMapScene::finishEditing() {
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
    removeItem(m_editProxy);
    m_editProxy->deleteLater();
    m_editProxy = nullptr;

    if (!newText.isEmpty() && newText != oldText) {
        m_undoStack->push(new EditTextCommand(node, oldText, newText));
    }
    clearSelection();
    node->setSelected(true);
}

void MindMapScene::cancelEditing() {
    if (!m_editingNode)
        return;

    // Cleanly remove event filter if present
    if (m_editLineEdit) {
        m_editLineEdit->removeEventFilter(this);
    }

    m_editingNode = nullptr;
    m_editLineEdit = nullptr;

    removeItem(m_editProxy);
    m_editProxy->deleteLater();
    m_editProxy = nullptr;
}

bool MindMapScene::eventFilter(QObject* obj, QEvent* event) {
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
    return QGraphicsScene::eventFilter(obj, event);
}

void MindMapScene::keyPressEvent(QKeyEvent* event) {
    if (m_editingNode) {
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
    if (m_editingNode && m_editProxy) {
        QRectF proxyRect = m_editProxy->sceneBoundingRect();
        if (!proxyRect.contains(event->scenePos())) {
            finishEditing();
        }
    }
    QGraphicsScene::mousePressEvent(event);
}

// --- Serialization ---

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
    setModified(true);
}

QJsonObject MindMapScene::nodeToJson(NodeItem* node) const {
    QJsonObject obj;
    obj["text"] = node->text();
    obj["x"] = node->pos().x();
    obj["y"] = node->pos().y();

    // Serialize edge lock state
    if (node->parentNode()) {
        EdgeItem* edge = findEdge(node->parentNode(), node);
        if (edge && edge->isLocked()) {
            obj["edgeLocked"] = true;
        }
    }

    QJsonArray children;
    for (auto* child : node->childNodes()) {
        children.append(nodeToJson(child));
    }
    obj["children"] = children;
    return obj;
}

QJsonObject MindMapScene::toJson() const {
    QJsonObject root;
    root["format"] = QStringLiteral("xmind");
    root["version"] = 1;
    root["layoutStyle"] = static_cast<int>(m_layoutStyle);
    if (m_rootNode) {
        root["root"] = nodeToJson(m_rootNode);
    }
    return root;
}

NodeItem* MindMapScene::nodeFromJson(const QJsonObject& json, NodeItem* parent) {
    QString text = json["text"].toString("Topic");
    qreal x = json["x"].toDouble(0);
    qreal y = json["y"].toDouble(0);

    NodeItem* node;
    if (!parent) {
        // This is the root node
        node = createRootNode(text);
        node->setPos(x, y);
    } else {
        node = addNode(text, parent);
        if (node)
            node->setPos(x, y);
    }

    if (!node)
        return nullptr;

    // Restore edge lock state
    if (parent && json["edgeLocked"].toBool(false)) {
        EdgeItem* edge = findEdge(parent, node);
        if (edge)
            edge->setLocked(true);
    }

    QJsonArray children = json["children"].toArray();
    for (const auto& childVal : children) {
        nodeFromJson(childVal.toObject(), node);
    }
    return node;
}

bool MindMapScene::fromJson(const QJsonObject& json) {
    if (json["format"].toString() != "xmind")
        return false;

    clearScene();

    // Restore layout style (default to Bilateral for old files)
    m_layoutStyle = static_cast<LayoutStyle>(json["layoutStyle"].toInt(0));

    QJsonObject rootObj = json["root"].toObject();
    m_rootNode = nodeFromJson(rootObj, nullptr);
    if (!m_rootNode) {
        // Fallback: create default root
        m_rootNode = createRootNode("Central Topic");
    }

    m_undoStack->clear();
    setModified(false);
    return true;
}

bool MindMapScene::saveToFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QJsonDocument doc(toJson());
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    m_undoStack->setClean();
    setModified(false);
    return true;
}

bool MindMapScene::loadFromFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError)
        return false;

    if (!fromJson(doc.object()))
        return false;

    emit fileLoaded(filePath);
    return true;
}

void MindMapScene::clearScene() {
    if (m_editingNode)
        cancelEditing();

    m_undoStack->clear();

    // Remove all edges
    for (auto* edge : m_edges) {
        removeItem(edge);
        delete edge;
    }
    m_edges.clear();

    // Remove all nodes (removeNode handles recursive children)
    if (m_rootNode) {
        // Remove children first (recursive)
        auto children = m_rootNode->childNodes();
        for (auto* child : children) {
            removeNode(child);
        }
        removeItem(m_rootNode);
        delete m_rootNode;
        m_rootNode = nullptr;
    }

    setModified(false);
}

// --- Export ---

void MindMapScene::exportNodeToText(NodeItem* node, int indent, QString& output) const {
    output += QString(indent, '\t') + node->text() + '\n';
    for (auto* child : node->childNodes()) {
        exportNodeToText(child, indent + 1, output);
    }
}

QString MindMapScene::exportToText() const {
    QString output;
    if (m_rootNode)
        exportNodeToText(m_rootNode, 0, output);
    return output;
}

void MindMapScene::exportNodeToMarkdown(NodeItem* node, int level, QString& output) const {
    if (level == 0) {
        output += "# " + node->text() + "\n\n";
    } else if (level == 1) {
        output += "## " + node->text() + "\n\n";
    } else {
        output += QString((level - 2) * 2, ' ') + "- " + node->text() + '\n';
    }
    for (auto* child : node->childNodes()) {
        exportNodeToMarkdown(child, level + 1, output);
    }
    if (level <= 1)
        output += '\n';
}

QString MindMapScene::exportToMarkdown() const {
    QString output;
    if (m_rootNode)
        exportNodeToMarkdown(m_rootNode, 0, output);
    return output;
}

bool MindMapScene::exportToPng(const QString& filePath, int scaleFactor) {
    QRectF contentRect = itemsBoundingRect().adjusted(-40, -40, 40, 40);
    QSize imageSize(static_cast<int>(contentRect.width() * scaleFactor),
                    static_cast<int>(contentRect.height() * scaleFactor));

    QImage image(imageSize, QImage::Format_ARGB32_Premultiplied);
    image.fill(ThemeManager::colors().exportBackground);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    render(&painter, QRectF(), contentRect);
    painter.end();

    return image.save(filePath, "PNG");
}

bool MindMapScene::exportToSvg(const QString& filePath) {
    QRectF contentRect = itemsBoundingRect().adjusted(-40, -40, 40, 40);

    QSvgGenerator generator;
    generator.setFileName(filePath);
    generator.setSize(contentRect.size().toSize());
    generator.setViewBox(QRectF(QPointF(0, 0), contentRect.size()));
    generator.setTitle("XMind Export");

    QPainter painter(&generator);
    painter.setRenderHint(QPainter::Antialiasing);
    render(&painter, QRectF(), contentRect);
    painter.end();

    return true;
}

bool MindMapScene::exportToPdf(const QString& filePath) {
    QRectF contentRect = itemsBoundingRect().adjusted(-40, -40, 40, 40);

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageSize(QPageSize(contentRect.size().toSize(), QPageSize::Point));
    printer.setPageMargins(QMarginsF(0, 0, 0, 0));

    QPainter painter(&printer);
    painter.setRenderHint(QPainter::Antialiasing);
    render(&painter, QRectF(), contentRect);
    painter.end();

    return true;
}

// --- Import ---

bool MindMapScene::importFromText(const QString& text) {
    QStringList lines = text.split('\n', Qt::SkipEmptyParts);
    if (lines.isEmpty())
        return false;

    clearScene();

    // Parse indented text into tree
    // Stack tracks (indent_level, node) pairs
    QList<QPair<int, NodeItem*>> stack;

    for (const QString& line : lines) {
        // Count leading tabs
        int indent = 0;
        while (indent < line.size() && line[indent] == '\t')
            indent++;

        QString nodeText = line.mid(indent).trimmed();
        if (nodeText.isEmpty())
            continue;

        if (stack.isEmpty()) {
            // First node becomes root
            m_rootNode = createRootNode(nodeText);
            stack.append({indent, m_rootNode});
        } else {
            // Find the parent: walk back up the stack to find the most recent
            // node with a smaller indent
            while (stack.size() > 1 && stack.last().first >= indent)
                stack.removeLast();

            NodeItem* parent = stack.last().second;
            NodeItem* node = addNode(nodeText, parent);
            stack.append({indent, node});
        }
    }

    if (!m_rootNode) {
        m_rootNode = createRootNode("Central Topic");
    }

    autoLayout();
    setModified(false);
    return true;
}

// --- Auto-layout ---

void MindMapScene::autoLayout() {
    if (!m_rootNode)
        return;
    if (m_editingNode)
        finishEditing();

    auto edgeFinder = [this](NodeItem* parent, NodeItem* child) -> EdgeItem* {
        return findEdge(parent, child);
    };

    QMap<NodeItem*, QPointF> positions =
        LayoutEngine::computeLayout(m_rootNode, m_layoutStyle, edgeFinder);

    // Animate to new positions
    auto* group = new QParallelAnimationGroup(this);
    for (auto it = positions.begin(); it != positions.end(); ++it) {
        auto* anim = new QPropertyAnimation(it.key(), "pos");
        anim->setDuration(400);
        anim->setEndValue(it.value());
        anim->setEasingCurve(QEasingCurve::OutCubic);
        group->addAnimation(anim);
    }
    connect(group, &QAbstractAnimation::finished, group, &QObject::deleteLater);
    group->start();
}
